using namespace std;

#include "TestSmallFloat.h"

namespace org::apache::lucene::util
{

float TestSmallFloat::orig_byteToFloat(char b)
{
  if (b == 0) // zero is a special case
  {
    return 0.0f;
  }
  int mantissa = b & 7;
  int exponent = (b >> 3) & 31;
  int bits = ((exponent + (63 - 15)) << 24) | (mantissa << 21);
  return Float::intBitsToFloat(bits);
}

char TestSmallFloat::orig_floatToByte_v13(float f)
{
  if (f < 0.0f) // round negatives up to zero
  {
    f = 0.0f;
  }

  if (f == 0.0f) // zero is a special case
  {
    return 0;
  }

  int bits = Float::floatToIntBits(f); // parse float into parts
  int mantissa = (bits & 0xffffff) >> 21;
  int exponent = (((bits >> 24) & 0x7f) - 63) + 15;

  if (exponent > 31) { // overflow: use max value
    exponent = 31;
    mantissa = 7;
  }

  if (exponent < 0) { // underflow: use min value
    exponent = 0;
    mantissa = 1;
  }

  return static_cast<char>((exponent << 3) | mantissa); // pack into a byte
}

char TestSmallFloat::orig_floatToByte(float f)
{
  if (f < 0.0f) // round negatives up to zero
  {
    f = 0.0f;
  }

  if (f == 0.0f) // zero is a special case
  {
    return 0;
  }

  int bits = Float::floatToIntBits(f); // parse float into parts
  int mantissa = (bits & 0xffffff) >> 21;
  int exponent = (((bits >> 24) & 0x7f) - 63) + 15;

  if (exponent > 31) { // overflow: use max value
    exponent = 31;
    mantissa = 7;
  }

  if (exponent < 0 ||
      exponent == 0 && mantissa == 0) { // underflow: use min value
    exponent = 0;
    mantissa = 1;
  }

  return static_cast<char>((exponent << 3) | mantissa); // pack into a byte
}

void TestSmallFloat::testByteToFloat()
{
  for (int i = 0; i < 256; i++) {
    float f1 = orig_byteToFloat(static_cast<char>(i));
    float f2 = SmallFloat::byteToFloat(static_cast<char>(i), 3, 15);
    float f3 = SmallFloat::byte315ToFloat(static_cast<char>(i));
    assertEquals(f1, f2, 0.0);
    assertEquals(f2, f3, 0.0);
  }
}

void TestSmallFloat::testFloatToByte()
{
  assertEquals(0, orig_floatToByte_v13(
                      5.8123817E-10f)); // verify the old bug (see LUCENE-2937)
  assertEquals(1, orig_floatToByte(
                      5.8123817E-10f)); // verify it's fixed in this test code
  assertEquals(1,
               SmallFloat::floatToByte315(5.8123817E-10f)); // verify it's fixed

  // test some constants
  assertEquals(0, SmallFloat::floatToByte315(0));
  assertEquals(
      1, SmallFloat::floatToByte315(
             Float::MIN_VALUE)); // underflow rounds up to smallest positive
  assertEquals(255, SmallFloat::floatToByte315(numeric_limits<float>::max()) &
                        0xff); // overflow rounds down to largest positive
  assertEquals(255,
               SmallFloat::floatToByte315(numeric_limits<float>::infinity()) &
                   0xff);

  // all negatives map_obj to 0
  assertEquals(0, SmallFloat::floatToByte315(-Float::MIN_VALUE));
  assertEquals(0, SmallFloat::floatToByte315(-numeric_limits<float>::max()));
  assertEquals(0,
               SmallFloat::floatToByte315(-numeric_limits<float>::infinity()));

  // up iterations for more exhaustive test after changing something
  int num = atLeast(100000);
  for (int i = 0; i < num; i++) {
    float f = Float::intBitsToFloat(random()->nextInt());
    if (isnan(f)) {
      continue; // skip NaN
    }
    char b1 = orig_floatToByte(f);
    char b2 = SmallFloat::floatToByte(f, 3, 15);
    char b3 = SmallFloat::floatToByte315(f);
    assertEquals(b1, b2);
    assertEquals(b2, b3);
  }
}

void TestSmallFloat::testInt4()
{
  for (int i = 0; i <= 16; ++i) {
    // all values in 0-16 are encoded accurately
    assertEquals(i, SmallFloat::int4ToLong(SmallFloat::longToInt4(i)));
  }
  constexpr int maxEncoded =
      SmallFloat::longToInt4(numeric_limits<int64_t>::max());
  for (int i = 1; i < maxEncoded; ++i) {
    assertTrue(SmallFloat::int4ToLong(i) > SmallFloat::int4ToLong(i - 1));
  }
  constexpr int iters = atLeast(1000);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int64_t l = TestUtil::nextLong(
        random(), 0, 1LL << TestUtil::nextInt(random(), 5, 61));
    int numBits = 64 - Long::numberOfLeadingZeros(l);
    int64_t expected = l;
    if (numBits > 4) {
      int64_t mask = ~0LL << (numBits - 4);
      expected &= mask;
    }
    int64_t l2 = SmallFloat::int4ToLong(SmallFloat::longToInt4(l));
    assertEquals(expected, l2);
  }
}

void TestSmallFloat::testByte4()
{
  std::deque<int> decoded(256);
  for (int b = 0; b < 256; ++b) {
    decoded[b] = SmallFloat::byte4ToInt(static_cast<char>(b));
    assertEquals(static_cast<char>(b), SmallFloat::intToByte4(decoded[b]));
  }
  for (int i = 1; i < 256; ++i) {
    assertTrue(decoded[i] > decoded[i - 1]);
  }
  assertEquals(static_cast<char>(255),
               SmallFloat::intToByte4(numeric_limits<int>::max()));
  constexpr int iters = atLeast(1000);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int i =
        random()->nextInt(1 << TestUtil::nextInt(random(), 5, 30));
    int idx = Arrays::binarySearch(decoded, i);
    if (idx < 0) {
      idx = -2 - idx;
    }
    assertTrue(decoded[idx] <= i);
    assertEquals(static_cast<char>(idx), SmallFloat::intToByte4(i));
  }
}
} // namespace org::apache::lucene::util