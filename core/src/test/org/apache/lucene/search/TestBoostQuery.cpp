using namespace std;

#include "TestBoostQuery.h"

namespace org::apache::lucene::search
{
using MultiReader = org::apache::lucene::index::MultiReader;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBoostQuery::testEquals()
{
  constexpr float boost = random()->nextFloat() * 3 - 1;
  shared_ptr<BoostQuery> q1 =
      make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), boost);
  shared_ptr<BoostQuery> q2 =
      make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), boost);
  assertEquals(q1, q2);
  assertEquals(q1->getBoost(), q2->getBoost(), 0.0f);

  float boost2 = boost;
  while (boost == boost2) {
    boost2 = random()->nextFloat() * 3 - 1;
  }
  shared_ptr<BoostQuery> q3 =
      make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), boost2);
  assertFalse(q1->equals(q3));
  assertFalse(q1->hashCode() == q3->hashCode());
}

void TestBoostQuery::testToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"(foo:bar)^2.0",
      (make_shared<BoostQuery>(
           make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 2))
          ->toString());
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::SHOULD)
          ->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(foo:bar foo:baz)^2.0",
               (make_shared<BoostQuery>(bq, 2))->toString());
}

void TestBoostQuery::testRewrite() 
{
  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(make_shared<MultiReader>());

  // inner queries are rewritten
  shared_ptr<Query> q = make_shared<BoostQuery>(
      (make_shared<BooleanQuery::Builder>())->build(), 2);
  assertEquals(make_shared<BoostQuery>(make_shared<MatchNoDocsQuery>(), 2),
               searcher->rewrite(q));

  // boosts are merged
  q = make_shared<BoostQuery>(
      make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), 3), 2);
  assertEquals(make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), 6),
               searcher->rewrite(q));

  // scores are not computed when the boost is 0
  q = make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), 0);
  assertEquals(
      make_shared<BoostQuery>(
          make_shared<ConstantScoreQuery>(make_shared<MatchAllDocsQuery>()), 0),
      searcher->rewrite(q));
}
} // namespace org::apache::lucene::search