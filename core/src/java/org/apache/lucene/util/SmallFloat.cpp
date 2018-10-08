using namespace std;

#include "SmallFloat.h"

namespace org::apache::lucene::util
{

SmallFloat::SmallFloat() {}

char SmallFloat::floatToByte(float f, int numMantissaBits, int zeroExp)
{
  // Adjustment from a float zero exponent to our zero exponent,
  // shifted over to our exponent position.
  int fzero = (63 - zeroExp) << numMantissaBits;
  int bits = Float::floatToRawIntBits(f);
  int smallfloat = bits >> (24 - numMantissaBits);
  if (smallfloat <= fzero) {
    return (bits <= 0)
               ? static_cast<char>(0)
               : static_cast<char>(
                     1); // underflow is mapped to smallest non-zero number.
  } else if (smallfloat >= fzero + 0x100) {
    return -1; // overflow maps to largest number
  } else {
    return static_cast<char>(smallfloat - fzero);
  }
}

float SmallFloat::byteToFloat(char b, int numMantissaBits, int zeroExp)
{
  // on Java1.5 & 1.6 JVMs, prebuilding a decoding array and doing a lookup
  // is only a little bit faster (anywhere from 0% to 7%)
  if (b == 0) {
    return 0.0f;
  }
  int bits = (b & 0xff) << (24 - numMantissaBits);
  bits += (63 - zeroExp) << 24;
  return Float::intBitsToFloat(bits);
}

char SmallFloat::floatToByte315(float f)
{
  int bits = Float::floatToRawIntBits(f);
  int smallfloat = bits >> (24 - 3);
  if (smallfloat <= ((63 - 15) << 3)) {
    return (bits <= 0) ? static_cast<char>(0) : static_cast<char>(1);
  }
  if (smallfloat >= ((63 - 15) << 3) + 0x100) {
    return -1;
  }
  return static_cast<char>(smallfloat - ((63 - 15) << 3));
}

float SmallFloat::byte315ToFloat(char b)
{
  // on Java1.5 & 1.6 JVMs, prebuilding a decoding array and doing a lookup
  // is only a little bit faster (anywhere from 0% to 7%)
  if (b == 0) {
    return 0.0f;
  }
  int bits = (b & 0xff) << (24 - 3);
  bits += (63 - 15) << 24;
  return Float::intBitsToFloat(bits);
}

int SmallFloat::longToInt4(int64_t i)
{
  if (i < 0) {
    throw invalid_argument(L"Only supports positive values, got " +
                           to_wstring(i));
  }
  int numBits = 64 - Long::numberOfLeadingZeros(i);
  if (numBits < 4) {
    // subnormal value
    return Math::toIntExact(i);
  } else {
    // normal value
    int shift = numBits - 4;
    // only keep the 5 most significant bits
    int encoded = Math::toIntExact(
        static_cast<int64_t>(static_cast<uint64_t>(i) >> shift));
    // clear the most significant bit, which is implicit
    encoded &= 0x07;
    // encode the shift, adding 1 because 0 is reserved for subnormal values
    encoded |= (shift + 1) << 3;
    return encoded;
  }
}

int64_t SmallFloat::int4ToLong(int i)
{
  int64_t bits = i & 0x07;
  int shift = (static_cast<int>(static_cast<unsigned int>(i) >> 3)) - 1;
  int64_t decoded;
  if (shift == -1) {
    // subnormal value
    decoded = bits;
  } else {
    // normal value
    decoded = (bits | 0x08) << shift;
  }
  return decoded;
}

char SmallFloat::intToByte4(int i)
{
  if (i < 0) {
    throw invalid_argument(L"Only supports positive values, got " +
                           to_wstring(i));
  }
  if (i < NUM_FREE_VALUES) {
    return static_cast<char>(i);
  } else {
    return static_cast<char>(NUM_FREE_VALUES + longToInt4(i - NUM_FREE_VALUES));
  }
}

int SmallFloat::byte4ToInt(char b)
{
  int i = Byte::toUnsignedInt(b);
  if (i < NUM_FREE_VALUES) {
    return i;
  } else {
    int64_t decoded = NUM_FREE_VALUES + int4ToLong(i - NUM_FREE_VALUES);
    return Math::toIntExact(decoded);
  }
}
} // namespace org::apache::lucene::util