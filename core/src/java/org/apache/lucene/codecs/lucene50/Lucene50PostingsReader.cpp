using namespace std;

#include "Lucene50PostingsReader.h"
#include "../../index/FieldInfo.h"
#include "../../index/IndexFileNames.h"
#include "../../index/IndexOptions.h"
#include "../../index/SegmentReadState.h"
#include "../../store/DataInput.h"
#include "../../store/IndexInput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../BlockTermState.h"
#include "../CodecUtil.h"
#include "ForUtil.h"
#include "Lucene50PostingsFormat.h"
#include "Lucene50SkipReader.h"

namespace org::apache::lucene::codecs::lucene50
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using IntBlockTermState = org::apache::lucene::codecs::lucene50::
    Lucene50PostingsFormat::IntBlockTermState;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
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
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.VERSION_START;

Lucene50PostingsReader::Lucene50PostingsReader(
    shared_ptr<SegmentReadState> state) 
{
  bool success = false;
  shared_ptr<IndexInput> docIn = nullptr;
  shared_ptr<IndexInput> posIn = nullptr;
  shared_ptr<IndexInput> payIn = nullptr;

  // NOTE: these data files are too costly to verify checksum against all the
  // bytes on open, but for now we at least verify proper structure of the
  // checksum footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap
  // and can detect some forms of corruption such as file truncation.

  wstring docName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      Lucene50PostingsFormat::DOC_EXTENSION);
  try {
    docIn = state->directory->openInput(docName, state->context);
    version = CodecUtil::checkIndexHeader(
        docIn, DOC_CODEC, VERSION_START, VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    forUtil = make_shared<ForUtil>(docIn);
    CodecUtil::retrieveChecksum(docIn);

    if (state->fieldInfos->hasProx()) {
      wstring proxName = IndexFileNames::segmentFileName(
          state->segmentInfo->name, state->segmentSuffix,
          Lucene50PostingsFormat::POS_EXTENSION);
      posIn = state->directory->openInput(proxName, state->context);
      CodecUtil::checkIndexHeader(posIn, POS_CODEC, version, version,
                                  state->segmentInfo->getId(),
                                  state->segmentSuffix);
      CodecUtil::retrieveChecksum(posIn);

      if (state->fieldInfos->hasPayloads() || state->fieldInfos->hasOffsets()) {
        wstring payName = IndexFileNames::segmentFileName(
            state->segmentInfo->name, state->segmentSuffix,
            Lucene50PostingsFormat::PAY_EXTENSION);
        payIn = state->directory->openInput(payName, state->context);
        CodecUtil::checkIndexHeader(payIn, PAY_CODEC, version, version,
                                    state->segmentInfo->getId(),
                                    state->segmentSuffix);
        CodecUtil::retrieveChecksum(payIn);
      }
    }

    this->docIn = docIn;
    this->posIn = posIn;
    this->payIn = payIn;
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({docIn, posIn, payIn});
    }
  }
}

void Lucene50PostingsReader::init(
    shared_ptr<IndexInput> termsIn,
    shared_ptr<SegmentReadState> state) 
{
  // Make sure we are talking to the matching postings writer
  CodecUtil::checkIndexHeader(termsIn, TERMS_CODEC, VERSION_START,
                              VERSION_CURRENT, state->segmentInfo->getId(),
                              state->segmentSuffix);
  constexpr int indexBlockSize = termsIn->readVInt();
  if (indexBlockSize != BLOCK_SIZE) {
    throw make_shared<IllegalStateException>(
        L"index-time BLOCK_SIZE (" + to_wstring(indexBlockSize) +
        L") != read-time BLOCK_SIZE (" + BLOCK_SIZE + L")");
  }
}

void Lucene50PostingsReader::readVIntBlock(shared_ptr<IndexInput> docIn,
                                           std::deque<int> &docBuffer,
                                           std::deque<int> &freqBuffer,
                                           int num,
                                           bool indexHasFreq) 
{
  if (indexHasFreq) {
    for (int i = 0; i < num; i++) {
      constexpr int code = docIn->readVInt();
      docBuffer[i] = static_cast<int>(static_cast<unsigned int>(code) >> 1);
      if ((code & 1) != 0) {
        freqBuffer[i] = 1;
      } else {
        freqBuffer[i] = docIn->readVInt();
      }
    }
  } else {
    for (int i = 0; i < num; i++) {
      docBuffer[i] = docIn->readVInt();
    }
  }
}

shared_ptr<BlockTermState> Lucene50PostingsReader::newTermState()
{
  return make_shared<IntBlockTermState>();
}

Lucene50PostingsReader::~Lucene50PostingsReader()
{
  IOUtils::close({docIn, posIn, payIn});
}

void Lucene50PostingsReader::decodeTerm(std::deque<int64_t> &longs,
                                        shared_ptr<DataInput> in_,
                                        shared_ptr<FieldInfo> fieldInfo,
                                        shared_ptr<BlockTermState> _termState,
                                        bool absolute) 
{
  shared_ptr<IntBlockTermState> *const termState =
      std::static_pointer_cast<IntBlockTermState>(_termState);
  constexpr bool fieldHasPositions =
      fieldInfo->getIndexOptions().compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  constexpr bool fieldHasOffsets =
      fieldInfo->getIndexOptions().compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
  constexpr bool fieldHasPayloads = fieldInfo->hasPayloads();

  if (absolute) {
    termState->docStartFP = 0;
    termState->posStartFP = 0;
    termState->payStartFP = 0;
  }

  termState->docStartFP += longs[0];
  if (fieldHasPositions) {
    termState->posStartFP += longs[1];
    if (fieldHasOffsets || fieldHasPayloads) {
      termState->payStartFP += longs[2];
    }
  }
  if (termState->docFreq == 1) {
    termState->singletonDocID = in_->readVInt();
  } else {
    termState->singletonDocID = -1;
  }
  if (fieldHasPositions) {
    if (termState->totalTermFreq > BLOCK_SIZE) {
      termState->lastPosBlockOffset = in_->readVLong();
    } else {
      termState->lastPosBlockOffset = -1;
    }
  }
  if (termState->docFreq > BLOCK_SIZE) {
    termState->skipOffset = in_->readVLong();
  } else {
    termState->skipOffset = -1;
  }
}

shared_ptr<PostingsEnum> Lucene50PostingsReader::postings(
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<BlockTermState> termState,
    shared_ptr<PostingsEnum> reuse, int flags) 
{

  bool indexHasPositions = fieldInfo->getIndexOptions().compareTo(
                               IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  bool indexHasOffsets =
      fieldInfo->getIndexOptions().compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
  bool indexHasPayloads = fieldInfo->hasPayloads();

  if (indexHasPositions == false ||
      PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS) == false) {
    shared_ptr<BlockDocsEnum> docsEnum;
    if (std::dynamic_pointer_cast<BlockDocsEnum>(reuse) != nullptr) {
      docsEnum = std::static_pointer_cast<BlockDocsEnum>(reuse);
      if (!docsEnum->canReuse(docIn, fieldInfo)) {
        docsEnum = make_shared<BlockDocsEnum>(shared_from_this(), fieldInfo);
      }
    } else {
      docsEnum = make_shared<BlockDocsEnum>(shared_from_this(), fieldInfo);
    }
    return docsEnum->reset(
        std::static_pointer_cast<IntBlockTermState>(termState), flags);
  } else if ((indexHasOffsets == false ||
              PostingsEnum::featureRequested(flags, PostingsEnum::OFFSETS) ==
                  false) &&
             (indexHasPayloads == false ||
              PostingsEnum::featureRequested(flags, PostingsEnum::PAYLOADS) ==
                  false)) {
    shared_ptr<BlockPostingsEnum> docsAndPositionsEnum;
    if (std::dynamic_pointer_cast<BlockPostingsEnum>(reuse) != nullptr) {
      docsAndPositionsEnum = std::static_pointer_cast<BlockPostingsEnum>(reuse);
      if (!docsAndPositionsEnum->canReuse(docIn, fieldInfo)) {
        docsAndPositionsEnum =
            make_shared<BlockPostingsEnum>(shared_from_this(), fieldInfo);
      }
    } else {
      docsAndPositionsEnum =
          make_shared<BlockPostingsEnum>(shared_from_this(), fieldInfo);
    }
    return docsAndPositionsEnum->reset(
        std::static_pointer_cast<IntBlockTermState>(termState));
  } else {
    shared_ptr<EverythingEnum> everythingEnum;
    if (std::dynamic_pointer_cast<EverythingEnum>(reuse) != nullptr) {
      everythingEnum = std::static_pointer_cast<EverythingEnum>(reuse);
      if (!everythingEnum->canReuse(docIn, fieldInfo)) {
        everythingEnum =
            make_shared<EverythingEnum>(shared_from_this(), fieldInfo);
      }
    } else {
      everythingEnum =
          make_shared<EverythingEnum>(shared_from_this(), fieldInfo);
    }
    return everythingEnum->reset(
        std::static_pointer_cast<IntBlockTermState>(termState), flags);
  }
}

Lucene50PostingsReader::BlockDocsEnum::BlockDocsEnum(
    shared_ptr<Lucene50PostingsReader> outerInstance,
    shared_ptr<FieldInfo> fieldInfo) 
    : encoded(std::deque<char>(MAX_ENCODED_SIZE)),
      startDocIn(outerInstance->docIn),
      indexHasFreq(fieldInfo->getIndexOptions().compareTo(
                       IndexOptions::DOCS_AND_FREQS) >= 0),
      indexHasPos(fieldInfo->getIndexOptions().compareTo(
                      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0),
      indexHasOffsets(
          fieldInfo->getIndexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0),
      indexHasPayloads(fieldInfo->hasPayloads()), outerInstance(outerInstance)
{
  this->docIn.reset();
}

bool Lucene50PostingsReader::BlockDocsEnum::canReuse(
    shared_ptr<IndexInput> docIn, shared_ptr<FieldInfo> fieldInfo)
{
  return docIn == startDocIn &&
         indexHasFreq == (fieldInfo->getIndexOptions().compareTo(
                              IndexOptions::DOCS_AND_FREQS) >= 0) &&
         indexHasPos ==
             (fieldInfo->getIndexOptions().compareTo(
                  IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0) &&
         indexHasPayloads == fieldInfo->hasPayloads();
}

shared_ptr<PostingsEnum> Lucene50PostingsReader::BlockDocsEnum::reset(
    shared_ptr<IntBlockTermState> termState, int flags) 
{
  docFreq = termState->docFreq;
  totalTermFreq = indexHasFreq ? termState->totalTermFreq : docFreq;
  docTermStartFP = termState->docStartFP;
  skipOffset = termState->skipOffset;
  singletonDocID = termState->singletonDocID;
  if (docFreq > 1) {
    if (docIn == nullptr) {
      // lazy init
      docIn = startDocIn->clone();
    }
    docIn->seek(docTermStartFP);
  }

  doc = -1;
  this->needsFreq = PostingsEnum::featureRequested(flags, PostingsEnum::FREQS);
  if (indexHasFreq == false || needsFreq == false) {
    Arrays::fill(freqBuffer, 1);
  }
  accum = 0;
  docUpto = 0;
  nextSkipDoc =
      BLOCK_SIZE - 1; // we won't skip if target is found in first block
  docBufferUpto = BLOCK_SIZE;
  skipped = false;
  return shared_from_this();
}

int Lucene50PostingsReader::BlockDocsEnum::freq() 
{
  return freq_;
}

int Lucene50PostingsReader::BlockDocsEnum::nextPosition() 
{
  return -1;
}

int Lucene50PostingsReader::BlockDocsEnum::startOffset() 
{
  return -1;
}

int Lucene50PostingsReader::BlockDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
Lucene50PostingsReader::BlockDocsEnum::getPayload() 
{
  return nullptr;
}

int Lucene50PostingsReader::BlockDocsEnum::docID() { return doc; }

void Lucene50PostingsReader::BlockDocsEnum::refillDocs() 
{
  constexpr int left = docFreq - docUpto;
  assert(left > 0);

  if (left >= BLOCK_SIZE) {
    outerInstance->forUtil->readBlock(docIn, encoded, docDeltaBuffer);

    if (indexHasFreq) {
      if (needsFreq) {
        outerInstance->forUtil->readBlock(docIn, encoded, freqBuffer);
      } else {
        outerInstance->forUtil->skipBlock(docIn); // skip over freqs
      }
    }
  } else if (docFreq == 1) {
    docDeltaBuffer[0] = singletonDocID;
    freqBuffer[0] = static_cast<int>(totalTermFreq);
  } else {
    // Read vInts:
    readVIntBlock(docIn, docDeltaBuffer, freqBuffer, left, indexHasFreq);
  }
  docBufferUpto = 0;
}

int Lucene50PostingsReader::BlockDocsEnum::nextDoc() 
{
  if (docUpto == docFreq) {
    return doc = NO_MORE_DOCS;
  }
  if (docBufferUpto == BLOCK_SIZE) {
    refillDocs();
  }

  accum += docDeltaBuffer[docBufferUpto];
  docUpto++;

  doc = accum;
  freq_ = freqBuffer[docBufferUpto];
  docBufferUpto++;
  return doc;
}

int Lucene50PostingsReader::BlockDocsEnum::advance(int target) throw(
    IOException)
{
  // TODO: make frq block load lazy/skippable

  // current skip docID < docIDs generated from current buffer <= next skip
  // docID we don't need to skip if target is buffered already
  if (docFreq > BLOCK_SIZE && target > nextSkipDoc) {

    if (skipper == nullptr) {
      // Lazy init: first time this enum has ever been used for skipping
      skipper = make_shared<Lucene50SkipReader>(docIn->clone(), MAX_SKIP_LEVELS,
                                                indexHasPos, indexHasOffsets,
                                                indexHasPayloads);
    }

    if (!skipped) {
      assert(skipOffset != -1);
      // This is the first time this enum has skipped
      // since reset() was called; load the skip data:
      skipper->init(docTermStartFP + skipOffset, docTermStartFP, 0, 0, docFreq);
      skipped = true;
    }

    // always plus one to fix the result, since skip position in
    // Lucene50SkipReader is a little different from MultiLevelSkipListReader
    constexpr int newDocUpto = skipper->skipTo(target) + 1;

    if (newDocUpto > docUpto) {
      // Skipper moved
      assert((newDocUpto % BLOCK_SIZE == 0, L"got " + to_wstring(newDocUpto)));
      docUpto = newDocUpto;

      // Force to read next block
      docBufferUpto = BLOCK_SIZE;
      accum = skipper->getDoc(); // actually, this is just lastSkipEntry
      docIn->seek(
          skipper->getDocPointer()); // now point to the block we want to search
    }
    // next time we call advance, this is used to
    // foresee whether skipper is necessary.
    nextSkipDoc = skipper->getNextSkipDoc();
  }
  if (docUpto == docFreq) {
    return doc = NO_MORE_DOCS;
  }
  if (docBufferUpto == BLOCK_SIZE) {
    refillDocs();
  }

  // Now scan... this is an inlined/pared down version
  // of nextDoc():
  while (true) {
    accum += docDeltaBuffer[docBufferUpto];
    docUpto++;

    if (accum >= target) {
      break;
    }
    docBufferUpto++;
    if (docUpto == docFreq) {
      return doc = NO_MORE_DOCS;
    }
  }

  freq_ = freqBuffer[docBufferUpto];
  docBufferUpto++;
  return doc = accum;
}

int64_t Lucene50PostingsReader::BlockDocsEnum::cost() { return docFreq; }

Lucene50PostingsReader::BlockPostingsEnum::BlockPostingsEnum(
    shared_ptr<Lucene50PostingsReader> outerInstance,
    shared_ptr<FieldInfo> fieldInfo) 
    : encoded(std::deque<char>(MAX_ENCODED_SIZE)),
      startDocIn(outerInstance->docIn), posIn(outerInstance->posIn->clone()),
      indexHasOffsets(
          fieldInfo->getIndexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0),
      indexHasPayloads(fieldInfo->hasPayloads()), outerInstance(outerInstance)
{
  this->docIn.reset();
}

bool Lucene50PostingsReader::BlockPostingsEnum::canReuse(
    shared_ptr<IndexInput> docIn, shared_ptr<FieldInfo> fieldInfo)
{
  return docIn == startDocIn &&
         indexHasOffsets ==
             (fieldInfo->getIndexOptions().compareTo(
                  IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >=
              0) &&
         indexHasPayloads == fieldInfo->hasPayloads();
}

shared_ptr<PostingsEnum> Lucene50PostingsReader::BlockPostingsEnum::reset(
    shared_ptr<IntBlockTermState> termState) 
{
  docFreq = termState->docFreq;
  docTermStartFP = termState->docStartFP;
  posTermStartFP = termState->posStartFP;
  payTermStartFP = termState->payStartFP;
  skipOffset = termState->skipOffset;
  totalTermFreq = termState->totalTermFreq;
  singletonDocID = termState->singletonDocID;
  if (docFreq > 1) {
    if (docIn == nullptr) {
      // lazy init
      docIn = startDocIn->clone();
    }
    docIn->seek(docTermStartFP);
  }
  posPendingFP = posTermStartFP;
  posPendingCount = 0;
  if (termState->totalTermFreq < BLOCK_SIZE) {
    lastPosBlockFP = posTermStartFP;
  } else if (termState->totalTermFreq == BLOCK_SIZE) {
    lastPosBlockFP = -1;
  } else {
    lastPosBlockFP = posTermStartFP + termState->lastPosBlockOffset;
  }

  doc = -1;
  accum = 0;
  docUpto = 0;
  if (docFreq > BLOCK_SIZE) {
    nextSkipDoc =
        BLOCK_SIZE - 1; // we won't skip if target is found in first block
  } else {
    nextSkipDoc = NO_MORE_DOCS; // not enough docs for skipping
  }
  docBufferUpto = BLOCK_SIZE;
  skipped = false;
  return shared_from_this();
}

int Lucene50PostingsReader::BlockPostingsEnum::freq() 
{
  return freq_;
}

int Lucene50PostingsReader::BlockPostingsEnum::docID() { return doc; }

void Lucene50PostingsReader::BlockPostingsEnum::refillDocs() 
{
  constexpr int left = docFreq - docUpto;
  assert(left > 0);

  if (left >= BLOCK_SIZE) {
    outerInstance->forUtil->readBlock(docIn, encoded, docDeltaBuffer);
    outerInstance->forUtil->readBlock(docIn, encoded, freqBuffer);
  } else if (docFreq == 1) {
    docDeltaBuffer[0] = singletonDocID;
    freqBuffer[0] = static_cast<int>(totalTermFreq);
  } else {
    // Read vInts:
    readVIntBlock(docIn, docDeltaBuffer, freqBuffer, left, true);
  }
  docBufferUpto = 0;
}

void Lucene50PostingsReader::BlockPostingsEnum::refillPositions() throw(
    IOException)
{
  if (posIn->getFilePointer() == lastPosBlockFP) {
    constexpr int count = static_cast<int>(totalTermFreq % BLOCK_SIZE);
    int payloadLength = 0;
    for (int i = 0; i < count; i++) {
      int code = posIn->readVInt();
      if (indexHasPayloads) {
        if ((code & 1) != 0) {
          payloadLength = posIn->readVInt();
        }
        posDeltaBuffer[i] =
            static_cast<int>(static_cast<unsigned int>(code) >> 1);
        if (payloadLength != 0) {
          posIn->seek(posIn->getFilePointer() + payloadLength);
        }
      } else {
        posDeltaBuffer[i] = code;
      }
      if (indexHasOffsets) {
        if ((posIn->readVInt() & 1) != 0) {
          // offset length changed
          posIn->readVInt();
        }
      }
    }
  } else {
    outerInstance->forUtil->readBlock(posIn, encoded, posDeltaBuffer);
  }
}

int Lucene50PostingsReader::BlockPostingsEnum::nextDoc() 
{
  if (docUpto == docFreq) {
    return doc = NO_MORE_DOCS;
  }
  if (docBufferUpto == BLOCK_SIZE) {
    refillDocs();
  }

  accum += docDeltaBuffer[docBufferUpto];
  freq_ = freqBuffer[docBufferUpto];
  posPendingCount += freq_;
  docBufferUpto++;
  docUpto++;

  doc = accum;
  position = 0;
  return doc;
}

int Lucene50PostingsReader::BlockPostingsEnum::advance(int target) throw(
    IOException)
{
  // TODO: make frq block load lazy/skippable

  if (target > nextSkipDoc) {
    if (skipper == nullptr) {
      // Lazy init: first time this enum has ever been used for skipping
      skipper =
          make_shared<Lucene50SkipReader>(docIn->clone(), MAX_SKIP_LEVELS, true,
                                          indexHasOffsets, indexHasPayloads);
    }

    if (!skipped) {
      assert(skipOffset != -1);
      // This is the first time this enum has skipped
      // since reset() was called; load the skip data:
      skipper->init(docTermStartFP + skipOffset, docTermStartFP, posTermStartFP,
                    payTermStartFP, docFreq);
      skipped = true;
    }

    constexpr int newDocUpto = skipper->skipTo(target) + 1;

    if (newDocUpto > docUpto) {
      // Skipper moved

      assert((newDocUpto % BLOCK_SIZE == 0, L"got " + to_wstring(newDocUpto)));
      docUpto = newDocUpto;

      // Force to read next block
      docBufferUpto = BLOCK_SIZE;
      accum = skipper->getDoc();
      docIn->seek(skipper->getDocPointer());
      posPendingFP = skipper->getPosPointer();
      posPendingCount = skipper->getPosBufferUpto();
    }
    nextSkipDoc = skipper->getNextSkipDoc();
  }
  if (docUpto == docFreq) {
    return doc = NO_MORE_DOCS;
  }
  if (docBufferUpto == BLOCK_SIZE) {
    refillDocs();
  }

  // Now scan... this is an inlined/pared down version
  // of nextDoc():
  while (true) {
    accum += docDeltaBuffer[docBufferUpto];
    freq_ = freqBuffer[docBufferUpto];
    posPendingCount += freq_;
    docBufferUpto++;
    docUpto++;

    if (accum >= target) {
      break;
    }
    if (docUpto == docFreq) {
      return doc = NO_MORE_DOCS;
    }
  }

  position = 0;
  return doc = accum;
}

void Lucene50PostingsReader::BlockPostingsEnum::skipPositions() throw(
    IOException)
{
  // Skip positions now:
  int toSkip = posPendingCount - freq_;

  constexpr int leftInBlock = BLOCK_SIZE - posBufferUpto;
  if (toSkip < leftInBlock) {
    posBufferUpto += toSkip;
  } else {
    toSkip -= leftInBlock;
    while (toSkip >= BLOCK_SIZE) {
      assert(posIn->getFilePointer() != lastPosBlockFP);
      outerInstance->forUtil->skipBlock(posIn);
      toSkip -= BLOCK_SIZE;
    }
    refillPositions();
    posBufferUpto = toSkip;
  }

  position = 0;
}

int Lucene50PostingsReader::BlockPostingsEnum::nextPosition() 
{

  assert(posPendingCount > 0);

  if (posPendingFP != -1) {
    posIn->seek(posPendingFP);
    posPendingFP = -1;

    // Force buffer refill:
    posBufferUpto = BLOCK_SIZE;
  }

  if (posPendingCount > freq_) {
    skipPositions();
    posPendingCount = freq_;
  }

  if (posBufferUpto == BLOCK_SIZE) {
    refillPositions();
    posBufferUpto = 0;
  }
  position += posDeltaBuffer[posBufferUpto++];
  posPendingCount--;
  return position;
}

int Lucene50PostingsReader::BlockPostingsEnum::startOffset() { return -1; }

int Lucene50PostingsReader::BlockPostingsEnum::endOffset() { return -1; }

shared_ptr<BytesRef> Lucene50PostingsReader::BlockPostingsEnum::getPayload()
{
  return nullptr;
}

int64_t Lucene50PostingsReader::BlockPostingsEnum::cost() { return docFreq; }

Lucene50PostingsReader::EverythingEnum::EverythingEnum(
    shared_ptr<Lucene50PostingsReader> outerInstance,
    shared_ptr<FieldInfo> fieldInfo) 
    : encoded(std::deque<char>(MAX_ENCODED_SIZE)),
      startDocIn(outerInstance->docIn), posIn(outerInstance->posIn->clone()),
      payIn(outerInstance->payIn->clone()),
      indexHasOffsets(
          fieldInfo->getIndexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0),
      indexHasPayloads(fieldInfo->hasPayloads()), outerInstance(outerInstance)
{
  this->docIn.reset();
  if (indexHasOffsets) {
    offsetStartDeltaBuffer = std::deque<int>(MAX_DATA_SIZE);
    offsetLengthBuffer = std::deque<int>(MAX_DATA_SIZE);
  } else {
    offsetStartDeltaBuffer.clear();
    offsetLengthBuffer.clear();
    startOffset_ = -1;
    endOffset_ = -1;
  }

  if (indexHasPayloads) {
    payloadLengthBuffer = std::deque<int>(MAX_DATA_SIZE);
    payloadBytes = std::deque<char>(128);
    payload = make_shared<BytesRef>();
  } else {
    payloadLengthBuffer.clear();
    payloadBytes.clear();
    payload.reset();
  }
}

bool Lucene50PostingsReader::EverythingEnum::canReuse(
    shared_ptr<IndexInput> docIn, shared_ptr<FieldInfo> fieldInfo)
{
  return docIn == startDocIn &&
         indexHasOffsets ==
             (fieldInfo->getIndexOptions().compareTo(
                  IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >=
              0) &&
         indexHasPayloads == fieldInfo->hasPayloads();
}

shared_ptr<EverythingEnum> Lucene50PostingsReader::EverythingEnum::reset(
    shared_ptr<IntBlockTermState> termState, int flags) 
{
  docFreq = termState->docFreq;
  docTermStartFP = termState->docStartFP;
  posTermStartFP = termState->posStartFP;
  payTermStartFP = termState->payStartFP;
  skipOffset = termState->skipOffset;
  totalTermFreq = termState->totalTermFreq;
  singletonDocID = termState->singletonDocID;
  if (docFreq > 1) {
    if (docIn == nullptr) {
      // lazy init
      docIn = startDocIn->clone();
    }
    docIn->seek(docTermStartFP);
  }
  posPendingFP = posTermStartFP;
  payPendingFP = payTermStartFP;
  posPendingCount = 0;
  if (termState->totalTermFreq < BLOCK_SIZE) {
    lastPosBlockFP = posTermStartFP;
  } else if (termState->totalTermFreq == BLOCK_SIZE) {
    lastPosBlockFP = -1;
  } else {
    lastPosBlockFP = posTermStartFP + termState->lastPosBlockOffset;
  }

  this->needsOffsets =
      PostingsEnum::featureRequested(flags, PostingsEnum::OFFSETS);
  this->needsPayloads =
      PostingsEnum::featureRequested(flags, PostingsEnum::PAYLOADS);

  doc = -1;
  accum = 0;
  docUpto = 0;
  if (docFreq > BLOCK_SIZE) {
    nextSkipDoc =
        BLOCK_SIZE - 1; // we won't skip if target is found in first block
  } else {
    nextSkipDoc = NO_MORE_DOCS; // not enough docs for skipping
  }
  docBufferUpto = BLOCK_SIZE;
  skipped = false;
  return shared_from_this();
}

int Lucene50PostingsReader::EverythingEnum::freq() 
{
  return freq_;
}

int Lucene50PostingsReader::EverythingEnum::docID() { return doc; }

void Lucene50PostingsReader::EverythingEnum::refillDocs() 
{
  constexpr int left = docFreq - docUpto;
  assert(left > 0);

  if (left >= BLOCK_SIZE) {
    outerInstance->forUtil->readBlock(docIn, encoded, docDeltaBuffer);
    outerInstance->forUtil->readBlock(docIn, encoded, freqBuffer);
  } else if (docFreq == 1) {
    docDeltaBuffer[0] = singletonDocID;
    freqBuffer[0] = static_cast<int>(totalTermFreq);
  } else {
    readVIntBlock(docIn, docDeltaBuffer, freqBuffer, left, true);
  }
  docBufferUpto = 0;
}

void Lucene50PostingsReader::EverythingEnum::refillPositions() throw(
    IOException)
{
  if (posIn->getFilePointer() == lastPosBlockFP) {
    constexpr int count = static_cast<int>(totalTermFreq % BLOCK_SIZE);
    int payloadLength = 0;
    int offsetLength = 0;
    payloadByteUpto = 0;
    for (int i = 0; i < count; i++) {
      int code = posIn->readVInt();
      if (indexHasPayloads) {
        if ((code & 1) != 0) {
          payloadLength = posIn->readVInt();
        }
        payloadLengthBuffer[i] = payloadLength;
        posDeltaBuffer[i] =
            static_cast<int>(static_cast<unsigned int>(code) >> 1);
        if (payloadLength != 0) {
          if (payloadByteUpto + payloadLength > payloadBytes.size()) {
            payloadBytes =
                ArrayUtil::grow(payloadBytes, payloadByteUpto + payloadLength);
          }
          posIn->readBytes(payloadBytes, payloadByteUpto, payloadLength);
          payloadByteUpto += payloadLength;
        }
      } else {
        posDeltaBuffer[i] = code;
      }

      if (indexHasOffsets) {
        int deltaCode = posIn->readVInt();
        if ((deltaCode & 1) != 0) {
          offsetLength = posIn->readVInt();
        }
        offsetStartDeltaBuffer[i] =
            static_cast<int>(static_cast<unsigned int>(deltaCode) >> 1);
        offsetLengthBuffer[i] = offsetLength;
      }
    }
    payloadByteUpto = 0;
  } else {
    outerInstance->forUtil->readBlock(posIn, encoded, posDeltaBuffer);

    if (indexHasPayloads) {
      if (needsPayloads) {
        outerInstance->forUtil->readBlock(payIn, encoded, payloadLengthBuffer);
        int numBytes = payIn->readVInt();

        if (numBytes > payloadBytes.size()) {
          payloadBytes = ArrayUtil::grow(payloadBytes, numBytes);
        }
        payIn->readBytes(payloadBytes, 0, numBytes);
      } else {
        // this works, because when writing a vint block we always force the
        // first length to be written
        outerInstance->forUtil->skipBlock(payIn); // skip over lengths
        int numBytes = payIn->readVInt();         // read length of payloadBytes
        payIn->seek(payIn->getFilePointer() +
                    numBytes); // skip over payloadBytes
      }
      payloadByteUpto = 0;
    }

    if (indexHasOffsets) {
      if (needsOffsets) {
        outerInstance->forUtil->readBlock(payIn, encoded,
                                          offsetStartDeltaBuffer);
        outerInstance->forUtil->readBlock(payIn, encoded, offsetLengthBuffer);
      } else {
        // this works, because when writing a vint block we always force the
        // first length to be written
        outerInstance->forUtil->skipBlock(payIn); // skip over starts
        outerInstance->forUtil->skipBlock(payIn); // skip over lengths
      }
    }
  }
}

int Lucene50PostingsReader::EverythingEnum::nextDoc() 
{
  if (docUpto == docFreq) {
    return doc = NO_MORE_DOCS;
  }
  if (docBufferUpto == BLOCK_SIZE) {
    refillDocs();
  }

  accum += docDeltaBuffer[docBufferUpto];
  freq_ = freqBuffer[docBufferUpto];
  posPendingCount += freq_;
  docBufferUpto++;
  docUpto++;

  doc = accum;
  position = 0;
  lastStartOffset = 0;
  return doc;
}

int Lucene50PostingsReader::EverythingEnum::advance(int target) throw(
    IOException)
{
  // TODO: make frq block load lazy/skippable

  if (target > nextSkipDoc) {
    if (skipper == nullptr) {
      // Lazy init: first time this enum has ever been used for skipping
      skipper =
          make_shared<Lucene50SkipReader>(docIn->clone(), MAX_SKIP_LEVELS, true,
                                          indexHasOffsets, indexHasPayloads);
    }

    if (!skipped) {
      assert(skipOffset != -1);
      // This is the first time this enum has skipped
      // since reset() was called; load the skip data:
      skipper->init(docTermStartFP + skipOffset, docTermStartFP, posTermStartFP,
                    payTermStartFP, docFreq);
      skipped = true;
    }

    constexpr int newDocUpto = skipper->skipTo(target) + 1;

    if (newDocUpto > docUpto) {
      // Skipper moved
      assert((newDocUpto % BLOCK_SIZE == 0, L"got " + to_wstring(newDocUpto)));
      docUpto = newDocUpto;

      // Force to read next block
      docBufferUpto = BLOCK_SIZE;
      accum = skipper->getDoc();
      docIn->seek(skipper->getDocPointer());
      posPendingFP = skipper->getPosPointer();
      payPendingFP = skipper->getPayPointer();
      posPendingCount = skipper->getPosBufferUpto();
      lastStartOffset = 0; // new document
      payloadByteUpto = skipper->getPayloadByteUpto();
    }
    nextSkipDoc = skipper->getNextSkipDoc();
  }
  if (docUpto == docFreq) {
    return doc = NO_MORE_DOCS;
  }
  if (docBufferUpto == BLOCK_SIZE) {
    refillDocs();
  }

  // Now scan:
  while (true) {
    accum += docDeltaBuffer[docBufferUpto];
    freq_ = freqBuffer[docBufferUpto];
    posPendingCount += freq_;
    docBufferUpto++;
    docUpto++;

    if (accum >= target) {
      break;
    }
    if (docUpto == docFreq) {
      return doc = NO_MORE_DOCS;
    }
  }

  position = 0;
  lastStartOffset = 0;
  return doc = accum;
}

void Lucene50PostingsReader::EverythingEnum::skipPositions() 
{
  // Skip positions now:
  int toSkip = posPendingCount - freq_;
  // if (DEBUG) {
  //   System.out.println("      FPR.skipPositions: toSkip=" + toSkip);
  // }

  constexpr int leftInBlock = BLOCK_SIZE - posBufferUpto;
  if (toSkip < leftInBlock) {
    int end = posBufferUpto + toSkip;
    while (posBufferUpto < end) {
      if (indexHasPayloads) {
        payloadByteUpto += payloadLengthBuffer[posBufferUpto];
      }
      posBufferUpto++;
    }
  } else {
    toSkip -= leftInBlock;
    while (toSkip >= BLOCK_SIZE) {
      assert(posIn->getFilePointer() != lastPosBlockFP);
      outerInstance->forUtil->skipBlock(posIn);

      if (indexHasPayloads) {
        // Skip payloadLength block:
        outerInstance->forUtil->skipBlock(payIn);

        // Skip payloadBytes block:
        int numBytes = payIn->readVInt();
        payIn->seek(payIn->getFilePointer() + numBytes);
      }

      if (indexHasOffsets) {
        outerInstance->forUtil->skipBlock(payIn);
        outerInstance->forUtil->skipBlock(payIn);
      }
      toSkip -= BLOCK_SIZE;
    }
    refillPositions();
    payloadByteUpto = 0;
    posBufferUpto = 0;
    while (posBufferUpto < toSkip) {
      if (indexHasPayloads) {
        payloadByteUpto += payloadLengthBuffer[posBufferUpto];
      }
      posBufferUpto++;
    }
  }

  position = 0;
  lastStartOffset = 0;
}

int Lucene50PostingsReader::EverythingEnum::nextPosition() 
{
  assert(posPendingCount > 0);

  if (posPendingFP != -1) {
    posIn->seek(posPendingFP);
    posPendingFP = -1;

    if (payPendingFP != -1) {
      payIn->seek(payPendingFP);
      payPendingFP = -1;
    }

    // Force buffer refill:
    posBufferUpto = BLOCK_SIZE;
  }

  if (posPendingCount > freq_) {
    skipPositions();
    posPendingCount = freq_;
  }

  if (posBufferUpto == BLOCK_SIZE) {
    refillPositions();
    posBufferUpto = 0;
  }
  position += posDeltaBuffer[posBufferUpto];

  if (indexHasPayloads) {
    payloadLength = payloadLengthBuffer[posBufferUpto];
    payload->bytes = payloadBytes;
    payload->offset = payloadByteUpto;
    payload->length = payloadLength;
    payloadByteUpto += payloadLength;
  }

  if (indexHasOffsets) {
    startOffset_ = lastStartOffset + offsetStartDeltaBuffer[posBufferUpto];
    endOffset_ = startOffset_ + offsetLengthBuffer[posBufferUpto];
    lastStartOffset = startOffset_;
  }

  posBufferUpto++;
  posPendingCount--;
  return position;
}

int Lucene50PostingsReader::EverythingEnum::startOffset()
{
  return startOffset_;
}

int Lucene50PostingsReader::EverythingEnum::endOffset() { return endOffset_; }

shared_ptr<BytesRef> Lucene50PostingsReader::EverythingEnum::getPayload()
{
  if (payloadLength == 0) {
    return nullptr;
  } else {
    return payload;
  }
}

int64_t Lucene50PostingsReader::EverythingEnum::cost() { return docFreq; }

int64_t Lucene50PostingsReader::ramBytesUsed() { return BASE_RAM_BYTES_USED; }

void Lucene50PostingsReader::checkIntegrity() 
{
  if (docIn != nullptr) {
    CodecUtil::checksumEntireFile(docIn);
  }
  if (posIn != nullptr) {
    CodecUtil::checksumEntireFile(posIn);
  }
  if (payIn != nullptr) {
    CodecUtil::checksumEntireFile(payIn);
  }
}

wstring Lucene50PostingsReader::toString()
{
  return getClass().getSimpleName() + L"(positions=" +
         StringHelper::toString(posIn != nullptr) + L",payloads=" +
         StringHelper::toString(payIn != nullptr) + L")";
}
} // namespace org::apache::lucene::codecs::lucene50