using namespace std;

#include "TestMathUtil.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
std::deque<int64_t> TestMathUtil::PRIMES = {2,  3,  5,  7,  11,
                                               13, 17, 19, 23, 29};

int64_t TestMathUtil::randomLong()
{
  if (random()->nextBoolean()) {
    int64_t l = 1;
    if (random()->nextBoolean()) {
      l *= -1;
    }
    for (auto i : PRIMES) {
      constexpr int m = random()->nextInt(3);
      for (int j = 0; j < m; ++j) {
        l *= i;
      }
    }
    return l;
  } else if (random()->nextBoolean()) {
    return random()->nextLong();
  } else {
    return RandomPicks::randomFrom(
        random(),
        Arrays::asList(numeric_limits<int64_t>::min(),
                       numeric_limits<int64_t>::max(), 0LL, -1LL, 1LL));
  }
}

int64_t TestMathUtil::gcd(int64_t l1, int64_t l2)
{
  shared_ptr<int64_t> *const gcd =
      static_cast<int64_t>(l1).gcd(static_cast<int64_t>(l2));
  assert(gcd->bitCount() <= 64);
  return gcd->longValue();
}

void TestMathUtil::testGCD()
{
  constexpr int iters = atLeast(100);
  for (int i = 0; i < iters; ++i) {
    constexpr int64_t l1 = randomLong();
    constexpr int64_t l2 = randomLong();
    constexpr int64_t gcd = MathUtil::gcd(l1, l2);
    constexpr int64_t actualGcd = TestMathUtil::gcd(l1, l2);
    assertEquals(actualGcd, gcd);
    if (gcd != 0) {
      assertEquals(l1, (l1 / gcd) * gcd);
      assertEquals(l2, (l2 / gcd) * gcd);
    }
  }
}

void TestMathUtil::testGCD2()
{
  int64_t a = 30;
  int64_t b = 50;
  int64_t c = 77;

  assertEquals(0, MathUtil::gcd(0, 0));
  assertEquals(b, MathUtil::gcd(0, b));
  assertEquals(a, MathUtil::gcd(a, 0));
  assertEquals(b, MathUtil::gcd(0, -b));
  assertEquals(a, MathUtil::gcd(-a, 0));

  assertEquals(10, MathUtil::gcd(a, b));
  assertEquals(10, MathUtil::gcd(-a, b));
  assertEquals(10, MathUtil::gcd(a, -b));
  assertEquals(10, MathUtil::gcd(-a, -b));

  assertEquals(1, MathUtil::gcd(a, c));
  assertEquals(1, MathUtil::gcd(-a, c));
  assertEquals(1, MathUtil::gcd(a, -c));
  assertEquals(1, MathUtil::gcd(-a, -c));

  assertEquals(3LL * (1LL << 45),
               MathUtil::gcd(3LL * (1LL << 50), 9LL * (1LL << 45)));
  assertEquals(1LL << 45,
               MathUtil::gcd(1LL << 45, numeric_limits<int64_t>::min()));

  assertEquals(numeric_limits<int64_t>::max(),
               MathUtil::gcd(numeric_limits<int64_t>::max(), 0LL));
  assertEquals(numeric_limits<int64_t>::max(),
               MathUtil::gcd(-numeric_limits<int64_t>::max(), 0LL));
  assertEquals(1, MathUtil::gcd(60247241209LL, 153092023LL));

  assertEquals(numeric_limits<int64_t>::min(),
               MathUtil::gcd(numeric_limits<int64_t>::min(), 0));
  assertEquals(numeric_limits<int64_t>::min(),
               MathUtil::gcd(0, numeric_limits<int64_t>::min()));
  assertEquals(numeric_limits<int64_t>::min(),
               MathUtil::gcd(numeric_limits<int64_t>::min(),
                             numeric_limits<int64_t>::min()));
}

void TestMathUtil::testAcoshMethod()
{
  // acosh(NaN) == NaN
  assertTrue(isnan(MathUtil::acosh(NAN)));
  // acosh(1) == +0
  assertEquals(0, Double::doubleToLongBits(MathUtil::acosh(1)));
  // acosh(POSITIVE_INFINITY) == POSITIVE_INFINITY
  assertEquals(Double::doubleToLongBits(numeric_limits<double>::infinity()),
               Double::doubleToLongBits(
                   MathUtil::acosh(numeric_limits<double>::infinity())));
  // acosh(x) : x < 1 == NaN
  assertTrue(isnan(MathUtil::acosh(0.9)));  // x < 1
  assertTrue(isnan(MathUtil::acosh(0)));    // x == 0
  assertTrue(isnan(MathUtil::acosh(-0)));   // x == -0
  assertTrue(isnan(MathUtil::acosh(-0.9))); // x < 0
  assertTrue(isnan(MathUtil::acosh(-1)));   // x == -1
  assertTrue(isnan(MathUtil::acosh(-10)));  // x < -1
  assertTrue(
      isnan(MathUtil::acosh(-numeric_limits<double>::infinity()))); // x == -Inf

  double epsilon = 0.000001;
  assertEquals(0, MathUtil::acosh(1), epsilon);
  assertEquals(1.5667992369724109, MathUtil::acosh(2.5), epsilon);
  assertEquals(14.719378760739708, MathUtil::acosh(1234567.89), epsilon);
}

void TestMathUtil::testAsinhMethod()
{

  // asinh(NaN) == NaN
  assertTrue(isnan(MathUtil::asinh(NAN)));
  // asinh(+0) == +0
  assertEquals(0, Double::doubleToLongBits(MathUtil::asinh(0)));
  // asinh(-0) == -0
  assertEquals(Double::doubleToLongBits(-0),
               Double::doubleToLongBits(MathUtil::asinh(-0)));
  // asinh(POSITIVE_INFINITY) == POSITIVE_INFINITY
  assertEquals(Double::doubleToLongBits(numeric_limits<double>::infinity()),
               Double::doubleToLongBits(
                   MathUtil::asinh(numeric_limits<double>::infinity())));
  // asinh(NEGATIVE_INFINITY) == NEGATIVE_INFINITY
  assertEquals(Double::doubleToLongBits(-numeric_limits<double>::infinity()),
               Double::doubleToLongBits(
                   MathUtil::asinh(-numeric_limits<double>::infinity())));

  double epsilon = 0.000001;
  assertEquals(-14.719378760740035, MathUtil::asinh(-1234567.89), epsilon);
  assertEquals(-1.6472311463710958, MathUtil::asinh(-2.5), epsilon);
  assertEquals(-0.8813735870195429, MathUtil::asinh(-1), epsilon);
  assertEquals(0, MathUtil::asinh(0), 0);
  assertEquals(0.8813735870195429, MathUtil::asinh(1), epsilon);
  assertEquals(1.6472311463710958, MathUtil::asinh(2.5), epsilon);
  assertEquals(14.719378760740035, MathUtil::asinh(1234567.89), epsilon);
}

void TestMathUtil::testAtanhMethod()
{
  // atanh(NaN) == NaN
  assertTrue(isnan(MathUtil::atanh(NAN)));
  // atanh(+0) == +0
  assertEquals(0, Double::doubleToLongBits(MathUtil::atanh(0)));
  // atanh(-0) == -0
  assertEquals(Double::doubleToLongBits(-0),
               Double::doubleToLongBits(MathUtil::atanh(-0)));
  // atanh(1) == POSITIVE_INFINITY
  assertEquals(Double::doubleToLongBits(numeric_limits<double>::infinity()),
               Double::doubleToLongBits(MathUtil::atanh(1)));
  // atanh(-1) == NEGATIVE_INFINITY
  assertEquals(Double::doubleToLongBits(-numeric_limits<double>::infinity()),
               Double::doubleToLongBits(MathUtil::atanh(-1)));
  // atanh(x) : Math.abs(x) > 1 == NaN
  assertTrue(isnan(MathUtil::atanh(1.1))); // x > 1
  assertTrue(
      isnan(MathUtil::atanh(numeric_limits<double>::infinity()))); // x == Inf
  assertTrue(isnan(MathUtil::atanh(-1.1)));                        // x < -1
  assertTrue(
      isnan(MathUtil::atanh(-numeric_limits<double>::infinity()))); // x == -Inf

  double epsilon = 0.000001;
  assertEquals(-numeric_limits<double>::infinity(), MathUtil::atanh(-1), 0);
  assertEquals(-0.5493061443340549, MathUtil::atanh(-0.5), epsilon);
  assertEquals(0, MathUtil::atanh(0), 0);
  assertEquals(0.5493061443340549, MathUtil::atanh(0.5), epsilon);
  assertEquals(numeric_limits<double>::infinity(), MathUtil::atanh(1), 0);
}
} // namespace org::apache::lucene::util