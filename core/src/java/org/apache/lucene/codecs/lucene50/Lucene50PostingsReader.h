#pragma once
#include "../../index/PostingsEnum.h"
#include "../../util/RamUsageEstimator.h"
#include "../PostingsReaderBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

#include  "core/src/java/org/apache/lucene/codecs/lucene50/ForUtil.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene50/Lucene50SkipReader.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene50/Lucene50PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene50/IntBlockTermState.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using IntBlockTermState = org::apache::lucene::codecs::lucene50::
    Lucene50PostingsFormat::IntBlockTermState;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

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
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.VERSION_START;

/**
 * Concrete class that reads docId(maybe frq,pos,offset,payloads) deque
 * with postings format.
 *
 * @lucene.experimental
 */
class Lucene50PostingsReader final : public PostingsReaderBase
{
  GET_CLASS_NAME(Lucene50PostingsReader)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(Lucene50PostingsReader::typeid);

  const std::shared_ptr<IndexInput> docIn;
  const std::shared_ptr<IndexInput> posIn;
  const std::shared_ptr<IndexInput> payIn;

public:
  const std::shared_ptr<ForUtil> forUtil;

private:
  int version = 0;

  /** Sole constructor. */
public:
  Lucene50PostingsReader(std::shared_ptr<SegmentReadState> state) throw(
      IOException);

  void
  init(std::shared_ptr<IndexInput> termsIn,
       std::shared_ptr<SegmentReadState> state)  override;

  /**
   * Read values that have been written using variable-length encoding instead
   * of bit-packing.
   */
  static void readVIntBlock(std::shared_ptr<IndexInput> docIn,
                            std::deque<int> &docBuffer,
                            std::deque<int> &freqBuffer, int num,
                            bool indexHasFreq) ;

  std::shared_ptr<BlockTermState> newTermState() override;

  virtual ~Lucene50PostingsReader();

  void decodeTerm(std::deque<int64_t> &longs, std::shared_ptr<DataInput> in_,
                  std::shared_ptr<FieldInfo> fieldInfo,
                  std::shared_ptr<BlockTermState> _termState,
                  bool absolute)  override;

  std::shared_ptr<PostingsEnum>
  postings(std::shared_ptr<FieldInfo> fieldInfo,
           std::shared_ptr<BlockTermState> termState,
           std::shared_ptr<PostingsEnum> reuse,
           int flags)  override;

public:
  class BlockDocsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(BlockDocsEnum)
  private:
    std::shared_ptr<Lucene50PostingsReader> outerInstance;

    std::deque<char> const encoded;

    std::deque<int> const docDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);
    std::deque<int> const freqBuffer = std::deque<int>(MAX_DATA_SIZE);

    int docBufferUpto = 0;

    std::shared_ptr<Lucene50SkipReader> skipper;
    bool skipped = false;

  public:
    const std::shared_ptr<IndexInput> startDocIn;

    std::shared_ptr<IndexInput> docIn;
    const bool indexHasFreq;
    const bool indexHasPos;
    const bool indexHasOffsets;
    const bool indexHasPayloads;

  private:
    int docFreq = 0; // number of docs in this posting deque
    int64_t totalTermFreq =
        0; // sum of freqs in this posting deque (or docFreq when omitted)
    int docUpto = 0; // how many docs we've read
    int doc = 0;     // doc we last read
    int accum = 0;   // accumulator for doc deltas
                     // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;   // freq we last read

    // Where this term's postings start in the .doc file:
    int64_t docTermStartFP = 0;

    // Where this term's skip data starts (after
    // docTermStartFP) in the .doc file (or -1 if there is
    // no skip data for this term):
    int64_t skipOffset = 0;

    // docID for next skip point, we won't use skipper if
    // target docID is not larger than this
    int nextSkipDoc = 0;

    bool needsFreq = false; // true if the caller actually needs frequencies
    int singletonDocID =
        0; // docid when there is a single pulsed posting, otherwise -1

  public:
    BlockDocsEnum(std::shared_ptr<Lucene50PostingsReader> outerInstance,
                  std::shared_ptr<FieldInfo> fieldInfo) ;

    bool canReuse(std::shared_ptr<IndexInput> docIn,
                  std::shared_ptr<FieldInfo> fieldInfo);

    std::shared_ptr<PostingsEnum>
    reset(std::shared_ptr<IntBlockTermState> termState,
          int flags) ;

    int freq()  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int docID() override;

  private:
    void refillDocs() ;

  public:
    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<BlockDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<BlockDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

public:
  class BlockPostingsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(BlockPostingsEnum)
  private:
    std::shared_ptr<Lucene50PostingsReader> outerInstance;

    std::deque<char> const encoded;

    std::deque<int> const docDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);
    std::deque<int> const freqBuffer = std::deque<int>(MAX_DATA_SIZE);
    std::deque<int> const posDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);

    int docBufferUpto = 0;
    int posBufferUpto = 0;

    std::shared_ptr<Lucene50SkipReader> skipper;
    bool skipped = false;

  public:
    const std::shared_ptr<IndexInput> startDocIn;

    std::shared_ptr<IndexInput> docIn;
    const std::shared_ptr<IndexInput> posIn;

    const bool indexHasOffsets;
    const bool indexHasPayloads;

  private:
    int docFreq = 0;             // number of docs in this posting deque
    int64_t totalTermFreq = 0; // number of positions in this posting deque
    int docUpto = 0;             // how many docs we've read
    int doc = 0;                 // doc we last read
    int accum = 0;               // accumulator for doc deltas
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;    // freq we last read
    int position = 0; // current position

    // how many positions "behind" we are; nextPosition must
    // skip these to "catch up":
    int posPendingCount = 0;

    // Lazy pos seek: if != -1 then we must seek to this FP
    // before reading positions:
    int64_t posPendingFP = 0;

    // Where this term's postings start in the .doc file:
    int64_t docTermStartFP = 0;

    // Where this term's postings start in the .pos file:
    int64_t posTermStartFP = 0;

    // Where this term's payloads/offsets start in the .pay
    // file:
    int64_t payTermStartFP = 0;

    // File pointer where the last (vInt encoded) pos delta
    // block is.  We need this to know whether to bulk
    // decode vs vInt decode the block:
    int64_t lastPosBlockFP = 0;

    // Where this term's skip data starts (after
    // docTermStartFP) in the .doc file (or -1 if there is
    // no skip data for this term):
    int64_t skipOffset = 0;

    int nextSkipDoc = 0;

    int singletonDocID =
        0; // docid when there is a single pulsed posting, otherwise -1

  public:
    BlockPostingsEnum(std::shared_ptr<Lucene50PostingsReader> outerInstance,
                      std::shared_ptr<FieldInfo> fieldInfo) ;

    bool canReuse(std::shared_ptr<IndexInput> docIn,
                  std::shared_ptr<FieldInfo> fieldInfo);

    std::shared_ptr<PostingsEnum>
    reset(std::shared_ptr<IntBlockTermState> termState) ;

    int freq()  override;

    int docID() override;

  private:
    void refillDocs() ;

    void refillPositions() ;

  public:
    int nextDoc()  override;

    int advance(int target)  override;

    // TODO: in theory we could avoid loading frq block
    // when not needed, ie, use skip data to load how far to
    // seek the pos pointer ... instead of having to load frq
    // blocks only to sum up how many positions to skip
  private:
    void skipPositions() ;

  public:
    int nextPosition()  override;

    int startOffset() override;

    int endOffset() override;

    std::shared_ptr<BytesRef> getPayload() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<BlockPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<BlockPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

  // Also handles payloads + offsets
public:
  class EverythingEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(EverythingEnum)
  private:
    std::shared_ptr<Lucene50PostingsReader> outerInstance;

    std::deque<char> const encoded;

    std::deque<int> const docDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);
    std::deque<int> const freqBuffer = std::deque<int>(MAX_DATA_SIZE);
    std::deque<int> const posDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);

    std::deque<int> const payloadLengthBuffer;
    std::deque<int> const offsetStartDeltaBuffer;
    std::deque<int> const offsetLengthBuffer;

    std::deque<char> payloadBytes;
    int payloadByteUpto = 0;
    int payloadLength = 0;

    int lastStartOffset = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int endOffset_ = 0;

    int docBufferUpto = 0;
    int posBufferUpto = 0;

    std::shared_ptr<Lucene50SkipReader> skipper;
    bool skipped = false;

  public:
    const std::shared_ptr<IndexInput> startDocIn;

    std::shared_ptr<IndexInput> docIn;
    const std::shared_ptr<IndexInput> posIn;
    const std::shared_ptr<IndexInput> payIn;
    const std::shared_ptr<BytesRef> payload;

    const bool indexHasOffsets;
    const bool indexHasPayloads;

  private:
    int docFreq = 0;             // number of docs in this posting deque
    int64_t totalTermFreq = 0; // number of positions in this posting deque
    int docUpto = 0;             // how many docs we've read
    int doc = 0;                 // doc we last read
    int accum = 0;               // accumulator for doc deltas
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;    // freq we last read
    int position = 0; // current position

    // how many positions "behind" we are; nextPosition must
    // skip these to "catch up":
    int posPendingCount = 0;

    // Lazy pos seek: if != -1 then we must seek to this FP
    // before reading positions:
    int64_t posPendingFP = 0;

    // Lazy pay seek: if != -1 then we must seek to this FP
    // before reading payloads/offsets:
    int64_t payPendingFP = 0;

    // Where this term's postings start in the .doc file:
    int64_t docTermStartFP = 0;

    // Where this term's postings start in the .pos file:
    int64_t posTermStartFP = 0;

    // Where this term's payloads/offsets start in the .pay
    // file:
    int64_t payTermStartFP = 0;

    // File pointer where the last (vInt encoded) pos delta
    // block is.  We need this to know whether to bulk
    // decode vs vInt decode the block:
    int64_t lastPosBlockFP = 0;

    // Where this term's skip data starts (after
    // docTermStartFP) in the .doc file (or -1 if there is
    // no skip data for this term):
    int64_t skipOffset = 0;

    int nextSkipDoc = 0;

    bool needsOffsets = false;  // true if we actually need offsets
    bool needsPayloads = false; // true if we actually need payloads
    int singletonDocID =
        0; // docid when there is a single pulsed posting, otherwise -1

  public:
    EverythingEnum(std::shared_ptr<Lucene50PostingsReader> outerInstance,
                   std::shared_ptr<FieldInfo> fieldInfo) ;

    bool canReuse(std::shared_ptr<IndexInput> docIn,
                  std::shared_ptr<FieldInfo> fieldInfo);

    std::shared_ptr<EverythingEnum>
    reset(std::shared_ptr<IntBlockTermState> termState,
          int flags) ;

    int freq()  override;

    int docID() override;

  private:
    void refillDocs() ;

    void refillPositions() ;

  public:
    int nextDoc()  override;

    int advance(int target)  override;

    // TODO: in theory we could avoid loading frq block
    // when not needed, ie, use skip data to load how far to
    // seek the pos pointer ... instead of having to load frq
    // blocks only to sum up how many positions to skip
  private:
    void skipPositions() ;

  public:
    int nextPosition()  override;

    int startOffset() override;

    int endOffset() override;

    std::shared_ptr<BytesRef> getPayload() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<EverythingEnum> shared_from_this()
    {
      return std::static_pointer_cast<EverythingEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

public:
  int64_t ramBytesUsed() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<Lucene50PostingsReader> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50PostingsReader>(
        org.apache.lucene.codecs.PostingsReaderBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene50/
