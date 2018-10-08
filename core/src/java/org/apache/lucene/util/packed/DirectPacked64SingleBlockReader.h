#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
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

using IndexInput = org::apache::lucene::store::IndexInput;

class DirectPacked64SingleBlockReader final : public PackedInts::ReaderImpl
{
  GET_CLASS_NAME(DirectPacked64SingleBlockReader)

private:
  const std::shared_ptr<IndexInput> in_;
  const int bitsPerValue;
  const int64_t startPointer;
  const int valuesPerBlock;
  const int64_t mask;

public:
  DirectPacked64SingleBlockReader(int bitsPerValue, int valueCount,
                                  std::shared_ptr<IndexInput> in_);

  int64_t get(int index) override;

  int64_t ramBytesUsed() override;

protected:
  std::shared_ptr<DirectPacked64SingleBlockReader> shared_from_this()
  {
    return std::static_pointer_cast<DirectPacked64SingleBlockReader>(
        PackedInts.ReaderImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
