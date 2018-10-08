using namespace std;

#include "TestAxiomaticSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestAxiomaticSimilarity::testIllegalS()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AxiomaticF2EXP>(numeric_limits<float>::infinity(), 0.1f);
  });
  assertTrue(expected.what()->contains(L"illegal s value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<AxiomaticF2EXP>(-1, 0.1f); });
  assertTrue(expected.what()->contains(L"illegal s value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<AxiomaticF2EXP>(NAN, 0.1f); });
  assertTrue(expected.what()->contains(L"illegal s value"));
}

void TestAxiomaticSimilarity::testIllegalK()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AxiomaticF2EXP>(0.35f, 2.0f);
  });
  assertTrue(expected.what()->contains(L"illegal k value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<AxiomaticF2EXP>(0.35f, -1.0f); });
  assertTrue(expected.what()->contains(L"illegal k value"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AxiomaticF2EXP>(0.35f, numeric_limits<float>::infinity());
  });
  assertTrue(expected.what()->contains(L"illegal k value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<AxiomaticF2EXP>(0.35f, NAN); });
  assertTrue(expected.what()->contains(L"illegal k value"));
}

void TestAxiomaticSimilarity::testIllegalQL()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AxiomaticF3EXP>(0.35f, -1);
  });
  assertTrue(expected.what()->contains(L"illegal query length value"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AxiomaticF2EXP>(0.35f, numeric_limits<int>::max() + 1);
  });
  assertTrue(expected.what()->contains(L"illegal k value"));
}
} // namespace org::apache::lucene::search::similarities