#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
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
namespace org::apache::lucene::codecs::blocktreeords
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/** Uses {@link OrdsBlockTreeTermsWriter} with {@link Lucene50PostingsWriter}.
 */
class BlockTreeOrdsPostingsFormat : public PostingsFormat
{
  GET_CLASS_NAME(BlockTreeOrdsPostingsFormat)

private:
  const int minTermBlockSize;
  const int maxTermBlockSize;

  /**
   * Fixed packed block size, number of integers encoded in
   * a single packed block.
   */
  // NOTE: must be multiple of 64 because of PackedInts long-aligned
  // encoding/decoding
public:
  static constexpr int BLOCK_SIZE = 128;

  /** Creates {@code Lucene41PostingsFormat} with default
   *  settings. */
  BlockTreeOrdsPostingsFormat();

  /** Creates {@code Lucene41PostingsFormat} with custom
   *  values for {@code minBlockSize} and {@code
   *  maxBlockSize} passed to block terms dictionary.
   *  @see
   * OrdsBlockTreeTermsWriter#OrdsBlockTreeTermsWriter(SegmentWriteState,PostingsWriterBase,int,int)
   */
  BlockTreeOrdsPostingsFormat(int minTermBlockSize, int maxTermBlockSize);

  virtual std::wstring toString();

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

protected:
  std::shared_ptr<BlockTreeOrdsPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<BlockTreeOrdsPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktreeords
