using namespace std;

#include "TestSimpleSearchEquivalence.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;

void TestSimpleSearchEquivalence::testTermVersusBooleanOr() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::SHOULD);
  q2->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  assertSubsetOf(q1, q2->build());
}

void TestSimpleSearchEquivalence::testTermVersusBooleanReqOpt() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::MUST);
  q2->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  assertSubsetOf(q1, q2->build());
}

void TestSimpleSearchEquivalence::testBooleanReqExclVersusTerm() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(t1), Occur::MUST);
  q1->add(make_shared<TermQuery>(t2), Occur::MUST_NOT);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t1);
  assertSubsetOf(q1->build(), q2);
}

void TestSimpleSearchEquivalence::testBooleanAndVersusBooleanOr() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(t1), Occur::SHOULD);
  q1->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::SHOULD);
  q2->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  assertSubsetOf(q1->build(), q2->build());
}

void TestSimpleSearchEquivalence::
    testDisjunctionSumVersusDisjunctionMax() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(t1), Occur::SHOULD);
  q1->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  shared_ptr<DisjunctionMaxQuery> q2 = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(t1), make_shared<TermQuery>(t2)),
      0.5f);
  assertSameSet(q1->build(), q2);
}

void TestSimpleSearchEquivalence::testExactPhraseVersusBooleanAnd() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery> q1 =
      make_shared<PhraseQuery>(t1->field(), t1->bytes(), t2->bytes());
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::MUST);
  q2->add(make_shared<TermQuery>(t2), Occur::MUST);
  assertSubsetOf(q1, q2->build());
}

void TestSimpleSearchEquivalence::
    testExactPhraseVersusBooleanAndWithHoles() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(t1, 0);
  builder->add(t2, 2);
  shared_ptr<PhraseQuery> q1 = builder->build();
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::MUST);
  q2->add(make_shared<TermQuery>(t2), Occur::MUST);
  assertSubsetOf(q1, q2->build());
}

void TestSimpleSearchEquivalence::testPhraseVersusSloppyPhrase() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery> q1 =
      make_shared<PhraseQuery>(t1->field(), t1->bytes(), t2->bytes());
  shared_ptr<PhraseQuery> q2 =
      make_shared<PhraseQuery>(1, t1->field(), t1->bytes(), t2->bytes());
  assertSubsetOf(q1, q2);
}

void TestSimpleSearchEquivalence::testPhraseVersusSloppyPhraseWithHoles() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(t1, 0);
  builder->add(t2, 2);
  shared_ptr<PhraseQuery> q1 = builder->build();
  builder->setSlop(2);
  shared_ptr<PhraseQuery> q2 = builder->build();
  assertSubsetOf(q1, q2);
}

void TestSimpleSearchEquivalence::testExactPhraseVersusMultiPhrase() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery> q1 =
      make_shared<PhraseQuery>(t1->field(), t1->bytes(), t2->bytes());
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<MultiPhraseQuery::Builder> q2b =
      make_shared<MultiPhraseQuery::Builder>();
  q2b->add(t1);
  q2b->add(std::deque<std::shared_ptr<Term>>{t2, t3});
  assertSubsetOf(q1, q2b->build());
}

void TestSimpleSearchEquivalence::
    testExactPhraseVersusMultiPhraseWithHoles() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(t1, 0);
  builder->add(t2, 2);
  shared_ptr<PhraseQuery> q1 = builder->build();
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<MultiPhraseQuery::Builder> q2b =
      make_shared<MultiPhraseQuery::Builder>();
  q2b->add(t1);
  q2b->add(std::deque<std::shared_ptr<Term>>{t2, t3}, 2);
  assertSubsetOf(q1, q2b->build());
}

void TestSimpleSearchEquivalence::testSloppyPhraseVersusBooleanAnd() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = nullptr;
  // semantics differ from SpanNear: SloppyPhrase handles repeats,
  // so we must ensure t1 != t2
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<PhraseQuery> q1 = make_shared<PhraseQuery>(
      numeric_limits<int>::max(), t1->field(), t1->bytes(), t2->bytes());
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::MUST);
  q2->add(make_shared<TermQuery>(t2), Occur::MUST);
  assertSameSet(q1, q2->build());
}

void TestSimpleSearchEquivalence::testPhraseRelativePositions() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery> q1 =
      make_shared<PhraseQuery>(t1->field(), t1->bytes(), t2->bytes());
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(t1, 10000);
  builder->add(t2, 10001);
  shared_ptr<PhraseQuery> q2 = builder->build();
  assertSameScores(q1, q2);
}

void TestSimpleSearchEquivalence::testSloppyPhraseRelativePositions() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<PhraseQuery> q1 =
      make_shared<PhraseQuery>(2, t1->field(), t1->bytes(), t2->bytes());
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(t1, 10000);
  builder->add(t2, 10001);
  builder->setSlop(2);
  shared_ptr<PhraseQuery> q2 = builder->build();
  assertSameScores(q1, q2);
}

void TestSimpleSearchEquivalence::testBoostQuerySimplification() throw(
    runtime_error)
{
  float b1 = random()->nextFloat() * 10;
  float b2 = random()->nextFloat() * 10;
  shared_ptr<Term> term = randomTerm();

  shared_ptr<Query> q1 = make_shared<BoostQuery>(
      make_shared<BoostQuery>(make_shared<TermQuery>(term), b2), b1);
  // Use AssertingQuery to prevent BoostQuery from merging inner and outer
  // boosts
  shared_ptr<Query> q2 = make_shared<BoostQuery>(
      make_shared<AssertingQuery>(
          random(), make_shared<BoostQuery>(make_shared<TermQuery>(term), b2)),
      b1);

  assertSameScores(q1, q2);
}

void TestSimpleSearchEquivalence::testBooleanBoostPropagation() throw(
    runtime_error)
{
  float boost1 = random()->nextFloat();
  shared_ptr<Query> tq =
      make_shared<BoostQuery>(make_shared<TermQuery>(randomTerm()), boost1);

  float boost2 = random()->nextFloat();
  // Applying boost2 over the term or bool query should have the same effect
  shared_ptr<Query> q1 = make_shared<BoostQuery>(tq, boost2);
  shared_ptr<Query> q2 = (make_shared<BooleanQuery::Builder>())
                             ->add(tq, Occur::MUST)
                             ->add(tq, Occur::FILTER)
                             ->build();
  q2 = make_shared<BoostQuery>(q2, boost2);

  assertSameScores(q1, q2);
}

void TestSimpleSearchEquivalence::testBooleanOrVsSynonym() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  assertEquals(t1->field(), t2->field());
  shared_ptr<SynonymQuery> q1 = make_shared<SynonymQuery>(t1, t2);
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(t1), Occur::SHOULD);
  q2->add(make_shared<TermQuery>(t2), Occur::SHOULD);
  assertSameSet(q1, q2->build());
}
} // namespace org::apache::lucene::search