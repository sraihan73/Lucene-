#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util
{

/* some code derived from jodk: http://code.google.com/p/jodk/ (apache 2.0)
 * asin() derived from fdlibm: http://www.netlib.org/fdlibm/e_asin.c (public
 * domain):
 * =============================================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * =============================================================================
 */

/** Math functions that trade off accuracy for speed. */
class SloppyMath : public std::enable_shared_from_this<SloppyMath>
{
  GET_CLASS_NAME(SloppyMath)

  /**
   * Returns the Haversine distance in meters between two points
   * specified in decimal degrees (latitude/longitude).  This works correctly
   * even if the dateline is between the two points.
   * <p>
   * Error is at most 4E-1 (40cm) from the actual haversine distance, but is
   * typically much smaller for reasonable distances: around 1E-5 (0.01mm) for
   * distances less than 1000km.
   *
   * @param lat1 Latitude of the first point.
   * @param lon1 Longitude of the first point.
   * @param lat2 Latitude of the second point.
   * @param lon2 Longitude of the second point.
   * @return distance in meters.
   */
public:
  static double haversinMeters(double lat1, double lon1, double lat2,
                               double lon2);

  /**
   * Returns the Haversine distance in meters between two points
   * given the previous result from {@link #haversinSortKey(double, double,
   * double, double)}
   * @return distance in meters.
   */
  static double haversinMeters(double sortKey);

  /**
   * Returns the Haversine distance in kilometers between two points
   * specified in decimal degrees (latitude/longitude).  This works correctly
   * even if the dateline is between the two points.
   *
   * @param lat1 Latitude of the first point.
   * @param lon1 Longitude of the first point.
   * @param lat2 Latitude of the second point.
   * @param lon2 Longitude of the second point.
   * @return distance in kilometers.
   * @deprecated Use {@link #haversinMeters(double, double, double, double)
   * instead}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static double haversinKilometers(double
  // lat1, double lon1, double lat2, double lon2)
  static double haversinKilometers(double lat1, double lon1, double lat2,
                                   double lon2);

  /**
   * Returns a sort key for distance. This is less expensive to compute than
   * {@link #haversinMeters(double, double, double, double)}, but it always
   * compares the same. This can be converted into an actual distance with
   * {@link #haversinMeters(double)}, which effectively does the second half of
   * the computation.
   */
  static double haversinSortKey(double lat1, double lon1, double lat2,
                                double lon2);

  /**
   * Returns the trigonometric cosine of an angle.
   * <p>
   * Error is around 1E-15.
   * <p>
   * Special cases:
   * <ul>
   *  <li>If the argument is {@code NaN} or an infinity, then the result is
   * {@code NaN}.
   * </ul>
   * @param a an angle, in radians.
   * @return the cosine of the argument.
   * @see Math#cos(double)
   */
  static double cos(double a);

  /**
   * Returns the arc sine of a value.
   * <p>
   * The returned angle is in the range <i>-pi</i>/2 through <i>pi</i>/2.
   * Error is around 1E-7.
   * <p>
   * Special cases:
   * <ul>
   *  <li>If the argument is {@code NaN} or its absolute value is greater than
   * 1, then the result is {@code NaN}.
   * </ul>
   * @param a the value whose arc sine is to be returned.
   * @return arc sine of the argument
   * @see Math#asin(double)
   */
  // because asin(-x) = -asin(x), asin(x) only needs to be computed on [0,1].
  // ---> we only have to compute asin(x) on [0,1].
  // For values not close to +-1, we use look-up tables;
  // for values near +-1, we use code derived from fdlibm.
  static double asin(double a);

  /**
   * Convert to degrees.
   * @param radians radians to convert to degrees
   * @return degrees
   */
  static double toDegrees(double const radians);

  /**
   * Convert to radians.
   * @param degrees degrees to convert to radians
   * @return radians
   */
  static double toRadians(double const degrees);

  // haversin
  // TODO: remove these for java 9, they fixed Math.toDegrees()/toRadians() to
  // work just like this.
  static const double TO_RADIANS;
  static const double TO_DEGREES;

  // Earth's mean radius, in meters and kilometers; see
  // http://earth-info.nga.mil/GandG/publications/tr8350.2/wgs84fin.pdf
private:
  static constexpr double TO_METERS = 6'371'008.7714;    // equatorial radius
  static constexpr double TO_KILOMETERS = 6'371.0087714; // equatorial radius

  // cos/asin
  static constexpr double ONE_DIV_F2 = 1 / 2.0;
  static constexpr double ONE_DIV_F3 = 1 / 6.0;
  static constexpr double ONE_DIV_F4 = 1 / 24.0;

  static const double
      PIO2_HI; // 1.57079632673412561417e+00 first 33 bits of pi/2
  static const double PIO2_LO; // 6.07710050650619224932e-11 pi/2 - PIO2_HI
  static const double TWOPI_HI;
  static const double TWOPI_LO;
  static const int SIN_COS_TABS_SIZE = (1 << 11) + 1;
  static const double SIN_COS_DELTA_HI;
  static const double SIN_COS_DELTA_LO;
  static const double SIN_COS_INDEXER;
  static std::deque<double> const sinTab;
  static std::deque<double> const cosTab;

  // Max abs value for fast modulo, above which we use regular angle
  // normalization. This value must be < (Integer.MAX_VALUE / SIN_COS_INDEXER),
  // to stay in range of int type. The higher it is, the higher the error, but
  // also the faster it is for lower values. If you set it to
  // ((Integer.MAX_VALUE / SIN_COS_INDEXER) * 0.99), worse accuracy on double
  // range is about 1e-10.
public:
  static const double SIN_COS_MAX_VALUE_FOR_INT_MODULO;

  // Supposed to be >= sin(77.2deg), as fdlibm code is supposed to work with
  // values > 0.975, but seems to work well enough as long as value >=
  // sin(25deg).
private:
  static const double ASIN_MAX_VALUE_FOR_TABS;

  static const int ASIN_TABS_SIZE = (1 << 13) + 1;
  static const double ASIN_DELTA;
  static const double ASIN_INDEXER;
  static std::deque<double> const asinTab;
  static std::deque<double> const asinDer1DivF1Tab;
  static std::deque<double> const asinDer2DivF2Tab;
  static std::deque<double> const asinDer3DivF3Tab;
  static std::deque<double> const asinDer4DivF4Tab;

  static const double ASIN_PIO2_HI; // 1.57079632679489655800e+00
  static const double ASIN_PIO2_LO; // 6.12323399573676603587e-17
  static const double ASIN_PS0;     //  1.66666666666666657415e-01
  static const double ASIN_PS1;     // -3.25565818622400915405e-01
  static const double ASIN_PS2;     //  2.01212532134862925881e-01
  static const double ASIN_PS3;     // -4.00555345006794114027e-02
  static const double ASIN_PS4;     //  7.91534994289814532176e-04
  static const double ASIN_PS5;     //  3.47933107596021167570e-05
  static const double ASIN_QS1;     // -2.40339491173441421878e+00
  static const double ASIN_QS2;     //  2.02094576023350569471e+00
  static const double ASIN_QS3;     // -6.88283971605453293030e-01
  static const double ASIN_QS4;     //  7.70381505559019352791e-02

  /** Initializes look-up tables. */
private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static SloppyMath::StaticConstructor staticConstructor;
};

} // #include  "core/src/java/org/apache/lucene/util/
