using namespace std;

#include "QueryParserBase.h"

namespace org::apache::lucene::queryparser::classic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Term = org::apache::lucene::index::Term;
using Operator =
    org::apache::lucene::queryparser::classic::QueryParser::Operator;
using CommonQueryParserConfiguration = org::apache::lucene::queryparser::
    flexible::standard::CommonQueryParserConfiguration;
using namespace org::apache::lucene::search;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using TooManyClauses =
    org::apache::lucene::search::BooleanQuery::TooManyClauses;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using QueryBuilder = org::apache::lucene::util::QueryBuilder;
using RegExp = org::apache::lucene::util::automaton::RegExp;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

QueryParserBase::QueryParserBase()
    : org::apache::lucene::util::QueryBuilder(nullptr)
{
}

void QueryParserBase::init(const wstring &f, shared_ptr<Analyzer> a)
{
  setAnalyzer(a);
  field = f;
  setAutoGeneratePhraseQueries(false);
}

shared_ptr<Query>
QueryParserBase::parse(const wstring &query) 
{
  ReInit(make_shared<FastCharStream>(make_shared<StringReader>(query)));
  try {
    // TopLevelQuery is a Query followed by the end-of-input (EOF)
    shared_ptr<Query> res = TopLevelQuery(field);
    return res != nullptr ? res : newBooleanQuery()->build();
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ParseException | TokenMgrError tme) {
    // rethrow to include the original query:
    shared_ptr<ParseException> e = make_shared<ParseException>(
        L"Cannot parse '" + query + L"': " + tme::getMessage());
    e->initCause(tme);
    throw e;
  } catch (const BooleanQuery::TooManyClauses &tmc) {
    shared_ptr<ParseException> e = make_shared<ParseException>(
        L"Cannot parse '" + query + L"': too many bool clauses");
    e->initCause(tmc);
    throw e;
  }
}

wstring QueryParserBase::getField() { return field; }

bool QueryParserBase::getAutoGeneratePhraseQueries()
{
  return autoGeneratePhraseQueries;
}

void QueryParserBase::setAutoGeneratePhraseQueries(bool value)
{
  this->autoGeneratePhraseQueries = value;
}

float QueryParserBase::getFuzzyMinSim() { return fuzzyMinSim; }

void QueryParserBase::setFuzzyMinSim(float fuzzyMinSim)
{
  this->fuzzyMinSim = fuzzyMinSim;
}

int QueryParserBase::getFuzzyPrefixLength() { return fuzzyPrefixLength; }

void QueryParserBase::setFuzzyPrefixLength(int fuzzyPrefixLength)
{
  this->fuzzyPrefixLength = fuzzyPrefixLength;
}

void QueryParserBase::setPhraseSlop(int phraseSlop)
{
  this->phraseSlop = phraseSlop;
}

int QueryParserBase::getPhraseSlop() { return phraseSlop; }

void QueryParserBase::setAllowLeadingWildcard(bool allowLeadingWildcard)
{
  this->allowLeadingWildcard = allowLeadingWildcard;
}

bool QueryParserBase::getAllowLeadingWildcard() { return allowLeadingWildcard; }

void QueryParserBase::setDefaultOperator(Operator op) { this->operator_ = op; }

Operator QueryParserBase::getDefaultOperator() { return operator_; }

void QueryParserBase::setMultiTermRewriteMethod(
    shared_ptr<MultiTermQuery::RewriteMethod> method)
{
  multiTermRewriteMethod = method;
}

shared_ptr<MultiTermQuery::RewriteMethod>
QueryParserBase::getMultiTermRewriteMethod()
{
  return multiTermRewriteMethod;
}

void QueryParserBase::setLocale(shared_ptr<Locale> locale)
{
  this->locale = locale;
}

shared_ptr<Locale> QueryParserBase::getLocale() { return locale; }

void QueryParserBase::setTimeZone(shared_ptr<TimeZone> timeZone)
{
  this->timeZone = timeZone;
}

shared_ptr<TimeZone> QueryParserBase::getTimeZone() { return timeZone; }

void QueryParserBase::setDateResolution(DateTools::Resolution dateResolution)
{
  this->dateResolution = dateResolution;
}

void QueryParserBase::setDateResolution(const wstring &fieldName,
                                        DateTools::Resolution dateResolution)
{
  if (fieldName == L"") {
    throw invalid_argument(L"Field must not be null.");
  }

  if (fieldToDateResolution.empty()) {
    // lazily initialize HashMap
    fieldToDateResolution = unordered_map<>();
  }

  fieldToDateResolution.emplace(fieldName, dateResolution);
}

DateTools::Resolution
QueryParserBase::getDateResolution(const wstring &fieldName)
{
  if (fieldName == L"") {
    throw invalid_argument(L"Field must not be null.");
  }

  if (fieldToDateResolution.empty()) {
    // no field specific date resolutions set; return default date resolution
    // instead
    return this->dateResolution;
  }

  DateTools::Resolution resolution = fieldToDateResolution[fieldName];
  if (resolution == nullptr) {
    // no date resolutions set for the given field; return default date
    // resolution instead
    resolution = this->dateResolution;
  }

  return resolution;
}

void QueryParserBase::setMaxDeterminizedStates(int maxDeterminizedStates)
{
  this->maxDeterminizedStates = maxDeterminizedStates;
}

int QueryParserBase::getMaxDeterminizedStates()
{
  return maxDeterminizedStates;
}

void QueryParserBase::addClause(deque<std::shared_ptr<BooleanClause>> &clauses,
                                int conj, int mods, shared_ptr<Query> q)
{
  bool required, prohibited;

  // If this term is introduced by AND, make the preceding term required,
  // unless it's already prohibited
  if (clauses.size() > 0 && conj == CONJ_AND) {
    shared_ptr<BooleanClause> c = clauses[clauses.size() - 1];
    if (!c->isProhibited()) {
      clauses[clauses.size() - 1] =
          make_shared<BooleanClause>(c->getQuery(), Occur::MUST);
    }
  }

  if (clauses.size() > 0 && operator_ == AND_OPERATOR && conj == CONJ_OR) {
    // If this term is introduced by OR, make the preceding term optional,
    // unless it's prohibited (that means we leave -a OR b but +a OR b-->a OR b)
    // notice if the input is a OR b, first term is parsed as required; without
    // this modification a OR b would parsed as +a OR b
    shared_ptr<BooleanClause> c = clauses[clauses.size() - 1];
    if (!c->isProhibited()) {
      clauses[clauses.size() - 1] =
          make_shared<BooleanClause>(c->getQuery(), Occur::SHOULD);
    }
  }

  // We might have been passed a null query; the term might have been
  // filtered away by the analyzer.
  if (q == nullptr) {
    return;
  }

  if (operator_ == OR_OPERATOR) {
    // We set REQUIRED if we're introduced by AND or +; PROHIBITED if
    // introduced by NOT or -; make sure not to set both.
    prohibited = (mods == MOD_NOT);
    required = (mods == MOD_REQ);
    if (conj == CONJ_AND && !prohibited) {
      required = true;
    }
  } else {
    // We set PROHIBITED if we're introduced by NOT or -; We set REQUIRED
    // if not PROHIBITED and not introduced by OR
    prohibited = (mods == MOD_NOT);
    required = (!prohibited && conj != CONJ_OR);
  }
  if (required && !prohibited) {
    clauses.push_back(newBooleanClause(q, BooleanClause::Occur::MUST));
  } else if (!required && !prohibited) {
    clauses.push_back(newBooleanClause(q, BooleanClause::Occur::SHOULD));
  } else if (!required && prohibited) {
    clauses.push_back(newBooleanClause(q, BooleanClause::Occur::MUST_NOT));
  } else {
    throw runtime_error(L"Clause cannot be both required and prohibited");
  }
}

void QueryParserBase::addMultiTermClauses(
    deque<std::shared_ptr<BooleanClause>> &clauses, shared_ptr<Query> q)
{
  // We might have been passed a null query; the term might have been
  // filtered away by the analyzer.
  if (q == nullptr) {
    return;
  }
  bool allNestedTermQueries = false;
  if (std::dynamic_pointer_cast<BooleanQuery>(q) != nullptr) {
    allNestedTermQueries = true;
    for (auto clause : (std::static_pointer_cast<BooleanQuery>(q))->clauses()) {
      if (!(std::dynamic_pointer_cast<TermQuery>(clause->getQuery()) !=
            nullptr)) {
        allNestedTermQueries = false;
        break;
      }
    }
  }
  if (allNestedTermQueries) {
    clauses.addAll((std::static_pointer_cast<BooleanQuery>(q))->clauses());
  } else {
    BooleanClause::Occur occur = operator_ == OR_OPERATOR
                                     ? BooleanClause::Occur::SHOULD
                                     : BooleanClause::Occur::MUST;
    if (std::dynamic_pointer_cast<BooleanQuery>(q) != nullptr) {
      for (auto clause :
           (std::static_pointer_cast<BooleanQuery>(q))->clauses()) {
        clauses.push_back(newBooleanClause(clause->getQuery(), occur));
      }
    } else {
      clauses.push_back(newBooleanClause(q, occur));
    }
  }
}

shared_ptr<Query>
QueryParserBase::getFieldQuery(const wstring &field, const wstring &queryText,
                               bool quoted) 
{
  return newFieldQuery(getAnalyzer(), field, queryText, quoted);
}

shared_ptr<Query>
QueryParserBase::newFieldQuery(shared_ptr<Analyzer> analyzer,
                               const wstring &field, const wstring &queryText,
                               bool quoted) 
{
  BooleanClause::Occur occur = operator_ == Operator::AND
                                   ? BooleanClause::Occur::MUST
                                   : BooleanClause::Occur::SHOULD;
  return createFieldQuery(analyzer, occur, field, queryText,
                          quoted || autoGeneratePhraseQueries, phraseSlop);
}

shared_ptr<Query> QueryParserBase::getFieldQuery(const wstring &field,
                                                 const wstring &queryText,
                                                 int slop) 
{
  shared_ptr<Query> query = getFieldQuery(field, queryText, true);

  if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
    query = addSlopToPhrase(std::static_pointer_cast<PhraseQuery>(query), slop);
  } else if (std::dynamic_pointer_cast<MultiPhraseQuery>(query) != nullptr) {
    shared_ptr<MultiPhraseQuery> mpq =
        std::static_pointer_cast<MultiPhraseQuery>(query);

    if (slop != mpq->getSlop()) {
      query =
          (make_shared<MultiPhraseQuery::Builder>(mpq))->setSlop(slop)->build();
    }
  }

  return query;
}

shared_ptr<PhraseQuery>
QueryParserBase::addSlopToPhrase(shared_ptr<PhraseQuery> query, int slop)
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->setSlop(slop);
  std::deque<std::shared_ptr<Term>> terms = query->getTerms();
  std::deque<int> positions = query->getPositions();
  for (int i = 0; i < terms.size(); ++i) {
    builder->add(terms[i], positions[i]);
  }

  return builder->build();
}

shared_ptr<Query>
QueryParserBase::getRangeQuery(const wstring &field, const wstring &part1,
                               const wstring &part2, bool startInclusive,
                               bool endInclusive) 
{
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, locale);
  df->setLenient(true);
  DateTools::Resolution resolution = getDateResolution(field);

  try {
    part1 = DateTools::dateToString(df->parse(part1), resolution);
  } catch (const runtime_error &e) {
  }

  try {
    Date d2 = df->parse(part2);
    if (endInclusive) {
      // The user can only specify the date, not the time, so make sure
      // the time is set to the latest possible time of that date to really
      // include all documents:
      shared_ptr<Calendar> cal = Calendar::getInstance(timeZone, locale);
      cal->setTime(d2);
      cal->set(Calendar::HOUR_OF_DAY, 23);
      cal->set(Calendar::MINUTE, 59);
      cal->set(Calendar::SECOND, 59);
      cal->set(Calendar::MILLISECOND, 999);
      d2 = cal->getTime();
    }
    part2 = DateTools::dateToString(d2, resolution);
  } catch (const runtime_error &e) {
  }

  return newRangeQuery(field, part1, part2, startInclusive, endInclusive);
}

shared_ptr<BooleanClause>
QueryParserBase::newBooleanClause(shared_ptr<Query> q,
                                  BooleanClause::Occur occur)
{
  return make_shared<BooleanClause>(q, occur);
}

shared_ptr<Query> QueryParserBase::newPrefixQuery(shared_ptr<Term> prefix)
{
  shared_ptr<PrefixQuery> query = make_shared<PrefixQuery>(prefix);
  query->setRewriteMethod(multiTermRewriteMethod);
  return query;
}

shared_ptr<Query> QueryParserBase::newRegexpQuery(shared_ptr<Term> regexp)
{
  shared_ptr<RegexpQuery> query =
      make_shared<RegexpQuery>(regexp, RegExp::ALL, maxDeterminizedStates);
  query->setRewriteMethod(multiTermRewriteMethod);
  return query;
}

shared_ptr<Query> QueryParserBase::newFuzzyQuery(shared_ptr<Term> term,
                                                 float minimumSimilarity,
                                                 int prefixLength)
{
  // FuzzyQuery doesn't yet allow constant score rewrite
  wstring text = term->text();
  int numEdits = FuzzyQuery::floatToEdits(
      minimumSimilarity, text.codePointCount(0, text.length()));
  return make_shared<FuzzyQuery>(term, numEdits, prefixLength);
}

shared_ptr<Query> QueryParserBase::newRangeQuery(const wstring &field,
                                                 const wstring &part1,
                                                 const wstring &part2,
                                                 bool startInclusive,
                                                 bool endInclusive)
{
  shared_ptr<BytesRef> *const start;
  shared_ptr<BytesRef> *const end;

  if (part1 == L"") {
    start.reset();
  } else {
    start = getAnalyzer()->normalize(field, part1);
  }

  if (part2 == L"") {
    end.reset();
  } else {
    end = getAnalyzer()->normalize(field, part2);
  }

  shared_ptr<TermRangeQuery> *const query = make_shared<TermRangeQuery>(
      field, start, end, startInclusive, endInclusive);

  query->setRewriteMethod(multiTermRewriteMethod);
  return query;
}

shared_ptr<Query> QueryParserBase::newMatchAllDocsQuery()
{
  return make_shared<MatchAllDocsQuery>();
}

shared_ptr<Query> QueryParserBase::newWildcardQuery(shared_ptr<Term> t)
{
  shared_ptr<WildcardQuery> query =
      make_shared<WildcardQuery>(t, maxDeterminizedStates);
  query->setRewriteMethod(multiTermRewriteMethod);
  return query;
}

shared_ptr<Query> QueryParserBase::getBooleanQuery(
    deque<std::shared_ptr<BooleanClause>> &clauses) 
{
  if (clauses.empty()) {
    return nullptr; // all clause words were filtered away by the analyzer.
  }
  shared_ptr<BooleanQuery::Builder> query = newBooleanQuery();
  for (auto clause : clauses) {
    query->add(clause);
  }
  return query->build();
}

shared_ptr<Query>
QueryParserBase::getWildcardQuery(const wstring &field,
                                  const wstring &termStr) 
{
  if (L"*" == field) {
    if (L"*" == termStr) {
      return newMatchAllDocsQuery();
    }
  }
  if (!allowLeadingWildcard && (StringHelper::startsWith(termStr, L"*") ||
                                StringHelper::startsWith(termStr, L"?"))) {
    throw make_shared<ParseException>(
        L"'*' or '?' not allowed as first character in WildcardQuery");
  }

  shared_ptr<Term> t =
      make_shared<Term>(field, analyzeWildcard(field, termStr));
  return newWildcardQuery(t);
}

const shared_ptr<java::util::regex::Pattern> QueryParserBase::WILDCARD_PATTERN =
    java::util::regex::Pattern::compile(L"(\\\\.)|([?*]+)");

shared_ptr<BytesRef> QueryParserBase::analyzeWildcard(const wstring &field,
                                                      const wstring &termStr)
{
  // best effort to not pass the wildcard characters and escaped characters
  // through #normalize
  shared_ptr<Matcher> wildcardMatcher = WILDCARD_PATTERN->matcher(termStr);
  shared_ptr<BytesRefBuilder> sb = make_shared<BytesRefBuilder>();
  int last = 0;

  while (wildcardMatcher->find()) {
    if (wildcardMatcher->start() > 0) {
      wstring chunk = termStr.substr(last, wildcardMatcher->start() - last);
      shared_ptr<BytesRef> normalized = getAnalyzer()->normalize(field, chunk);
      sb->append(normalized);
    }
    // append the matched group - without normalizing
    sb->append(make_shared<BytesRef>(wildcardMatcher->group()));

    last = wildcardMatcher->end();
  }
  if (last < termStr.length()) {
    wstring chunk = termStr.substr(last);
    shared_ptr<BytesRef> normalized = getAnalyzer()->normalize(field, chunk);
    sb->append(normalized);
  }
  return sb->toBytesRef();
}

shared_ptr<Query>
QueryParserBase::getRegexpQuery(const wstring &field,
                                const wstring &termStr) 
{
  // We need to pass the whole string to #normalize, which will not work with
  // custom attribute factories for the binary term impl, and may not work
  // with some analyzers
  shared_ptr<BytesRef> term = getAnalyzer()->normalize(field, termStr);
  shared_ptr<Term> t = make_shared<Term>(field, term);
  return newRegexpQuery(t);
}

shared_ptr<Query>
QueryParserBase::getPrefixQuery(const wstring &field,
                                const wstring &termStr) 
{
  if (!allowLeadingWildcard && StringHelper::startsWith(termStr, L"*")) {
    throw make_shared<ParseException>(
        L"'*' not allowed as first character in PrefixQuery");
  }
  shared_ptr<BytesRef> term = getAnalyzer()->normalize(field, termStr);
  shared_ptr<Term> t = make_shared<Term>(field, term);
  return newPrefixQuery(t);
}

shared_ptr<Query>
QueryParserBase::getFuzzyQuery(const wstring &field, const wstring &termStr,
                               float minSimilarity) 
{
  shared_ptr<BytesRef> term = getAnalyzer()->normalize(field, termStr);
  shared_ptr<Term> t = make_shared<Term>(field, term);
  return newFuzzyQuery(t, minSimilarity, fuzzyPrefixLength);
}

shared_ptr<Query> QueryParserBase::handleBareTokenQuery(
    const wstring &qfield, shared_ptr<Token> term, shared_ptr<Token> fuzzySlop,
    bool prefix, bool wildcard, bool fuzzy, bool regexp) 
{
  shared_ptr<Query> q;

  wstring termImage = discardEscapeChar(term->image);
  if (wildcard) {
    q = getWildcardQuery(qfield, term->image);
  } else if (prefix) {
    q = getPrefixQuery(qfield, discardEscapeChar(term->image.substr(
                                   0, term->image.length() - 1)));
  } else if (regexp) {
    q = getRegexpQuery(qfield,
                       term->image.substr(1, (term->image.length() - 1) - 1));
  } else if (fuzzy) {
    q = handleBareFuzzy(qfield, fuzzySlop, termImage);
  } else {
    q = getFieldQuery(qfield, termImage, false);
  }
  return q;
}

shared_ptr<Query>
QueryParserBase::handleBareFuzzy(const wstring &qfield,
                                 shared_ptr<Token> fuzzySlop,
                                 const wstring &termImage) 
{
  shared_ptr<Query> q;
  float fms = fuzzyMinSim;
  try {
    fms = stof(fuzzySlop->image.substr(1));
  } catch (const runtime_error &ignored) {
  }
  if (fms < 0.0f) {
    throw make_shared<ParseException>(L"Minimum similarity for a FuzzyQuery "
                                      L"has to be between 0.0f and 1.0f !");
  } else if (fms >= 1.0f && fms != static_cast<int>(fms)) {
    throw make_shared<ParseException>(
        L"Fractional edit distances are not allowed!");
  }
  q = getFuzzyQuery(qfield, termImage, fms);
  return q;
}

shared_ptr<Query> QueryParserBase::handleQuotedTerm(
    const wstring &qfield, shared_ptr<Token> term,
    shared_ptr<Token> fuzzySlop) 
{
  int s = phraseSlop; // default
  if (fuzzySlop != nullptr) {
    try {
      s = static_cast<int>(stof(fuzzySlop->image.substr(1)));
    } catch (const runtime_error &ignored) {
    }
  }
  return getFieldQuery(
      qfield,
      discardEscapeChar(term->image.substr(1, (term->image.length() - 1) - 1)),
      s);
}

shared_ptr<Query> QueryParserBase::handleBoost(shared_ptr<Query> q,
                                               shared_ptr<Token> boost)
{
  if (boost != nullptr) {
    float f = static_cast<float>(1.0);
    try {
      f = stof(boost->image);
    } catch (const runtime_error &ignored) {
      /* Should this be handled somehow? (defaults to "no boost", if
       * boost number is invalid)
       */
    }

    // avoid boosting null queries, such as those caused by stop words
    if (q != nullptr) {
      q = make_shared<BoostQuery>(q, f);
    }
  }
  return q;
}

wstring
QueryParserBase::discardEscapeChar(const wstring &input) 
{
  // Create char array to hold unescaped char sequence
  std::deque<wchar_t> output(input.length());

  // The length of the output can be less than the input
  // due to discarded escape chars. This variable holds
  // the actual length of the output
  int length = 0;

  // We remember whether the last processed character was
  // an escape character
  bool lastCharWasEscapeChar = false;

  // The multiplier the current unicode digit must be multiplied with.
  // E. g. the first digit must be multiplied with 16^3, the second with 16^2...
  int codePointMultiplier = 0;

  // Used to calculate the codepoint of the escaped unicode character
  int codePoint = 0;

  for (int i = 0; i < input.length(); i++) {
    wchar_t curChar = input[i];
    if (codePointMultiplier > 0) {
      codePoint += hexToInt(curChar) * codePointMultiplier;
      codePointMultiplier =
          static_cast<int>(static_cast<unsigned int>(codePointMultiplier) >> 4);
      if (codePointMultiplier == 0) {
        output[length++] = static_cast<wchar_t>(codePoint);
        codePoint = 0;
      }
    } else if (lastCharWasEscapeChar) {
      if (curChar == L'u') {
        // found an escaped unicode character
        codePointMultiplier = 16 * 16 * 16;
      } else {
        // this character was escaped
        output[length] = curChar;
        length++;
      }
      lastCharWasEscapeChar = false;
    } else {
      if (curChar == L'\\') {
        lastCharWasEscapeChar = true;
      } else {
        output[length] = curChar;
        length++;
      }
    }
  }

  if (codePointMultiplier > 0) {
    throw make_shared<ParseException>(L"Truncated unicode escape sequence.");
  }

  if (lastCharWasEscapeChar) {
    throw make_shared<ParseException>(
        L"Term can not end with escape character.");
  }

  return wstring(output, 0, length);
}

int QueryParserBase::hexToInt(wchar_t c) 
{
  if (L'0' <= c && c <= L'9') {
    return c - L'0';
  } else if (L'a' <= c && c <= L'f') {
    return c - L'a' + 10;
  } else if (L'A' <= c && c <= L'F') {
    return c - L'A' + 10;
  } else {
    throw make_shared<ParseException>(
        L"Non-hex character in Unicode escape sequence: " +
        StringHelper::toString(c));
  }
}

wstring QueryParserBase::escape(const wstring &s)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < s.length(); i++) {
    wchar_t c = s[i];
    // These characters are part of the query syntax and must be escaped
    if (c == L'\\' || c == L'+' || c == L'-' || c == L'!' || c == L'(' ||
        c == L')' || c == L':' || c == L'^' || c == L'[' || c == L']' ||
        c == L'\"' || c == L'{' || c == L'}' || c == L'~' || c == L'*' ||
        c == L'?' || c == L'|' || c == L'&' || c == L'/') {
      sb->append(L'\\');
    }
    sb->append(c);
  }
  return sb->toString();
}
} // namespace org::apache::lucene::queryparser::classic