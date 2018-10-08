#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <type_traits>
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
namespace org::apache::lucene::util::packed
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.packed.PackedInts.numBlocks;

using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;

/**
 * Base implementation for {@link PagedMutable} and {@link PagedGrowableWriter}.
 * @lucene.internal
 */
template <typename T>
class AbstractPagedMutable : public LongValues, public Accountable
{
  GET_CLASS_NAME(AbstractPagedMutable)
  static_assert(std::is_base_of<AbstractPagedMutable<T>, T>::value,
                L"T must inherit from AbstractPagedMutable<T>");

public:
  static const int MIN_BLOCK_SIZE = 1 << 6;
  static const int MAX_BLOCK_SIZE = 1 << 30;

  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t size_;
  const int pageShift;
  const int pageMask;
  std::deque<std::shared_ptr<PackedInts::Mutable>> const subMutables;
  const int bitsPerValue;

  AbstractPagedMutable(int bitsPerValue, int64_t size, int pageSize)
      : size(size),
        pageShift(checkBlockSize(pageSize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE)),
        pageMask(pageSize - 1),
        subMutables(
            std::deque<std::shared_ptr<PackedInts::Mutable>>(numPages)),
        bitsPerValue(bitsPerValue)
  {
    constexpr int numPages = numBlocks(size, pageSize);
  }

protected:
  void fillPages()
  {
    constexpr int numPages = numBlocks(size_, pageSize());
    for (int i = 0; i < numPages; ++i) {
      // do not allocate for more entries than necessary on the last page
      constexpr int valueCount =
          i == numPages - 1 ? lastPageSize(size_) : pageSize();
      subMutables[i] = newMutable(valueCount, bitsPerValue);
    }
  }

  virtual std::shared_ptr<PackedInts::Mutable> newMutable(int valueCount,
                                                          int bitsPerValue) = 0;

public:
  int lastPageSize(int64_t size)
  {
    constexpr int sz = indexInPage(size);
    return sz == 0 ? pageSize() : sz;
  }

  int pageSize() { return pageMask + 1; }

  /** The number of values. */
  int64_t size() { return size_; }

  int pageIndex(int64_t index)
  {
    return static_cast<int>(static_cast<int64_t>(
        static_cast<uint64_t>(index) >> pageShift));
  }

  int indexInPage(int64_t index)
  {
    return static_cast<int>(index) & pageMask;
  }

  int64_t get(int64_t index) override final
  {
    assert((index >= 0 && index < size_, L"index=" + std::to_wstring(index) +
                                             L" size=" +
                                             std::to_wstring(size_)));
    constexpr int pageIndex = this->pageIndex(index);
    constexpr int indexInPage = this->indexInPage(index);
    return subMutables[pageIndex]->get(indexInPage);
  }

  /** Set value at <code>index</code>. */
  void set(int64_t index, int64_t value)
  {
    assert(index >= 0 && index < size_);
    constexpr int pageIndex = this->pageIndex(index);
    constexpr int indexInPage = this->indexInPage(index);
    subMutables[pageIndex]->set(indexInPage, value);
  }

protected:
  virtual int64_t baseRamBytesUsed()
  {
    return RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
           RamUsageEstimator::NUM_BYTES_OBJECT_REF + Long::BYTES +
           3 * Integer::BYTES;
  }

public:
  int64_t ramBytesUsed() override
  {
    int64_t bytesUsed =
        RamUsageEstimator::alignObjectSize(baseRamBytesUsed());
    bytesUsed += RamUsageEstimator::alignObjectSize(
        RamUsageEstimator::shallowSizeOf(subMutables));
    for (auto gw : subMutables) {
      bytesUsed += gw->ramBytesUsed();
    }
    return bytesUsed;
  }

protected:
  virtual T newUnfilledCopy(int64_t newSize) = 0;

  /** Create a new copy of size <code>newSize</code> based on the content of
   *  this buffer. This method is much more efficient than creating a new
   *  instance and copying values one by one. */
public:
  T resize(int64_t newSize)
  {
    constexpr T copy = newUnfilledCopy(newSize);
    constexpr int numCommonPages =
        std::min(copy.subMutables->length, subMutables.size());
    const std::deque<int64_t> copyBuffer = std::deque<int64_t>(1024);
    for (int i = 0; i < copy.subMutables->length; ++i) {
      constexpr int valueCount = i == copy.subMutables->length - 1
                                     ? lastPageSize(newSize)
                                     : pageSize();
      constexpr int bpv = i < numCommonPages ? subMutables[i]->getBitsPerValue()
                                             : this->bitsPerValue;
      copy.subMutables[i] = newMutable(valueCount, bpv);
      if (i < numCommonPages) {
        constexpr int copyLength = std::min(valueCount, subMutables[i]->size());
        PackedInts::copy(subMutables[i], 0, copy.subMutables[i], 0, copyLength,
                         copyBuffer);
      }
    }
    return copy;
  }

  /** Similar to {@link ArrayUtil#grow(long[], int)}. */
  T grow(int64_t minSize)
  {
    assert(minSize >= 0);
    if (minSize <= size()) {
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("unchecked") final T result = (T)
      // this;
      constexpr T result = static_cast<T>(shared_from_this());
      return result;
    }
    int64_t extra =
        static_cast<int64_t>(static_cast<uint64_t>(minSize) >> 3);
    if (extra < 3) {
      extra = 3;
    }
    constexpr int64_t newSize = minSize + extra;
    return resize(newSize);
  }

  /** Similar to {@link ArrayUtil#grow(long[])}. */
  T grow() { return grow(size() + 1); }

  std::wstring toString() override
  {
    return getClass().getSimpleName() + L"(size=" + std::to_wstring(size()) +
           L",pageSize=" + std::to_wstring(pageSize()) + L")";
  }

protected:
  std::shared_ptr<AbstractPagedMutable> shared_from_this()
  {
    return std::static_pointer_cast<AbstractPagedMutable>(
        org.apache.lucene.util.LongValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
