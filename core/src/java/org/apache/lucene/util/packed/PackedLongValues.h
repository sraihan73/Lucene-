#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class Builder;
}

namespace org::apache::lucene::util::packed
{
class PackedInts;
}
namespace org::apache::lucene::util::packed
{
class Reader;
}
namespace org::apache::lucene::util::packed
{
class Iterator;
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
namespace org::apache::lucene::util::packed
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;

using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * Utility class to compress integers into a {@link LongValues} instance.
 */
class PackedLongValues : public LongValues, public Accountable
{
  GET_CLASS_NAME(PackedLongValues)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(PackedLongValues::typeid);

public:
  static constexpr int DEFAULT_PAGE_SIZE = 1024;
  static constexpr int MIN_PAGE_SIZE = 64;
  // More than 1M doesn't really makes sense with these appending buffers
  // since their goal is to try to have small numbers of bits per value
  static const int MAX_PAGE_SIZE = 1 << 20;

  /** Return a new {@link Builder} that will compress efficiently positive
   * integers. */
  static std::shared_ptr<PackedLongValues::Builder>
  packedBuilder(int pageSize, float acceptableOverheadRatio);

  /** @see #packedBuilder(int, float) */
  static std::shared_ptr<PackedLongValues::Builder>
  packedBuilder(float acceptableOverheadRatio);

  /** Return a new {@link Builder} that will compress efficiently integers that
   *  are close to each other. */
  static std::shared_ptr<PackedLongValues::Builder>
  deltaPackedBuilder(int pageSize, float acceptableOverheadRatio);

  /** @see #deltaPackedBuilder(int, float) */
  static std::shared_ptr<PackedLongValues::Builder>
  deltaPackedBuilder(float acceptableOverheadRatio);

  /** Return a new {@link Builder} that will compress efficiently integers that
   *  would be a monotonic function of their index. */
  static std::shared_ptr<PackedLongValues::Builder>
  monotonicBuilder(int pageSize, float acceptableOverheadRatio);

  /** @see #monotonicBuilder(int, float) */
  static std::shared_ptr<PackedLongValues::Builder>
  monotonicBuilder(float acceptableOverheadRatio);

  std::deque<std::shared_ptr<PackedInts::Reader>> const values;
  const int pageShift, pageMask;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t size_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t ramBytesUsed_;

public:
  PackedLongValues(int pageShift, int pageMask,
                   std::deque<std::shared_ptr<PackedInts::Reader>> &values,
                   int64_t size, int64_t ramBytesUsed);

  /** Get the number of values in this array. */
  int64_t size();

  virtual int decodeBlock(int block, std::deque<int64_t> &dest);

  virtual int64_t get(int block, int element);

  int64_t get(int64_t index) override final;

  int64_t ramBytesUsed() override;

  /** Return an iterator over the values of this array. */
  virtual std::shared_ptr<Iterator> iterator();

  /** An iterator over long values. */
public:
  class Iterator final : public std::enable_shared_from_this<Iterator>
  {
    GET_CLASS_NAME(Iterator)
  private:
    std::shared_ptr<PackedLongValues> outerInstance;

  public:
    std::deque<int64_t> const currentValues;
    int vOff = 0, pOff = 0;
    int currentCount = 0; // number of entries of the current page

    Iterator(std::shared_ptr<PackedLongValues> outerInstance);

  private:
    void fillBlock();

    /** Whether or not there are remaining values. */
  public:
    bool hasNext();

    /** Return the next long in the buffer. */
    int64_t next();
  };

  /** A Builder for a {@link PackedLongValues} instance. */
public:
  class Builder : public std::enable_shared_from_this<Builder>,
                  public Accountable
  {
    GET_CLASS_NAME(Builder)

  private:
    static constexpr int INITIAL_PAGE_COUNT = 16;
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(Builder::typeid);

  public:
    const int pageShift, pageMask;
    const float acceptableOverheadRatio;
    std::deque<int64_t> pending;
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t size_ = 0;

    std::deque<std::shared_ptr<PackedInts::Reader>> values;
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t ramBytesUsed_ = 0;
    int valuesOff = 0;
    int pendingOff = 0;

    Builder(int pageSize, float acceptableOverheadRatio);

    /** Build a {@link PackedLongValues} instance that contains values that
     *  have been added to this builder. This operation is destructive. */
    virtual std::shared_ptr<PackedLongValues> build();

    virtual int64_t baseRamBytesUsed();

    int64_t ramBytesUsed() override;

    /** Return the number of elements that have been added to this builder. */
    int64_t size();

    /** Add a new element to this builder. */
    virtual std::shared_ptr<Builder> add(int64_t l);

    void finish();

  private:
    void pack();

  public:
    virtual void pack(std::deque<int64_t> &values, int numValues, int block,
                      float acceptableOverheadRatio);

    virtual void grow(int newBlockCount);
  };

protected:
  std::shared_ptr<PackedLongValues> shared_from_this()
  {
    return std::static_pointer_cast<PackedLongValues>(
        org.apache.lucene.util.LongValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
