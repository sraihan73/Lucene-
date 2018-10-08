using namespace std;

#include "FSTTermsReader.h"
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
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/ByteRunAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "FSTTermOutputs.h"
#include "FSTTermsWriter.h"

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
using Util = org::apache::lucene::util::fst::Util;

FSTTermsReader::FSTTermsReader(
    shared_ptr<SegmentReadState> state,
    shared_ptr<PostingsReaderBase> postingsReader) 
    : postingsReader(postingsReader)
{
  const wstring termsFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      FSTTermsWriter::TERMS_EXTENSION);

  shared_ptr<IndexInput> *const in_ =
      state->directory->openInput(termsFileName, state->context);

  bool success = false;
  try {
    CodecUtil::checkIndexHeader(in_, FSTTermsWriter::TERMS_CODEC_NAME,
                                FSTTermsWriter::TERMS_VERSION_START,
                                FSTTermsWriter::TERMS_VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    CodecUtil::checksumEntireFile(in_);
    this->postingsReader->init(in_, state);
    seekDir(in_);

    shared_ptr<FieldInfos> *const fieldInfos = state->fieldInfos;
    constexpr int numFields = in_->readVInt();
    for (int i = 0; i < numFields; i++) {
      int fieldNumber = in_->readVInt();
      shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(fieldNumber);
      int64_t numTerms = in_->readVLong();
      int64_t sumTotalTermFreq =
          fieldInfo->getIndexOptions() == IndexOptions::DOCS ? -1
                                                             : in_->readVLong();
      int64_t sumDocFreq = in_->readVLong();
      int docCount = in_->readVInt();
      int longsSize = in_->readVInt();
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<TermsReader> current = make_shared<TermsReader>(
          shared_from_this(), fieldInfo, in_, numTerms, sumTotalTermFreq,
          sumDocFreq, docCount, longsSize);
      shared_ptr<TermsReader> previous =
          fields.emplace(fieldInfo->name, current);
      checkFieldSummary(state->segmentInfo, in_, current, previous);
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({in_});
    } else {
      IOUtils::closeWhileHandlingException({in_});
    }
  }
}

void FSTTermsReader::seekDir(shared_ptr<IndexInput> in_) 
{
  in_->seek(in_->length() - CodecUtil::footerLength() - 8);
  in_->seek(in_->readLong());
}

void FSTTermsReader::checkFieldSummary(
    shared_ptr<SegmentInfo> info, shared_ptr<IndexInput> in_,
    shared_ptr<TermsReader> field,
    shared_ptr<TermsReader> previous) 
{
  // #docs with field must be <= #docs
  if (field->docCount < 0 || field->docCount > info->maxDoc()) {
    throw make_shared<CorruptIndexException>(
        L"invalid docCount: " + to_wstring(field->docCount) + L" maxDoc: " +
            to_wstring(info->maxDoc()),
        in_);
  }
  // #postings must be >= #docs with field
  if (field->sumDocFreq < field->docCount) {
    throw make_shared<CorruptIndexException>(
        L"invalid sumDocFreq: " + to_wstring(field->sumDocFreq) +
            L" docCount: " + to_wstring(field->docCount),
        in_);
  }
  // #positions must be >= #postings
  if (field->sumTotalTermFreq != -1 &&
      field->sumTotalTermFreq < field->sumDocFreq) {
    throw make_shared<CorruptIndexException>(
        L"invalid sumTotalTermFreq: " + to_wstring(field->sumTotalTermFreq) +
            L" sumDocFreq: " + to_wstring(field->sumDocFreq),
        in_);
  }
  if (previous != nullptr) {
    throw make_shared<CorruptIndexException>(
        L"duplicate fields: " + field->fieldInfo->name, in_);
  }
}

shared_ptr<Iterator<wstring>> FSTTermsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms> FSTTermsReader::terms(const wstring &field) 
{
  assert(field != L"");
  return fields[field];
}

int FSTTermsReader::size() { return fields.size(); }

FSTTermsReader::~FSTTermsReader()
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

FSTTermsReader::TermsReader::TermsReader(
    shared_ptr<FSTTermsReader> outerInstance, shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<IndexInput> in_, int64_t numTerms, int64_t sumTotalTermFreq,
    int64_t sumDocFreq, int docCount, int longsSize) 
    : fieldInfo(fieldInfo), numTerms(numTerms),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq),
      docCount(docCount), longsSize(longsSize),
      dict(make_shared<FST<>>(
          in_, make_shared<FSTTermOutputs>(fieldInfo, longsSize))),
      outerInstance(outerInstance)
{
}

int64_t FSTTermsReader::TermsReader::ramBytesUsed()
{
  int64_t bytesUsed = BASE_RAM_BYTES_USED;
  if (dict != nullptr) {
    bytesUsed += dict->ramBytesUsed();
  }
  return bytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FSTTermsReader::TermsReader::getChildResources()
{
  if (dict == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"terms", dict));
  }
}

wstring FSTTermsReader::TermsReader::toString()
{
  return L"FSTTerms(terms=" + to_wstring(numTerms) + L",postings=" +
         to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}

bool FSTTermsReader::TermsReader::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool FSTTermsReader::TermsReader::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool FSTTermsReader::TermsReader::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool FSTTermsReader::TermsReader::hasPayloads()
{
  return fieldInfo->hasPayloads();
}

int64_t FSTTermsReader::TermsReader::size() { return numTerms; }

int64_t FSTTermsReader::TermsReader::getSumTotalTermFreq()
{
  return sumTotalTermFreq;
}

int64_t FSTTermsReader::TermsReader::getSumDocFreq() 
{
  return sumDocFreq;
}

int FSTTermsReader::TermsReader::getDocCount() 
{
  return docCount;
}

shared_ptr<TermsEnum> FSTTermsReader::TermsReader::iterator() 
{
  return make_shared<SegmentTermsEnum>(shared_from_this());
}

shared_ptr<TermsEnum> FSTTermsReader::TermsReader::intersect(
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

FSTTermsReader::TermsReader::BaseTermsEnum::BaseTermsEnum(
    shared_ptr<FSTTermsReader::TermsReader> outerInstance) 
    : state(outerInstance->outerInstance->postingsReader.newTermState()),
      outerInstance(outerInstance)
{
  this->bytesReader = make_shared<ByteArrayDataInput>();
  // NOTE: metadata will only be initialized in child class
}

shared_ptr<TermState>
FSTTermsReader::TermsReader::BaseTermsEnum::termState() 
{
  decodeMetaData();
  return state->clone();
}

int FSTTermsReader::TermsReader::BaseTermsEnum::docFreq() 
{
  return state->docFreq;
}

int64_t
FSTTermsReader::TermsReader::BaseTermsEnum::totalTermFreq() 
{
  return state->totalTermFreq;
}

shared_ptr<PostingsEnum> FSTTermsReader::TermsReader::BaseTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  decodeMetaData();
  return outerInstance->outerInstance->postingsReader.postings(
      outerInstance->fieldInfo, state, reuse, flags);
}

void FSTTermsReader::TermsReader::BaseTermsEnum::seekExact(int64_t ord) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t FSTTermsReader::TermsReader::BaseTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

FSTTermsReader::TermsReader::SegmentTermsEnum::SegmentTermsEnum(
    shared_ptr<FSTTermsReader::TermsReader> outerInstance) 
    : BaseTermsEnum(outerInstance),
      fstEnum(make_shared<BytesRefFSTEnum<>>(outerInstance->dict)),
      outerInstance(outerInstance)
{
  this->decoded = false;
  this->seekPending = false;
  this->meta.reset();
}

shared_ptr<BytesRef>
FSTTermsReader::TermsReader::SegmentTermsEnum::term() 
{
  return term_;
}

void FSTTermsReader::TermsReader::SegmentTermsEnum::decodeMetaData() throw(
    IOException)
{
  if (!decoded && !seekPending) {
    if (meta->bytes.size() > 0) {
      bytesReader->reset(meta->bytes, 0, meta->bytes.size());
    }
    outerInstance->outerInstance->postingsReader.decodeTerm(
        meta->longs, bytesReader, outerInstance->fieldInfo, state, true);
    decoded = true;
  }
}

void FSTTermsReader::TermsReader::SegmentTermsEnum::updateEnum(
    shared_ptr<
        BytesRefFSTEnum::InputOutput<std::shared_ptr<FSTTermOutputs::TermData>>>
        pair)
{
  if (pair == nullptr) {
    term_.reset();
  } else {
    term_ = pair->input;
    meta = pair->output;
    state->docFreq = meta->docFreq;
    state->totalTermFreq = meta->totalTermFreq;
  }
  decoded = false;
  seekPending = false;
}

shared_ptr<BytesRef>
FSTTermsReader::TermsReader::SegmentTermsEnum::next() 
{
  if (seekPending) { // previously positioned, but termOutputs not fetched
    seekPending = false;
    SeekStatus status = seekCeil(term_);
    assert(status == SeekStatus::FOUND); // must positioned on valid term
  }
  updateEnum(fstEnum->next());
  return term_;
}

bool FSTTermsReader::TermsReader::SegmentTermsEnum::seekExact(
    shared_ptr<BytesRef> target) 
{
  updateEnum(fstEnum->seekExact(target));
  return term_ != nullptr;
}

SeekStatus FSTTermsReader::TermsReader::SegmentTermsEnum::seekCeil(
    shared_ptr<BytesRef> target) 
{
  updateEnum(fstEnum->seekCeil(target));
  if (term_ == nullptr) {
    return SeekStatus::END;
  } else {
    return term_->equals(target) ? SeekStatus::FOUND : SeekStatus::NOT_FOUND;
  }
}

void FSTTermsReader::TermsReader::SegmentTermsEnum::seekExact(
    shared_ptr<BytesRef> target, shared_ptr<TermState> otherState)
{
  if (!target->equals(term_)) {
    state->copyFrom(otherState);
    term_ = BytesRef::deepCopyOf(target);
    seekPending = true;
  }
}

FSTTermsReader::TermsReader::IntersectTermsEnum::Frame::Frame(
    shared_ptr<FSTTermsReader::TermsReader::IntersectTermsEnum> outerInstance)
    : outerInstance(outerInstance)
{
  this->fstArc = make_shared<FST::Arc<FSTTermOutputs::TermData>>();
  this->fsaState = -1;
}

wstring FSTTermsReader::TermsReader::IntersectTermsEnum::Frame::toString()
{
  return L"arc=" + fstArc + L" state=" + to_wstring(fsaState);
}

FSTTermsReader::TermsReader::IntersectTermsEnum::IntersectTermsEnum(
    shared_ptr<FSTTermsReader::TermsReader> outerInstance,
    shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
    : BaseTermsEnum(outerInstance), fst(outerInstance->dict),
      fstReader(fst->getBytesReader()),
      fstOutputs(outerInstance->dict->outputs), fsa(compiled->runAutomaton),
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

  this->meta.reset();
  this->metaUpto = 1;
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
FSTTermsReader::TermsReader::IntersectTermsEnum::term() 
{
  return term_ == nullptr ? nullptr : term_->get();
}

void FSTTermsReader::TermsReader::IntersectTermsEnum::decodeMetaData() throw(
    IOException)
{
  assert(term_ != nullptr);
  if (!decoded) {
    if (meta->bytes.size() > 0) {
      bytesReader->reset(meta->bytes, 0, meta->bytes.size());
    }
    outerInstance->outerInstance->postingsReader.decodeTerm(
        meta->longs, bytesReader, outerInstance->fieldInfo, state, true);
    decoded = true;
  }
}

void FSTTermsReader::TermsReader::IntersectTermsEnum::loadMetaData() throw(
    IOException)
{
  shared_ptr<FST::Arc<std::shared_ptr<FSTTermOutputs::TermData>>> last, next;
  last = stack[metaUpto]->fstArc;
  while (metaUpto != level) {
    metaUpto++;
    next = stack[metaUpto]->fstArc;
    next->output = fstOutputs->add(next->output, last->output);
    last = next;
  }
  if (last->isFinal()) {
    meta = fstOutputs->add(last->output, last->nextFinalOutput);
  } else {
    meta = last->output;
  }
  state->docFreq = meta->docFreq;
  state->totalTermFreq = meta->totalTermFreq;
}

SeekStatus FSTTermsReader::TermsReader::IntersectTermsEnum::seekCeil(
    shared_ptr<BytesRef> target) 
{
  decoded = false;
  doSeekCeil(target);
  loadMetaData();
  if (term_ == nullptr) {
    return SeekStatus::END;
  } else {
    return term_->equals(target) ? SeekStatus::FOUND : SeekStatus::NOT_FOUND;
  }
}

shared_ptr<BytesRef>
FSTTermsReader::TermsReader::IntersectTermsEnum::next() 
{
  // if (TEST) System.out.println("Enum next()");
  if (pending) {
    pending = false;
    loadMetaData();
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
  loadMetaData();
  return term();
}

shared_ptr<BytesRef>
FSTTermsReader::TermsReader::IntersectTermsEnum::doSeekCeil(
    shared_ptr<BytesRef> target) 
{
  // if (TEST) System.out.println("Enum doSeekCeil()");
  shared_ptr<Frame> frame = nullptr;
  int label, upto = 0, limit = target->length;
  while (upto < limit) { // to target prefix, or ceil label (rewind prefix)
    frame = newFrame();
    label = target->bytes[upto] & 0xff;
    frame = loadCeilFrame(label, topFrame(), frame);
    if (frame == nullptr || frame->fstArc->label != label) {
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
FSTTermsReader::TermsReader::IntersectTermsEnum::loadVirtualFrame(
    shared_ptr<Frame> frame) 
{
  frame->fstArc->output = fstOutputs->getNoOutput();
  frame->fstArc->nextFinalOutput = fstOutputs->getNoOutput();
  frame->fsaState = -1;
  return frame;
}

shared_ptr<Frame>
FSTTermsReader::TermsReader::IntersectTermsEnum::loadFirstFrame(
    shared_ptr<Frame> frame) 
{
  frame->fstArc = fst->getFirstArc(frame->fstArc);
  frame->fsaState = 0;
  return frame;
}

shared_ptr<Frame>
FSTTermsReader::TermsReader::IntersectTermsEnum::loadExpandFrame(
    shared_ptr<Frame> top, shared_ptr<Frame> frame) 
{
  if (!canGrow(top)) {
    return nullptr;
  }
  frame->fstArc = fst->readFirstRealTargetArc(top->fstArc->target,
                                              frame->fstArc, fstReader);
  frame->fsaState = fsa->step(top->fsaState, frame->fstArc->label);
  // if (TEST) System.out.println(" loadExpand frame="+frame);
  if (frame->fsaState == -1) {
    return loadNextFrame(top, frame);
  }
  return frame;
}

shared_ptr<Frame>
FSTTermsReader::TermsReader::IntersectTermsEnum::loadNextFrame(
    shared_ptr<Frame> top, shared_ptr<Frame> frame) 
{
  if (!canRewind(frame)) {
    return nullptr;
  }
  while (!frame->fstArc->isLast()) {
    frame->fstArc = fst->readNextRealArc(frame->fstArc, fstReader);
    frame->fsaState = fsa->step(top->fsaState, frame->fstArc->label);
    if (frame->fsaState != -1) {
      break;
    }
  }
  // if (TEST) System.out.println(" loadNext frame="+frame);
  if (frame->fsaState == -1) {
    return nullptr;
  }
  return frame;
}

shared_ptr<Frame>
FSTTermsReader::TermsReader::IntersectTermsEnum::loadCeilFrame(
    int label, shared_ptr<Frame> top,
    shared_ptr<Frame> frame) 
{
  shared_ptr<FST::Arc<std::shared_ptr<FSTTermOutputs::TermData>>> arc =
      frame->fstArc;
  arc = Util::readCeilArc(label, fst, top->fstArc, arc, fstReader);
  if (arc == nullptr) {
    return nullptr;
  }
  frame->fsaState = fsa->step(top->fsaState, arc->label);
  // if (TEST) System.out.println(" loadCeil frame="+frame);
  if (frame->fsaState == -1) {
    return loadNextFrame(top, frame);
  }
  return frame;
}

bool FSTTermsReader::TermsReader::IntersectTermsEnum::isAccept(
    shared_ptr<Frame> frame)
{ // reach a term both fst&fsa accepts
  return fsa->isAccept(frame->fsaState) && frame->fstArc->isFinal();
}

bool FSTTermsReader::TermsReader::IntersectTermsEnum::isValid(
    shared_ptr<Frame> frame)
{ // reach a prefix both fst&fsa won't reject
  return frame->fsaState != -1;
}

bool FSTTermsReader::TermsReader::IntersectTermsEnum::canGrow(
    shared_ptr<Frame> frame)
{ // can walk forward on both fst&fsa
  return frame->fsaState != -1 && FST::targetHasArcs(frame->fstArc);
}

bool FSTTermsReader::TermsReader::IntersectTermsEnum::canRewind(
    shared_ptr<Frame> frame)
{ // can jump to sibling
  return !frame->fstArc->isLast();
}

void FSTTermsReader::TermsReader::IntersectTermsEnum::pushFrame(
    shared_ptr<Frame> frame)
{
  term_ = grow(frame->fstArc->label);
  level++;
  // if (TEST) System.out.println("  term=" + term + " level=" + level);
}

shared_ptr<Frame> FSTTermsReader::TermsReader::IntersectTermsEnum::popFrame()
{
  term_ = shrink();
  level--;
  metaUpto = metaUpto > level ? level : metaUpto;
  // if (TEST) System.out.println("  term=" + term + " level=" + level);
  return stack[level + 1];
}

shared_ptr<Frame> FSTTermsReader::TermsReader::IntersectTermsEnum::newFrame()
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

shared_ptr<Frame> FSTTermsReader::TermsReader::IntersectTermsEnum::topFrame()
{
  return stack[level];
}

shared_ptr<BytesRefBuilder>
FSTTermsReader::TermsReader::IntersectTermsEnum::grow(int label)
{
  if (term_ == nullptr) {
    term_ = make_shared<BytesRefBuilder>();
  } else {
    term_->append(static_cast<char>(label));
  }
  return term_;
}

shared_ptr<BytesRefBuilder>
FSTTermsReader::TermsReader::IntersectTermsEnum::shrink()
{
  if (term_->length() == 0) {
    term_.reset();
  } else {
    term_->setLength(term_->length() - 1);
  }
  return term_;
}

template <typename T>
void FSTTermsReader::walk(shared_ptr<FST<T>> fst) 
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

int64_t FSTTermsReader::ramBytesUsed()
{
  int64_t ramBytesUsed = postingsReader->ramBytesUsed();
  for (auto r : fields) {
    ramBytesUsed += r->second.ramBytesUsed();
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FSTTermsReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(Accountables::namedAccountables(L"field", fields));
  resources.push_back(
      Accountables::namedAccountable(L"delegate", postingsReader));
  return Collections::unmodifiableCollection(resources);
}

wstring FSTTermsReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() +
         L",delegate=" + postingsReader + L")";
}

void FSTTermsReader::checkIntegrity() 
{
  postingsReader->checkIntegrity();
}
} // namespace org::apache::lucene::codecs::memory