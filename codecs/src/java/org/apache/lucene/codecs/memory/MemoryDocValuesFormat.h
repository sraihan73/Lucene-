#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::memory
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/** In-memory docvalues format */
class MemoryDocValuesFormat : public DocValuesFormat
{
  GET_CLASS_NAME(MemoryDocValuesFormat)

  /** Maximum length for each binary doc values field. */
public:
  static const int MAX_BINARY_FIELD_LENGTH = (1 << 15) - 2;

  const float acceptableOverheadRatio;

  /**
   * Calls {@link #MemoryDocValuesFormat(float)
   * MemoryDocValuesFormat(PackedInts.DEFAULT)}
   */
  MemoryDocValuesFormat();

  /**
   * Creates a new MemoryDocValuesFormat with the specified
   * <code>acceptableOverheadRatio</code> for NumericDocValues.
   * @param acceptableOverheadRatio compression parameter for numerics.
   *        Currently this is only used when the number of unique values is
   * small.
   *
   * @lucene.experimental
   */
  MemoryDocValuesFormat(float acceptableOverheadRatio);

  std::shared_ptr<DocValuesConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<DocValuesProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  static const std::wstring DATA_CODEC;
  static const std::wstring DATA_EXTENSION;
  static const std::wstring METADATA_CODEC;
  static const std::wstring METADATA_EXTENSION;

protected:
  std::shared_ptr<MemoryDocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<MemoryDocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
