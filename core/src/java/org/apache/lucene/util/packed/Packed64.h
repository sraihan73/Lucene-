#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataInput.h"

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
namespace org::apache::lucene::util::packed
{

using DataInput = org::apache::lucene::store::DataInput;

/**
 * Space optimized random access capable array of values with a fixed number of
 * bits/value. Values are packed contiguously.
 * <p>
 * The implementation strives to perform as fast as possible under the
 * constraint of contiguous bits, by avoiding expensive operations. This comes
 * at the cost of code clarity.
 * <p>
 * Technical details: This implementation is a refinement of a non-branching
 * version. The non-branching get and set methods meant that 2 or 4 atomics in
 * the underlying array were always accessed, even for the cases where only
 * 1 or 2 were needed. Even with caching, this had a detrimental effect on
 * performance.
 * Related to this issue, the old implementation used lookup tables for shifts
 * and masks, which also proved to be a bit slower than calculating the shifts
 * and masks on the fly.
 * See https://issues.apache.org/jira/browse/LUCENE-4062 for details.
 *
 */
class Packed64 : public PackedInts::MutableImpl
{
  GET_CLASS_NAME(Packed64)
public:
  static constexpr int BLOCK_SIZE = 64; // 32 = int, 64 = long
  static constexpr int BLOCK_BITS = 6;  // The #bits representing BLOCK_SIZE
  static const int MOD_MASK = BLOCK_SIZE - 1; // x % BLOCK_SIZE

  /**
   * Values are stores contiguously in the blocks array.
   */
private:
  std::deque<int64_t> const blocks;
  /**
   * A right-aligned mask of width BitsPerValue used by {@link #get(int)}.
   */
  const int64_t maskRight;
  /**
   * Optimization: Saves one lookup in {@link #get(int)}.
   */
  const int bpvMinusBlockSize;

  /**
   * Creates an array with the internal structures adjusted for the given
   * limits and initialized to 0.
   * @param valueCount   the number of elements.
   * @param bitsPerValue the number of bits available for any given value.
   */
public:
  Packed64(int valueCount, int bitsPerValue);

  /**
   * Creates an array with content retrieved from the given DataInput.
   * @param in       a DataInput, positioned at the start of Packed64-content.
   * @param valueCount  the number of elements.
   * @param bitsPerValue the number of bits available for any given value.
   * @throws java.io.IOException if the values for the backing array could not
   *                             be retrieved.
   */
  Packed64(int packedIntsVersion, std::shared_ptr<DataInput> in_,
           int valueCount, int bitsPerValue) ;

  /**
   * @param index the position of the value.
   * @return the value at the given index.
   */
  int64_t get(int const index) override;

  int get(int index, std::deque<int64_t> &arr, int off, int len) override;

  void set(int const index, int64_t const value) override;

  int set(int index, std::deque<int64_t> &arr, int off, int len) override;

  virtual std::wstring toString();

  int64_t ramBytesUsed() override;

  void fill(int fromIndex, int toIndex, int64_t val) override;

private:
  static int gcd(int a, int b);

public:
  void clear() override;

protected:
  std::shared_ptr<Packed64> shared_from_this()
  {
    return std::static_pointer_cast<Packed64>(
        PackedInts.MutableImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
