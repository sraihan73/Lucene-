#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/packed/PackedInts.h"

#include  "core/src/java/org/apache/lucene/util/packed/Mutable.h"

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

using Mutable = org::apache::lucene::util::packed::PackedInts::Mutable;

/**
 * A {@link PagedGrowableWriter}. This class slices data into fixed-size blocks
 * which have independent numbers of bits per value and grow on-demand.
 * <p>You should use this class instead of the {@link PackedLongValues} related
 * ones only when you need random write-access. Otherwise this class will likely
 * be slower and less memory-efficient.
 * @lucene.internal
 */
class PagedGrowableWriter final
    : public AbstractPagedMutable<std::shared_ptr<PagedGrowableWriter>>
{
  GET_CLASS_NAME(PagedGrowableWriter)

public:
  const float acceptableOverheadRatio;

  /**
   * Create a new {@link PagedGrowableWriter} instance.
   *
   * @param size the number of values to store.
   * @param pageSize the number of values per page
   * @param startBitsPerValue the initial number of bits per value
   * @param acceptableOverheadRatio an acceptable overhead ratio
   */
  PagedGrowableWriter(int64_t size, int pageSize, int startBitsPerValue,
                      float acceptableOverheadRatio);

  PagedGrowableWriter(int64_t size, int pageSize, int startBitsPerValue,
                      float acceptableOverheadRatio, bool fillPages);

protected:
  std::shared_ptr<Mutable> newMutable(int valueCount,
                                      int bitsPerValue) override;

  std::shared_ptr<PagedGrowableWriter>
  newUnfilledCopy(int64_t newSize) override;

  int64_t baseRamBytesUsed() override;

protected:
  std::shared_ptr<PagedGrowableWriter> shared_from_this()
  {
    return std::static_pointer_cast<PagedGrowableWriter>(
        AbstractPagedMutable<PagedGrowableWriter>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
