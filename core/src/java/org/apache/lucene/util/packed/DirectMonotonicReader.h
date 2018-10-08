#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/LongValues.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/RandomAccessInput.h"

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

using IndexInput = org::apache::lucene::store::IndexInput;
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * Retrieves an instance previously written by {@link DirectMonotonicWriter}.
 * @see DirectMonotonicWriter
 */
class DirectMonotonicReader final
    : public std::enable_shared_from_this<DirectMonotonicReader>
{
  GET_CLASS_NAME(DirectMonotonicReader)

  /** An instance that always returns {@code 0}. */
private:
  static const std::shared_ptr<LongValues> EMPTY;

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  public:
    LongValuesAnonymousInnerClass();

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

  /** In-memory metadata that needs to be kept around for
   *  {@link DirectMonotonicReader} to read data from disk. */
public:
  class Meta : public std::enable_shared_from_this<Meta>, public Accountable
  {
    GET_CLASS_NAME(Meta)
  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(Meta::typeid);

  public:
    const int blockShift;
    const int numBlocks;
    std::deque<int64_t> const mins;
    std::deque<float> const avgs;
    std::deque<char> const bpvs;
    std::deque<int64_t> const offsets;

    Meta(int64_t numValues, int blockShift);

    int64_t ramBytesUsed() override;
  };

  /** Load metadata from the given {@link IndexInput}.
   *  @see DirectMonotonicReader#getInstance(Meta, RandomAccessInput) */
public:
  static std::shared_ptr<Meta> loadMeta(std::shared_ptr<IndexInput> metaIn,
                                        int64_t numValues,
                                        int blockShift) ;

  /**
   * Retrieves an instance from the specified slice.
   */
  static std::shared_ptr<LongValues>
  getInstance(std::shared_ptr<Meta> meta,
              std::shared_ptr<RandomAccessInput> data) ;

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<LongValues>> readers;
    int blockShift = 0;
    std::deque<int64_t> mins;
    std::deque<float> avgs;

  public:
    LongValuesAnonymousInnerClass(
        std::deque<std::shared_ptr<LongValues>> &readers, int blockShift,
        std::deque<int64_t> &mins, std::deque<float> &avgs);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
