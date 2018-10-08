#pragma once
#include "stringhelper.h"
#include <any>
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
 * BitSet of fixed length (numBits), backed by accessible ({@link #getBits})
 * long[], accessed with a long index. Use it only if you intend to store more
 * than 2.1B bits, otherwise you should use {@link FixedBitSet}.
 *
 * @lucene.internal
 */
class LongBitSet final : public std::enable_shared_from_this<LongBitSet>
{
  GET_CLASS_NAME(LongBitSet)

private:
  std::deque<int64_t> const bits; // Array of longs holding the bits
  const int64_t numBits;           // The number of bits in use
  const int numWords; // The exact number of longs needed to hold numBits (<=
                      // bits.length)

  /**
   * If the given {@link LongBitSet} is large enough to hold
   * {@code numBits+1}, returns the given bits, otherwise returns a new
   * {@link LongBitSet} which can hold the requested number of bits.
   * <p>
   * <b>NOTE:</b> the returned bitset reuses the underlying {@code long[]} of
   * the given {@code bits} if possible. Also, calling {@link #length()} on the
   * returned bits may return a value greater than {@code numBits}.
   */
public:
  static std::shared_ptr<LongBitSet>
  ensureCapacity(std::shared_ptr<LongBitSet> bits, int64_t numBits);

  /** The maximum {@code numBits} supported. */
  static const int64_t MAX_NUM_BITS =
      64 * static_cast<int64_t>(ArrayUtil::MAX_ARRAY_LENGTH);

  /** Returns the number of 64 bit words it would take to hold numBits */
  static int bits2words(int64_t numBits);

  /**
   * Creates a new LongBitSet.
   * The internally allocated long array will be exactly the size needed to
   * accommodate the numBits specified.
   * @param numBits the number of bits needed
   */
  LongBitSet(int64_t numBits);

  /**
   * Creates a new LongBitSet using the provided long[] array as backing store.
   * The storedBits array must be large enough to accommodate the numBits
   * specified, but may be larger. In that case the 'extra' or 'ghost' bits must
   * be clear (or they may provoke spurious side-effects)
   * @param storedBits the array to use as backing store
   * @param numBits the number of bits actually needed
   */
  LongBitSet(std::deque<int64_t> &storedBits, int64_t numBits);

  /**
   * Checks if the bits past numBits are clear.
   * Some methods rely on this implicit assumption: search for "Depends on the
   * ghost bits being clear!"
   * @return true if the bits past numBits are clear.
   */
private:
  bool verifyGhostBitsClear();

  /** Returns the number of bits stored in this bitset. */
public:
  int64_t length();

  /** Expert. */
  std::deque<int64_t> getBits();

  /** Returns number of set bits.  NOTE: this visits every
   *  long in the backing bits array, and the result is not
   *  internally cached!
   */
  int64_t cardinality();

  bool get(int64_t index);

  void set(int64_t index);

  bool getAndSet(int64_t index);

  void clear(int64_t index);

  bool getAndClear(int64_t index);

  /** Returns the index of the first set bit starting at the index specified.
   *  -1 is returned if there are no more set bits.
   */
  int64_t nextSetBit(int64_t index);

  /** Returns the index of the last set bit before or on the index specified.
   *  -1 is returned if there are no more set bits.
   */
  int64_t prevSetBit(int64_t index);

  /** this = this OR other */
  void or (std::shared_ptr<LongBitSet> other);

  /** this = this XOR other */
  void xor (std::shared_ptr<LongBitSet> other);

  /** returns true if the sets have any elements in common */
  bool intersects(std::shared_ptr<LongBitSet> other);

  /** this = this AND other */
  void and (std::shared_ptr<LongBitSet> other);

  /** this = this AND NOT other */
  void andNot(std::shared_ptr<LongBitSet> other);

  /**
   * Scans the backing store to check if all bits are clear.
   * The method is deliberately not called "isEmpty" to emphasize it is not low
   * cost (as isEmpty usually is).
   * @return true if all bits are clear.
   */
  bool scanIsEmpty();

  /** Flips a range of bits
   *
   * @param startIndex lower index
   * @param endIndex one-past the last bit to flip
   */
  void flip(int64_t startIndex, int64_t endIndex);

  /** Flip the bit at the provided index. */
  void flip(int64_t index);

  /** Sets a range of bits
   *
   * @param startIndex lower index
   * @param endIndex one-past the last bit to set
   */
  void set(int64_t startIndex, int64_t endIndex);

  /** Clears a range of bits.
   *
   * @param startIndex lower index
   * @param endIndex one-past the last bit to clear
   */
  void clear(int64_t startIndex, int64_t endIndex);

  std::shared_ptr<LongBitSet> clone() override;

  /** returns true if both sets have the same bits set */
  virtual bool equals(std::any o);

  virtual int hashCode();
};

} // namespace org::apache::lucene::util
