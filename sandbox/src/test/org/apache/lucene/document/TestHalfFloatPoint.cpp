using namespace std;

#include "TestHalfFloatPoint.h"

namespace org::apache::lucene::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestHalfFloatPoint::testHalfFloat(const wstring &sbits, float value)
{
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: short bits = (short) Integer.parseInt(sbits, 2);
  short bits = static_cast<short>(Integer::valueOf(sbits, 2));
  float converted = HalfFloatPoint::shortBitsToHalfFloat(bits);
  assertEquals(value, converted, 0.0f);
  short bits2 = HalfFloatPoint::halfFloatToShortBits(converted);
  TestUtil::assertEquals(bits, bits2);
}

void TestHalfFloatPoint::testHalfFloatConversion()
{
  TestUtil::assertEquals(0, HalfFloatPoint::halfFloatToShortBits(0.0f));
  TestUtil::assertEquals(static_cast<short>(1 << 15),
                         HalfFloatPoint::halfFloatToShortBits(-0.0f));
  TestUtil::assertEquals(0, HalfFloatPoint::halfFloatToShortBits(
                                Float::MIN_VALUE)); // rounded to zero

  testHalfFloat(L"0011110000000000", 1);
  testHalfFloat(L"0011110000000001", 1.0009765625f);
  testHalfFloat(L"1100000000000000", -2);
  testHalfFloat(L"0111101111111111", 65504); // max value
  testHalfFloat(L"0000010000000000",
                static_cast<float>(pow(2, -14))); // minimum positive normal
  testHalfFloat(
      L"0000001111111111",
      static_cast<float>(pow(2, -14) - pow(2, -24))); // maximum subnormal
  testHalfFloat(L"0000000000000001",
                static_cast<float>(pow(2, -24))); // minimum positive subnormal
  testHalfFloat(L"0000000000000000", 0.0f);
  testHalfFloat(L"1000000000000000", -0.0f);
  testHalfFloat(L"0111110000000000", numeric_limits<float>::infinity());
  testHalfFloat(L"1111110000000000", -numeric_limits<float>::infinity());
  testHalfFloat(L"0111111000000000", NAN);
  testHalfFloat(L"0011010101010101", 0.333251953125f);
}

void TestHalfFloatPoint::testRoundShift()
{
  TestUtil::assertEquals(0, HalfFloatPoint::roundShift(0, 2));
  TestUtil::assertEquals(0, HalfFloatPoint::roundShift(1, 2));
  TestUtil::assertEquals(0,
                         HalfFloatPoint::roundShift(
                             2, 2)); // tie so round to 0 since it ends with a 0
  TestUtil::assertEquals(1, HalfFloatPoint::roundShift(3, 2));
  TestUtil::assertEquals(1, HalfFloatPoint::roundShift(4, 2));
  TestUtil::assertEquals(1, HalfFloatPoint::roundShift(5, 2));
  TestUtil::assertEquals(2,
                         HalfFloatPoint::roundShift(
                             6, 2)); // tie so round to 2 since it ends with a 0
  TestUtil::assertEquals(2, HalfFloatPoint::roundShift(7, 2));
  TestUtil::assertEquals(2, HalfFloatPoint::roundShift(8, 2));
  TestUtil::assertEquals(2, HalfFloatPoint::roundShift(9, 2));
  TestUtil::assertEquals(
      2, HalfFloatPoint::roundShift(
             10, 2)); // tie so round to 2 since it ends with a 0
  TestUtil::assertEquals(3, HalfFloatPoint::roundShift(11, 2));
  TestUtil::assertEquals(3, HalfFloatPoint::roundShift(12, 2));
  TestUtil::assertEquals(3, HalfFloatPoint::roundShift(13, 2));
  TestUtil::assertEquals(
      4, HalfFloatPoint::roundShift(
             14, 2)); // tie so round to 4 since it ends with a 0
  TestUtil::assertEquals(4, HalfFloatPoint::roundShift(15, 2));
  TestUtil::assertEquals(4, HalfFloatPoint::roundShift(16, 2));
}

void TestHalfFloatPoint::testRounding()
{
  std::deque<float> values(0);
  int o = 0;
  for (int i = numeric_limits<short>::min(); i <= numeric_limits<short>::max();
       ++i) {
    float v = HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(i));
    if (Float::isFinite(v)) {
      if (o == values.size()) {
        values = ArrayUtil::grow(values);
      }
      values[o++] = v;
    }
  }
  values = Arrays::copyOf(values, o);

  int iters = atLeast(1000000);
  for (int iter = 0; iter < iters; ++iter) {
    float f;
    if (random()->nextBoolean()) {
      int floatBits = random()->nextInt();
      f = Float::intBitsToFloat(floatBits);
    } else {
      f = static_cast<float>((2 * random()->nextFloat() - 1) *
                             pow(2, TestUtil::nextInt(random(), -16, 16)));
    }
    float rounded = HalfFloatPoint::shortBitsToHalfFloat(
        HalfFloatPoint::halfFloatToShortBits(f));
    if (Float::isFinite(f) == false) {
      assertEquals(Float::floatToIntBits(f), Float::floatToIntBits(rounded),
                   0.0f);
    } else if (Float::isFinite(rounded) == false) {
      assertFalse(isnan(rounded));
      assertTrue(abs(f) >= 65520);
    } else {
      int index = Arrays::binarySearch(values, f);
      float closest;
      if (index >= 0) {
        closest = values[index];
      } else {
        index = -1 - index;
        closest = numeric_limits<float>::infinity();
        if (index < values.size()) {
          closest = values[index];
        }
        if (index - 1 >= 0) {
          if (f - values[index - 1] < closest - f) {
            closest = values[index - 1];
          } else if (f - values[index - 1] == closest - f &&
                     Integer::numberOfTrailingZeros(
                         Float::floatToIntBits(values[index - 1])) >
                         Integer::numberOfTrailingZeros(
                             Float::floatToIntBits(closest))) {
            // in case of tie, round to even
            closest = values[index - 1];
          }
        }
      }
      assertEquals(closest, rounded, 0.0f);
    }
  }
}

void TestHalfFloatPoint::testSortableBits()
{
  int low = numeric_limits<short>::min();
  int high = numeric_limits<short>::max();
  while (isnan(
      HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(low)))) {
    ++low;
  }
  while (HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(low)) ==
         -numeric_limits<float>::infinity()) {
    ++low;
  }
  while (isnan(
      HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(high)))) {
    --high;
  }
  while (HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(high)) ==
         numeric_limits<float>::infinity()) {
    --high;
  }
  for (int i = low; i <= high + 1; ++i) {
    float previous =
        HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(i - 1));
    float current =
        HalfFloatPoint::sortableShortToHalfFloat(static_cast<short>(i));
    TestUtil::assertEquals(i,
                           HalfFloatPoint::halfFloatToSortableShort(current));
    assertTrue(Float::compare(previous, current) < 0);
  }
}

void TestHalfFloatPoint::testSortableBytes()
{
  for (int i = numeric_limits<short>::min() + 1;
       i <= numeric_limits<short>::max(); ++i) {
    std::deque<char> previous(HalfFloatPoint::BYTES);
    HalfFloatPoint::shortToSortableBytes(static_cast<short>(i - 1), previous,
                                         0);
    std::deque<char> current(HalfFloatPoint::BYTES);
    HalfFloatPoint::shortToSortableBytes(static_cast<short>(i), current, 0);
    assertTrue(StringHelper::compare(HalfFloatPoint::BYTES, previous, 0,
                                     current, 0) < 0);
    TestUtil::assertEquals(i, HalfFloatPoint::sortableBytesToShort(current, 0));
  }
}

void TestHalfFloatPoint::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with an single dimension
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<HalfFloatPoint>(L"field", 1.25f));
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(
      1, searcher->count(HalfFloatPoint::newExactQuery(L"field", 1.25f)));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newExactQuery(L"field", 1.0f)));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newExactQuery(L"field", 2.0f)));
  TestUtil::assertEquals(
      1, searcher->count(HalfFloatPoint::newRangeQuery(L"field", 1.0f, 2.0f)));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newRangeQuery(L"field", 0.0f, 1.0f)));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newRangeQuery(L"field", 1.5f, 2.0f)));
  TestUtil::assertEquals(
      1, searcher->count(HalfFloatPoint::newSetQuery(L"field", {1.25f})));
  TestUtil::assertEquals(
      1, searcher->count(HalfFloatPoint::newSetQuery(L"field", {1.0f, 1.25f})));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newSetQuery(L"field", {1.0f})));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newSetQuery(L"field")));

  delete reader;
  delete writer;
  delete dir;
}

void TestHalfFloatPoint::testBasicsMultiDims() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with two dimensions
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<HalfFloatPoint>(L"field", 1.25f, -2.0f));
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(1, searcher->count(HalfFloatPoint::newRangeQuery(
                                L"field", std::deque<float>{0, -5},
                                std::deque<float>{1.25f, -1})));
  TestUtil::assertEquals(
      0, searcher->count(HalfFloatPoint::newRangeQuery(
             L"field", std::deque<float>{0, 0}, std::deque<float>{2, 2})));
  TestUtil::assertEquals(0, searcher->count(HalfFloatPoint::newRangeQuery(
                                L"field", std::deque<float>{-10, -10},
                                std::deque<float>{1, 2})));

  delete reader;
  delete writer;
  delete dir;
}

void TestHalfFloatPoint::testNextUp()
{
  assertEquals(NAN, HalfFloatPoint::nextUp(NAN), 0.0f);
  assertEquals(numeric_limits<float>::infinity(),
               HalfFloatPoint::nextUp(numeric_limits<float>::infinity()), 0.0f);
  assertEquals(
      -65504, HalfFloatPoint::nextUp(-numeric_limits<float>::infinity()), 0.0f);
  assertEquals(HalfFloatPoint::shortBitsToHalfFloat(static_cast<short>(0)),
               HalfFloatPoint::nextUp(-0.0f), 0.0f);
  assertEquals(HalfFloatPoint::shortBitsToHalfFloat(static_cast<short>(1)),
               HalfFloatPoint::nextUp(0.0f), 0.0f);
  // values that cannot be exactly represented as a half float
  assertEquals(HalfFloatPoint::nextUp(0.0f),
               HalfFloatPoint::nextUp(Float::MIN_VALUE), 0.0f);
  TestUtil::assertEquals(
      Float::floatToIntBits(-0.0f),
      Float::floatToIntBits(HalfFloatPoint::nextUp(-Float::MIN_VALUE)));
  TestUtil::assertEquals(Float::floatToIntBits(0.0f),
                         Float::floatToIntBits(HalfFloatPoint::nextUp(-0.0f)));
}

void TestHalfFloatPoint::testNextDown()
{
  assertEquals(NAN, HalfFloatPoint::nextDown(NAN), 0.0f);
  assertEquals(-numeric_limits<float>::infinity(),
               HalfFloatPoint::nextDown(-numeric_limits<float>::infinity()),
               0.0f);
  assertEquals(
      65504, HalfFloatPoint::nextDown(numeric_limits<float>::infinity()), 0.0f);
  TestUtil::assertEquals(Float::floatToIntBits(-0.0f),
                         Float::floatToIntBits(HalfFloatPoint::nextDown(0.0f)));
  // values that cannot be exactly represented as a half float
  TestUtil::assertEquals(
      Float::floatToIntBits(0.0f),
      Float::floatToIntBits(HalfFloatPoint::nextDown(Float::MIN_VALUE)));
  assertEquals(HalfFloatPoint::nextDown(-0.0f),
               HalfFloatPoint::nextDown(-Float::MIN_VALUE), 0.0f);
  TestUtil::assertEquals(
      Float::floatToIntBits(-0.0f),
      Float::floatToIntBits(HalfFloatPoint::nextDown(+0.0f)));
}
} // namespace org::apache::lucene::document