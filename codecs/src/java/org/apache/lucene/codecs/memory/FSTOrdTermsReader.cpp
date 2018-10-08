using namespace std;

#include "FSTOrdTermsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsReaderBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/ByteRunAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::codecs::memory
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using ByteRunAutomaton = org::apache::lucene::util::automaton::ByteRunAutomaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using org::apache::lucene::util::fst::BytesRefFSTEnum::InputOutput;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;

FSTOrdTermsReader::FSTOrdTermsReader(
    shared_ptr<SegmentReadState> state,
    shared_ptr<PostingsReaderBase> postingsReader) 
    : postingsReader(postingsReader)
{
  const wstring termsIndexFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      FSTOrdTermsWriter::TERMS_INDEX_EXTENSION);
  const wstring termsBlockFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      FSTOrdTermsWriter::TERMS_BLOCK_EXTENSION);

  shared_ptr<ChecksumIndexInput> indexIn = nullptr;
  shared_ptr<IndexInput> blockIn = nullptr;
  bool success = false;
  try {
    indexIn =
        state->directory->openChecksumInput(termsIndexFileName, state->context);
    blockIn = state->directory->openInput(termsBlockFileName, state->context);
    int version = CodecUtil::checkIndexHeader(
        indexIn, FSTOrdTermsWriter::TERMS_INDEX_CODEC_NAME,
        FSTOrdTermsWriter::VERSION_START, FSTOrdTermsWriter::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    int version2 = CodecUtil::checkIndexHeader(
        blockIn, FSTOrdTermsWriter::TERMS_CODEC_NAME,
        FSTOrdTermsWriter::VERSION_START, FSTOrdTermsWriter::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);

    if (version != version2) {
      throw make_shared<CorruptIndexException>(
          L"Format versions mismatch: index=" + to_wstring(version) +
              L", terms=" + to_wstring(version2),
          blockIn);
    }

    CodecUtil::checksumEntireFile(blockIn);

    this->postingsReader->init(blockIn, state);
    seekDir(blockIn);

    shared_ptr<FieldInfos> *const fieldInfos = state->fieldInfos;
    constexpr int numFields = blockIn->readVInt();
    for (int i = 0; i < numFields; i++) {
      shared_ptr<FieldInfo> fieldInfo =
          fieldInfos->fieldInfo(blockIn->readVInt());
      bool hasFreq = fieldInfo->getIndexOptions() != IndexOptions::DOCS;
      int64_t numTerms = blockIn->readVLong();
      int64_t sumTotalTermFreq = hasFreq ? blockIn->readVLong() : -1;
      int64_t sumDocFreq = blockIn->readVLong();
      int docCount = blockIn->readVInt();
      int longsSize = blockIn->readVInt();
      shared_ptr<FST<int64_t>> index = make_shared<FST<int64_t>>(
          indexIn, PositiveIntOutputs::getSingleton());

      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<TermsReader> current = make_shared<TermsReader>(
          shared_from_this(), fieldInfo, blockIn, numTerms, sumTotalTermFreq,
          sumDocFreq, docCount, longsSize, index);
      shared_ptr<TermsReader> previous =
          fields.emplace(fieldInfo->name, current);
      checkFieldSummary(state->segmentInfo, indexIn, blockIn, current,
                        previous);
    }
    CodecUtil::checkFooter(indexIn);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({indexIn, blockIn});
    } else {
      IOUtils::closeWhileHandlingException({indexIn, blockIn});
    }
  }
}

void FSTOrdTermsReader::seekDir(shared_ptr<IndexInput> in_) 
{
  in_->seek(in_->length() - CodecUtil::footerLength() - 8);
  in_->seek(in_->readLong());
}

void FSTOrdTermsReader::checkFieldSummary(
    shared_ptr<SegmentInfo> info, shared_ptr<IndexInput> indexIn,
    shared_ptr<IndexInput> blockIn, shared_ptr<TermsReader> field,
    shared_ptr<TermsReader> previous) 
{
  // #docs with field must be <= #docs
  if (field->docCount < 0 || field->docCount > info->maxDoc()) {
    throw make_shared<CorruptIndexException>(
        L"invalid docCount: " + to_wstring(field->docCount) + L" maxDoc: " +
            to_wstring(info->maxDoc()) + L" (blockIn=" + blockIn + L")",
        indexIn);
  }
  // #postings must be >= #docs with field
  if (field->sumDocFreq < field->docCount) {
    throw make_shared<CorruptIndexException>(
        L"invalid sumDocFreq: " + to_wstring(field->sumDocFreq) +
            L" docCount: " + to_wstring(field->docCount) + L" (blockIn=" +
            blockIn + L")",
        indexIn);
  }
  // #positions must be >= #postings
  if (field->sumTotalTermFreq != -1 &&
      field->sumTotalTermFreq < field->sumDocFreq) {
    throw make_shared<CorruptIndexException>(
        L"invalid sumTotalTermFreq: " + to_wstring(field->sumTotalTermFreq) +
            L" sumDocFreq: " + to_wstring(field->sumDocFreq) + L" (blockIn=" +
            blockIn + L")",
        indexIn);
  }
  if (previous != nullptr) {
    throw make_shared<CorruptIndexException>(L"duplicate fields: " +
                                                 field->fieldInfo->name +
                                                 L" (blockIn=" + blockIn + L")",
                                             indexIn);
  }
}

shared_ptr<Iterator<wstring>> FSTOrdTermsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms>
FSTOrdTermsReader::terms(const wstring &field) 
{
  assert(field != L"");
  return fields[field];
}

int FSTOrdTermsReader::size() { return fields.size(); }

FSTOrdTermsReader::~FSTOrdTermsReader()
{
  try {
    IOUtils::close({postingsReader});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    fields.clear();
  }
}

FSTOrdTermsReader::TermsReader::TermsReader(
    shared_ptr<FSTOrdTermsReader> outerInstance,
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<IndexInput> blockIn,
    int64_t numTerms, int64_t sumTotalTermFreq, int64_t sumDocFreq,
    int docCount, int longsSize,
    shared_ptr<FST<int64_t>> index) 
    : fieldInfo(fieldInfo), numTerms(numTerms),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq),
      docCount(docCount), longsSize(longsSize), index(index),
      numSkipInfo(longsSize + 3),
      skipInfo(std::deque<int64_t>(numBlocks * numSkipInfo)),
      statsBlock(std::deque<char>(static_cast<int>(blockIn->readVLong()))),
      metaLongsBlock(std::deque<char>(static_cast<int>(blockIn->readVLong()))),
      metaBytesBlock(std::deque<char>(static_cast<int>(blockIn->readVLong()))),
      outerInstance(outerInstance)
{

  assert(numTerms & (~0xffffffffLL)) == 0;
  constexpr int numBlocks =
      static_cast<int>(numTerms + INTERVAL - 1) / INTERVAL;

  int last = 0, next = 0;
  for (int i = 1; i < numBlocks; i++) {
    next = numSkipInfo * i;
    for (int j = 0; j < numSkipInfo; j++) {
      skipInfo[next + j] = skipInfo[last + j] + blockIn->readVLong();
    }
    last = next;
  }
  blockIn->readBytes(statsBlock, 0, statsBlock.size());
  blockIn->readBytes(metaLongsBlock, 0, metaLongsBlock.size());
  blockIn->readBytes(metaBytesBlock, 0, metaBytesBlock.size());
}

bool FSTOrdTermsReader::TermsReader::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool FSTOrdTermsReader::TermsReader::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool FSTOrdTermsReader::TermsReader::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool FSTOrdTermsReader::TermsReader::hasPayloads()
{
  return fieldInfo->hasPayloads();
}

int64_t FSTOrdTermsReader::TermsReader::size() { return numTerms; }

int64_t FSTOrdTermsReader::TermsReader::getSumTotalTermFreq()
{
  return sumTotalTermFreq;
}

int64_t FSTOrdTermsReader::TermsReader::getSumDocFreq() 
{
  return sumDocFreq;
}

int FSTOrdTermsReader::TermsReader::getDocCount() 
{
  return docCount;
}

shared_ptr<TermsEnum>
FSTOrdTermsReader::TermsReader::iterator() 
{
  return make_shared<SegmentTermsEnum>(shared_from_this());
}

shared_ptr<TermsEnum> FSTOrdTermsReader::TermsReader::intersect(
    shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
{
  if (compiled->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
    throw invalid_argument(
        L"please use CompiledAutomaton.getTermsEnum instead");
  }
  return make_shared<IntersectTermsEnum>(shared_from_this(), compiled,
                                         startTerm);
}

int64_t FSTOrdTermsReader::TermsReader::ramBytesUsed()
{
  int64_t ramBytesUsed = 0;
  if (index != nullptr) {
    ramBytesUsed += index->ramBytesUsed();
    ramBytesUsed += RamUsageEstimator::sizeOf(metaBytesBlock);
    ramBytesUsed += RamUsageEstimator::sizeOf(metaLongsBlock);
    ramBytesUsed += RamUsageEstimator::sizeOf(skipInfo);
    ramBytesUsed += RamUsageEstimator::sizeOf(statsBlock);
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FSTOrdTermsReader::TermsReader::getChildResources()
{
  if (index == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"terms", index));
  }
}

wstring FSTOrdTermsReader::TermsReader::toString()
{
  return L"FSTOrdTerms(terms=" + to_wstring(numTerms) + L",postings=" +
         to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}

FSTOrdTermsReader::TermsReader::BaseTermsEnum::BaseTermsEnum(
    shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance) 
    : state(outerInstance->outerInstance->postingsReader.newTermState()),
      outerInstance(outerInstance)
{
  this->statsReader->reset(outerInstance->statsBlock);
  this->metaLongsReader->reset(outerInstance->metaLongsBlock);
  this->metaBytesReader->reset(outerInstance->metaBytesBlock);

  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: this.longs = new long[INTERVAL][longsSize];
  this->longs = RectangularVectors::ReturnRectangularTangibleTemplonglongVector(
      INTERVAL, outerInstance->longsSize);
  this->bytesStart = std::deque<int>(INTERVAL);
  this->bytesLength = std::deque<int>(INTERVAL);
  this->docFreq_ = std::deque<int>(INTERVAL);
  this->totalTermFreq_ = std::deque<int64_t>(INTERVAL);
  this->statsBlockOrd = -1;
  this->metaBlockOrd = -1;
  if (!outerInstance->hasFreqs()) {
    Arrays::fill(totalTermFreq_, -1);
  }
}

void FSTOrdTermsReader::TermsReader::BaseTermsEnum::decodeStats() throw(
    IOException)
{
  constexpr int upto = static_cast<int>(ord_) % INTERVAL;
  constexpr int oldBlockOrd = statsBlockOrd;
  statsBlockOrd = static_cast<int>(ord_) / INTERVAL;
  if (oldBlockOrd != statsBlockOrd) {
    refillStats();
  }
  state->docFreq = docFreq_[upto];
  state->totalTermFreq = totalTermFreq_[upto];
}

void FSTOrdTermsReader::TermsReader::BaseTermsEnum::decodeMetaData() throw(
    IOException)
{
  constexpr int upto = static_cast<int>(ord_) % INTERVAL;
  constexpr int oldBlockOrd = metaBlockOrd;
  metaBlockOrd = static_cast<int>(ord_) / INTERVAL;
  if (metaBlockOrd != oldBlockOrd) {
    refillMetadata();
  }
  metaBytesReader->setPosition(bytesStart[upto]);
  outerInstance->outerInstance->postingsReader.decodeTerm(
      longs[upto], metaBytesReader, outerInstance->fieldInfo, state, true);
}

void FSTOrdTermsReader::TermsReader::BaseTermsEnum::refillStats() throw(
    IOException)
{
  constexpr int offset = statsBlockOrd * outerInstance->numSkipInfo;
  constexpr int statsFP = static_cast<int>(outerInstance->skipInfo[offset]);
  statsReader->setPosition(statsFP);
  for (int i = 0; i < INTERVAL && !statsReader->eof(); i++) {
    int code = statsReader->readVInt();
    if (outerInstance->hasFreqs()) {
      docFreq_[i] = (static_cast<int>(static_cast<unsigned int>(code) >> 1));
      if ((code & 1) == 1) {
        totalTermFreq_[i] = docFreq_[i];
      } else {
        totalTermFreq_[i] = docFreq_[i] + statsReader->readVLong();
      }
    } else {
      docFreq_[i] = code;
    }
  }
}

void FSTOrdTermsReader::TermsReader::BaseTermsEnum::refillMetadata() throw(
    IOException)
{
  constexpr int offset = metaBlockOrd * outerInstance->numSkipInfo;
  constexpr int metaLongsFP =
      static_cast<int>(outerInstance->skipInfo[offset + 1]);
  constexpr int metaBytesFP =
      static_cast<int>(outerInstance->skipInfo[offset + 2]);
  metaLongsReader->setPosition(metaLongsFP);
  for (int j = 0; j < outerInstance->longsSize; j++) {
    longs[0][j] =
        outerInstance->skipInfo[offset + 3 + j] + metaLongsReader->readVLong();
  }
  bytesStart[0] = metaBytesFP;
  bytesLength[0] = static_cast<int>(metaLongsReader->readVLong());
  for (int i = 1; i < INTERVAL && !metaLongsReader->eof(); i++) {
    for (int j = 0; j < outerInstance->longsSize; j++) {
      longs[i][j] = longs[i - 1][j] + metaLongsReader->readVLong();
    }
    bytesStart[i] = bytesStart[i - 1] + bytesLength[i - 1];
    bytesLength[i] = static_cast<int>(metaLongsReader->readVLong());
  }
}

shared_ptr<TermState>
FSTOrdTermsReader::TermsReader::BaseTermsEnum::termState() 
{
  decodeMetaData();
  return state->clone();
}

int FSTOrdTermsReader::TermsReader::BaseTermsEnum::docFreq() 
{
  return state->docFreq;
}

int64_t FSTOrdTermsReader::TermsReader::BaseTermsEnum::totalTermFreq() throw(
    IOException)
{
  return state->totalTermFreq;
}

shared_ptr<PostingsEnum>
FSTOrdTermsReader::TermsReader::BaseTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  decodeMetaData();
  return outerInstance->outerInstance->postingsReader.postings(
      outerInstance->fieldInfo, state, reuse, flags);
}

void FSTOrdTermsReader::TermsReader::BaseTermsEnum::seekExact(
    int64_t ord) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t FSTOrdTermsReader::TermsReader::BaseTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

FSTOrdTermsReader::TermsReader::SegmentTermsEnum::SegmentTermsEnum(
    shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance) 
    : BaseTermsEnum(outerInstance),
      fstEnum(make_shared<BytesRefFSTEnum<>>(outerInstance->index)),
      outerInstance(outerInstance)
{
  this->decoded = false;
  this->seekPending = false;
}

shared_ptr<BytesRef>
FSTOrdTermsReader::TermsReader::SegmentTermsEnum::term() 
{
  return term_;
}

void FSTOrdTermsReader::TermsReader::SegmentTermsEnum::decodeMetaData() throw(
    IOException)
{
  if (!decoded && !seekPending) {
    BaseTermsEnum::decodeMetaData();
    decoded = true;
  }
}

void FSTOrdTermsReader::TermsReader::SegmentTermsEnum::updateEnum(
    shared_ptr<BytesRefFSTEnum::InputOutput<int64_t>> pair) 
{
  if (pair == nullptr) {
    term_.reset();
  } else {
    term_ = pair->input;
    ord_ = pair->output;
    decodeStats();
  }
  decoded = false;
  seekPending = false;
}

shared_ptr<BytesRef>
FSTOrdTermsReader::TermsReader::SegmentTermsEnum::next() 
{
  if (seekPending) { // previously positioned, but termOutputs not fetched
    seekPending = false;
    SeekStatus status = seekCeil(term_);
    assert(status == SeekStatus::FOUND); // must positioned on valid term
  }
  updateEnum(fstEnum->next());
  return term_;
}

bool FSTOrdTermsReader::TermsReader::SegmentTermsEnum::seekExact(
    shared_ptr<BytesRef> target) 
{
  updateEnum(fstEnum->seekExact(target));
  return term_ != nullptr;
}

SeekStatus FSTOrdTermsReader::TermsReader::SegmentTermsEnum::seekCeil(
    shared_ptr<BytesRef> target) 
{
  updateEnum(fstEnum->seekCeil(target));
  if (term_ == nullptr) {
    return SeekStatus::END;
  } else {
    return term_->equals(target) ? SeekStatus::FOUND : SeekStatus::NOT_FOUND;
  }
}

void FSTOrdTermsReader::TermsReader::SegmentTermsEnum::seekExact(
    shared_ptr<BytesRef> target, shared_ptr<TermState> otherState)
{
  if (!target->equals(term_)) {
    state->copyFrom(otherState);
    term_ = BytesRef::deepCopyOf(target);
    seekPending = true;
  }
}

FSTOrdTermsReader::TermsReader::IntersectTermsEnum::Frame::Frame(
    shared_ptr<FSTOrdTermsReader::TermsReader::IntersectTermsEnum>
        outerInstance)
    : outerInstance(outerInstance)
{
  this->arc = make_shared<FST::Arc<int64_t>>();
  this->state = -1;
}

wstring FSTOrdTermsReader::TermsReader::IntersectTermsEnum::Frame::toString()
{
  return L"arc=" + arc + L" state=" + to_wstring(state);
}

FSTOrdTermsReader::TermsReader::IntersectTermsEnum::IntersectTermsEnum(
    shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance,
    shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
    : BaseTermsEnum(outerInstance), fst(outerInstance->index),
      fstReader(fst->getBytesReader()),
      fstOutputs(outerInstance->index->outputs), fsa(compiled->runAutomaton),
      outerInstance(outerInstance)
{
  // if (TEST) System.out.println("Enum init, startTerm=" + startTerm);
  this->level = -1;
  this->stack = std::deque<std::shared_ptr<Frame>>(16);
  for (int i = 0; i < stack.size(); i++) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    this->stack[i] = make_shared<Frame>(shared_from_this());
  }

  shared_ptr<Frame> frame;
  frame = loadVirtualFrame(newFrame());
  this->level++;
  frame = loadFirstFrame(newFrame());
  pushFrame(frame);

  this->decoded = false;
  this->pending = false;

  if (startTerm == nullptr) {
    pending = isAccept(topFrame());
  } else {
    doSeekCeil(startTerm);
    pending = (term_ == nullptr || !startTerm->equals(term_->get())) &&
              isValid(topFrame()) && isAccept(topFrame());
  }
}

shared_ptr<BytesRef>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::term() 
{
  return term_ == nullptr ? nullptr : term_->get();
}

void FSTOrdTermsReader::TermsReader::IntersectTermsEnum::decodeMetaData() throw(
    IOException)
{
  if (!decoded) {
    BaseTermsEnum::decodeMetaData();
    decoded = true;
  }
}

void FSTOrdTermsReader::TermsReader::IntersectTermsEnum::decodeStats() throw(
    IOException)
{
  shared_ptr<FST::Arc<int64_t>> *const arc = topFrame()->arc;
  assert(arc->nextFinalOutput == fstOutputs->getNoOutput());
  ord_ = arc->output;
  BaseTermsEnum::decodeStats();
}

SeekStatus FSTOrdTermsReader::TermsReader::IntersectTermsEnum::seekCeil(
    shared_ptr<BytesRef> target) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::next() 
{
  // if (TEST) System.out.println("Enum next()");
  if (pending) {
    pending = false;
    decodeStats();
    return term();
  }
  decoded = false;
  while (level > 0) {
    shared_ptr<Frame> frame = newFrame();
    if (loadExpandFrame(topFrame(), frame) != nullptr) { // has valid target
      pushFrame(frame);
      if (isAccept(frame)) { // gotcha
        break;
      }
      continue; // check next target
    }
    frame = popFrame();
    while (level > 0) {
      if (loadNextFrame(topFrame(), frame) != nullptr) { // has valid sibling
        pushFrame(frame);
        if (isAccept(frame)) { // gotcha
          goto DFSBreak;
        }
        goto DFSContinue; // check next target
      }
      frame = popFrame();
    }
    return nullptr;
  DFSContinue:;
  }
DFSBreak:
  decodeStats();
  return term();
}

shared_ptr<BytesRef>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::doSeekCeil(
    shared_ptr<BytesRef> target) 
{
  // if (TEST) System.out.println("Enum doSeekCeil()");
  shared_ptr<Frame> frame = nullptr;
  int label, upto = 0, limit = target->length;
  while (upto < limit) { // to target prefix, or ceil label (rewind prefix)
    frame = newFrame();
    label = target->bytes[upto] & 0xff;
    frame = loadCeilFrame(label, topFrame(), frame);
    if (frame == nullptr || frame->arc->label != label) {
      break;
    }
    assert(isValid(frame)); // target must be fetched from automaton
    pushFrame(frame);
    upto++;
  }
  if (upto == limit) { // got target
    return term();
  }
  if (frame != nullptr) { // got larger term('s prefix)
    pushFrame(frame);
    return isAccept(frame) ? term() : next();
  }
  while (level > 0) { // got target's prefix, advance to larger term
    frame = popFrame();
    while (level > 0 && !canRewind(frame)) {
      frame = popFrame();
    }
    if (loadNextFrame(topFrame(), frame) != nullptr) {
      pushFrame(frame);
      return isAccept(frame) ? term() : next();
    }
  }
  return nullptr;
}

shared_ptr<Frame>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::loadVirtualFrame(
    shared_ptr<Frame> frame) 
{
  frame->arc->output = fstOutputs->getNoOutput();
  frame->arc->nextFinalOutput = fstOutputs->getNoOutput();
  frame->state = -1;
  return frame;
}

shared_ptr<Frame>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::loadFirstFrame(
    shared_ptr<Frame> frame) 
{
  frame->arc = fst->getFirstArc(frame->arc);
  frame->state = 0;
  return frame;
}

shared_ptr<Frame>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::loadExpandFrame(
    shared_ptr<Frame> top, shared_ptr<Frame> frame) 
{
  if (!canGrow(top)) {
    return nullptr;
  }
  frame->arc =
      fst->readFirstRealTargetArc(top->arc->target, frame->arc, fstReader);
  frame->state = fsa->step(top->state, frame->arc->label);
  // if (TEST) System.out.println(" loadExpand frame="+frame);
  if (frame->state == -1) {
    return loadNextFrame(top, frame);
  }
  return frame;
}

shared_ptr<Frame>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::loadNextFrame(
    shared_ptr<Frame> top, shared_ptr<Frame> frame) 
{
  if (!canRewind(frame)) {
    return nullptr;
  }
  while (!frame->arc->isLast()) {
    frame->arc = fst->readNextRealArc(frame->arc, fstReader);
    frame->state = fsa->step(top->state, frame->arc->label);
    if (frame->state != -1) {
      break;
    }
  }
  // if (TEST) System.out.println(" loadNext frame="+frame);
  if (frame->state == -1) {
    return nullptr;
  }
  return frame;
}

shared_ptr<Frame>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::loadCeilFrame(
    int label, shared_ptr<Frame> top,
    shared_ptr<Frame> frame) 
{
  shared_ptr<FST::Arc<int64_t>> arc = frame->arc;
  arc = Util::readCeilArc(label, fst, top->arc, arc, fstReader);
  if (arc == nullptr) {
    return nullptr;
  }
  frame->state = fsa->step(top->state, arc->label);
  // if (TEST) System.out.println(" loadCeil frame="+frame);
  if (frame->state == -1) {
    return loadNextFrame(top, frame);
  }
  return frame;
}

bool FSTOrdTermsReader::TermsReader::IntersectTermsEnum::isAccept(
    shared_ptr<Frame> frame)
{ // reach a term both fst&fsa accepts
  return fsa->isAccept(frame->state) && frame->arc->isFinal();
}

bool FSTOrdTermsReader::TermsReader::IntersectTermsEnum::isValid(
    shared_ptr<Frame> frame)
{ // reach a prefix both fst&fsa won't reject
  return frame->state != -1;
}

bool FSTOrdTermsReader::TermsReader::IntersectTermsEnum::canGrow(
    shared_ptr<Frame> frame)
{ // can walk forward on both fst&fsa
  return frame->state != -1 && FST::targetHasArcs(frame->arc);
}

bool FSTOrdTermsReader::TermsReader::IntersectTermsEnum::canRewind(
    shared_ptr<Frame> frame)
{ // can jump to sibling
  return !frame->arc->isLast();
}

void FSTOrdTermsReader::TermsReader::IntersectTermsEnum::pushFrame(
    shared_ptr<Frame> frame)
{
  shared_ptr<FST::Arc<int64_t>> *const arc = frame->arc;
  arc->output = fstOutputs->add(topFrame()->arc->output, arc->output);
  term_ = grow(arc->label);
  level++;
  assert(frame == stack[level]);
}

shared_ptr<Frame> FSTOrdTermsReader::TermsReader::IntersectTermsEnum::popFrame()
{
  term_ = shrink();
  return stack[level--];
}

shared_ptr<Frame> FSTOrdTermsReader::TermsReader::IntersectTermsEnum::newFrame()
{
  if (level + 1 == stack.size()) {
    std::deque<std::shared_ptr<Frame>> temp(ArrayUtil::oversize(
        level + 2, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(stack, 0, temp, 0, stack.size());
    for (int i = stack.size(); i < temp.size(); i++) {
      temp[i] = make_shared<Frame>(shared_from_this());
    }
    stack = temp;
  }
  return stack[level + 1];
}

shared_ptr<Frame> FSTOrdTermsReader::TermsReader::IntersectTermsEnum::topFrame()
{
  return stack[level];
}

shared_ptr<BytesRefBuilder>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::grow(int label)
{
  if (term_ == nullptr) {
    term_ = make_shared<BytesRefBuilder>();
  } else {
    term_->append(static_cast<char>(label));
  }
  return term_;
}

shared_ptr<BytesRefBuilder>
FSTOrdTermsReader::TermsReader::IntersectTermsEnum::shrink()
{
  if (term_->length() == 0) {
    term_.reset();
  } else {
    term_->setLength(term_->length() - 1);
  }
  return term_;
}

template <typename T>
void FSTOrdTermsReader::walk(shared_ptr<FST<T>> fst) 
{
  const deque<FST::Arc<T>> queue = deque<FST::Arc<T>>();
  shared_ptr<BitSet> *const seen = make_shared<BitSet>();
  shared_ptr<FST::BytesReader> *const reader = fst->getBytesReader();
  shared_ptr<FST::Arc<T>> *const startArc =
      fst->getFirstArc(make_shared<FST::Arc<T>>());
  queue.push_back(startArc);
  while (!queue.empty()) {
    shared_ptr<FST::Arc<T>> *const arc = queue.erase(queue.begin());
    constexpr int64_t node = arc->target;
    // System.out.println(arc);
    if (FST::targetHasArcs(arc) && !seen->get(static_cast<int>(node))) {
      seen->set(static_cast<int>(node));
      fst->readFirstRealTargetArc(node, arc, reader);
      while (true) {
        queue.push_back((make_shared<FST::Arc<T>>())->copyFrom(arc));
        if (arc->isLast()) {
          break;
        } else {
          fst->readNextRealArc(arc, reader);
        }
      }
    }
  }
}

int64_t FSTOrdTermsReader::ramBytesUsed()
{
  int64_t ramBytesUsed = postingsReader->ramBytesUsed();
  for (auto r : fields) {
    ramBytesUsed += r->second.ramBytesUsed();
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FSTOrdTermsReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(Accountables::namedAccountables(L"field", fields));
  resources.push_back(
      Accountables::namedAccountable(L"delegate", postingsReader));
  return Collections::unmodifiableList(resources);
}

wstring FSTOrdTermsReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() +
         L",delegate=" + postingsReader + L")";
}

void FSTOrdTermsReader::checkIntegrity() 
{
  postingsReader->checkIntegrity();
}
} // namespace org::apache::lucene::codecs::memory