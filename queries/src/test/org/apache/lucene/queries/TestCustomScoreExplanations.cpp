using namespace std;

#include "TestCustomScoreExplanations.h"

namespace org::apache::lucene::queries
{
using Term = org::apache::lucene::index::Term;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using ConstValueSource =
    org::apache::lucene::queries::function::valuesource::ConstValueSource;
using BaseExplanationTestCase =
    org::apache::lucene::search::BaseExplanationTestCase;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;

void TestCustomScoreExplanations::testOneTerm() 
{
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<CustomScoreQuery> csq = make_shared<CustomScoreQuery>(
      q, make_shared<FunctionQuery>(make_shared<ConstValueSource>(5)));
  qtest(csq, std::deque<int>{0, 1, 2, 3});
}

void TestCustomScoreExplanations::testBoost() 
{
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<CustomScoreQuery> csq = make_shared<CustomScoreQuery>(
      q, make_shared<FunctionQuery>(make_shared<ConstValueSource>(5)));
  qtest(make_shared<BoostQuery>(csq, 4), std::deque<int>{0, 1, 2, 3});
}

void TestCustomScoreExplanations::testTopLevelBoost() 
{
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<CustomScoreQuery> csq = make_shared<CustomScoreQuery>(
      q, make_shared<FunctionQuery>(make_shared<ConstValueSource>(5)));
  shared_ptr<BooleanQuery::Builder> bqB = make_shared<BooleanQuery::Builder>();
  bqB->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::MUST);
  bqB->add(csq, BooleanClause::Occur::MUST);
  shared_ptr<BooleanQuery> bq = bqB->build();
  qtest(make_shared<BoostQuery>(bq, 6), std::deque<int>{0, 1, 2, 3});
}

void TestCustomScoreExplanations::testSubExplanations() 
{
  shared_ptr<Query> query =
      make_shared<FunctionQuery>(make_shared<ConstValueSource>(5));
  shared_ptr<IndexSearcher> searcher =
      newSearcher(BaseExplanationTestCase::searcher->getIndexReader());
  searcher->setSimilarity(make_shared<BM25Similarity>());

  shared_ptr<Explanation> expl = searcher->explain(query, 0);
  assertEquals(2, expl->getDetails().size());
  // function
  assertEquals(5.0f, expl->getDetails()[0]->getValue(), 0.0f);
  // boost
  assertEquals(L"boost", expl->getDetails()[1]->getDescription());
  assertEquals(1.0f, expl->getDetails()[1]->getValue(), 0.0f);

  query = make_shared<BoostQuery>(query, 2);
  expl = searcher->explain(query, 0);
  assertEquals(2, expl->getDetails().size());
  // function
  assertEquals(5.0f, expl->getDetails()[0]->getValue(), 0.0f);
  // boost
  assertEquals(L"boost", expl->getDetails()[1]->getDescription());
  assertEquals(2.0f, expl->getDetails()[1]->getValue(), 0.0f);

  searcher->setSimilarity(
      make_shared<ClassicSimilarity>()); // in order to have a queryNorm != 1
  expl = searcher->explain(query, 0);
  assertEquals(2, expl->getDetails().size());
  // function
  assertEquals(5.0f, expl->getDetails()[0]->getValue(), 0.0f);
  // boost
  assertEquals(L"boost", expl->getDetails()[1]->getDescription());
  assertEquals(2.0f, expl->getDetails()[1]->getValue(), 0.0f);
}
} // namespace org::apache::lucene::queries