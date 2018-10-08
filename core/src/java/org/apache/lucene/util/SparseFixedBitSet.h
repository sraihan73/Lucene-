#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

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

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

/**
 * A bit set that only stores longs that have at least one bit which is set.
 * The way it works is that the space of bits is divided into blocks of
 * 4096 bits, which is 64 longs. Then for each block, we have:<ul>
 * <li>a long[] which stores the non-zero longs for that block</li>
 * <li>a long so that bit <tt>i</tt> being set means that the <code>i-th</code>
 *     long of the block is non-null, and its offset in the array of longs is
 *     the number of one bits on the right of the <code>i-th</code>
 * bit.</li></ul>
 *
 * @lucene.internal
 */
class SparseFixedBitSet : public BitSet, public Bits, public Accountable
{
  GET_CLASS_NAME(SparseFixedBitSet)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(SparseFixedBitSet::typeid);
  static const int64_t SINGLE_ELEMENT_ARRAY_BYTES_USED =
      RamUsageEstimator::sizeOf(std::deque<int64_t>(1));
  static const int MASK_4096 = (1 << 12) - 1;

  static int blockCount(int length);

public:
  std::deque<int64_t> const indices;
  std::deque<std::deque<int64_t>> const bits;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int length_;
  int nonZeroLongCount = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t ramBytesUsed_ = 0;

  /** Create a {@link SparseFixedBitSet} that can contain bits between
   *  <code>0</code> included and <code>length</code> excluded. */
  SparseFixedBitSet(int length);

  int length() override;

private:
  bool consistent(int index);

public:
  int cardinality() override;

  int approximateCardinality() override;

  bool get(int i) override;

private:
  static int oversize(int s);

  /**
   * Set the bit at index <tt>i</tt>.
   */
public:
  void set(int i) override;

private:
  void insertBlock(int i4096, int i64, int i);

  void insertLong(int i4096, int i64, int i, int64_t index);

  /**
   * Clear the bit at index <tt>i</tt>.
   */
public:
  void clear(int i) override;

private:
  void and (int i4096, int i64, int64_t mask);

  void removeLong(int i4096, int i64, int64_t index, int o);

public:
  void clear(int from, int to) override;

  // create a long that has bits set to one between from and to
private:
  static int64_t mask(int from, int to);

  void clearWithinBlock(int i4096, int from, int to);

  /** Return the first document that occurs on or after the provided block
   * index. */
  int firstDoc(int i4096);

public:
  int nextSetBit(int i) override;

  /** Return the last document that occurs on or before the provided block
   * index. */
private:
  int lastDoc(int i4096);

public:
  int prevSetBit(int i) override;

  /** Return the long bits at the given <code>i64</code> index. */
private:
  int64_t longBits(int64_t index, std::deque<int64_t> &bits, int i64);

  void or (int const i4096, int64_t const index, std::deque<int64_t> &bits,
           int nonZeroLongCount);

  void or (std::shared_ptr<SparseFixedBitSet> other);

  /**
   * {@link #or(DocIdSetIterator)} impl that works best when <code>it</code> is
   * dense
   */
  void orDense(std::shared_ptr<DocIdSetIterator> it) ;

public:
  void or (std::shared_ptr<DocIdSetIterator> it)  override;

  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<SparseFixedBitSet> shared_from_this()
  {
    return std::static_pointer_cast<SparseFixedBitSet>(
        BitSet::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
