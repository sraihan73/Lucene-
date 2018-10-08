using namespace std;

#include "NumericUtils.h"

namespace org::apache::lucene::util
{

NumericUtils::NumericUtils() {} // no instance!

int64_t NumericUtils::doubleToSortableLong(double value)
{
  return sortableDoubleBits(Double::doubleToLongBits(value));
}

double NumericUtils::sortableLongToDouble(int64_t encoded)
{
  return Double::longBitsToDouble(sortableDoubleBits(encoded));
}

int NumericUtils::floatToSortableInt(float value)
{
  return sortableFloatBits(Float::floatToIntBits(value));
}

float NumericUtils::sortableIntToFloat(int encoded)
{
  return Float::intBitsToFloat(sortableFloatBits(encoded));
}

int64_t NumericUtils::sortableDoubleBits(int64_t bits)
{
  return bits ^ (bits >> 63) & 0x7fffffffffffffffLL;
}

int NumericUtils::sortableFloatBits(int bits)
{
  return bits ^ (bits >> 31) & 0x7fffffff;
}

void NumericUtils::subtract(int bytesPerDim, int dim, std::deque<char> &a,
                            std::deque<char> &b, std::deque<char> &result)
{
  int start = dim * bytesPerDim;
  int end = start + bytesPerDim;
  int borrow = 0;
  for (int i = end - 1; i >= start; i--) {
    int diff = (a[i] & 0xff) - (b[i] & 0xff) - borrow;
    if (diff < 0) {
      diff += 256;
      borrow = 1;
    } else {
      borrow = 0;
    }
    result[i - start] = static_cast<char>(diff);
  }
  if (borrow != 0) {
    throw invalid_argument(L"a < b");
  }
}

void NumericUtils::add(int bytesPerDim, int dim, std::deque<char> &a,
                       std::deque<char> &b, std::deque<char> &result)
{
  int start = dim * bytesPerDim;
  int end = start + bytesPerDim;
  int carry = 0;
  for (int i = end - 1; i >= start; i--) {
    int digitSum = (a[i] & 0xff) + (b[i] & 0xff) + carry;
    if (digitSum > 255) {
      digitSum -= 256;
      carry = 1;
    } else {
      carry = 0;
    }
    result[i - start] = static_cast<char>(digitSum);
  }
  if (carry != 0) {
    throw invalid_argument(L"a + b overflows bytesPerDim=" +
                           to_wstring(bytesPerDim));
  }
}

void NumericUtils::intToSortableBytes(int value, std::deque<char> &result,
                                      int offset)
{
  // Flip the sign bit, so negative ints sort before positive ints correctly:
  value ^= 0x80000000;
  result[offset] = static_cast<char>(value >> 24);
  result[offset + 1] = static_cast<char>(value >> 16);
  result[offset + 2] = static_cast<char>(value >> 8);
  result[offset + 3] = static_cast<char>(value);
}

int NumericUtils::sortableBytesToInt(std::deque<char> &encoded, int offset)
{
  int x = ((encoded[offset] & 0xFF) << 24) |
          ((encoded[offset + 1] & 0xFF) << 16) |
          ((encoded[offset + 2] & 0xFF) << 8) | (encoded[offset + 3] & 0xFF);
  // Re-flip the sign bit to restore the original value:
  return x ^ 0x80000000;
}

void NumericUtils::longToSortableBytes(int64_t value,
                                       std::deque<char> &result, int offset)
{
  // Flip the sign bit so negative longs sort before positive longs:
  value ^= 0x8000000000000000LL;
  result[offset] = static_cast<char>(value >> 56);
  result[offset + 1] = static_cast<char>(value >> 48);
  result[offset + 2] = static_cast<char>(value >> 40);
  result[offset + 3] = static_cast<char>(value >> 32);
  result[offset + 4] = static_cast<char>(value >> 24);
  result[offset + 5] = static_cast<char>(value >> 16);
  result[offset + 6] = static_cast<char>(value >> 8);
  result[offset + 7] = static_cast<char>(value);
}

int64_t NumericUtils::sortableBytesToLong(std::deque<char> &encoded,
                                            int offset)
{
  int64_t v = ((encoded[offset] & 0xFFLL) << 56) |
                ((encoded[offset + 1] & 0xFFLL) << 48) |
                ((encoded[offset + 2] & 0xFFLL) << 40) |
                ((encoded[offset + 3] & 0xFFLL) << 32) |
                ((encoded[offset + 4] & 0xFFLL) << 24) |
                ((encoded[offset + 5] & 0xFFLL) << 16) |
                ((encoded[offset + 6] & 0xFFLL) << 8) |
                (encoded[offset + 7] & 0xFFLL);
  // Flip the sign bit back
  v ^= 0x8000000000000000LL;
  return v;
}

void NumericUtils::bigIntToSortableBytes(shared_ptr<int64_t> bigInt,
                                         int bigIntSize,
                                         std::deque<char> &result, int offset)
{
  std::deque<char> bigIntBytes = bigInt->toByteArray();
  std::deque<char> fullBigIntBytes;

  if (bigIntBytes.size() < bigIntSize) {
    fullBigIntBytes = std::deque<char>(bigIntSize);
    System::arraycopy(bigIntBytes, 0, fullBigIntBytes,
                      bigIntSize - bigIntBytes.size(), bigIntBytes.size());
    if ((bigIntBytes[0] & 0x80) != 0) {
      // sign extend
      Arrays::fill(fullBigIntBytes, 0, bigIntSize - bigIntBytes.size(),
                   static_cast<char>(0xff));
    }
  } else if (bigIntBytes.size() == bigIntSize) {
    fullBigIntBytes = bigIntBytes;
  } else {
    throw invalid_argument(L"int64_t: " + bigInt + L" requires more than " +
                           to_wstring(bigIntSize) + L" bytes storage");
  }
  // Flip the sign bit so negative bigints sort before positive bigints:
  fullBigIntBytes[0] ^= 0x80;

  System::arraycopy(fullBigIntBytes, 0, result, offset, bigIntSize);

  assert((sortableBytesToBigInt(result, offset, bigIntSize)->equals(bigInt),
          L"bigInt=" + bigInt + L" converted=" +
              sortableBytesToBigInt(result, offset, bigIntSize)));
}

shared_ptr<int64_t>
NumericUtils::sortableBytesToBigInt(std::deque<char> &encoded, int offset,
                                    int length)
{
  std::deque<char> bigIntBytes(length);
  System::arraycopy(encoded, offset, bigIntBytes, 0, length);
  // Flip the sign bit back to the original
  bigIntBytes[0] ^= 0x80;
  return make_shared<int64_t>(bigIntBytes);
}
} // namespace org::apache::lucene::util