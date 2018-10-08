#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

namespace org::apache::lucene::util::packed
{
class Reader;
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
//    import static
//    org.apache.lucene.util.packed.MonotonicBlockPackedReader.expected;

using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Reader = org::apache::lucene::util::packed::PackedInts::Reader;

class MonotonicLongValues : public DeltaPackedLongValues
{
  GET_CLASS_NAME(MonotonicLongValues)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(MonotonicLongValues::typeid);

public:
  std::deque<float> const averages;

  MonotonicLongValues(int pageShift, int pageMask,
                      std::deque<std::shared_ptr<Reader>> &values,
                      std::deque<int64_t> &mins,
                      std::deque<float> &averages, int64_t size,
                      int64_t ramBytesUsed);

  int64_t get(int block, int element) override;

  int decodeBlock(int block, std::deque<int64_t> &dest) override;

public:
  class Builder : public DeltaPackedLongValues::Builder
  {
    GET_CLASS_NAME(Builder)

  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(Builder::typeid);

  public:
    std::deque<float> averages;

    Builder(int pageSize, float acceptableOverheadRatio);

    int64_t baseRamBytesUsed() override;

    std::shared_ptr<MonotonicLongValues> build() override;

    void pack(std::deque<int64_t> &values, int numValues, int block,
              float acceptableOverheadRatio) override;

    void grow(int newBlockCount) override;

  protected:
    std::shared_ptr<Builder> shared_from_this()
    {
      return std::static_pointer_cast<Builder>(
          DeltaPackedLongValues.Builder::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MonotonicLongValues> shared_from_this()
  {
    return std::static_pointer_cast<MonotonicLongValues>(
        DeltaPackedLongValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
