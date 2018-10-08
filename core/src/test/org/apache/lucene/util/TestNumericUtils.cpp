using namespace std;

#include "TestNumericUtils.h"

namespace org::apache::lucene::util
{

void TestNumericUtils::testLongConversionAndOrdering() 
{
  shared_ptr<BytesRef> previous = nullptr;
  shared_ptr<BytesRef> current =
      make_shared<BytesRef>(std::deque<char>(Long::BYTES));
  for (int64_t value = -100000LL; value < 100000LL; value++) {
    NumericUtils::longToSortableBytes(value, current->bytes, current->offset);
    if (previous == nullptr) {
      previous = make_shared<BytesRef>(std::deque<char>(Long::BYTES));
    } else {
      // test if smaller
      assertTrue(L"current bigger than previous: ",
                 previous->compareTo(current) < 0);
    }
    // test is back and forward conversion works
    assertEquals(
        L"forward and back conversion should generate same long", value,
        NumericUtils::sortableBytesToLong(current->bytes, current->offset));
    // next step
    System::arraycopy(current->bytes, current->offset, previous->bytes,
                      previous->offset, current->length);
  }
}

void TestNumericUtils::testIntConversionAndOrdering() 
{
  shared_ptr<BytesRef> previous = nullptr;
  shared_ptr<BytesRef> current =
      make_shared<BytesRef>(std::deque<char>(Integer::BYTES));
  for (int value = -100000; value < 100000; value++) {
    NumericUtils::intToSortableBytes(value, current->bytes, current->offset);
    if (previous == nullptr) {
      previous = make_shared<BytesRef>(std::deque<char>(Integer::BYTES));
    } else {
      // test if smaller
      assertTrue(L"current bigger than previous: ",
                 previous->compareTo(current) < 0);
    }
    // test is back and forward conversion works
    assertEquals(
        L"forward and back conversion should generate same int", value,
        NumericUtils::sortableBytesToInt(current->bytes, current->offset));
    // next step
    System::arraycopy(current->bytes, current->offset, previous->bytes,
                      previous->offset, current->length);
  }
}

void TestNumericUtils::testBigIntConversionAndOrdering() 
{
  // we need at least 3 bytes of storage.
  int size = TestUtil::nextInt(random(), 3, 16);
  shared_ptr<BytesRef> previous = nullptr;
  shared_ptr<BytesRef> current = make_shared<BytesRef>(std::deque<char>(size));
  for (int64_t value = -100000LL; value < 100000LL; value++) {
    NumericUtils::bigIntToSortableBytes(static_cast<int64_t>(value), size,
                                        current->bytes, current->offset);
    if (previous == nullptr) {
      previous = make_shared<BytesRef>(std::deque<char>(size));
    } else {
      // test if smaller
      assertTrue(L"current bigger than previous: ",
                 previous->compareTo(current) < 0);
    }
    // test is back and forward conversion works
    assertEquals(L"forward and back conversion should generate same int64_t",
                 static_cast<int64_t>(value),
                 NumericUtils::sortableBytesToBigInt(
                     current->bytes, current->offset, current->length));
    // next step
    System::arraycopy(current->bytes, current->offset, previous->bytes,
                      previous->offset, current->length);
  }
}

void TestNumericUtils::testLongSpecialValues() 
{
  std::deque<int64_t> values = {numeric_limits<int64_t>::min(),
                                   numeric_limits<int64_t>::min() + 1,
                                   numeric_limits<int64_t>::min() + 2,
                                   -5003400000000LL,
                                   -4000LL,
                                   -3000LL,
                                   -2000LL,
                                   -1000LL,
                                   -1LL,
                                   0LL,
                                   1LL,
                                   10LL,
                                   300LL,
                                   50006789999999999LL,
                                   numeric_limits<int64_t>::max() - 2,
                                   numeric_limits<int64_t>::max() - 1,
                                   numeric_limits<int64_t>::max()};
  std::deque<std::shared_ptr<BytesRef>> encoded(values.size());

  for (int i = 0; i < values.size(); i++) {
    encoded[i] = make_shared<BytesRef>(std::deque<char>(Long::BYTES));
    NumericUtils::longToSortableBytes(values[i], encoded[i]->bytes,
                                      encoded[i]->offset);

    // check forward and back conversion
    assertEquals(L"forward and back conversion should generate same long",
                 values[i],
                 NumericUtils::sortableBytesToLong(encoded[i]->bytes,
                                                   encoded[i]->offset));
  }

  // check sort order (encoded values should be ascending)
  for (int i = 1; i < encoded.size(); i++) {
    assertTrue(L"check sort order", encoded[i - 1]->compareTo(encoded[i]) < 0);
  }
}

void TestNumericUtils::testIntSpecialValues() 
{
  std::deque<int> values = {numeric_limits<int>::min(),
                             numeric_limits<int>::min() + 1,
                             numeric_limits<int>::min() + 2,
                             -64765767,
                             -4000,
                             -3000,
                             -2000,
                             -1000,
                             -1,
                             0,
                             1,
                             10,
                             300,
                             765878989,
                             numeric_limits<int>::max() - 2,
                             numeric_limits<int>::max() - 1,
                             numeric_limits<int>::max()};
  std::deque<std::shared_ptr<BytesRef>> encoded(values.size());

  for (int i = 0; i < values.size(); i++) {
    encoded[i] = make_shared<BytesRef>(std::deque<char>(Integer::BYTES));
    NumericUtils::intToSortableBytes(values[i], encoded[i]->bytes,
                                     encoded[i]->offset);

    // check forward and back conversion
    assertEquals(L"forward and back conversion should generate same int",
                 values[i],
                 NumericUtils::sortableBytesToInt(encoded[i]->bytes,
                                                  encoded[i]->offset));
  }

  // check sort order (encoded values should be ascending)
  for (int i = 1; i < encoded.size(); i++) {
    assertTrue(L"check sort order", encoded[i - 1]->compareTo(encoded[i]) < 0);
  }
}

void TestNumericUtils::testBigIntSpecialValues() 
{
  std::deque<std::shared_ptr<int64_t>> values = {
      static_cast<int64_t>(numeric_limits<int>::min()),
      static_cast<int64_t>(numeric_limits<int>::min() + 1),
      static_cast<int64_t>(numeric_limits<int>::min() + 2),
      static_cast<int64_t>(-64765767),
      static_cast<int64_t>(-4000),
      static_cast<int64_t>(-3000),
      static_cast<int64_t>(-2000),
      static_cast<int64_t>(-1000),
      static_cast<int64_t>(-1),
      static_cast<int64_t>(0),
      static_cast<int64_t>(1),
      static_cast<int64_t>(10),
      static_cast<int64_t>(300),
      static_cast<int64_t>(765878989),
      static_cast<int64_t>(numeric_limits<int>::max() - 2),
      static_cast<int64_t>(numeric_limits<int>::max() - 1),
      static_cast<int64_t>(numeric_limits<int>::max())};
  std::deque<std::shared_ptr<BytesRef>> encoded(values.size());

  for (int i = 0; i < values.size(); i++) {
    encoded[i] = make_shared<BytesRef>(std::deque<char>(Integer::BYTES));
    NumericUtils::bigIntToSortableBytes(values[i], Integer::BYTES,
                                        encoded[i]->bytes, encoded[i]->offset);

    // check forward and back conversion
    assertEquals(
        L"forward and back conversion should generate same big integer",
        values[i],
        NumericUtils::sortableBytesToBigInt(
            encoded[i]->bytes, encoded[i]->offset, Integer::BYTES));
  }

  // check sort order (encoded values should be ascending)
  for (int i = 1; i < encoded.size(); i++) {
    assertTrue(L"check sort order", encoded[i - 1]->compareTo(encoded[i]) < 0);
  }
}

void TestNumericUtils::testDoubles() 
{
  std::deque<double> values = {-numeric_limits<double>::infinity(),
                                -2.3E25,
                                -1.0E15,
                                -1.0,
                                -1.0E-1,
                                -1.0E-2,
                                -0.0,
                                +0.0,
                                1.0E-2,
                                1.0E-1,
                                1.0,
                                1.0E15,
                                2.3E25,
                                numeric_limits<double>::infinity(),
                                NAN};
  std::deque<int64_t> encoded(values.size());

  // check forward and back conversion
  for (int i = 0; i < values.size(); i++) {
    encoded[i] = NumericUtils::doubleToSortableLong(values[i]);
    assertTrue(L"forward and back conversion should generate same double",
               Double::compare(values[i], NumericUtils::sortableLongToDouble(
                                              encoded[i])) == 0);
  }

  // check sort order (encoded values should be ascending)
  for (int i = 1; i < encoded.size(); i++) {
    assertTrue(L"check sort order", encoded[i - 1] < encoded[i]);
  }
}

std::deque<double> const TestNumericUtils::DOUBLE_NANs = {
    NAN, Double::longBitsToDouble(0x7ff0000000000001LL),
    Double::longBitsToDouble(0x7fffffffffffffffLL),
    Double::longBitsToDouble(0xfff0000000000001LL),
    Double::longBitsToDouble(0xffffffffffffffffLL)};

void TestNumericUtils::testSortableDoubleNaN()
{
  constexpr int64_t plusInf =
      NumericUtils::doubleToSortableLong(numeric_limits<double>::infinity());
  for (auto nan : DOUBLE_NANs) {
    assertTrue(isnan(nan));
    constexpr int64_t sortable = NumericUtils::doubleToSortableLong(nan);
    assertTrue(L"Double not sorted correctly: " + to_wstring(nan) +
                   L", long repr: " + to_wstring(sortable) +
                   L", positive inf.: " + to_wstring(plusInf),
               sortable > plusInf);
  }
}

void TestNumericUtils::testFloats() 
{
  std::deque<float> values = {-numeric_limits<float>::infinity(),
                               -2.3E25f,
                               -1.0E15f,
                               -1.0f,
                               -1.0E-1f,
                               -1.0E-2f,
                               -0.0f,
                               +0.0f,
                               1.0E-2f,
                               1.0E-1f,
                               1.0f,
                               1.0E15f,
                               2.3E25f,
                               numeric_limits<float>::infinity(),
                               NAN};
  std::deque<int> encoded(values.size());

  // check forward and back conversion
  for (int i = 0; i < values.size(); i++) {
    encoded[i] = NumericUtils::floatToSortableInt(values[i]);
    assertTrue(L"forward and back conversion should generate same float",
               Float::compare(values[i], NumericUtils::sortableIntToFloat(
                                             encoded[i])) == 0);
  }

  // check sort order (encoded values should be ascending)
  for (int i = 1; i < encoded.size(); i++) {
    assertTrue(L"check sort order", encoded[i - 1] < encoded[i]);
  }
}

std::deque<float> const TestNumericUtils::FLOAT_NANs = {
    NAN, Float::intBitsToFloat(0x7f800001), Float::intBitsToFloat(0x7fffffff),
    Float::intBitsToFloat(0xff800001), Float::intBitsToFloat(0xffffffff)};

void TestNumericUtils::testSortableFloatNaN()
{
  constexpr int plusInf =
      NumericUtils::floatToSortableInt(numeric_limits<float>::infinity());
  for (auto nan : FLOAT_NANs) {
    assertTrue(isnan(nan));
    constexpr int sortable = NumericUtils::floatToSortableInt(nan);
    assertTrue(L"Float not sorted correctly: " + to_wstring(nan) +
                   L", int repr: " + to_wstring(sortable) +
                   L", positive inf.: " + to_wstring(plusInf),
               sortable > plusInf);
  }
}

void TestNumericUtils::testAdd() 
{
  int iters = atLeast(10000);
  int numBytes = TestUtil::nextInt(random(), 1, 100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<int64_t> v1 =
        make_shared<int64_t>(8 * numBytes - 1, random());
    shared_ptr<int64_t> v2 =
        make_shared<int64_t>(8 * numBytes - 1, random());

    std::deque<char> v1Bytes(numBytes);
    std::deque<char> v1RawBytes = v1->toByteArray();
    assert(v1RawBytes.size() <= numBytes);
    System::arraycopy(v1RawBytes, 0, v1Bytes,
                      v1Bytes.size() - v1RawBytes.size(), v1RawBytes.size());

    std::deque<char> v2Bytes(numBytes);
    std::deque<char> v2RawBytes = v2->toByteArray();
    assert(v1RawBytes.size() <= numBytes);
    System::arraycopy(v2RawBytes, 0, v2Bytes,
                      v2Bytes.size() - v2RawBytes.size(), v2RawBytes.size());

    std::deque<char> result(numBytes);
    NumericUtils::add(numBytes, 0, v1Bytes, v2Bytes, result);

    shared_ptr<int64_t> sum = v1->add(v2);
    assertTrue(L"sum=" + sum + L" v1=" + v1 + L" v2=" + v2 + L" but result=" +
                   make_shared<int64_t>(1, result),
               sum->equals(make_shared<int64_t>(1, result)));
  }
}

void TestNumericUtils::testIllegalAdd() 
{
  std::deque<char> bytes(4);
  Arrays::fill(bytes, static_cast<char>(0xff));
  std::deque<char> one(4);
  one[3] = 1;
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    NumericUtils::add(4, 0, bytes, one, std::deque<char>(4));
  });
  assertEquals(L"a + b overflows bytesPerDim=4", expected.what());
}

void TestNumericUtils::testSubtract() 
{
  int iters = atLeast(10000);
  int numBytes = TestUtil::nextInt(random(), 1, 100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<int64_t> v1 =
        make_shared<int64_t>(8 * numBytes - 1, random());
    shared_ptr<int64_t> v2 =
        make_shared<int64_t>(8 * numBytes - 1, random());
    if (v1->compareTo(v2) < 0) {
      shared_ptr<int64_t> tmp = v1;
      v1 = v2;
      v2 = tmp;
    }

    std::deque<char> v1Bytes(numBytes);
    std::deque<char> v1RawBytes = v1->toByteArray();
    assert((v1RawBytes.size() <= numBytes, L"length=" + v1RawBytes.size() +
                                               L" vs numBytes=" +
                                               to_wstring(numBytes)));
    System::arraycopy(v1RawBytes, 0, v1Bytes,
                      v1Bytes.size() - v1RawBytes.size(), v1RawBytes.size());

    std::deque<char> v2Bytes(numBytes);
    std::deque<char> v2RawBytes = v2->toByteArray();
    assert(v2RawBytes.size() <= numBytes);
    assert((v2RawBytes.size() <= numBytes, L"length=" + v2RawBytes.size() +
                                               L" vs numBytes=" +
                                               to_wstring(numBytes)));
    System::arraycopy(v2RawBytes, 0, v2Bytes,
                      v2Bytes.size() - v2RawBytes.size(), v2RawBytes.size());

    std::deque<char> result(numBytes);
    NumericUtils::subtract(numBytes, 0, v1Bytes, v2Bytes, result);

    shared_ptr<int64_t> diff = v1->subtract(v2);

    assertTrue(L"diff=" + diff + L" vs result=" +
                   make_shared<int64_t>(result) + L" v1=" + v1 + L" v2=" +
                   v2,
               diff->equals(make_shared<int64_t>(result)));
  }
}

void TestNumericUtils::testIllegalSubtract() 
{
  std::deque<char> v1(4);
  v1[3] = static_cast<char>(0xf0);
  std::deque<char> v2(4);
  v2[3] = static_cast<char>(0xf1);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    NumericUtils::subtract(4, 0, v1, v2, std::deque<char>(4));
  });
  assertEquals(L"a < b", expected.what());
}

void TestNumericUtils::testIntsRoundTrip()
{
  std::deque<char> encoded(Integer::BYTES);

  for (int i = 0; i < 10000; i++) {
    int value = random()->nextInt();
    NumericUtils::intToSortableBytes(value, encoded, 0);
    assertEquals(value, NumericUtils::sortableBytesToInt(encoded, 0));
  }
}

void TestNumericUtils::testLongsRoundTrip()
{
  std::deque<char> encoded(Long::BYTES);

  for (int i = 0; i < 10000; i++) {
    int64_t value =
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max());
    NumericUtils::longToSortableBytes(value, encoded, 0);
    assertEquals(value, NumericUtils::sortableBytesToLong(encoded, 0));
  }
}

void TestNumericUtils::testFloatsRoundTrip()
{
  std::deque<char> encoded(Float::BYTES);

  for (int i = 0; i < 10000; i++) {
    float value = Float::intBitsToFloat(random()->nextInt());
    NumericUtils::intToSortableBytes(NumericUtils::floatToSortableInt(value),
                                     encoded, 0);
    float actual = NumericUtils::sortableIntToFloat(
        NumericUtils::sortableBytesToInt(encoded, 0));
    assertEquals(Float::floatToIntBits(value), Float::floatToIntBits(actual));
  }
}

void TestNumericUtils::testDoublesRoundTrip()
{
  std::deque<char> encoded(Double::BYTES);

  for (int i = 0; i < 10000; i++) {
    double value = Double::longBitsToDouble(
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max()));
    NumericUtils::longToSortableBytes(NumericUtils::doubleToSortableLong(value),
                                      encoded, 0);
    double actual = NumericUtils::sortableLongToDouble(
        NumericUtils::sortableBytesToLong(encoded, 0));
    assertEquals(Double::doubleToLongBits(value),
                 Double::doubleToLongBits(actual));
  }
}

void TestNumericUtils::testBigIntsRoundTrip()
{
  for (int i = 0; i < 10000; i++) {
    shared_ptr<int64_t> value = TestUtil::nextBigInteger(random(), 16);
    int length = value->toByteArray()->length;

    // make sure sign extension is tested: sometimes pad to more bytes when
    // encoding.
    int maxLength = TestUtil::nextInt(random(), length, length + 3);
    std::deque<char> encoded(maxLength);
    NumericUtils::bigIntToSortableBytes(value, maxLength, encoded, 0);
    assertEquals(value,
                 NumericUtils::sortableBytesToBigInt(encoded, 0, maxLength));
  }
}

void TestNumericUtils::testIntsCompare()
{
  shared_ptr<BytesRef> left =
      make_shared<BytesRef>(std::deque<char>(Integer::BYTES));
  shared_ptr<BytesRef> right =
      make_shared<BytesRef>(std::deque<char>(Integer::BYTES));

  for (int i = 0; i < 10000; i++) {
    int leftValue = random()->nextInt();
    NumericUtils::intToSortableBytes(leftValue, left->bytes, left->offset);

    int rightValue = random()->nextInt();
    NumericUtils::intToSortableBytes(rightValue, right->bytes, right->offset);

    assertEquals(Integer::signum(Integer::compare(leftValue, rightValue)),
                 Integer::signum(left->compareTo(right)));
  }
}

void TestNumericUtils::testLongsCompare()
{
  shared_ptr<BytesRef> left =
      make_shared<BytesRef>(std::deque<char>(Long::BYTES));
  shared_ptr<BytesRef> right =
      make_shared<BytesRef>(std::deque<char>(Long::BYTES));

  for (int i = 0; i < 10000; i++) {
    int64_t leftValue =
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max());
    NumericUtils::longToSortableBytes(leftValue, left->bytes, left->offset);

    int64_t rightValue =
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max());
    NumericUtils::longToSortableBytes(rightValue, right->bytes, right->offset);

    assertEquals(Integer::signum(Long::compare(leftValue, rightValue)),
                 Integer::signum(left->compareTo(right)));
  }
}

void TestNumericUtils::testFloatsCompare()
{
  shared_ptr<BytesRef> left =
      make_shared<BytesRef>(std::deque<char>(Float::BYTES));
  shared_ptr<BytesRef> right =
      make_shared<BytesRef>(std::deque<char>(Float::BYTES));

  for (int i = 0; i < 10000; i++) {
    float leftValue = Float::intBitsToFloat(random()->nextInt());
    NumericUtils::intToSortableBytes(
        NumericUtils::floatToSortableInt(leftValue), left->bytes, left->offset);

    float rightValue = Float::intBitsToFloat(random()->nextInt());
    NumericUtils::intToSortableBytes(
        NumericUtils::floatToSortableInt(rightValue), right->bytes,
        right->offset);

    assertEquals(Integer::signum(Float::compare(leftValue, rightValue)),
                 Integer::signum(left->compareTo(right)));
  }
}

void TestNumericUtils::testDoublesCompare()
{
  shared_ptr<BytesRef> left =
      make_shared<BytesRef>(std::deque<char>(Double::BYTES));
  shared_ptr<BytesRef> right =
      make_shared<BytesRef>(std::deque<char>(Double::BYTES));

  for (int i = 0; i < 10000; i++) {
    double leftValue = Double::longBitsToDouble(
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max()));
    NumericUtils::longToSortableBytes(
        NumericUtils::doubleToSortableLong(leftValue), left->bytes,
        left->offset);

    double rightValue = Double::longBitsToDouble(
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max()));
    NumericUtils::longToSortableBytes(
        NumericUtils::doubleToSortableLong(rightValue), right->bytes,
        right->offset);

    assertEquals(Integer::signum(Double::compare(leftValue, rightValue)),
                 Integer::signum(left->compareTo(right)));
  }
}

void TestNumericUtils::testBigIntsCompare()
{
  for (int i = 0; i < 10000; i++) {
    int maxLength = TestUtil::nextInt(random(), 1, 16);

    shared_ptr<int64_t> leftValue =
        TestUtil::nextBigInteger(random(), maxLength);
    shared_ptr<BytesRef> left =
        make_shared<BytesRef>(std::deque<char>(maxLength));
    NumericUtils::bigIntToSortableBytes(leftValue, maxLength, left->bytes,
                                        left->offset);

    shared_ptr<int64_t> rightValue =
        TestUtil::nextBigInteger(random(), maxLength);
    shared_ptr<BytesRef> right =
        make_shared<BytesRef>(std::deque<char>(maxLength));
    NumericUtils::bigIntToSortableBytes(rightValue, maxLength, right->bytes,
                                        right->offset);

    assertEquals(Integer::signum(leftValue->compareTo(rightValue)),
                 Integer::signum(left->compareTo(right)));
  }
}
} // namespace org::apache::lucene::util