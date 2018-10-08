#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

/**
 * Helper APIs to encode numeric values as sortable bytes and vice-versa.
 *
 * <p>
 * To also index floating point numbers, this class supplies two methods to
convert them
 * to integer values by changing their bit layout: {@link
#doubleToSortableLong}, GET_CLASS_NAME(supplies)
 * {@link #floatToSortableInt}. You will have no precision loss by
 * converting floating point numbers to integers and back (only that the integer
form
 * is not usable). Other data types like dates can easily converted to longs or
ints (e.g.
 * date to long: {@link java.util.Date#getTime}).
 *
 * @lucene.internal
 */
class NumericUtils final : public std::enable_shared_from_this<NumericUtils>
{
  GET_CLASS_NAME(NumericUtils)

private:
  NumericUtils();

  /**
   * Converts a <code>double</code> value to a sortable signed
   * <code>long</code>. The value is converted by getting their IEEE 754
   * floating-point &quot;double format&quot; bit layout and then some bits are
   * swapped, to be able to compare the result as long. By this the precision is
   * not reduced, but the value can easily used as a long. The sort order
   * (including {@link Double#NaN}) is defined by
   * {@link Double#compareTo}; {@code NaN} is greater than positive infinity.
   * @see #sortableLongToDouble
   */
public:
  static int64_t doubleToSortableLong(double value);

  /**
   * Converts a sortable <code>long</code> back to a <code>double</code>.
   * @see #doubleToSortableLong
   */
  static double sortableLongToDouble(int64_t encoded);

  /**
   * Converts a <code>float</code> value to a sortable signed <code>int</code>.
   * The value is converted by getting their IEEE 754 floating-point &quot;float
   * format&quot; bit layout and then some bits are swapped, to be able to
   * compare the result as int. By this the precision is not reduced, but the
   * value can easily used as an int. The sort order (including {@link
   * Float#NaN}) is defined by
   * {@link Float#compareTo}; {@code NaN} is greater than positive infinity.
   * @see #sortableIntToFloat
   */
  static int floatToSortableInt(float value);

  /**
   * Converts a sortable <code>int</code> back to a <code>float</code>.
   * @see #floatToSortableInt
   */
  static float sortableIntToFloat(int encoded);

  /** Converts IEEE 754 representation of a double to sortable order (or back to
   * the original) */
  static int64_t sortableDoubleBits(int64_t bits);

  /** Converts IEEE 754 representation of a float to sortable order (or back to
   * the original) */
  static int sortableFloatBits(int bits);

  /** Result = a - b, where a &gt;= b, else {@code IllegalArgumentException} is
   * thrown.  */
  static void subtract(int bytesPerDim, int dim, std::deque<char> &a,
                       std::deque<char> &b, std::deque<char> &result);

  /** Result = a + b, where a and b are unsigned.  If there is an overflow,
   * {@code IllegalArgumentException} is thrown. */
  static void add(int bytesPerDim, int dim, std::deque<char> &a,
                  std::deque<char> &b, std::deque<char> &result);

  /**
   * Encodes an integer {@code value} such that unsigned byte order comparison
   * is consistent with {@link Integer#compare(int, int)}
   * @see #sortableBytesToInt(byte[], int)
   */
  static void intToSortableBytes(int value, std::deque<char> &result,
                                 int offset);

  /**
   * Decodes an integer value previously written with {@link
   * #intToSortableBytes}
   * @see #intToSortableBytes(int, byte[], int)
   */
  static int sortableBytesToInt(std::deque<char> &encoded, int offset);

  /**
   * Encodes an long {@code value} such that unsigned byte order comparison
   * is consistent with {@link Long#compare(long, long)}
   * @see #sortableBytesToLong(byte[], int)
   */
  static void longToSortableBytes(int64_t value, std::deque<char> &result,
                                  int offset);

  /**
   * Decodes a long value previously written with {@link #longToSortableBytes}
   * @see #longToSortableBytes(long, byte[], int)
   */
  static int64_t sortableBytesToLong(std::deque<char> &encoded, int offset);

  /**
   * Encodes a int64_t {@code value} such that unsigned byte order comparison
   * is consistent with {@link int64_t#compareTo(int64_t)}. This also
   * sign-extends the value to {@code bigIntSize} bytes if necessary: useful to
   * create a fixed-width size.
   * @see #sortableBytesToBigInt(byte[], int, int)
   */
  static void bigIntToSortableBytes(std::shared_ptr<int64_t> bigInt,
                                    int bigIntSize, std::deque<char> &result,
                                    int offset);

  /**
   * Decodes a int64_t value previously written with {@link
   * #bigIntToSortableBytes}
   * @see #bigIntToSortableBytes(int64_t, int, byte[], int)
   */
  static std::shared_ptr<int64_t>
  sortableBytesToBigInt(std::deque<char> &encoded, int offset, int length);
};

} // #include  "core/src/java/org/apache/lucene/util/
