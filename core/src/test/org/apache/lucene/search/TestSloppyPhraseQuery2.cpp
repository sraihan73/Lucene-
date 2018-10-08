using namespace std;

#include "TestSloppyPhraseQuery2.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSloppyPhraseQuery2::testIncreasingSloppiness() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery> q1 =
        make_shared<PhraseQuery>(i, t1->field(), t1->bytes(), t2->bytes());
    shared_ptr<PhraseQuery> q2 =
        make_shared<PhraseQuery>(i + 1, t1->field(), t1->bytes(), t2->bytes());
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::testIncreasingSloppinessWithHoles() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    builder->add(t1, 0);
    builder->add(t2, 2);
    builder->setSlop(i);
    shared_ptr<PhraseQuery> q1 = builder->build();
    builder->setSlop(i + 1);
    shared_ptr<PhraseQuery> q2 = builder->build();
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::testIncreasingSloppiness3() 
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery> q1 = make_shared<PhraseQuery>(
        i, t1->field(), t1->bytes(), t2->bytes(), t3->bytes());
    shared_ptr<PhraseQuery> q2 = make_shared<PhraseQuery>(
        i + 1, t1->field(), t1->bytes(), t2->bytes(), t3->bytes());
    assertSubsetOf(q1, q2);
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::testIncreasingSloppiness3WithHoles() throw(
    runtime_error)
{
  shared_ptr<Term> t1 = randomTerm();
  shared_ptr<Term> t2 = randomTerm();
  shared_ptr<Term> t3 = randomTerm();
  int pos1 = 1 + random()->nextInt(3);
  int pos2 = pos1 + 1 + random()->nextInt(3);
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    builder->add(t1, 0);
    builder->add(t2, pos1);
    builder->add(t3, pos2);
    builder->setSlop(i);
    shared_ptr<PhraseQuery> q1 = builder->build();
    builder->setSlop(i + 1);
    shared_ptr<PhraseQuery> q2 = builder->build();
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::testRepetitiveIncreasingSloppiness() throw(
    runtime_error)
{
  shared_ptr<Term> t = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery> q1 =
        make_shared<PhraseQuery>(i, t->field(), t->bytes(), t->bytes());
    shared_ptr<PhraseQuery> q2 =
        make_shared<PhraseQuery>(i + 1, t->field(), t->bytes(), t->bytes());
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::
    testRepetitiveIncreasingSloppinessWithHoles() 
{
  shared_ptr<Term> t = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    builder->add(t, 0);
    builder->add(t, 2);
    builder->setSlop(i);
    shared_ptr<PhraseQuery> q1 = builder->build();
    builder->setSlop(i + 1);
    shared_ptr<PhraseQuery> q2 = builder->build();
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::testRepetitiveIncreasingSloppiness3() throw(
    runtime_error)
{
  shared_ptr<Term> t = randomTerm();
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery> q1 = make_shared<PhraseQuery>(
        i, t->field(), t->bytes(), t->bytes(), t->bytes());
    shared_ptr<PhraseQuery> q2 = make_shared<PhraseQuery>(
        i + 1, t->field(), t->bytes(), t->bytes(), t->bytes());
    assertSubsetOf(q1, q2);
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::
    testRepetitiveIncreasingSloppiness3WithHoles() 
{
  shared_ptr<Term> t = randomTerm();
  int pos1 = 1 + random()->nextInt(3);
  int pos2 = pos1 + 1 + random()->nextInt(3);
  for (int i = 0; i < 10; i++) {
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    builder->add(t, 0);
    builder->add(t, pos1);
    builder->add(t, pos2);
    builder->setSlop(i);
    shared_ptr<PhraseQuery> q1 = builder->build();
    builder->setSlop(i + 1);
    shared_ptr<PhraseQuery> q2 = builder->build();
    assertSubsetOf(q1, q2);
    assertSubsetOf(q1, q2);
  }
}

void TestSloppyPhraseQuery2::testRandomIncreasingSloppiness() throw(
    runtime_error)
{
  int64_t seed = random()->nextLong();
  for (int i = 0; i < 10; i++) {
    shared_ptr<MultiPhraseQuery> q1 = randomPhraseQuery(seed);
    shared_ptr<MultiPhraseQuery> q2 = randomPhraseQuery(seed);
    q1 = (make_shared<MultiPhraseQuery::Builder>(q1))->setSlop(i)->build();
    q2 = (make_shared<MultiPhraseQuery::Builder>(q2))->setSlop(i + 1)->build();
    assertSubsetOf(q1, q2);
  }
}

shared_ptr<MultiPhraseQuery>
TestSloppyPhraseQuery2::randomPhraseQuery(int64_t seed)
{
  shared_ptr<Random> random = make_shared<Random>(seed);
  int length = TestUtil::nextInt(random, 2, 5);
  shared_ptr<MultiPhraseQuery::Builder> pqb =
      make_shared<MultiPhraseQuery::Builder>();
  int position = 0;
  for (int i = 0; i < length; i++) {
    int depth = TestUtil::nextInt(random, 1, 3);
    std::deque<std::shared_ptr<Term>> terms(depth);
    for (int j = 0; j < depth; j++) {
      terms[j] = make_shared<Term>(
          L"field", L"" + StringHelper::toString(static_cast<wchar_t>(
                              TestUtil::nextInt(random, L'a', L'z'))));
    }
    pqb->add(terms, position);
    position += TestUtil::nextInt(random, 1, 3);
  }
  return pqb->build();
}
} // namespace org::apache::lucene::search