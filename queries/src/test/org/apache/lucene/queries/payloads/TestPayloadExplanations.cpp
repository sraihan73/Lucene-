using namespace std;

#include "TestPayloadExplanations.h"

namespace org::apache::lucene::queries::payloads
{
using Term = org::apache::lucene::index::Term;
using BaseExplanationTestCase =
    org::apache::lucene::search::BaseExplanationTestCase;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
std::deque<std::shared_ptr<PayloadFunction>>
    TestPayloadExplanations::functions = {make_shared<AveragePayloadFunction>(),
                                          make_shared<MinPayloadFunction>(),
                                          make_shared<MaxPayloadFunction>()};

void TestPayloadExplanations::setUp() 
{
  BaseExplanationTestCase::setUp();
  searcher->setSimilarity(
      make_shared<ClassicSimilarityAnonymousInnerClass>(shared_from_this()));
}

TestPayloadExplanations::ClassicSimilarityAnonymousInnerClass::
    ClassicSimilarityAnonymousInnerClass(
        shared_ptr<TestPayloadExplanations> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestPayloadExplanations::ClassicSimilarityAnonymousInnerClass::
    scorePayload(int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return 1 + (payload->hashCode() % 10);
}

shared_ptr<SpanQuery>
TestPayloadExplanations::pt(const wstring &s, shared_ptr<PayloadFunction> fn)
{
  return make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, s)), fn,
      random()->nextBoolean());
}

void TestPayloadExplanations::testPT1() 
{
  for (auto fn : functions) {
    qtest(pt(L"w1", fn), std::deque<int>{0, 1, 2, 3});
  }
}

void TestPayloadExplanations::testPT2() 
{
  for (auto fn : functions) {
    shared_ptr<SpanQuery> q = pt(L"w1", fn);
    qtest(make_shared<SpanBoostQuery>(q, 1000), std::deque<int>{0, 1, 2, 3});
  }
}

void TestPayloadExplanations::testPT4() 
{
  for (auto fn : functions) {
    qtest(pt(L"xx", fn), std::deque<int>{2, 3});
  }
}

void TestPayloadExplanations::testPT5() 
{
  for (auto fn : functions) {
    shared_ptr<SpanQuery> q = pt(L"xx", fn);
    qtest(make_shared<SpanBoostQuery>(q, 1000), std::deque<int>{2, 3});
  }
}

void TestPayloadExplanations::testAllFunctions(
    shared_ptr<SpanQuery> query,
    std::deque<int> &expected) 
{
  for (auto fn : functions) {
    qtest(make_shared<PayloadScoreQuery>(query, fn, random()->nextBoolean()),
          expected);
  }
}

void TestPayloadExplanations::testSimpleTerm() 
{
  shared_ptr<SpanTermQuery> q =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2"));
  testAllFunctions(q, std::deque<int>{0, 1, 2, 3});
}

void TestPayloadExplanations::testOrTerm() 
{
  shared_ptr<SpanOrQuery> q = make_shared<SpanOrQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"xx")),
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"yy")));
  testAllFunctions(q, std::deque<int>{2, 3});
}

void TestPayloadExplanations::testOrderedNearQuery() 
{
  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w3")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2"))},
      1, true);
  testAllFunctions(q, std::deque<int>{1, 3});
}

void TestPayloadExplanations::testUnorderedNearQuery() 
{
  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w3"))},
      1, false);
  testAllFunctions(q, std::deque<int>{0, 1, 2, 3});
}
} // namespace org::apache::lucene::queries::payloads