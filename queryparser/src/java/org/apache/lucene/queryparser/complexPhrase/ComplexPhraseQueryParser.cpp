using namespace std;

#include "ComplexPhraseQueryParser.h"

namespace org::apache::lucene::queryparser::complexPhrase
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using RewriteMethod =
    org::apache::lucene::search::MultiTermQuery::RewriteMethod;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;

void ComplexPhraseQueryParser::setInOrder(bool const inOrder)
{
  this->inOrder = inOrder;
}

ComplexPhraseQueryParser::ComplexPhraseQueryParser(const wstring &f,
                                                   shared_ptr<Analyzer> a)
    : org::apache::lucene::queryparser::classic::QueryParser(f, a)
{
}

shared_ptr<Query>
ComplexPhraseQueryParser::getFieldQuery(const wstring &field,
                                        const wstring &queryText, int slop)
{
  shared_ptr<ComplexPhraseQuery> cpq =
      make_shared<ComplexPhraseQuery>(field, queryText, slop, inOrder);
  complexPhrases.push_back(cpq); // add to deque of phrases to be parsed once
  // we
  // are through with this pass
  return cpq;
}

shared_ptr<Query>
ComplexPhraseQueryParser::parse(const wstring &query) 
{
  if (isPass2ResolvingPhrases) {
    shared_ptr<MultiTermQuery::RewriteMethod> oldMethod =
        getMultiTermRewriteMethod();
    try {
      // Temporarily force BooleanQuery rewrite so that Parser will
      // generate visible
      // collection of terms which we can convert into SpanQueries.
      // ConstantScoreRewrite mode produces an
      // opaque ConstantScoreQuery object which cannot be interrogated for
      // terms in the same way a BooleanQuery can.
      // QueryParser is not guaranteed threadsafe anyway so this temporary
      // state change should not
      // present an issue
      setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
      return QueryParser::parse(query);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      setMultiTermRewriteMethod(oldMethod);
    }
  }

  // First pass - parse the top-level query recording any PhraseQuerys
  // which will need to be resolved
  complexPhrases = deque<std::shared_ptr<ComplexPhraseQuery>>();
  shared_ptr<Query> q = QueryParser::parse(query);

  // Perform second pass, using this QueryParser to parse any nested
  // PhraseQueries with different
  // set of syntax restrictions (i.e. all fields must be same)
  isPass2ResolvingPhrases = true;
  try {
    for (deque<std::shared_ptr<ComplexPhraseQuery>>::const_iterator iterator =
             complexPhrases.begin();
         iterator != complexPhrases.end(); ++iterator) {
      currentPhraseQuery = *iterator;
      // in each phrase, now parse the contents between quotes as a
      // separate parse operation
      currentPhraseQuery->parsePhraseElements(shared_from_this());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    isPass2ResolvingPhrases = false;
  }
  return q;
}

shared_ptr<Query> ComplexPhraseQueryParser::newTermQuery(shared_ptr<Term> term)
{
  if (isPass2ResolvingPhrases) {
    try {
      checkPhraseClauseIsForSameField(term->field());
    } catch (const ParseException &pe) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Error parsing complex
      // phrase", pe);
      throw runtime_error(L"Error parsing complex phrase");
    }
  }
  return QueryParser::newTermQuery(term);
}

void ComplexPhraseQueryParser::checkPhraseClauseIsForSameField(
    const wstring &field) 
{
  if (field != currentPhraseQuery->field) {
    throw make_shared<ParseException>(
        L"Cannot have clause for field \"" + field + L"\" nested in phrase " +
        L" for field \"" + currentPhraseQuery->field + L"\"");
  }
}

shared_ptr<Query> ComplexPhraseQueryParser::getWildcardQuery(
    const wstring &field, const wstring &termStr) 
{
  if (isPass2ResolvingPhrases) {
    checkPhraseClauseIsForSameField(field);
  }
  return QueryParser::getWildcardQuery(field, termStr);
}

shared_ptr<Query> ComplexPhraseQueryParser::getRangeQuery(
    const wstring &field, const wstring &part1, const wstring &part2,
    bool startInclusive, bool endInclusive) 
{
  if (isPass2ResolvingPhrases) {
    checkPhraseClauseIsForSameField(field);
  }
  return QueryParser::getRangeQuery(field, part1, part2, startInclusive,
                                    endInclusive);
}

shared_ptr<Query> ComplexPhraseQueryParser::newRangeQuery(const wstring &field,
                                                          const wstring &part1,
                                                          const wstring &part2,
                                                          bool startInclusive,
                                                          bool endInclusive)
{
  shared_ptr<MultiTermQuery::RewriteMethod> originalRewriteMethod =
      getMultiTermRewriteMethod();
  try {
    if (isPass2ResolvingPhrases) {
      setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
    }
    return QueryParser::newRangeQuery(field, part1, part2, startInclusive,
                                      endInclusive);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    setMultiTermRewriteMethod(originalRewriteMethod);
  }
}

shared_ptr<Query> ComplexPhraseQueryParser::getFuzzyQuery(
    const wstring &field, const wstring &termStr,
    float minSimilarity) 
{
  if (isPass2ResolvingPhrases) {
    checkPhraseClauseIsForSameField(field);
  }
  return QueryParser::getFuzzyQuery(field, termStr, minSimilarity);
}

ComplexPhraseQueryParser::ComplexPhraseQuery::ComplexPhraseQuery(
    const wstring &field, const wstring &phrasedQueryStringContents,
    int slopFactor, bool inOrder)
    : field(Objects::requireNonNull(field)),
      phrasedQueryStringContents(
          Objects::requireNonNull(phrasedQueryStringContents)),
      slopFactor(slopFactor), inOrder(inOrder)
{
}

void ComplexPhraseQueryParser::ComplexPhraseQuery::parsePhraseElements(
    shared_ptr<ComplexPhraseQueryParser> qp) 
{
  // TODO ensure that field-sensitivity is preserved ie the query
  // string below is parsed as
  // field+":("+phrasedQueryStringContents+")"
  // but this will need code in rewrite to unwrap the first layer of
  // bool query

  wstring oldDefaultParserField = qp->field;
  try {
    // temporarily set the QueryParser to be parsing the default field for this
    // phrase e.g author:"fred* smith"
    qp->field = this->field;
    contents[0] = qp->parse(phrasedQueryStringContents);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    qp->field = oldDefaultParserField;
  }
}

shared_ptr<Query> ComplexPhraseQueryParser::ComplexPhraseQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const contents = this->contents[0];
  // ArrayList spanClauses = new ArrayList();
  if (std::dynamic_pointer_cast<TermQuery>(contents) != nullptr ||
      std::dynamic_pointer_cast<MultiTermQuery>(contents) != nullptr ||
      std::dynamic_pointer_cast<SynonymQuery>(contents) != nullptr) {
    return contents;
  }
  // Build a sequence of Span clauses arranged in a SpanNear - child
  // clauses can be complex
  // Booleans e.g. nots and ors etc
  int numNegatives = 0;
  if (!(std::dynamic_pointer_cast<BooleanQuery>(contents) != nullptr)) {
    throw invalid_argument(L"Unknown query type \"" + contents->getClassName() +
                           L"\" found in phrase query string \"" +
                           phrasedQueryStringContents + L"\"");
  }
  shared_ptr<BooleanQuery> bq =
      std::static_pointer_cast<BooleanQuery>(contents);
  std::deque<std::shared_ptr<SpanQuery>> allSpanClauses(bq->clauses().size());
  // For all clauses e.g. one* two~
  int i = 0;
  for (auto clause : bq) {
    // HashSet bclauseterms=new HashSet();
    shared_ptr<Query> qc = clause->getQuery();
    // Rewrite this clause e.g one* becomes (one OR onerous)
    qc = (make_shared<IndexSearcher>(reader))->rewrite(qc);
    if (clause->getOccur().equals(BooleanClause::Occur::MUST_NOT)) {
      numNegatives++;
    }

    while (std::dynamic_pointer_cast<BoostQuery>(qc) != nullptr) {
      qc = (std::static_pointer_cast<BoostQuery>(qc))->getQuery();
    }

    if (std::dynamic_pointer_cast<BooleanQuery>(qc) != nullptr ||
        std::dynamic_pointer_cast<SynonymQuery>(qc) != nullptr) {
      deque<std::shared_ptr<SpanQuery>> sc =
          deque<std::shared_ptr<SpanQuery>>();
      shared_ptr<BooleanQuery> booleanCaluse =
          std::dynamic_pointer_cast<BooleanQuery>(qc) != nullptr
              ? std::static_pointer_cast<BooleanQuery>(qc)
              : convert(std::static_pointer_cast<SynonymQuery>(qc));
      addComplexPhraseClause(sc, booleanCaluse);
      if (sc.size() > 0) {
        allSpanClauses[i] = sc[0];
      } else {
        // Insert fake term e.g. phrase query was for "Fred Smithe*" and
        // there were no "Smithe*" terms - need to
        // prevent match on just "Fred".
        allSpanClauses[i] = make_shared<SpanTermQuery>(make_shared<Term>(
            field,
            L"Dummy clause because no terms found - must match nothing"));
      }
    } else if (std::dynamic_pointer_cast<MatchNoDocsQuery>(qc) != nullptr) {
      // Insert fake term e.g. phrase query was for "Fred Smithe*" and
      // there were no "Smithe*" terms - need to
      // prevent match on just "Fred".
      allSpanClauses[i] = make_shared<SpanTermQuery>(make_shared<Term>(
          field, L"Dummy clause because no terms found - must match nothing"));
    } else {
      if (std::dynamic_pointer_cast<TermQuery>(qc) != nullptr) {
        shared_ptr<TermQuery> tq = std::static_pointer_cast<TermQuery>(qc);
        allSpanClauses[i] = make_shared<SpanTermQuery>(tq->getTerm());
      } else {
        throw invalid_argument(L"Unknown query type \"" + qc->getClassName() +
                               L"\" found in phrase query string \"" +
                               phrasedQueryStringContents + L"\"");
      }
    }

    i += 1;
  }
  if (numNegatives == 0) {
    // The simple case - no negative elements in phrase
    return make_shared<SpanNearQuery>(allSpanClauses, slopFactor, inOrder);
  }
  // Complex case - we have mixed positives and negatives in the
  // sequence.
  // Need to return a SpanNotQuery
  deque<std::shared_ptr<SpanQuery>> positiveClauses =
      deque<std::shared_ptr<SpanQuery>>();
  i = 0;
  for (auto clause : bq) {
    if (!clause->getOccur().equals(BooleanClause::Occur::MUST_NOT)) {
      positiveClauses.push_back(allSpanClauses[i]);
    }
    i += 1;
  }

  std::deque<std::shared_ptr<SpanQuery>> includeClauses =
      positiveClauses.toArray(
          std::deque<std::shared_ptr<SpanQuery>>(positiveClauses.size()));

  shared_ptr<SpanQuery> include = nullptr;
  if (includeClauses.size() == 1) {
    include = includeClauses[0]; // only one positive clause
  } else {
    // need to increase slop factor based on gaps introduced by
    // negatives
    include = make_shared<SpanNearQuery>(includeClauses,
                                         slopFactor + numNegatives, inOrder);
  }
  // Use sequence of positive and negative values as the exclude.
  shared_ptr<SpanNearQuery> exclude =
      make_shared<SpanNearQuery>(allSpanClauses, slopFactor, inOrder);
  shared_ptr<SpanNotQuery> snot = make_shared<SpanNotQuery>(include, exclude);
  return snot;
}

shared_ptr<BooleanQuery> ComplexPhraseQueryParser::ComplexPhraseQuery::convert(
    shared_ptr<SynonymQuery> qc)
{
  shared_ptr<BooleanQuery::Builder> bqb = make_shared<BooleanQuery::Builder>();
  for (auto t : qc->getTerms()) {
    bqb->add(make_shared<BooleanClause>(make_shared<TermQuery>(t),
                                        BooleanClause::Occur::SHOULD));
  }
  return bqb->build();
}

void ComplexPhraseQueryParser::ComplexPhraseQuery::addComplexPhraseClause(
    deque<std::shared_ptr<SpanQuery>> &spanClauses,
    shared_ptr<BooleanQuery> qc)
{
  deque<std::shared_ptr<SpanQuery>> ors = deque<std::shared_ptr<SpanQuery>>();
  deque<std::shared_ptr<SpanQuery>> nots =
      deque<std::shared_ptr<SpanQuery>>();

  // For all clauses e.g. one* two~
  for (auto clause : qc) {
    shared_ptr<Query> childQuery = clause->getQuery();

    float boost = 1.0f;
    while (std::dynamic_pointer_cast<BoostQuery>(childQuery) != nullptr) {
      shared_ptr<BoostQuery> bq =
          std::static_pointer_cast<BoostQuery>(childQuery);
      boost *= bq->getBoost();
      childQuery = bq->getQuery();
    }

    // select the deque to which we will add these options
    deque<std::shared_ptr<SpanQuery>> &chosenList = ors;
    if (clause->getOccur() == BooleanClause::Occur::MUST_NOT) {
      chosenList = nots;
    }

    if (std::dynamic_pointer_cast<TermQuery>(childQuery) != nullptr) {
      shared_ptr<TermQuery> tq =
          std::static_pointer_cast<TermQuery>(childQuery);
      shared_ptr<SpanQuery> stq = make_shared<SpanTermQuery>(tq->getTerm());
      if (boost != 1.0f) {
        stq = make_shared<SpanBoostQuery>(stq, boost);
      }
      chosenList.push_back(stq);
    } else if (std::dynamic_pointer_cast<BooleanQuery>(childQuery) != nullptr) {
      shared_ptr<BooleanQuery> cbq =
          std::static_pointer_cast<BooleanQuery>(childQuery);
      addComplexPhraseClause(chosenList, cbq);
    } else if (std::dynamic_pointer_cast<MatchNoDocsQuery>(childQuery) !=
               nullptr) {
      // Insert fake term e.g. phrase query was for "Fred Smithe*" and
      // there were no "Smithe*" terms - need to
      // prevent match on just "Fred".
      shared_ptr<SpanQuery> stq = make_shared<SpanTermQuery>(make_shared<Term>(
          field, L"Dummy clause because no terms found - must match nothing"));
      chosenList.push_back(stq);
    } else {
      // TODO alternatively could call extract terms here?
      throw invalid_argument(L"Unknown query type:" +
                             childQuery->getClassName());
    }
  }
  if (ors.empty()) {
    return;
  }
  shared_ptr<SpanOrQuery> soq = make_shared<SpanOrQuery>(
      ors.toArray(std::deque<std::shared_ptr<SpanQuery>>(ors.size())));
  if (nots.empty()) {
    spanClauses.push_back(soq);
  } else {
    shared_ptr<SpanOrQuery> snqs = make_shared<SpanOrQuery>(
        nots.toArray(std::deque<std::shared_ptr<SpanQuery>>(nots.size())));
    shared_ptr<SpanNotQuery> snq = make_shared<SpanNotQuery>(soq, snqs);
    spanClauses.push_back(snq);
  }
}

wstring
ComplexPhraseQueryParser::ComplexPhraseQuery::toString(const wstring &field)
{
  if (slopFactor == 0) {
    return L"\"" + phrasedQueryStringContents + L"\"";
  } else {
    return L"\"" + phrasedQueryStringContents + L"\"" + L"~" +
           to_wstring(slopFactor);
  }
}

int ComplexPhraseQueryParser::ComplexPhraseQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + field.hashCode();
  result = prime * result + phrasedQueryStringContents.hashCode();
  result = prime * result + slopFactor;
  result = prime * result + (inOrder ? 1 : 0);
  return result;
}

bool ComplexPhraseQueryParser::ComplexPhraseQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool ComplexPhraseQueryParser::ComplexPhraseQuery::equalsTo(
    shared_ptr<ComplexPhraseQuery> other)
{
  return field == other->field &&
         phrasedQueryStringContents == other->phrasedQueryStringContents &&
         slopFactor == other->slopFactor && inOrder == other->inOrder;
}
} // namespace org::apache::lucene::queryparser::complexPhrase