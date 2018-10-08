#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::util
{
class Bits;
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
 * BitSet of fixed length (numBits), backed by accessible ({@link #getBits})
 * long[], accessed with an int index, implementing {@link Bits} and
 * {@link DocIdSet}. If you need to manage more than 2.1B bits, use
 * {@link LongBitSet}.
 *
 * @lucene.internal
 */
class FixedBitSet final : public BitSet, public Bits, public Accountable
{
  GET_CLASS_NAME(FixedBitSet)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(FixedBitSet::typeid);

  std::deque<int64_t> const bits; // Array of longs holding the bits
  const int numBits;                 // The number of bits in use
  const int numWords; // The exact number of longs needed to hold numBits (<=
                      // bits.length)

  /**
   * If the given {@link FixedBitSet} is large enough to hold {@code numBits+1},
   * returns the given bits, otherwise returns a new {@link FixedBitSet} which
   * can hold the requested number of bits.
   * <p>
   * <b>NOTE:</b> the returned bitset reuses the underlying {@code long[]} of
   * the given {@code bits} if possible. Also, calling {@link #length()} on the
   * returned bits may return a value greater than {@code numBits}.
   */
public:
  static std::shared_ptr<FixedBitSet>
  ensureCapacity(std::shared_ptr<FixedBitSet> bits, int numBits);

  /** returns the number of 64 bit words it would take to hold numBits */
  static int bits2words(int numBits);

  /**
   * Returns the popcount or cardinality of the intersection of the two sets.
   * Neither set is modified.
   */
  static int64_t intersectionCount(std::shared_ptr<FixedBitSet> a,
                                     std::shared_ptr<FixedBitSet> b);

  /**
   * Returns the popcount or cardinality of the union of the two sets. Neither
   * set is modified.
   */
  static int64_t unionCount(std::shared_ptr<FixedBitSet> a,
                              std::shared_ptr<FixedBitSet> b);

  /**
   * Returns the popcount or cardinality of "a and not b" or
   * "intersection(a, not(b))". Neither set is modified.
   */
  static int64_t andNotCount(std::shared_ptr<FixedBitSet> a,
                               std::shared_ptr<FixedBitSet> b);

  /**
   * Creates a new LongBitSet.
   * The internally allocated long array will be exactly the size needed to
   * accommodate the numBits specified.
   * @param numBits the number of bits needed
   */
  FixedBitSet(int numBits);

  /**
   * Creates a new LongBitSet using the provided long[] array as backing store.
   * The storedBits array must be large enough to accommodate the numBits
   * specified, but may be larger. In that case the 'extra' or 'ghost' bits must
   * be clear (or they may provoke spurious side-effects)
   * @param storedBits the array to use as backing store
   * @param numBits the number of bits actually needed
   */
  FixedBitSet(std::deque<int64_t> &storedBits, int numBits);

  /**
   * Checks if the bits past numBits are clear.
   * Some methods rely on this implicit assumption: search for "Depends on the
   * ghost bits being clear!"
   * @return true if the bits past numBits are clear.
   */
private:
  bool verifyGhostBitsClear();

public:
  int length() override;

  int64_t ramBytesUsed() override;

  /** Expert. */
  std::deque<int64_t> getBits();

  /** Returns number of set bits.  NOTE: this visits every
   *  long in the backing bits array, and the result is not
   *  internally cached!
   */
  int cardinality() override;

  bool get(int index) override;

  void set(int index) override;

  bool getAndSet(int index);

  void clear(int index) override;

  bool getAndClear(int index);

  int nextSetBit(int index) override;

  int prevSetBit(int index) override;

  void or (std::shared_ptr<DocIdSetIterator> iter)  override;

  /** this = this OR other */
  void or (std::shared_ptr<FixedBitSet> other);

private:
  void or (std::deque<int64_t> & otherArr, int const otherNumWords);

  /** this = this XOR other */
public:
  void xor (std::shared_ptr<FixedBitSet> other);

  /** Does in-place XOR of the bits provided by the iterator. */
  void xor (std::shared_ptr<DocIdSetIterator> iter) ;

private:
  void xor (std::deque<int64_t> & otherBits, int otherNumWords);

  /** returns true if the sets have any elements in common */
public:
  bool intersects(std::shared_ptr<FixedBitSet> other);

  /** this = this AND other */
  void and (std::shared_ptr<FixedBitSet> other);

private:
  void and (std::deque<int64_t> & otherArr, int const otherNumWords);

  /** this = this AND NOT other */
public:
  void andNot(std::shared_ptr<FixedBitSet> other);

private:
  void andNot(std::deque<int64_t> &otherArr, int const otherNumWords);

  /**
   * Scans the backing store to check if all bits are clear.
   * The method is deliberately not called "isEmpty" to emphasize it is not low
   * cost (as isEmpty usually is).
   * @return true if all bits are clear.
   */
public:
  bool scanIsEmpty();

  /** Flips a range of bits
   *
   * @param startIndex lower index
   * @param endIndex one-past the last bit to flip
   */
  void flip(int startIndex, int endIndex);

  /** Flip the bit at the provided index. */
  void flip(int index);

  /** Sets a range of bits
   *
   * @param startIndex lower index
   * @param endIndex one-past the last bit to set
   */
  void set(int startIndex, int endIndex);

  void clear(int startIndex, int endIndex) override;

  std::shared_ptr<FixedBitSet> clone() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  /**
   * Make a copy of the given bits.
   */
  static std::shared_ptr<FixedBitSet> copyOf(std::shared_ptr<Bits> bits);

  /**
   * Convert this instance to read-only {@link Bits}.
   * This is useful in the case that this {@link FixedBitSet} is returned as a
   * {@link Bits} instance, to make sure that consumers may not get write access
   * back by casting to a {@link FixedBitSet}.
   * NOTE: Changes to this {@link FixedBitSet} will be reflected on the returned
   * {@link Bits}.
   */
  std::shared_ptr<Bits> asReadOnlyBits();

protected:
  std::shared_ptr<FixedBitSet> shared_from_this()
  {
    return std::static_pointer_cast<FixedBitSet>(BitSet::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
