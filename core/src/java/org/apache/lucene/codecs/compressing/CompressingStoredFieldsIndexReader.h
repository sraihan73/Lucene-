#pragma once
#include "../../util/Accountable.h"
#include "../../util/RamUsageEstimator.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/packed/PackedInts.h"

#include  "core/src/java/org/apache/lucene/util/packed/Reader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::codecs::compressing
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.BitUtil.zigZagDecode;

using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

/**
 * Random-access reader for {@link CompressingStoredFieldsIndexWriter}.
 * @lucene.internal
 */
class CompressingStoredFieldsIndexReader final
    : public std::enable_shared_from_this<CompressingStoredFieldsIndexReader>,
      public Cloneable,
      public Accountable
{
  GET_CLASS_NAME(CompressingStoredFieldsIndexReader)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(
          CompressingStoredFieldsIndexReader::typeid);

public:
  const int maxDoc;
  std::deque<int> const docBases;
  std::deque<int64_t> const startPointers;
  std::deque<int> const avgChunkDocs;
  std::deque<int64_t> const avgChunkSizes;
  std::deque<std::shared_ptr<PackedInts::Reader>> const
      docBasesDeltas; // delta from the avg
  std::deque<std::shared_ptr<PackedInts::Reader>> const
      startPointersDeltas; // delta from the avg

  // It is the responsibility of the caller to close fieldsIndexIn after this
  // constructor has been called
  CompressingStoredFieldsIndexReader(
      std::shared_ptr<IndexInput> fieldsIndexIn,
      std::shared_ptr<SegmentInfo> si) ;

private:
  int block(int docID);

  int relativeDocBase(int block, int relativeChunk);

  int64_t relativeStartPointer(int block, int relativeChunk);

  int relativeChunk(int block, int relativeDoc);

public:
  int64_t getStartPointer(int docID);

  std::shared_ptr<CompressingStoredFieldsIndexReader> clone() override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/codecs/compressing/
