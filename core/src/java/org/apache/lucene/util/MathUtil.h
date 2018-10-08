#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

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

/**
 * Math static utility methods.
 */
class MathUtil final : public std::enable_shared_from_this<MathUtil>
{
  GET_CLASS_NAME(MathUtil)

  // No instance:
private:
  MathUtil();

  /**
   * Returns {@code x <= 0 ? 0 : Math.floor(Math.log(x) / Math.log(base))}
   * @param base must be {@code > 1}
   */
public:
  static int log(int64_t x, int base);

  /**
   * Calculates logarithm in a given base with doubles.
   */
  static double log(double base, double x);

  /** Return the greatest common divisor of <code>a</code> and <code>b</code>,
   *  consistently with {@link int64_t#gcd(int64_t)}.
   *  <p><b>NOTE</b>: A greatest common divisor must be positive, but
   *  <code>2^64</code> cannot be expressed as a long although it
   *  is the GCD of {@link Long#MIN_VALUE} and <code>0</code> and the GCD of
   *  {@link Long#MIN_VALUE} and {@link Long#MIN_VALUE}. So in these 2 cases,
   *  and only them, this method will return {@link Long#MIN_VALUE}. */
  // see
  // http://en.wikipedia.org/wiki/Binary_GCD_algorithm#Iterative_version_in_C.2B.2B_using_ctz_.28count_trailing_zeros.29
  static int64_t gcd(int64_t a, int64_t b);

  /**
   * Calculates inverse hyperbolic sine of a {@code double} value.
   * <p>
   * Special cases:
   * <ul>
   *    <li>If the argument is NaN, then the result is NaN.
   *    <li>If the argument is zero, then the result is a zero with the same
   * sign as the argument. <li>If the argument is infinite, then the result is
   * infinity with the same sign as the argument.
   * </ul>
   */
  static double asinh(double a);

  /**
   * Calculates inverse hyperbolic cosine of a {@code double} value.
   * <p>
   * Special cases:
   * <ul>
   *    <li>If the argument is NaN, then the result is NaN.
   *    <li>If the argument is +1, then the result is a zero.
   *    <li>If the argument is positive infinity, then the result is positive
   * infinity. <li>If the argument is less than 1, then the result is NaN.
   * </ul>
   */
  static double acosh(double a);

  /**
   * Calculates inverse hyperbolic tangent of a {@code double} value.
   * <p>
   * Special cases:
   * <ul>
   *    <li>If the argument is NaN, then the result is NaN.
   *    <li>If the argument is zero, then the result is a zero with the same
   * sign as the argument. <li>If the argument is +1, then the result is
   * positive infinity. <li>If the argument is -1, then the result is negative
   * infinity. <li>If the argument's absolute value is greater than 1, then the
   * result is NaN.
   * </ul>
   */
  static double atanh(double a);
};

} // namespace org::apache::lucene::util
