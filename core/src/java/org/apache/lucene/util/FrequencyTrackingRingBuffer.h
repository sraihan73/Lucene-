#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class IntBag;
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

/**
 * A ring buffer that tracks the frequency of the integers that it contains.
 * This is typically useful to track the hash codes of popular recently-used
 * items.
 *
 * This data-structure requires 22 bytes per entry on average (between 16 and
 * 28).
 *
 * @lucene.internal
 */
class FrequencyTrackingRingBuffer final
    : public std::enable_shared_from_this<FrequencyTrackingRingBuffer>,
      public Accountable
{
  GET_CLASS_NAME(FrequencyTrackingRingBuffer)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(
          FrequencyTrackingRingBuffer::typeid);

  const int maxSize;
  std::deque<int> const buffer;
  int position = 0;
  const std::shared_ptr<IntBag> frequencies;

  /** Create a new ring buffer that will contain at most <code>maxSize</code>
   * items. This buffer will initially contain <code>maxSize</code> times the
   *  <code>sentinel</code> value. */
public:
  FrequencyTrackingRingBuffer(int maxSize, int sentinel);

  int64_t ramBytesUsed() override;

  /**
   * Add a new item to this ring buffer, potentially removing the oldest
   * entry from this buffer if it is already full.
   */
  void add(int i);

  /**
   * Returns the frequency of the provided key in the ring buffer.
   */
  int frequency(int key);

  // pkg-private for testing
  std::unordered_map<int, int> asFrequencyMap();

  /**
   * A bag of integers.
   * Since in the context of the ring buffer the maximum size is known up-front
   * there is no need to worry about resizing the underlying storage.
   */
private:
  class IntBag : public std::enable_shared_from_this<IntBag>, public Accountable
  {
    GET_CLASS_NAME(IntBag)

  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(IntBag::typeid);

    std::deque<int> const keys;
    std::deque<int> const freqs;
    const int mask;

  public:
    IntBag(int maxSize);

    int64_t ramBytesUsed() override;

    /** Return the frequency of the give key in the bag. */
    virtual int frequency(int key);

    /** Increment the frequency of the given key by 1 and return its new
     * frequency. */
    virtual int add(int key);

    /** Decrement the frequency of the given key by one, or do nothing if the
     *  key is not present in the bag. Returns true iff the key was contained
     *  in the bag. */
    virtual bool remove(int key);

  private:
    void relocateAdjacentKeys(int freeSlot);

    /** Given a chain of occupied slots between <code>chainStart</code>
     *  and <code>chainEnd</code>, return whether <code>slot</code> is
     *  between the start and end of the chain. */
    static bool between(int chainStart, int chainEnd, int slot);

  public:
    virtual std::unordered_map<int, int> asMap();
  };
};

} // namespace org::apache::lucene::util
