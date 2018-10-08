#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/packed/BulkOperation.h"

#include  "core/src/java/org/apache/lucene/util/LongsRef.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/util/packed/PackedInts.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using LongsRef = org::apache::lucene::util::LongsRef;

class PackedReaderIterator final : public PackedInts::ReaderIteratorImpl
{
  GET_CLASS_NAME(PackedReaderIterator)

public:
  const int packedIntsVersion;
  const PackedInts::Format format;
  const std::shared_ptr<BulkOperation> bulkOperation;
  std::deque<char> const nextBlocks;
  const std::shared_ptr<LongsRef> nextValues;
  const int iterations;
  int position = 0;

  PackedReaderIterator(PackedInts::Format format, int packedIntsVersion,
                       int valueCount, int bitsPerValue,
                       std::shared_ptr<DataInput> in_, int mem);

  std::shared_ptr<LongsRef> next(int count)  override;

  int ord() override;

protected:
  std::shared_ptr<PackedReaderIterator> shared_from_this()
  {
    return std::static_pointer_cast<PackedReaderIterator>(
        PackedInts.ReaderIteratorImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
