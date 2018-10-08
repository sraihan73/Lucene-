#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::codecs::idversion
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using BytesRef = org::apache::lucene::util::BytesRef;

/** A PostingsFormat optimized for primary-key (ID) fields that also
 *  record a version (long) for each ID, delivered as a payload
 *  created by {@link #longToBytes} during indexing.  At search time,
 *  the TermsEnum implementation {@link IDVersionSegmentTermsEnum}
 *  enables fast (using only the terms index when possible) lookup for
 *  whether a given ID was previously indexed with version &gt; N (see
 *  {@link IDVersionSegmentTermsEnum#seekExact(BytesRef,long)}.
 *
 *  <p>This is most effective if the app assigns monotonically
 *  increasing global version to each indexed doc.  Then, during
 *  indexing, use {@link
 *  IDVersionSegmentTermsEnum#seekExact(BytesRef,long)} (along with
 *  {@link LiveFieldValues}) to decide whether the document you are
 *  about to index was already indexed with a higher version, and skip
 *  it if so.
 *
 *  <p>The field is effectively indexed as DOCS_ONLY and the docID is
 *  pulsed into the terms dictionary, but the user must feed in the
 *  version as a payload on the first token.
 *
 *  <p>NOTE: term vectors cannot be indexed with this field (not that
 *  you should really ever want to do this).
 *
 *  @lucene.experimental */

class IDVersionPostingsFormat : public PostingsFormat
{
  GET_CLASS_NAME(IDVersionPostingsFormat)

  /** version must be &gt;= this. */
public:
  static constexpr int64_t MIN_VERSION = 0;

  // TODO: we could delta encode instead, and keep the last bit:

  /** version must be &lt;= this, because we encode with ZigZag. */
  static constexpr int64_t MAX_VERSION = 0x3fffffffffffffffLL;

private:
  const int minTermsInBlock;
  const int maxTermsInBlock;

public:
  IDVersionPostingsFormat();

  IDVersionPostingsFormat(int minTermsInBlock, int maxTermsInBlock);

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  static int64_t bytesToLong(std::shared_ptr<BytesRef> bytes);

  static void longToBytes(int64_t v, std::shared_ptr<BytesRef> bytes);

protected:
  std::shared_ptr<IDVersionPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<IDVersionPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::idversion
