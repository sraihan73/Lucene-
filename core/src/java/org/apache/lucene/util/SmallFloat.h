#pragma once
#include "stringhelper.h"
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

/** Floating point numbers smaller than 32 bits.
 *
 * @lucene.internal
 */
class SmallFloat : public std::enable_shared_from_this<SmallFloat>
{
  GET_CLASS_NAME(SmallFloat)

  /** No instance */
private:
  SmallFloat();

  /** Converts a 32 bit float to an 8 bit float.
   * <br>Values less than zero are all mapped to zero.
   * <br>Values are truncated (rounded down) to the nearest 8 bit value.
   * <br>Values between zero and the smallest representable value
   *  are rounded up.
   *
   * @param f the 32 bit float to be converted to an 8 bit float (byte)
   * @param numMantissaBits the number of mantissa bits to use in the byte, with
   * the remainder to be used in the exponent
   * @param zeroExp the zero-point in the range of exponent values
   * @return the 8 bit float representation
   */
public:
  static char floatToByte(float f, int numMantissaBits, int zeroExp);

  /** Converts an 8 bit float to a 32 bit float. */
  static float byteToFloat(char b, int numMantissaBits, int zeroExp);

  //
  // Some specializations of the generic functions follow.
  // The generic functions are just as fast with current (1.5)
  // -server JVMs, but still slower with client JVMs.
  //

  /** floatToByte(b, mantissaBits=3, zeroExponent=15)
   * <br>smallest non-zero value = 5.820766E-10
   * <br>largest value = 7.5161928E9
   * <br>epsilon = 0.125
   */
  static char floatToByte315(float f);

  /** byteToFloat(b, mantissaBits=3, zeroExponent=15) */
  static float byte315ToFloat(char b);

  /** Float-like encoding for positive longs that preserves ordering and 4
   * significant bits. */
  static int longToInt4(int64_t i);

  /**
   * Decode values encoded with {@link #longToInt4(long)}.
   */
  static int64_t int4ToLong(int i);

private:
  static const int MAX_INT4 = longToInt4(std::numeric_limits<int>::max());
  static const int NUM_FREE_VALUES = 255 - MAX_INT4;

  /**
   * Encode an integer to a byte. It is built upon {@link #longToInt4(long)}
   * and leverages the fact that {@code longToInt4(Integer.MAX_VALUE)} is
   * less than 255 to encode low values more accurately.
   */
public:
  static char intToByte4(int i);

  /**
   * Decode values that have been encoded with {@link #intToByte4(int)}.
   */
  static int byte4ToInt(char b);
};

} // #include  "core/src/java/org/apache/lucene/util/
