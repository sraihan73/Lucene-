using namespace std;

#include "SloppyMath.h"

namespace org::apache::lucene::util
{

double SloppyMath::haversinMeters(double lat1, double lon1, double lat2,
                                  double lon2)
{
  return haversinMeters(haversinSortKey(lat1, lon1, lat2, lon2));
}

double SloppyMath::haversinMeters(double sortKey)
{
  return TO_METERS * 2 * asin(min(1, sqrt(sortKey * 0.5)));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static double haversinKilometers(double
// lat1, double lon1, double lat2, double lon2)
double SloppyMath::haversinKilometers(double lat1, double lon1, double lat2,
                                      double lon2)
{
  double h = haversinSortKey(lat1, lon1, lat2, lon2);
  return TO_KILOMETERS * 2 * asin(min(1, sqrt(h * 0.5)));
}

double SloppyMath::haversinSortKey(double lat1, double lon1, double lat2,
                                   double lon2)
{
  double x1 = lat1 * TO_RADIANS;
  double x2 = lat2 * TO_RADIANS;
  double h1 = 1 - cos(x1 - x2);
  double h2 = 1 - cos((lon1 - lon2) * TO_RADIANS);
  double h = h1 + cos(x1) * cos(x2) * h2;
  // clobber crazy precision so subsequent rounding does not create ties.
  return Double::longBitsToDouble(Double::doubleToRawLongBits(h) &
                                  0xFFFFFFFFFFFFFFF8LL);
}

double SloppyMath::cos(double a)
{
  if (a < 0.0) {
    a = -a;
  }
  if (a > SIN_COS_MAX_VALUE_FOR_INT_MODULO) {
    return cos(a);
  }
  // index: possibly outside tables range.
  int index = static_cast<int>(a * SIN_COS_INDEXER + 0.5);
  double delta = (a - index * SIN_COS_DELTA_HI) - index * SIN_COS_DELTA_LO;
  // Making sure index is within tables range.
  // Last value of each table is the same than first, so we ignore it (tabs size
  // minus one) for modulo.
  index &= (SIN_COS_TABS_SIZE - 2); // index % (SIN_COS_TABS_SIZE-1)
  double indexCos = cosTab[index];
  double indexSin = sinTab[index];
  return indexCos + delta * (-indexSin +
                             delta * (-indexCos * ONE_DIV_F2 +
                                      delta * (indexSin * ONE_DIV_F3 +
                                               delta * indexCos * ONE_DIV_F4)));
}

double SloppyMath::asin(double a)
{
  bool negateResult;
  if (a < 0.0) {
    a = -a;
    negateResult = true;
  } else {
    negateResult = false;
  }
  if (a <= ASIN_MAX_VALUE_FOR_TABS) {
    int index = static_cast<int>(a * ASIN_INDEXER + 0.5);
    double delta = a - index * ASIN_DELTA;
    double result =
        asinTab[index] +
        delta * (asinDer1DivF1Tab[index] +
                 delta * (asinDer2DivF2Tab[index] +
                          delta * (asinDer3DivF3Tab[index] +
                                   delta * asinDer4DivF4Tab[index])));
    return negateResult ? -result : result;
  } else { // value > ASIN_MAX_VALUE_FOR_TABS, or value is NaN
    // This part is derived from fdlibm.
    if (a < 1.0) {
      double t = (1.0 - a) * 0.5;
      double p =
          t * (ASIN_PS0 +
               t * (ASIN_PS1 +
                    t * (ASIN_PS2 +
                         t * (ASIN_PS3 + t * (ASIN_PS4 + t * ASIN_PS5)))));
      double q =
          1.0 + t * (ASIN_QS1 + t * (ASIN_QS2 + t * (ASIN_QS3 + t * ASIN_QS4)));
      double s = sqrt(t);
      double z = s + s * (p / q);
      double result = ASIN_PIO2_HI - ((z + z) - ASIN_PIO2_LO);
      return negateResult ? -result : result;
    } else { // value >= 1.0, or value is NaN
      if (a == 1.0) {
        return negateResult ? -M_PI / 2 : M_PI / 2;
      } else {
        return NAN;
      }
    }
  }
}

double SloppyMath::toDegrees(double const radians)
{
  return radians * TO_DEGREES;
}

double SloppyMath::toRadians(double const degrees)
{
  return degrees * TO_RADIANS;
}

const double SloppyMath::TO_RADIANS = M_PI / 180;
const double SloppyMath::TO_DEGREES = 180 / M_PI;
const double SloppyMath::PIO2_HI =
    Double::longBitsToDouble(0x3FF921FB54400000LL);
const double SloppyMath::PIO2_LO =
    Double::longBitsToDouble(0x3DD0B4611A626331LL);
const double SloppyMath::TWOPI_HI = 4 * PIO2_HI;
const double SloppyMath::TWOPI_LO = 4 * PIO2_LO;
const double SloppyMath::SIN_COS_DELTA_HI = TWOPI_HI / (SIN_COS_TABS_SIZE - 1);
const double SloppyMath::SIN_COS_DELTA_LO = TWOPI_LO / (SIN_COS_TABS_SIZE - 1);
const double SloppyMath::SIN_COS_INDEXER =
    1 / (SIN_COS_DELTA_HI + SIN_COS_DELTA_LO);
std::deque<double> const SloppyMath::sinTab =
    std::deque<double>(SIN_COS_TABS_SIZE);
std::deque<double> const SloppyMath::cosTab =
    std::deque<double>(SIN_COS_TABS_SIZE);
const double SloppyMath::SIN_COS_MAX_VALUE_FOR_INT_MODULO =
    ((numeric_limits<int>::max() >> 9) / SIN_COS_INDEXER) * 0.99;
const double SloppyMath::ASIN_MAX_VALUE_FOR_TABS = sin(toRadians(73.0));
const double SloppyMath::ASIN_DELTA =
    ASIN_MAX_VALUE_FOR_TABS / (ASIN_TABS_SIZE - 1);
const double SloppyMath::ASIN_INDEXER = 1 / ASIN_DELTA;
std::deque<double> const SloppyMath::asinTab =
    std::deque<double>(ASIN_TABS_SIZE);
std::deque<double> const SloppyMath::asinDer1DivF1Tab =
    std::deque<double>(ASIN_TABS_SIZE);
std::deque<double> const SloppyMath::asinDer2DivF2Tab =
    std::deque<double>(ASIN_TABS_SIZE);
std::deque<double> const SloppyMath::asinDer3DivF3Tab =
    std::deque<double>(ASIN_TABS_SIZE);
std::deque<double> const SloppyMath::asinDer4DivF4Tab =
    std::deque<double>(ASIN_TABS_SIZE);
const double SloppyMath::ASIN_PIO2_HI =
    Double::longBitsToDouble(0x3FF921FB54442D18LL);
const double SloppyMath::ASIN_PIO2_LO =
    Double::longBitsToDouble(0x3C91A62633145C07LL);
const double SloppyMath::ASIN_PS0 =
    Double::longBitsToDouble(0x3fc5555555555555LL);
const double SloppyMath::ASIN_PS1 =
    Double::longBitsToDouble(0xbfd4d61203eb6f7dLL);
const double SloppyMath::ASIN_PS2 =
    Double::longBitsToDouble(0x3fc9c1550e884455LL);
const double SloppyMath::ASIN_PS3 =
    Double::longBitsToDouble(0xbfa48228b5688f3bLL);
const double SloppyMath::ASIN_PS4 =
    Double::longBitsToDouble(0x3f49efe07501b288LL);
const double SloppyMath::ASIN_PS5 =
    Double::longBitsToDouble(0x3f023de10dfdf709LL);
const double SloppyMath::ASIN_QS1 =
    Double::longBitsToDouble(0xc0033a271c8a2d4bLL);
const double SloppyMath::ASIN_QS2 =
    Double::longBitsToDouble(0x40002ae59c598ac8LL);
const double SloppyMath::ASIN_QS3 =
    Double::longBitsToDouble(0xbfe6066c1b8d0159LL);
const double SloppyMath::ASIN_QS4 =
    Double::longBitsToDouble(0x3fb3b8c5b12e9282LL);

SloppyMath::StaticConstructor::StaticConstructor()
{
  // sin and cos
  constexpr int SIN_COS_PI_INDEX = (SIN_COS_TABS_SIZE - 1) / 2;
  constexpr int SIN_COS_PI_MUL_2_INDEX = 2 * SIN_COS_PI_INDEX;
  constexpr int SIN_COS_PI_MUL_0_5_INDEX = SIN_COS_PI_INDEX / 2;
  constexpr int SIN_COS_PI_MUL_1_5_INDEX = 3 * SIN_COS_PI_INDEX / 2;
  for (int i = 0; i < SIN_COS_TABS_SIZE; i++) {
    // angle: in [0,2*PI].
    double angle = i * SIN_COS_DELTA_HI + i * SIN_COS_DELTA_LO;
    double sinAngle = sin(angle);
    double cosAngle = cos(angle);
    // For indexes corresponding to null cosine or sine, we make sure the value
    // is zero and not an epsilon. This allows for a much better accuracy for
    // results close to zero.
    if (i == SIN_COS_PI_INDEX) {
      sinAngle = 0.0;
    } else if (i == SIN_COS_PI_MUL_2_INDEX) {
      sinAngle = 0.0;
    } else if (i == SIN_COS_PI_MUL_0_5_INDEX) {
      cosAngle = 0.0;
    } else if (i == SIN_COS_PI_MUL_1_5_INDEX) {
      cosAngle = 0.0;
    }
    sinTab[i] = sinAngle;
    cosTab[i] = cosAngle;
  }

  // asin
  for (int i = 0; i < ASIN_TABS_SIZE; i++) {
    // x: in [0,ASIN_MAX_VALUE_FOR_TABS].
    double x = i * ASIN_DELTA;
    asinTab[i] = asin(x);
    double oneMinusXSqInv = 1.0 / (1 - x * x);
    double oneMinusXSqInv0_5 = sqrt(oneMinusXSqInv);
    double oneMinusXSqInv1_5 = oneMinusXSqInv0_5 * oneMinusXSqInv;
    double oneMinusXSqInv2_5 = oneMinusXSqInv1_5 * oneMinusXSqInv;
    double oneMinusXSqInv3_5 = oneMinusXSqInv2_5 * oneMinusXSqInv;
    asinDer1DivF1Tab[i] = oneMinusXSqInv0_5;
    asinDer2DivF2Tab[i] = (x * oneMinusXSqInv1_5) * ONE_DIV_F2;
    asinDer3DivF3Tab[i] = ((1 + 2 * x * x) * oneMinusXSqInv2_5) * ONE_DIV_F3;
    asinDer4DivF4Tab[i] =
        ((5 + 2 * x * (2 + x * (5 - 2 * x))) * oneMinusXSqInv3_5) * ONE_DIV_F4;
  }
}

SloppyMath::StaticConstructor SloppyMath::staticConstructor;
} // namespace org::apache::lucene::util