using namespace std;

#include "Lucene50PostingsWriter.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/IndexFileNames.h"
#include "../../index/IndexWriter.h"
#include "../../index/SegmentWriteState.h"
#include "../../store/DataOutput.h"
#include "../../store/IndexOutput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../../util/packed/PackedInts.h"
#include "../BlockTermState.h"
#include "../CodecUtil.h"
#include "ForUtil.h"
#include "Lucene50PostingsFormat.h"
#include "Lucene50SkipWriter.h"

namespace org::apache::lucene::codecs::lucene50
{
//    import static org.apache.lucene.codecs.lucene50.ForUtil.MAX_DATA_SIZE;
//    import static org.apache.lucene.codecs.lucene50.ForUtil.MAX_ENCODED_SIZE;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.DOC_CODEC; import
//    static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.MAX_SKIP_LEVELS;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.PAY_CODEC; import
//    static org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.POS_CODEC;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.TERMS_CODEC;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.VERSION_CURRENT;
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using PushPostingsWriterBase =
    org::apache::lucene::codecs::PushPostingsWriterBase;
using IntBlockTermState = org::apache::lucene::codecs::lucene50::
    Lucene50PostingsFormat::IntBlockTermState;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
const shared_ptr<org::apache::lucene::codecs::lucene50::Lucene50PostingsFormat::
                     IntBlockTermState>
    Lucene50PostingsWriter::emptyState =
        make_shared<org::apache::lucene::codecs::lucene50::
                        Lucene50PostingsFormat::IntBlockTermState>();

Lucene50PostingsWriter::Lucene50PostingsWriter(
    shared_ptr<SegmentWriteState> state) 
    : docDeltaBuffer(std::deque<int>(MAX_DATA_SIZE)),
      freqBuffer(std::deque<int>(MAX_DATA_SIZE)),
      encoded(std::deque<char>(MAX_ENCODED_SIZE)),
      skipWriter(make_shared<Lucene50SkipWriter>(MAX_SKIP_LEVELS, BLOCK_SIZE,
                                                 state->segmentInfo->maxDoc(),
                                                 docOut, posOut, payOut))
{
  constexpr float acceptableOverheadRatio = PackedInts::COMPACT;

  wstring docFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      Lucene50PostingsFormat::DOC_EXTENSION);
  docOut = state->directory->createOutput(docFileName, state->context);
  shared_ptr<IndexOutput> posOut = nullptr;
  shared_ptr<IndexOutput> payOut = nullptr;
  bool success = false;
  try {
    CodecUtil::writeIndexHeader(docOut, DOC_CODEC, PackedInts::VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    forUtil = make_shared<ForUtil>(acceptableOverheadRatio, docOut);
    if (state->fieldInfos->hasProx()) {
      posDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);
      wstring posFileName = IndexFileNames::segmentFileName(
          state->segmentInfo->name, state->segmentSuffix,
          Lucene50PostingsFormat::POS_EXTENSION);
      posOut = state->directory->createOutput(posFileName, state->context);
      CodecUtil::writeIndexHeader(
          posOut, POS_CODEC, PackedInts::VERSION_CURRENT,
          state->segmentInfo->getId(), state->segmentSuffix);

      if (state->fieldInfos->hasPayloads()) {
        payloadBytes = std::deque<char>(128);
        payloadLengthBuffer = std::deque<int>(MAX_DATA_SIZE);
      } else {
        payloadBytes.clear();
        payloadLengthBuffer.clear();
      }

      if (state->fieldInfos->hasOffsets()) {
        offsetStartDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);
        offsetLengthBuffer = std::deque<int>(MAX_DATA_SIZE);
      } else {
        offsetStartDeltaBuffer.clear();
        offsetLengthBuffer.clear();
      }

      if (state->fieldInfos->hasPayloads() || state->fieldInfos->hasOffsets()) {
        wstring payFileName = IndexFileNames::segmentFileName(
            state->segmentInfo->name, state->segmentSuffix,
            Lucene50PostingsFormat::PAY_EXTENSION);
        payOut = state->directory->createOutput(payFileName, state->context);
        CodecUtil::writeIndexHeader(
            payOut, PAY_CODEC, PackedInts::VERSION_CURRENT,
            state->segmentInfo->getId(), state->segmentSuffix);
      }
    } else {
      posDeltaBuffer.clear();
      payloadLengthBuffer.clear();
      offsetStartDeltaBuffer.clear();
      offsetLengthBuffer.clear();
      payloadBytes.clear();
    }
    this->payOut = payOut;
    this->posOut = posOut;
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({docOut, posOut, payOut});
    }
  }

  // TODO: should we try skipping every 2/4 blocks...?
}

shared_ptr<IntBlockTermState> Lucene50PostingsWriter::newTermState()
{
  return make_shared<IntBlockTermState>();
}

void Lucene50PostingsWriter::init(
    shared_ptr<IndexOutput> termsOut,
    shared_ptr<SegmentWriteState> state) 
{
  CodecUtil::writeIndexHeader(
      termsOut, TERMS_CODEC, PackedInts::VERSION_CURRENT,
      state->segmentInfo->getId(), state->segmentSuffix);
  termsOut->writeVInt(BLOCK_SIZE);
}

int Lucene50PostingsWriter::setField(shared_ptr<FieldInfo> fieldInfo)
{
  PushPostingsWriterBase::setField(fieldInfo);
  skipWriter->setField(writePositions, writeOffsets, writePayloads);
  lastState = emptyState;
  if (writePositions) {
    if (writePayloads || writeOffsets) {
      return 3; // doc + pos + pay FP
    } else {
      return 2; // doc + pos FP
    }
  } else {
    return 1; // doc FP
  }
}

void Lucene50PostingsWriter::startTerm()
{
  docStartFP = docOut->getFilePointer();
  if (writePositions) {
    posStartFP = posOut->getFilePointer();
    if (writePayloads || writeOffsets) {
      payStartFP = payOut->getFilePointer();
    }
  }
  lastDocID = 0;
  lastBlockDocID = -1;
  skipWriter->resetSkip();
}

void Lucene50PostingsWriter::startDoc(int docID,
                                      int termDocFreq) 
{
  // Have collected a block of docs, and get a new doc.
  // Should write skip data as well as postings deque for
  // current block.
  if (lastBlockDocID != -1 && docBufferUpto == 0) {
    skipWriter->bufferSkip(lastBlockDocID, docCount, lastBlockPosFP,
                           lastBlockPayFP, lastBlockPosBufferUpto,
                           lastBlockPayloadByteUpto);
  }

  constexpr int docDelta = docID - lastDocID;

  if (docID < 0 || (docCount > 0 && docDelta <= 0)) {
    throw make_shared<CorruptIndexException>(L"docs out of order (" +
                                                 to_wstring(docID) + L" <= " +
                                                 to_wstring(lastDocID) + L" )",
                                             docOut);
  }

  docDeltaBuffer[docBufferUpto] = docDelta;
  if (writeFreqs) {
    freqBuffer[docBufferUpto] = termDocFreq;
  }

  docBufferUpto++;
  docCount++;

  if (docBufferUpto == BLOCK_SIZE) {
    forUtil->writeBlock(docDeltaBuffer, encoded, docOut);
    if (writeFreqs) {
      forUtil->writeBlock(freqBuffer, encoded, docOut);
    }
    // NOTE: don't set docBufferUpto back to 0 here;
    // finishDoc will do so (because it needs to see that
    // the block was filled so it can save skip data)
  }

  lastDocID = docID;
  lastPosition = 0;
  lastStartOffset = 0;
}

void Lucene50PostingsWriter::addPosition(int position,
                                         shared_ptr<BytesRef> payload,
                                         int startOffset,
                                         int endOffset) 
{
  if (position > IndexWriter::MAX_POSITION) {
    throw make_shared<CorruptIndexException>(
        L"position=" + to_wstring(position) +
            L" is too large (> IndexWriter.MAX_POSITION=" +
            to_wstring(IndexWriter::MAX_POSITION) + L")",
        docOut);
  }
  if (position < 0) {
    throw make_shared<CorruptIndexException>(
        L"position=" + to_wstring(position) + L" is < 0", docOut);
  }
  posDeltaBuffer[posBufferUpto] = position - lastPosition;
  if (writePayloads) {
    if (payload == nullptr || payload->length == 0) {
      // no payload
      payloadLengthBuffer[posBufferUpto] = 0;
    } else {
      payloadLengthBuffer[posBufferUpto] = payload->length;
      if (payloadByteUpto + payload->length > payloadBytes.size()) {
        payloadBytes =
            ArrayUtil::grow(payloadBytes, payloadByteUpto + payload->length);
      }
      System::arraycopy(payload->bytes, payload->offset, payloadBytes,
                        payloadByteUpto, payload->length);
      payloadByteUpto += payload->length;
    }
  }

  if (writeOffsets) {
    assert(startOffset >= lastStartOffset);
    assert(endOffset >= startOffset);
    offsetStartDeltaBuffer[posBufferUpto] = startOffset - lastStartOffset;
    offsetLengthBuffer[posBufferUpto] = endOffset - startOffset;
    lastStartOffset = startOffset;
  }

  posBufferUpto++;
  lastPosition = position;
  if (posBufferUpto == BLOCK_SIZE) {
    forUtil->writeBlock(posDeltaBuffer, encoded, posOut);

    if (writePayloads) {
      forUtil->writeBlock(payloadLengthBuffer, encoded, payOut);
      payOut->writeVInt(payloadByteUpto);
      payOut->writeBytes(payloadBytes, 0, payloadByteUpto);
      payloadByteUpto = 0;
    }
    if (writeOffsets) {
      forUtil->writeBlock(offsetStartDeltaBuffer, encoded, payOut);
      forUtil->writeBlock(offsetLengthBuffer, encoded, payOut);
    }
    posBufferUpto = 0;
  }
}

void Lucene50PostingsWriter::finishDoc() 
{
  // Since we don't know df for current term, we had to buffer
  // those skip data for each block, and when a new doc comes,
  // write them to skip file.
  if (docBufferUpto == BLOCK_SIZE) {
    lastBlockDocID = lastDocID;
    if (posOut != nullptr) {
      if (payOut != nullptr) {
        lastBlockPayFP = payOut->getFilePointer();
      }
      lastBlockPosFP = posOut->getFilePointer();
      lastBlockPosBufferUpto = posBufferUpto;
      lastBlockPayloadByteUpto = payloadByteUpto;
    }
    docBufferUpto = 0;
  }
}

void Lucene50PostingsWriter::finishTerm(
    shared_ptr<BlockTermState> _state) 
{
  shared_ptr<IntBlockTermState> state =
      std::static_pointer_cast<IntBlockTermState>(_state);
  assert(state->docFreq > 0);

  // TODO: wasteful we are counting this (counting # docs
  // for this term) in two places?
  assert((state->docFreq == docCount,
          to_wstring(state->docFreq) + L" vs " + to_wstring(docCount)));

  // docFreq == 1, don't write the single docid/freq to a separate file along
  // with a pointer to it.
  constexpr int singletonDocID;
  if (state->docFreq == 1) {
    // pulse the singleton docid into the term dictionary, freq is implicitly
    // totalTermFreq
    singletonDocID = docDeltaBuffer[0];
  } else {
    singletonDocID = -1;
    // vInt encode the remaining doc deltas and freqs:
    for (int i = 0; i < docBufferUpto; i++) {
      constexpr int docDelta = docDeltaBuffer[i];
      constexpr int freq = freqBuffer[i];
      if (!writeFreqs) {
        docOut->writeVInt(docDelta);
      } else if (freqBuffer[i] == 1) {
        docOut->writeVInt((docDelta << 1) | 1);
      } else {
        docOut->writeVInt(docDelta << 1);
        docOut->writeVInt(freq);
      }
    }
  }

  constexpr int64_t lastPosBlockOffset;

  if (writePositions) {
    // totalTermFreq is just total number of positions(or payloads, or offsets)
    // associated with current term.
    assert(state->totalTermFreq != -1);
    if (state->totalTermFreq > BLOCK_SIZE) {
      // record file offset for last pos in last block
      lastPosBlockOffset = posOut->getFilePointer() - posStartFP;
    } else {
      lastPosBlockOffset = -1;
    }
    if (posBufferUpto > 0) {
      // TODO: should we send offsets/payloads to
      // .pay...?  seems wasteful (have to store extra
      // vLong for low (< BLOCK_SIZE) DF terms = vast vast
      // majority)

      // vInt encode the remaining positions/payloads/offsets:
      int lastPayloadLength = -1; // force first payload length to be written
      int lastOffsetLength = -1;  // force first offset length to be written
      int payloadBytesReadUpto = 0;
      for (int i = 0; i < posBufferUpto; i++) {
        constexpr int posDelta = posDeltaBuffer[i];
        if (writePayloads) {
          constexpr int payloadLength = payloadLengthBuffer[i];
          if (payloadLength != lastPayloadLength) {
            lastPayloadLength = payloadLength;
            posOut->writeVInt((posDelta << 1) | 1);
            posOut->writeVInt(payloadLength);
          } else {
            posOut->writeVInt(posDelta << 1);
          }

          if (payloadLength != 0) {
            posOut->writeBytes(payloadBytes, payloadBytesReadUpto,
                               payloadLength);
            payloadBytesReadUpto += payloadLength;
          }
        } else {
          posOut->writeVInt(posDelta);
        }

        if (writeOffsets) {
          int delta = offsetStartDeltaBuffer[i];
          int length = offsetLengthBuffer[i];
          if (length == lastOffsetLength) {
            posOut->writeVInt(delta << 1);
          } else {
            posOut->writeVInt(delta << 1 | 1);
            posOut->writeVInt(length);
            lastOffsetLength = length;
          }
        }
      }

      if (writePayloads) {
        assert(payloadBytesReadUpto == payloadByteUpto);
        payloadByteUpto = 0;
      }
    }
  } else {
    lastPosBlockOffset = -1;
  }

  int64_t skipOffset;
  if (docCount > BLOCK_SIZE) {
    skipOffset = skipWriter->writeSkip(docOut) - docStartFP;
  } else {
    skipOffset = -1;
  }

  state->docStartFP = docStartFP;
  state->posStartFP = posStartFP;
  state->payStartFP = payStartFP;
  state->singletonDocID = singletonDocID;
  state->skipOffset = skipOffset;
  state->lastPosBlockOffset = lastPosBlockOffset;
  docBufferUpto = 0;
  posBufferUpto = 0;
  lastDocID = 0;
  docCount = 0;
}

void Lucene50PostingsWriter::encodeTerm(std::deque<int64_t> &longs,
                                        shared_ptr<DataOutput> out,
                                        shared_ptr<FieldInfo> fieldInfo,
                                        shared_ptr<BlockTermState> _state,
                                        bool absolute) 
{
  shared_ptr<IntBlockTermState> state =
      std::static_pointer_cast<IntBlockTermState>(_state);
  if (absolute) {
    lastState = emptyState;
  }
  longs[0] = state->docStartFP - lastState->docStartFP;
  if (writePositions) {
    longs[1] = state->posStartFP - lastState->posStartFP;
    if (writePayloads || writeOffsets) {
      longs[2] = state->payStartFP - lastState->payStartFP;
    }
  }
  if (state->singletonDocID != -1) {
    out->writeVInt(state->singletonDocID);
  }
  if (writePositions) {
    if (state->lastPosBlockOffset != -1) {
      out->writeVLong(state->lastPosBlockOffset);
    }
  }
  if (state->skipOffset != -1) {
    out->writeVLong(state->skipOffset);
  }
  lastState = state;
}

Lucene50PostingsWriter::~Lucene50PostingsWriter()
{
  // TODO: add a finish() at least to PushBase? DV too...?
  bool success = false;
  try {
    if (docOut != nullptr) {
      CodecUtil::writeFooter(docOut);
    }
    if (posOut != nullptr) {
      CodecUtil::writeFooter(posOut);
    }
    if (payOut != nullptr) {
      CodecUtil::writeFooter(payOut);
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({docOut, posOut, payOut});
    } else {
      IOUtils::closeWhileHandlingException({docOut, posOut, payOut});
    }
    docOut = posOut = payOut = nullptr;
  }
}
} // namespace org::apache::lucene::codecs::lucene50