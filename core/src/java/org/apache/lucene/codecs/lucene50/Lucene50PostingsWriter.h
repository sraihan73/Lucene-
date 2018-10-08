#pragma once
#include "../PushPostingsWriterBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/codecs/lucene50/Lucene50PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene50/IntBlockTermState.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene50/ForUtil.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene50/Lucene50SkipWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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
namespace org::apache::lucene::codecs::lucene50
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.codecs.lucene50.ForUtil.MAX_DATA_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.codecs.lucene50.ForUtil.MAX_ENCODED_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.DOC_CODEC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.MAX_SKIP_LEVELS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.PAY_CODEC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.POS_CODEC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.TERMS_CODEC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.VERSION_CURRENT;

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using PushPostingsWriterBase =
    org::apache::lucene::codecs::PushPostingsWriterBase;
using IntBlockTermState = org::apache::lucene::codecs::lucene50::
    Lucene50PostingsFormat::IntBlockTermState;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Concrete class that writes docId(maybe frq,pos,offset,payloads) deque
 * with postings format.
 *
 * Postings deque for each term will be stored separately.
 *
 * @see Lucene50SkipWriter for details about skipping setting and postings
 * layout.
 * @lucene.experimental
 */
class Lucene50PostingsWriter final : public PushPostingsWriterBase
{
  GET_CLASS_NAME(Lucene50PostingsWriter)

public:
  std::shared_ptr<IndexOutput> docOut;
  std::shared_ptr<IndexOutput> posOut;
  std::shared_ptr<IndexOutput> payOut;

  static const std::shared_ptr<IntBlockTermState> emptyState;
  std::shared_ptr<IntBlockTermState> lastState;

  // Holds starting file pointers for current term:
private:
  int64_t docStartFP = 0;
  int64_t posStartFP = 0;
  int64_t payStartFP = 0;

public:
  std::deque<int> const docDeltaBuffer;
  std::deque<int> const freqBuffer;

private:
  int docBufferUpto = 0;

public:
  std::deque<int> const posDeltaBuffer;
  std::deque<int> const payloadLengthBuffer;
  std::deque<int> const offsetStartDeltaBuffer;
  std::deque<int> const offsetLengthBuffer;

private:
  int posBufferUpto = 0;

  std::deque<char> payloadBytes;
  int payloadByteUpto = 0;

  int lastBlockDocID = 0;
  int64_t lastBlockPosFP = 0;
  int64_t lastBlockPayFP = 0;
  int lastBlockPosBufferUpto = 0;
  int lastBlockPayloadByteUpto = 0;

  int lastDocID = 0;
  int lastPosition = 0;
  int lastStartOffset = 0;
  int docCount = 0;

public:
  std::deque<char> const encoded;

private:
  const std::shared_ptr<ForUtil> forUtil;
  const std::shared_ptr<Lucene50SkipWriter> skipWriter;

  /** Creates a postings writer */
public:
  Lucene50PostingsWriter(std::shared_ptr<SegmentWriteState> state) throw(
      IOException);

  std::shared_ptr<IntBlockTermState> newTermState() override;

  void
  init(std::shared_ptr<IndexOutput> termsOut,
       std::shared_ptr<SegmentWriteState> state)  override;

  int setField(std::shared_ptr<FieldInfo> fieldInfo) override;

  void startTerm() override;

  void startDoc(int docID, int termDocFreq)  override;

  void addPosition(int position, std::shared_ptr<BytesRef> payload,
                   int startOffset, int endOffset)  override;

  void finishDoc()  override;

  /** Called when we are done adding docs to this term */
  void finishTerm(std::shared_ptr<BlockTermState> _state) throw(
      IOException) override;

  void encodeTerm(std::deque<int64_t> &longs,
                  std::shared_ptr<DataOutput> out,
                  std::shared_ptr<FieldInfo> fieldInfo,
                  std::shared_ptr<BlockTermState> _state,
                  bool absolute)  override;

  virtual ~Lucene50PostingsWriter();

protected:
  std::shared_ptr<Lucene50PostingsWriter> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50PostingsWriter>(
        org.apache.lucene.codecs.PushPostingsWriterBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene50/
