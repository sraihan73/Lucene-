using namespace std;

#include "TestApproximationSearchEquivalence.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;

void TestApproximationSearchEquivalence::testConjunction() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::MUST);
  bq1->add(q2, Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq2->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::MUST);

  assertSameScores(bq1->build(), bq2->build());
}

void TestApproximationSearchEquivalence::testNestedConjunction() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2;
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(t3);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::MUST);
  bq1->add(q2, Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(bq1->build(), Occur::MUST);
  bq2->add(q3, Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::MUST);
  bq4->add(q3, Occur::MUST);

  assertSameScores(bq2->build(), bq4->build());
}

void TestApproximationSearchEquivalence::testDisjunction() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::SHOULD);
  bq1->add(q2, Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::SHOULD);
  bq2->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::SHOULD);

  assertSameScores(bq1->build(), bq2->build());
}

void TestApproximationSearchEquivalence::testNestedDisjunction() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2;
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(t3);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::SHOULD);
  bq1->add(q2, Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(bq1->build(), Occur::SHOULD);
  bq2->add(q3, Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::SHOULD);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::SHOULD);
  bq4->add(q3, Occur::SHOULD);

  assertSameScores(bq2->build(), bq4->build());
}

void TestApproximationSearchEquivalence::testDisjunctionInConjunction() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2;
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(t3);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::SHOULD);
  bq1->add(q2, Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(bq1->build(), Occur::MUST);
  bq2->add(q3, Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::SHOULD);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::MUST);
  bq4->add(q3, Occur::MUST);

  assertSameScores(bq2->build(), bq4->build());
}

void TestApproximationSearchEquivalence::testConjunctionInDisjunction() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2;
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(t3);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::MUST);
  bq1->add(q2, Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(bq1->build(), Occur::SHOULD);
  bq2->add(q3, Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::SHOULD);
  bq4->add(q3, Occur::SHOULD);

  assertSameScores(bq2->build(), bq4->build());
}

void TestApproximationSearchEquivalence::testConstantScore() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(make_shared<ConstantScoreQuery>(q1), Occur::MUST);
  bq1->add(make_shared<ConstantScoreQuery>(q2), Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<ConstantScoreQuery>(
               make_shared<RandomApproximationQuery>(q1, random())),
           Occur::MUST);
  bq2->add(make_shared<ConstantScoreQuery>(
               make_shared<RandomApproximationQuery>(q2, random())),
           Occur::MUST);

  assertSameScores(bq1->build(), bq2->build());
}

void TestApproximationSearchEquivalence::testExclusion() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::MUST);
  bq1->add(q2, Occur::MUST_NOT);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq2->add(make_shared<RandomApproximationQuery>(q2, random()),
           Occur::MUST_NOT);

  assertSameScores(bq1->build(), bq2->build());
}

void TestApproximationSearchEquivalence::testNestedExclusion() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2;
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(t3);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::MUST);
  bq1->add(q2, Occur::MUST_NOT);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(bq1->build(), Occur::MUST);
  bq2->add(q3, Occur::MUST);

  // Both req and excl have approximations
  shared_ptr<BooleanQuery::Builder> bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()),
           Occur::MUST_NOT);

  shared_ptr<BooleanQuery::Builder> bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::MUST);
  bq4->add(q3, Occur::MUST);

  assertSameScores(bq2->build(), bq4->build());

  // Only req has an approximation
  bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq3->add(q2, Occur::MUST_NOT);

  bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::MUST);
  bq4->add(q3, Occur::MUST);

  assertSameScores(bq2->build(), bq4->build());

  // Only excl has an approximation
  bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(q1, Occur::MUST);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()),
           Occur::MUST_NOT);

  bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::MUST);
  bq4->add(q3, Occur::MUST);

  assertSameScores(bq2->build(), bq4->build());
}

void TestApproximationSearchEquivalence::testReqOpt() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2;
  do {
    t2 = randomTerm();
  } while (t1->equals(t2));
  shared_ptr<Term> t3 = randomTerm();
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(t1);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(t2);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(t3);

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(q1, Occur::MUST);
  bq1->add(q2, Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(bq1->build(), Occur::MUST);
  bq2->add(q3, Occur::MUST);

  shared_ptr<BooleanQuery::Builder> bq3 = make_shared<BooleanQuery::Builder>();
  bq3->add(make_shared<RandomApproximationQuery>(q1, random()), Occur::MUST);
  bq3->add(make_shared<RandomApproximationQuery>(q2, random()), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq4 = make_shared<BooleanQuery::Builder>();
  bq4->add(bq3->build(), Occur::MUST);
  bq4->add(q3, Occur::MUST);

  assertSameScores(bq2->build(), bq4->build());
}
} // namespace org::apache::lucene::search