using namespace std;

#include "MathUtil.h"

namespace org::apache::lucene::util
{

MathUtil::MathUtil() {}

int MathUtil::log(int64_t x, int base)
{
  if (base <= 1) {
    throw invalid_argument(L"base must be > 1");
  }
  int ret = 0;
  while (x >= base) {
    x /= base;
    ret++;
  }
  return ret;
}

double MathUtil::log(double base, double x) { return log(x) / log(base); }

int64_t MathUtil::gcd(int64_t a, int64_t b)
{
  a = abs(a);
  b = abs(b);
  if (a == 0) {
    return b;
  } else if (b == 0) {
    return a;
  }
  constexpr int commonTrailingZeros = Long::numberOfTrailingZeros(a | b);
  a = static_cast<int64_t>(static_cast<uint64_t>(a) >>
                             Long::numberOfTrailingZeros(a));
  while (true) {
    b = static_cast<int64_t>(static_cast<uint64_t>(b) >>
                               Long::numberOfTrailingZeros(b));
    if (a == b) {
      break;
    } else if (a > b ||
               a == numeric_limits<int64_t>::min()) { // MIN_VALUE is treated
                                                        // as 2^64
      constexpr int64_t tmp = a;
      a = b;
      b = tmp;
    }
    if (a == 1) {
      break;
    }
    b -= a;
  }
  return a << commonTrailingZeros;
}

double MathUtil::asinh(double a)
{
  constexpr double sign;
  // check the sign bit of the raw representation to handle -0
  if (Double::doubleToRawLongBits(a) < 0) {
    a = abs(a);
    sign = -1.0;
  } else {
    sign = 1.0;
  }

  return sign * log(sqrt(a * a + 1.0) + a);
}

double MathUtil::acosh(double a) { return log(sqrt(a * a - 1.0) + a); }

double MathUtil::atanh(double a)
{
  constexpr double mult;
  // check the sign bit of the raw representation to handle -0
  if (Double::doubleToRawLongBits(a) < 0) {
    a = abs(a);
    mult = -0.5;
  } else {
    mult = 0.5;
  }
  return mult * log((1.0 + a) / (1.0 - a));
}
} // namespace org::apache::lucene::util