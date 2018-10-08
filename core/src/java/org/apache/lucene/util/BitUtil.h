#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::util // from org::apache::solr::util rev 555343
{

/**  A variety of high efficiency bit twiddling routines.
 * @lucene.internal
 */
class BitUtil final : public std::enable_shared_from_this<BitUtil>
{
  GET_CLASS_NAME(BitUtil)

  // magic numbers for bit interleaving
private:
  static std::deque<int64_t> const MAGIC;
  // shift values for bit interleaving
  static std::deque<short> const SHIFT;

  BitUtil();

  // The pop methods used to rely on bit-manipulation tricks for speed but it
  // turns out that it is faster to use the Long.bitCount method (which is an
  // intrinsic since Java 6u18) in a naive loop, see LUCENE-2221

  /** Returns the number of set bits in an array of longs. */
public:
  static int64_t pop_array(std::deque<int64_t> &arr, int wordOffset,
                             int numWords);

  /** Returns the popcount or cardinality of the two sets after an intersection.
   *  Neither array is modified. */
  static int64_t pop_intersect(std::deque<int64_t> &arr1,
                                 std::deque<int64_t> &arr2, int wordOffset,
                                 int numWords);

  /** Returns the popcount or cardinality of the union of two sets.
   *  Neither array is modified. */
  static int64_t pop_union(std::deque<int64_t> &arr1,
                             std::deque<int64_t> &arr2, int wordOffset,
                             int numWords);

  /** Returns the popcount or cardinality of {@code A & ~B}.
   *  Neither array is modified. */
  static int64_t pop_andnot(std::deque<int64_t> &arr1,
                              std::deque<int64_t> &arr2, int wordOffset,
                              int numWords);

  /** Returns the popcount or cardinality of A ^ B
   * Neither array is modified. */
  static int64_t pop_xor(std::deque<int64_t> &arr1,
                           std::deque<int64_t> &arr2, int wordOffset,
                           int numWords);

  /** returns the next highest power of two, or the current value if it's
   * already a power of two or zero*/
  static int nextHighestPowerOfTwo(int v);

  /** returns the next highest power of two, or the current value if it's
   * already a power of two or zero*/
  static int64_t nextHighestPowerOfTwo(int64_t v);

  /**
   * Interleaves the first 32 bits of each long value
   *
   * Adapted from:
   * http://graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
   */
  static int64_t interleave(int even, int odd);

  /**
   * Extract just the even-bits value as a long from the bit-interleaved value
   */
  static int64_t deinterleave(int64_t b);

  /**
   * flip flops odd with even bits
   */
  static int64_t flipFlop(int64_t const b);

  /** Same as {@link #zigZagEncode(long)} but on integers. */
  static int zigZagEncode(int i);

  /**
   * <a
   * href="https://developers.google.com/protocol-buffers/docs/encoding#types">Zig-zag</a>
   * encode the provided long. Assuming the input is a signed long whose
   * absolute value can be stored on <tt>n</tt> bits, the returned value will
   * be an unsigned long that can be stored on <tt>n+1</tt> bits.
   */
  static int64_t zigZagEncode(int64_t l);

  /** Decode an int previously encoded with {@link #zigZagEncode(int)}. */
  static int zigZagDecode(int i);

  /** Decode a long previously encoded with {@link #zigZagEncode(long)}. */
  static int64_t zigZagDecode(int64_t l);
};

} // namespace org::apache::lucene::util
