using namespace std;

#include "TestDoubleRangeField.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestDoubleRangeField::FIELD_NAME = L"rangeField";

void TestDoubleRangeField::testIllegalNaNValues()
{
  shared_ptr<Document> doc = make_shared<Document>();
  invalid_argument expected;

  expected = expectThrows(invalid_argument::typeid, [&]() {
    doc->push_back(make_shared<DoubleRange>(
        FIELD_NAME, std::deque<double>{NAN}, std::deque<double>{5}));
  });
  assertTrue(expected.what()->contains(L"invalid min value"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    doc->push_back(make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{5},
                                            std::deque<double>{NAN}));
  });
  assertTrue(expected.what()->contains(L"invalid max value"));
}

void TestDoubleRangeField::testUnevenArrays()
{
  shared_ptr<Document> doc = make_shared<Document>();
  invalid_argument expected;
  expected = expectThrows(invalid_argument::typeid, [&]() {
    doc->push_back(make_shared<DoubleRange>(
        FIELD_NAME, std::deque<double>{5, 6}, std::deque<double>{5}));
  });
  assertTrue(expected.what()->contains(L"min/max ranges must agree"));
}

void TestDoubleRangeField::testOversizeDimensions()
{
  shared_ptr<Document> doc = make_shared<Document>();
  invalid_argument expected;
  expected = expectThrows(invalid_argument::typeid, [&]() {
    doc->push_back(make_shared<DoubleRange>(FIELD_NAME,
                                            std::deque<double>{1, 2, 3, 4, 5},
                                            std::deque<double>{5}));
  });
  assertTrue(
      expected.what()->contains(L"does not support greater than 4 dimensions"));
}

void TestDoubleRangeField::testMinGreaterThanMax()
{
  shared_ptr<Document> doc = make_shared<Document>();
  invalid_argument expected;
  expected = expectThrows(invalid_argument::typeid, [&]() {
    doc->push_back(make_shared<DoubleRange>(
        FIELD_NAME, std::deque<double>{3, 4}, std::deque<double>{1, 2}));
  });
  assertTrue(expected.what()->contains(L"is greater than max value"));
}
} // namespace org::apache::lucene::document