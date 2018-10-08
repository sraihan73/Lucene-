#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

namespace org::apache::lucene::util::packed
{
class FormatAndBits;
}
namespace org::apache::lucene::util::packed
{
class Mutable;
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

using Mutable = org::apache::lucene::util::packed::PackedInts::Mutable;

/**
 * A {@link PagedMutable}. This class slices data into fixed-size blocks
 * which have the same number of bits per value. It can be a useful replacement
 * for {@link PackedInts.Mutable} to store more than 2B values.
 * @lucene.internal
 */
class PagedMutable final
    : public AbstractPagedMutable<std::shared_ptr<PagedMutable>>
{
  GET_CLASS_NAME(PagedMutable)

public:
  const PackedInts::Format format;

  /**
   * Create a new {@link PagedMutable} instance.
   *
   * @param size the number of values to store.
   * @param pageSize the number of values per page
   * @param bitsPerValue the number of bits per value
   * @param acceptableOverheadRatio an acceptable overhead ratio
   */
  PagedMutable(int64_t size, int pageSize, int bitsPerValue,
               float acceptableOverheadRatio);

  PagedMutable(int64_t size, int pageSize,
               std::shared_ptr<PackedInts::FormatAndBits> formatAndBits);

  PagedMutable(int64_t size, int pageSize, int bitsPerValue,
               PackedInts::Format format);

protected:
  std::shared_ptr<Mutable> newMutable(int valueCount,
                                      int bitsPerValue) override;

  std::shared_ptr<PagedMutable> newUnfilledCopy(int64_t newSize) override;

  int64_t baseRamBytesUsed() override;

protected:
  std::shared_ptr<PagedMutable> shared_from_this()
  {
    return std::static_pointer_cast<PagedMutable>(
        AbstractPagedMutable<PagedMutable>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
