using namespace std;

#include "TestFunctionScoreExplanations.h"

namespace org::apache::lucene::queries::function
{
using Term = org::apache::lucene::index::Term;
using BaseExplanationTestCase =
    org::apache::lucene::search::BaseExplanationTestCase;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;

void TestFunctionScoreExplanations::testOneTerm() 
{
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<FunctionScoreQuery> fsq =
      make_shared<FunctionScoreQuery>(q, DoubleValuesSource::constant(5));
  qtest(fsq, std::deque<int>{0, 1, 2, 3});
}

void TestFunctionScoreExplanations::testBoost() 
{
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<FunctionScoreQuery> csq =
      make_shared<FunctionScoreQuery>(q, DoubleValuesSource::constant(5));
  qtest(make_shared<BoostQuery>(csq, 4), std::deque<int>{0, 1, 2, 3});
}

void TestFunctionScoreExplanations::testTopLevelBoost() 
{
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<FunctionScoreQuery> csq =
      make_shared<FunctionScoreQuery>(q, DoubleValuesSource::constant(5));
  shared_ptr<BooleanQuery::Builder> bqB = make_shared<BooleanQuery::Builder>();
  bqB->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::MUST);
  bqB->add(csq, BooleanClause::Occur::MUST);
  shared_ptr<BooleanQuery> bq = bqB->build();
  qtest(make_shared<BoostQuery>(bq, 6), std::deque<int>{0, 1, 2, 3});
}

void TestFunctionScoreExplanations::testExplanationsIncludingScore() throw(
    runtime_error)
{

  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<FunctionScoreQuery> csq =
      make_shared<FunctionScoreQuery>(q, DoubleValuesSource::SCORES);

  qtest(csq, std::deque<int>{0, 1, 2, 3});

  shared_ptr<Explanation> e1 = searcher->explain(q, 0);
  shared_ptr<Explanation> e = searcher->explain(csq, 0);

  assertEquals(e->getValue(), e1->getValue(), 0.00001);
  assertEquals(e->getDetails()[1], e1);
}

void TestFunctionScoreExplanations::testSubExplanations() 
{
  shared_ptr<Query> query = make_shared<FunctionScoreQuery>(
      make_shared<MatchAllDocsQuery>(), DoubleValuesSource::constant(5));
  shared_ptr<IndexSearcher> searcher =
      newSearcher(BaseExplanationTestCase::searcher->getIndexReader());
  searcher->setSimilarity(make_shared<BM25Similarity>());

  shared_ptr<Explanation> expl = searcher->explain(query, 0);
  shared_ptr<Explanation> subExpl = expl->getDetails()[1];
  assertEquals(L"constant(5.0)", subExpl->getDescription());
  assertEquals(0, subExpl->getDetails().size());

  query = make_shared<BoostQuery>(query, 2);
  expl = searcher->explain(query, 0);
  assertEquals(2, expl->getDetails().size());
  // function
  assertEquals(5.0f, expl->getDetails()[1]->getValue(), 0.0f);
  // boost
  assertEquals(L"boost", expl->getDetails()[0]->getDescription());
  assertEquals(2.0f, expl->getDetails()[0]->getValue(), 0.0f);

  searcher->setSimilarity(
      make_shared<ClassicSimilarity>()); // in order to have a queryNorm != 1
  expl = searcher->explain(query, 0);
  assertEquals(2, expl->getDetails().size());
  // function
  assertEquals(5.0f, expl->getDetails()[1]->getValue(), 0.0f);
  // boost
  assertEquals(L"boost", expl->getDetails()[0]->getDescription());
  assertEquals(2.0f, expl->getDetails()[0]->getValue(), 0.0f);
}
} // namespace org::apache::lucene::queries::function