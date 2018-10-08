using namespace std;

#include "TestPrecedenceQueryParser.h"

namespace org::apache::lucene::queryparser::flexible::precedence
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using DateTools = org::apache::lucene::document::DateTools;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNodeParseException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeParseException;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using ParseException = org::apache::lucene::queryparser::flexible::standard::
    parser::ParseException;
using QueryParserTestBase =
    org::apache::lucene::queryparser::util::QueryParserTestBase;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<Analyzer> TestPrecedenceQueryParser::qpAnalyzer;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass()
void TestPrecedenceQueryParser::beforeClass()
{
  qpAnalyzer = make_shared<QueryParserTestBase::QPTestAnalyzer>();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestPrecedenceQueryParser::afterClass() { qpAnalyzer.reset(); }

TestPrecedenceQueryParser::QPTestFilter::QPTestFilter(
    shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

bool TestPrecedenceQueryParser::QPTestFilter::incrementToken() throw(
    IOException)
{
  if (inPhrase) {
    inPhrase = false;
    termAtt->setEmpty()->append(L"phrase2");
    offsetAtt->setOffset(savedStart, savedEnd);
    return true;
  } else {
    while (input->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      if (termAtt->toString()->equals(L"phrase")) {
        inPhrase = true;
        savedStart = offsetAtt->startOffset();
        savedEnd = offsetAtt->endOffset();
        termAtt->setEmpty()->append(L"phrase1");
        offsetAtt->setOffset(savedStart, savedEnd);
        return true;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      else if (!termAtt->toString()->equals(L"stop")) {
        return true;
      }
    }
  }
  return false;
}

void TestPrecedenceQueryParser::QPTestFilter::reset() 
{
  TokenFilter::reset();
  this->inPhrase = false;
  this->savedStart = 0;
  this->savedEnd = 0;
}

shared_ptr<TokenStreamComponents>
TestPrecedenceQueryParser::QPTestAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<QueryParserTestBase::QPTestFilter>(tokenizer));
}

void TestPrecedenceQueryParser::setUp() 
{
  LuceneTestCase::setUp();
  originalMaxClauses = BooleanQuery::getMaxClauseCount();
}

shared_ptr<PrecedenceQueryParser> TestPrecedenceQueryParser::getParser(
    shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<PrecedenceQueryParser> qp = make_shared<PrecedenceQueryParser>();
  qp->setAnalyzer(a);
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::OR);
  return qp;
}

shared_ptr<Query>
TestPrecedenceQueryParser::getQuery(const wstring &query,
                                    shared_ptr<Analyzer> a) 
{
  return getParser(a)->parse(query, L"field");
}

void TestPrecedenceQueryParser::assertQueryEquals(
    const wstring &query, shared_ptr<Analyzer> a,
    const wstring &result) 
{
  shared_ptr<Query> q = getQuery(query, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestPrecedenceQueryParser::assertMatchNoDocsQuery(
    const wstring &queryString, shared_ptr<Analyzer> a) 
{
  assertMatchNoDocsQuery(getQuery(queryString, a));
}

void TestPrecedenceQueryParser::assertMatchNoDocsQuery(
    shared_ptr<Query> query) 
{
  if (std::dynamic_pointer_cast<MatchNoDocsQuery>(query) != nullptr) {
    // good
  } else if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr &&
             (std::static_pointer_cast<BooleanQuery>(query))
                 ->clauses()
                 .empty()) {
    // good
  } else {
    fail(L"expected MatchNoDocsQuery or an empty BooleanQuery but got: " +
         query);
  }
}

void TestPrecedenceQueryParser::assertWildcardQueryEquals(
    const wstring &query, const wstring &result) 
{
  shared_ptr<PrecedenceQueryParser> qp = getParser(nullptr);
  shared_ptr<Query> q = qp->parse(query, L"field");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"WildcardQuery /" + query + L"/ yielded /" + s + L"/, expecting /" +
         result + L"/");
  }
}

shared_ptr<Query> TestPrecedenceQueryParser::getQueryDOA(
    const wstring &query, shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<PrecedenceQueryParser> qp = make_shared<PrecedenceQueryParser>();
  qp->setAnalyzer(a);
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  return qp->parse(query, L"field");
}

void TestPrecedenceQueryParser::assertQueryEqualsDOA(
    const wstring &query, shared_ptr<Analyzer> a,
    const wstring &result) 
{
  shared_ptr<Query> q = getQueryDOA(query, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestPrecedenceQueryParser::testSimple() 
{
  assertQueryEquals(L"term term term", nullptr, L"term term term");
  assertQueryEquals(L"türm term term", nullptr, L"türm term term");
  assertQueryEquals(L"ümlaut", nullptr, L"ümlaut");

  assertQueryEquals(L"a AND b", nullptr, L"+a +b");
  assertQueryEquals(L"(a AND b)", nullptr, L"+a +b");
  assertQueryEquals(L"c OR (a AND b)", nullptr, L"c (+a +b)");
  assertQueryEquals(L"a AND NOT b", nullptr, L"+a -b");
  assertQueryEquals(L"a AND -b", nullptr, L"+a -b");
  assertQueryEquals(L"a AND !b", nullptr, L"+a -b");
  assertQueryEquals(L"a && b", nullptr, L"+a +b");
  assertQueryEquals(L"a && ! b", nullptr, L"+a -b");

  assertQueryEquals(L"a OR b", nullptr, L"a b");
  assertQueryEquals(L"a || b", nullptr, L"a b");

  assertQueryEquals(L"+term -term term", nullptr, L"+term -term term");
  assertQueryEquals(L"foo:term AND field:anotherTerm", nullptr,
                    L"+foo:term +anotherterm");
  assertQueryEquals(L"term AND \"phrase phrase\"", nullptr,
                    L"+term +\"phrase phrase\"");
  assertQueryEquals(L"\"hello there\"", nullptr, L"\"hello there\"");
  assertTrue(std::dynamic_pointer_cast<BooleanQuery>(
                 getQuery(L"a AND b", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(
                 getQuery(L"hello", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(
                 getQuery(L"\"hello there\"", nullptr)) != nullptr);

  assertQueryEquals(L"germ term^2.0", nullptr, L"germ (term)^2.0");
  assertQueryEquals(L"(term)^2.0", nullptr, L"(term)^2.0");
  assertQueryEquals(L"(germ term)^2.0", nullptr, L"(germ term)^2.0");
  assertQueryEquals(L"term^2.0", nullptr, L"(term)^2.0");
  assertQueryEquals(L"term^2", nullptr, L"(term)^2.0");
  assertQueryEquals(L"\"germ term\"^2.0", nullptr, L"(\"germ term\")^2.0");
  assertQueryEquals(L"\"term germ\"^2", nullptr, L"(\"term germ\")^2.0");

  assertQueryEquals(L"(foo OR bar) AND (baz OR boo)", nullptr,
                    L"+(foo bar) +(baz boo)");
  assertQueryEquals(L"((a OR b) AND NOT c) OR d", nullptr, L"(+(a b) -c) d");
  assertQueryEquals(L"+(apple \"steve jobs\") -(foo bar baz)", nullptr,
                    L"+(apple \"steve jobs\") -(foo bar baz)");
  assertQueryEquals(L"+title:(dog OR cat) -author:\"bob dole\"", nullptr,
                    L"+(title:dog title:cat) -author:\"bob dole\"");

  shared_ptr<PrecedenceQueryParser> qp = make_shared<PrecedenceQueryParser>();
  qp->setAnalyzer(make_shared<MockAnalyzer>(random()));
  // make sure OR is the default:
  assertEquals(StandardQueryConfigHandler::Operator::OR,
               qp->getDefaultOperator());
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  assertEquals(StandardQueryConfigHandler::Operator::AND,
               qp->getDefaultOperator());
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::OR);
  assertEquals(StandardQueryConfigHandler::Operator::OR,
               qp->getDefaultOperator());

  assertQueryEquals(L"a OR !b", nullptr, L"a -b");
  assertQueryEquals(L"a OR ! b", nullptr, L"a -b");
  assertQueryEquals(L"a OR -b", nullptr, L"a -b");
}

void TestPrecedenceQueryParser::testPunct() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"a&b", a, L"a&b");
  assertQueryEquals(L"a&&b", a, L"a&&b");
  assertQueryEquals(L".NET", a, L".NET");
}

void TestPrecedenceQueryParser::testSlop() 
{
  assertQueryEquals(L"\"term germ\"~2", nullptr, L"\"term germ\"~2");
  assertQueryEquals(L"\"term germ\"~2 flork", nullptr,
                    L"\"term germ\"~2 flork");
  assertQueryEquals(L"\"term\"~2", nullptr, L"term");
  assertQueryEquals(L"\" \"~2 germ", nullptr, L"germ");
  assertQueryEquals(L"\"term germ\"~2^2", nullptr, L"(\"term germ\"~2)^2.0");
}

void TestPrecedenceQueryParser::testNumber() 
{
  // The numbers go away because SimpleAnalzyer ignores them
  assertMatchNoDocsQuery(L"3", nullptr);
  assertQueryEquals(L"term 1.0 1 2", nullptr, L"term");
  assertQueryEquals(L"term term1 term2", nullptr, L"term term term");

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  assertQueryEquals(L"3", a, L"3");
  assertQueryEquals(L"term 1.0 1 2", a, L"term 1.0 1 2");
  assertQueryEquals(L"term term1 term2", a, L"term term1 term2");
}

void TestPrecedenceQueryParser::testWildcard() 
{
  assertQueryEquals(L"term*", nullptr, L"term*");
  assertQueryEquals(L"term*^2", nullptr, L"(term*)^2.0");
  assertQueryEquals(L"term~", nullptr, L"term~2");
  assertQueryEquals(L"term~0.7", nullptr, L"term~1");
  assertQueryEquals(L"term~^3", nullptr, L"(term~2)^3.0");
  assertQueryEquals(L"term^3~", nullptr, L"(term~2)^3.0");
  assertQueryEquals(L"term*germ", nullptr, L"term*germ");
  assertQueryEquals(L"term*germ^3", nullptr, L"(term*germ)^3.0");

  assertTrue(std::dynamic_pointer_cast<PrefixQuery>(
                 getQuery(L"term*", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<BoostQuery>(
                 getQuery(L"term*^2", nullptr)) != nullptr);
  assertTrue(
      std::dynamic_pointer_cast<PrefixQuery>(
          (std::static_pointer_cast<BoostQuery>(getQuery(L"term*^2", nullptr)))
              ->getQuery()) != nullptr);
  assertTrue(std::dynamic_pointer_cast<FuzzyQuery>(
                 getQuery(L"term~", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<FuzzyQuery>(
                 getQuery(L"term~0.7", nullptr)) != nullptr);
  shared_ptr<FuzzyQuery> fq =
      std::static_pointer_cast<FuzzyQuery>(getQuery(L"term~0.7", nullptr));
  assertEquals(1, fq->getMaxEdits());
  assertEquals(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());
  fq = std::static_pointer_cast<FuzzyQuery>(getQuery(L"term~", nullptr));
  assertEquals(2, fq->getMaxEdits());
  assertEquals(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());
  expectThrows(ParseException::typeid,
               [&]() { getQuery(L"term~1.1", nullptr); });
  assertTrue(std::dynamic_pointer_cast<WildcardQuery>(
                 getQuery(L"term*germ", nullptr)) != nullptr);

  /*
   * Tests to see that wild card terms are (or are not) properly lower-cased
   * with propery parser configuration
   */
  // First prefix queries:
  // by default, convert to lowercase:
  assertWildcardQueryEquals(L"Term*", L"term*");
  // explicitly set lowercase:
  assertWildcardQueryEquals(L"term*", L"term*");
  assertWildcardQueryEquals(L"Term*", L"term*");
  assertWildcardQueryEquals(L"TERM*", L"term*");
  // Then 'full' wildcard queries:
  // by default, convert to lowercase:
  assertWildcardQueryEquals(L"Te?m", L"te?m");
  // explicitly set lowercase:
  assertWildcardQueryEquals(L"te?m", L"te?m");
  assertWildcardQueryEquals(L"Te?m", L"te?m");
  assertWildcardQueryEquals(L"TE?M", L"te?m");
  assertWildcardQueryEquals(L"Te?m*gerM", L"te?m*germ");
  // Fuzzy queries:
  assertWildcardQueryEquals(L"Term~", L"term~2");
  // Range queries:
  assertWildcardQueryEquals(L"[A TO C]", L"[a TO c]");
}

void TestPrecedenceQueryParser::testQPA() 
{
  assertQueryEquals(L"term term term", qpAnalyzer, L"term term term");
  assertQueryEquals(L"term +stop term", qpAnalyzer, L"term term");
  assertQueryEquals(L"term -stop term", qpAnalyzer, L"term term");
  assertQueryEquals(L"drop AND stop AND roll", qpAnalyzer, L"+drop +roll");
  assertQueryEquals(L"term phrase term", qpAnalyzer,
                    L"term (phrase1 phrase2) term");
  // note the parens in this next assertion differ from the original
  // QueryParser behavior
  assertQueryEquals(L"term AND NOT phrase term", qpAnalyzer,
                    L"(+term -(phrase1 phrase2)) term");
  assertMatchNoDocsQuery(L"stop", qpAnalyzer);
  assertMatchNoDocsQuery(L"stop OR stop AND stop", qpAnalyzer);
  assertTrue(std::dynamic_pointer_cast<BooleanQuery>(
                 getQuery(L"term term term", qpAnalyzer)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(
                 getQuery(L"term +stop", qpAnalyzer)) != nullptr);
}

void TestPrecedenceQueryParser::testRange() 
{
  assertQueryEquals(L"[ a TO z]", nullptr, L"[a TO z]");
  assertTrue(std::dynamic_pointer_cast<TermRangeQuery>(
                 getQuery(L"[ a TO z]", nullptr)) != nullptr);
  assertQueryEquals(L"[ a TO z ]", nullptr, L"[a TO z]");
  assertQueryEquals(L"{ a TO z}", nullptr, L"{a TO z}");
  assertQueryEquals(L"{ a TO z }", nullptr, L"{a TO z}");
  assertQueryEquals(L"{ a TO z }^2.0", nullptr, L"({a TO z})^2.0");
  assertQueryEquals(L"[ a TO z] OR bar", nullptr, L"[a TO z] bar");
  assertQueryEquals(L"[ a TO z] AND bar", nullptr, L"+[a TO z] +bar");
  assertQueryEquals(L"( bar blar { a TO z}) ", nullptr, L"bar blar {a TO z}");
  assertQueryEquals(L"gack ( bar blar { a TO z}) ", nullptr,
                    L"gack (bar blar {a TO z})");
}

wstring TestPrecedenceQueryParser::escapeDateString(const wstring &s)
{
  if (s.find(L" ") != wstring::npos) {
    return L"\"" + s + L"\"";
  } else {
    return s;
  }
}

wstring
TestPrecedenceQueryParser::getDate(const wstring &s) 
{
  // we use the default Locale since LuceneTestCase randomizes it
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, Locale::getDefault());
  return DateTools::dateToString(df->parse(s), DateTools::Resolution::DAY);
}

wstring TestPrecedenceQueryParser::getLocalizedDate(int year, int month,
                                                    int day,
                                                    bool extendLastDate)
{
  // we use the default Locale/TZ since LuceneTestCase randomizes it
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, Locale::getDefault());
  shared_ptr<Calendar> calendar = make_shared<GregorianCalendar>(
      TimeZone::getDefault(), Locale::getDefault());
  calendar->set(year, month, day);
  if (extendLastDate) {
    calendar->set(Calendar::HOUR_OF_DAY, 23);
    calendar->set(Calendar::MINUTE, 59);
    calendar->set(Calendar::SECOND, 59);
    calendar->set(Calendar::MILLISECOND, 999);
  }
  return df->format(calendar->getTime());
}

void TestPrecedenceQueryParser::testDateRange() 
{
  wstring startDate = getLocalizedDate(2002, 1, 1, false);
  wstring endDate = getLocalizedDate(2002, 1, 4, false);
  // we use the default Locale/TZ since LuceneTestCase randomizes it
  shared_ptr<Calendar> endDateExpected = make_shared<GregorianCalendar>(
      TimeZone::getDefault(), Locale::getDefault());
  endDateExpected->set(2002, 1, 4, 23, 59, 59);
  endDateExpected->set(Calendar::MILLISECOND, 999);
  const wstring defaultField = L"default";
  const wstring monthField = L"month";
  const wstring hourField = L"hour";
  shared_ptr<PrecedenceQueryParser> qp =
      make_shared<PrecedenceQueryParser>(make_shared<MockAnalyzer>(random()));

  unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution> fieldMap =
      unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>();
  // set a field specific date resolution
  fieldMap.emplace(monthField, DateTools::Resolution::MONTH);
  qp->setDateResolutionMap(fieldMap);

  // set default date resolution to MILLISECOND
  qp->setDateResolution(DateTools::Resolution::MILLISECOND);

  // set second field specific date resolution
  fieldMap.emplace(hourField, DateTools::Resolution::HOUR);
  qp->setDateResolutionMap(fieldMap);

  // for this field no field specific date resolution has been set,
  // so verify if the default resolution is used
  assertDateRangeQueryEquals(qp, defaultField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::MILLISECOND);

  // verify if field specific date resolutions are used for these two fields
  assertDateRangeQueryEquals(qp, monthField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::MONTH);

  assertDateRangeQueryEquals(qp, hourField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::HOUR);
}

wstring TestPrecedenceQueryParser::getDate(
    const wstring &s, DateTools::Resolution resolution) 
{
  // we use the default Locale since LuceneTestCase randomizes it
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, Locale::getDefault());
  return getDate(df->parse(s), resolution);
}

wstring TestPrecedenceQueryParser::getDate(Date d,
                                           DateTools::Resolution resolution)
{
  return DateTools::dateToString(d, resolution);
}

void TestPrecedenceQueryParser::assertQueryEquals(
    shared_ptr<PrecedenceQueryParser> qp, const wstring &field,
    const wstring &query, const wstring &result) 
{
  shared_ptr<Query> q = qp->parse(query, field);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(field);
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestPrecedenceQueryParser::assertDateRangeQueryEquals(
    shared_ptr<PrecedenceQueryParser> qp, const wstring &field,
    const wstring &startDate, const wstring &endDate, Date endDateInclusive,
    DateTools::Resolution resolution) 
{
  assertQueryEquals(qp, field,
                    field + L":[" + escapeDateString(startDate) + L" TO " +
                        escapeDateString(endDate) + L"]",
                    L"[" + getDate(startDate, resolution) + L" TO " +
                        getDate(endDateInclusive, resolution) + L"]");
  assertQueryEquals(qp, field,
                    field + L":{" + escapeDateString(startDate) + L" TO " +
                        escapeDateString(endDate) + L"}",
                    L"{" + getDate(startDate, resolution) + L" TO " +
                        getDate(endDate, resolution) + L"}");
}

void TestPrecedenceQueryParser::testEscaped() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);

  assertQueryEquals(L"a\\-b:c", a, L"a-b:c");
  assertQueryEquals(L"a\\+b:c", a, L"a+b:c");
  assertQueryEquals(L"a\\:b:c", a, L"a:b:c");
  assertQueryEquals(L"a\\\\b:c", a, L"a\\b:c");

  assertQueryEquals(L"a:b\\-c", a, L"a:b-c");
  assertQueryEquals(L"a:b\\+c", a, L"a:b+c");
  assertQueryEquals(L"a:b\\:c", a, L"a:b:c");
  assertQueryEquals(L"a:b\\\\c", a, L"a:b\\c");

  assertQueryEquals(L"a:b\\-c*", a, L"a:b-c*");
  assertQueryEquals(L"a:b\\+c*", a, L"a:b+c*");
  assertQueryEquals(L"a:b\\:c*", a, L"a:b:c*");

  assertQueryEquals(L"a:b\\\\c*", a, L"a:b\\c*");

  assertQueryEquals(L"a:b\\-?c", a, L"a:b-?c");
  assertQueryEquals(L"a:b\\+?c", a, L"a:b+?c");
  assertQueryEquals(L"a:b\\:?c", a, L"a:b:?c");

  assertQueryEquals(L"a:b\\\\?c", a, L"a:b\\?c");

  assertQueryEquals(L"a:b\\-c~", a, L"a:b-c~2");
  assertQueryEquals(L"a:b\\+c~", a, L"a:b+c~2");
  assertQueryEquals(L"a:b\\:c~", a, L"a:b:c~2");
  assertQueryEquals(L"a:b\\\\c~", a, L"a:b\\c~2");

  assertQueryEquals(L"[ a\\- TO a\\+ ]", nullptr, L"[a- TO a+]");
  assertQueryEquals(L"[ a\\: TO a\\~ ]", nullptr, L"[a: TO a~]");
  assertQueryEquals(L"[ a\\\\ TO a\\* ]", nullptr, L"[a\\ TO a*]");
}

void TestPrecedenceQueryParser::testTabNewlineCarriageReturn() throw(
    runtime_error)
{
  assertQueryEqualsDOA(L"+weltbank +worlbank", nullptr, L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \n +worlbank", nullptr,
                       L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\r+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r +worlbank", nullptr,
                       L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\r\n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r\n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r\n +worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r \n +worlbank", nullptr,
                       L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\t+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \t+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \t +worlbank", nullptr,
                       L"+weltbank +worlbank");
}

void TestPrecedenceQueryParser::testSimpleDAO() 
{
  assertQueryEqualsDOA(L"term term term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term +term term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term term +term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term +term +term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"-term term term", nullptr, L"-term +term +term");
}

void TestPrecedenceQueryParser::testBoost() 
{
  shared_ptr<CharacterRunAutomaton> stopSet =
      make_shared<CharacterRunAutomaton>(Automata::makeString(L"on"));
  shared_ptr<Analyzer> oneStopAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true, stopSet);

  shared_ptr<PrecedenceQueryParser> qp = make_shared<PrecedenceQueryParser>();
  qp->setAnalyzer(oneStopAnalyzer);
  shared_ptr<Query> q = qp->parse(L"on^1.0", L"field");
  assertNotNull(q);
  q = qp->parse(L"\"hello\"^2.0", L"field");
  assertNotNull(q);
  assertEquals((std::static_pointer_cast<BoostQuery>(q))->getBoost(),
               static_cast<float>(2.0), static_cast<float>(0.5));
  q = qp->parse(L"hello^2.0", L"field");
  assertNotNull(q);
  assertEquals((std::static_pointer_cast<BoostQuery>(q))->getBoost(),
               static_cast<float>(2.0), static_cast<float>(0.5));
  q = qp->parse(L"\"on\"^1.0", L"field");
  assertNotNull(q);

  q = getParser(make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                          MockTokenFilter::ENGLISH_STOPSET))
          ->parse(L"the^3", L"field");
  assertNotNull(q);
}

void TestPrecedenceQueryParser::testException() 
{
  expectThrows(QueryNodeParseException::typeid,
               [&]() { assertQueryEquals(L"\"some phrase", nullptr, L"abc"); });
}

void TestPrecedenceQueryParser::testBooleanQuery() 
{
  BooleanQuery::setMaxClauseCount(2);
  expectThrows(QueryNodeException::typeid, [&]() {
    getParser(
        make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false))
        ->parse(L"one two three", L"field");
  });
}

void TestPrecedenceQueryParser::testNOT() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"NOT foo AND bar", a, L"-foo +bar");
}

void TestPrecedenceQueryParser::testPrecedence() 
{
  shared_ptr<PrecedenceQueryParser> parser = getParser(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  shared_ptr<Query> query1 = parser->parse(L"A AND B OR C AND D", L"field");
  shared_ptr<Query> query2 = parser->parse(L"(A AND B) OR (C AND D)", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A OR B C", L"field");
  query2 = parser->parse(L"(A B) C", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A AND B C", L"field");
  query2 = parser->parse(L"(+A +B) C", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A AND NOT B", L"field");
  query2 = parser->parse(L"+A -B", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A OR NOT B", L"field");
  query2 = parser->parse(L"A -B", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A OR NOT B AND C", L"field");
  query2 = parser->parse(L"A (-B +C)", L"field");
  assertEquals(query1, query2);

  parser->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  query1 = parser->parse(L"A AND B OR C AND D", L"field");
  query2 = parser->parse(L"(A AND B) OR (C AND D)", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A AND B C", L"field");
  query2 = parser->parse(L"(A B) C", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A AND B C", L"field");
  query2 = parser->parse(L"(+A +B) C", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A AND NOT B", L"field");
  query2 = parser->parse(L"+A -B", L"field");
  assertEquals(query1, query2);

  query1 = parser->parse(L"A AND NOT B OR C", L"field");
  query2 = parser->parse(L"(+A -B) OR C", L"field");
  assertEquals(query1, query2);
}

void TestPrecedenceQueryParser::tearDown() 
{
  BooleanQuery::setMaxClauseCount(originalMaxClauses);
  LuceneTestCase::tearDown();
}
} // namespace org::apache::lucene::queryparser::flexible::precedence