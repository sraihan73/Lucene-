using namespace std;

#include "TestSpanSearchEquivalence.h"

namespace org::apache::lucene::search::spans
{
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SearchEquivalenceTestBase =
    org::apache::lucene::search::SearchEquivalenceTestBase;
using TermQuery = org::apache::lucene::search::TermQuery;
using namespace org::apache::lucene::search::spans;
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

void TestSpanSearchEquivalence::testSpanTermVersusTerm() 
{
  shared_ptr<Term> t1 = randomTerm();
  assertSameScores(make_shared<TermQuery>(t1),
                   SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)));
}

void TestSpanSearchEquivalence::testSpanOrVersusTerm() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<SpanQuery> term =
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1));
  assertSameSet(SpanTestUtil::spanQuery(make_shared<SpanOrQuery>(term)), term);
}

void TestSpanSearchEquivalence::testSpanOrDoubleVersusTerm() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<SpanQuery> term =
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1));
  assertSameSet(SpanTestUtil::spanQuery(make_shared<SpanOrQuery>(term, term)),
                term);
}

void TestSpanSearchEquivalence::testSpanOrVersusBooleanTerm() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(t1), Occur::SHOULD);
  q1->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  shared_ptr<SpanQuery> q2 = SpanTestUtil::spanQuery(make_shared<SpanOrQuery>(
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))));
  assertSameSet(q1->build(), q2);
}

void TestSpanSearchEquivalence::testSpanOrVersusBooleanNear() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<Term> t4 = randomTerm();
  shared_ptr<SpanQuery> near1 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))},
          10, random()->nextBoolean()));
  shared_ptr<SpanQuery> near2 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3)),
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t4))},
          10, random()->nextBoolean()));
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(near1, Occur::SHOULD);
  q1->add(near2, Occur::SHOULD);
  shared_ptr<SpanQuery> q2 =
      SpanTestUtil::spanQuery(make_shared<SpanOrQuery>(near1, near2));
  assertSameSet(q1->build(), q2);
}

void TestSpanSearchEquivalence::testSpanNotVersusSpanTerm() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  assertSubsetOf(SpanTestUtil::spanQuery(make_shared<SpanNotQuery>(
                     SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
                     SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2)))),
                 SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)));
}

void TestSpanSearchEquivalence::testSpanNotNearVersusSpanTerm() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<SpanQuery> near =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2)),
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3))},
          10, random()->nextBoolean()));
  assertSubsetOf(
      SpanTestUtil::spanQuery(make_shared<SpanNotQuery>(
          SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), near)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)));
}

void TestSpanSearchEquivalence::testSpanNotVersusSpanNear() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<SpanQuery> near =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))},
          10, random()->nextBoolean()));
  assertSubsetOf(
      SpanTestUtil::spanQuery(make_shared<SpanNotQuery>(
          near, SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3)))),
      near);
}

void TestSpanSearchEquivalence::testSpanNotNearVersusSpanNear() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<Term> t4 = randomTerm();
  shared_ptr<SpanQuery> near1 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))},
          10, random()->nextBoolean()));
  shared_ptr<SpanQuery> near2 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3)),
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t4))},
          10, random()->nextBoolean()));
  assertSubsetOf(
      SpanTestUtil::spanQuery(make_shared<SpanNotQuery>(near1, near2)), near1);
}

void TestSpanSearchEquivalence::testSpanFirstVersusSpanTerm() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  assertSubsetOf(
      SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(
          SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), 10)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)));
}

void TestSpanSearchEquivalence::testSpanNearVersusPhrase() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> q1 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 0, true));
  shared_ptr<PhraseQuery> q2 =
      make_shared<PhraseQuery>(t1->field(), t1->bytes(), t2->bytes());
  if (t1->equals(t2)) {
    assertSameSet(q1, q2);
  } else {
    assertSameScores(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanNearVersusBooleanAnd() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> q1 = SpanTestUtil::spanQuery(
      make_shared<SpanNearQuery>(subquery, numeric_limits<int>::max(), false));
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::MUST);
  q2->add(make_shared<TermQuery>(t2), Occur::MUST);
  assertSameSet(q1, q2->build());
}

void TestSpanSearchEquivalence::testSpanNearVersusSloppySpanNear() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> q1 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 0, false));
  shared_ptr<SpanQuery> q2 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 1, false));
  assertSubsetOf(q1, q2);
}

void TestSpanSearchEquivalence::testSpanNearInOrderVersusOutOfOrder() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> q1 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 3, true));
  shared_ptr<SpanQuery> q2 =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 3, false));
  assertSubsetOf(q1, q2);
}

void TestSpanSearchEquivalence::testSpanNearIncreasingSloppiness() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  for (int i = 0; i < 10; i++) {
    shared_ptr<SpanQuery> q1 =
        SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, i, false));
    shared_ptr<SpanQuery> q2 = SpanTestUtil::spanQuery(
        make_shared<SpanNearQuery>(subquery, i + 1, false));
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanNearIncreasingSloppiness3() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3))};
  for (int i = 0; i < 10; i++) {
    shared_ptr<SpanQuery> q1 =
        SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, i, false));
    shared_ptr<SpanQuery> q2 = SpanTestUtil::spanQuery(
        make_shared<SpanNearQuery>(subquery, i + 1, false));
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanNearIncreasingOrderedSloppiness() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  for (int i = 0; i < 10; i++) {
    shared_ptr<SpanQuery> q1 =
        SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, i, false));
    shared_ptr<SpanQuery> q2 = SpanTestUtil::spanQuery(
        make_shared<SpanNearQuery>(subquery, i + 1, false));
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::
    testSpanNearIncreasingOrderedSloppiness3() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3))};
  for (int i = 0; i < 10; i++) {
    shared_ptr<SpanQuery> q1 =
        SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, i, true));
    shared_ptr<SpanQuery> q2 = SpanTestUtil::spanQuery(
        make_shared<SpanNearQuery>(subquery, i + 1, true));
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanRangeTerm() 
{
  shared_ptr<Term> t1 = randomTerm();
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      shared_ptr<Query> q1 =
          SpanTestUtil::spanQuery(make_shared<SpanPositionRangeQuery>(
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), i,
              i + j));
      shared_ptr<Query> q2 = make_shared<TermQuery>(t1);
      assertSubsetOf(q1, q2);
    }
  }
}

void TestSpanSearchEquivalence::testSpanRangeTermIncreasingEnd() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      shared_ptr<Query> q1 =
          SpanTestUtil::spanQuery(make_shared<SpanPositionRangeQuery>(
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), i,
              i + j));
      shared_ptr<Query> q2 =
          SpanTestUtil::spanQuery(make_shared<SpanPositionRangeQuery>(
              SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), i,
              i + j + 1));
      assertSubsetOf(q1, q2);
    }
  }
}

void TestSpanSearchEquivalence::testSpanRangeTermEverything() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Query> q1 =
      SpanTestUtil::spanQuery(make_shared<SpanPositionRangeQuery>(
          SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), 0,
          numeric_limits<int>::max()));
  shared_ptr<Query> q2 = make_shared<TermQuery>(t1);
  assertSameSet(q1, q2);
}

void TestSpanSearchEquivalence::testSpanRangeNear() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      shared_ptr<Query> q1 = SpanTestUtil::spanQuery(
          make_shared<SpanPositionRangeQuery>(nearQuery, i, i + j));
      shared_ptr<Query> q2 = nearQuery;
      assertSubsetOf(q1, q2);
    }
  }
}

void TestSpanSearchEquivalence::testSpanRangeNearIncreasingEnd() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      shared_ptr<Query> q1 = SpanTestUtil::spanQuery(
          make_shared<SpanPositionRangeQuery>(nearQuery, i, i + j));
      shared_ptr<Query> q2 = SpanTestUtil::spanQuery(
          make_shared<SpanPositionRangeQuery>(nearQuery, i, i + j + 1));
      assertSubsetOf(q1, q2);
    }
  }
}

void TestSpanSearchEquivalence::testSpanRangeNearEverything() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));
  shared_ptr<Query> q1 =
      SpanTestUtil::spanQuery(make_shared<SpanPositionRangeQuery>(
          nearQuery, 0, numeric_limits<int>::max()));
  shared_ptr<Query> q2 = nearQuery;
  assertSameSet(q1, q2);
}

void TestSpanSearchEquivalence::testSpanFirstTerm() 
{
  shared_ptr<Term> t1 = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<Query> q1 = SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(
        SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), i));
    shared_ptr<Query> q2 = make_shared<TermQuery>(t1);
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanFirstTermIncreasing() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<Query> q1 = SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(
        SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), i));
    shared_ptr<Query> q2 = SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(
        SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)), i + 1));
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanFirstTermEverything() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Query> q1 = SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      numeric_limits<int>::max()));
  shared_ptr<Query> q2 = make_shared<TermQuery>(t1);
  assertSameSet(q1, q2);
}

void TestSpanSearchEquivalence::testSpanFirstNear() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));
  for (int i = 0; i < 10; i++) {
    shared_ptr<Query> q1 =
        SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(nearQuery, i));
    shared_ptr<Query> q2 = nearQuery;
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanFirstNearIncreasing() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));
  for (int i = 0; i < 10; i++) {
    shared_ptr<Query> q1 =
        SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(nearQuery, i));
    shared_ptr<Query> q2 =
        SpanTestUtil::spanQuery(make_shared<SpanFirstQuery>(nearQuery, i + 1));
    assertSubsetOf(q1, q2);
  }
}

void TestSpanSearchEquivalence::testSpanFirstNearEverything() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));
  shared_ptr<Query> q1 = SpanTestUtil::spanQuery(
      make_shared<SpanFirstQuery>(nearQuery, numeric_limits<int>::max()));
  shared_ptr<Query> q2 = nearQuery;
  assertSameSet(q1, q2);
}

void TestSpanSearchEquivalence::testSpanWithinVsNear() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));

  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<SpanQuery> termQuery =
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3));
  shared_ptr<Query> q1 = SpanTestUtil::spanQuery(
      make_shared<SpanWithinQuery>(nearQuery, termQuery));
  assertSubsetOf(q1, termQuery);
}

void TestSpanSearchEquivalence::testSpanWithinVsContaining() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  std::deque<std::shared_ptr<SpanQuery>> subquery = {
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t1)),
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t2))};
  shared_ptr<SpanQuery> nearQuery =
      SpanTestUtil::spanQuery(make_shared<SpanNearQuery>(subquery, 10, true));

  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<SpanQuery> termQuery =
      SpanTestUtil::spanQuery(make_shared<SpanTermQuery>(t3));
  shared_ptr<Query> q1 = SpanTestUtil::spanQuery(
      make_shared<SpanWithinQuery>(nearQuery, termQuery));
  shared_ptr<Query> q2 = SpanTestUtil::spanQuery(
      make_shared<SpanContainingQuery>(nearQuery, termQuery));
  assertSameSet(q1, q2);
}

void TestSpanSearchEquivalence::testSpanBoostQuerySimplification() throw(
    runtime_error)
{
  float b1 = random()->nextFloat() * 10;
  float b2 = random()->nextFloat() * 10;
  shared_ptr<Term> term = randomTerm();

  shared_ptr<Query> q1 = make_shared<SpanBoostQuery>(
      make_shared<SpanBoostQuery>(make_shared<SpanTermQuery>(term), b2), b1);
  // Use AssertingQuery to prevent BoostQuery from merging inner and outer
  // boosts
  shared_ptr<Query> q2 = make_shared<SpanBoostQuery>(
      make_shared<AssertingSpanQuery>(
          make_shared<SpanBoostQuery>(make_shared<SpanTermQuery>(term), b2)),
      b1);

  assertSameScores(q1, q2);
}
} // namespace org::apache::lucene::search::spans