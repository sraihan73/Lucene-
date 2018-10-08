using namespace std;

#include "IntersectTermsEnumFrame.h"
#include "../../index/IndexOptions.h"
#include "../../store/ByteArrayDataInput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/automaton/Transition.h"
#include "../BlockTermState.h"
#include "BlockTreeTermsReader.h"
#include "IntersectTermsEnum.h"

namespace org::apache::lucene::codecs::blocktree
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using Transition = org::apache::lucene::util::automaton::Transition;
using FST = org::apache::lucene::util::fst::FST;

IntersectTermsEnumFrame::IntersectTermsEnumFrame(
    shared_ptr<IntersectTermsEnum> ite, int ord) 
    : ord(ord), termState(ite->fr->parent->postingsReader->newTermState()),
      longs(std::deque<int64_t>(ite->fr->longsSize)), ite(ite)
{
  this->termState->totalTermFreq = -1;
}

void IntersectTermsEnumFrame::loadNextFloorBlock() 
{
  assert((numFollowFloorBlocks > 0,
          L"nextFloorLabel=" + to_wstring(nextFloorLabel)));

  do {
    fp = fpOrig + (static_cast<int64_t>(static_cast<uint64_t>(
                                              floorDataReader->readVLong()) >>
                                          1));
    numFollowFloorBlocks--;
    if (numFollowFloorBlocks != 0) {
      nextFloorLabel = floorDataReader->readByte() & 0xff;
    } else {
      nextFloorLabel = 256;
    }
  } while (numFollowFloorBlocks != 0 && nextFloorLabel <= transition->min);

  load(nullptr);
}

void IntersectTermsEnumFrame::setState(int state)
{
  this->state = state;
  transitionIndex = 0;
  transitionCount = ite->automaton->getNumTransitions(state);
  if (transitionCount != 0) {
    ite->automaton->initTransition(state, transition);
    ite->automaton->getNextTransition(transition);
  } else {

    // Must set min to -1 so the "label < min" check never falsely triggers:
    transition->min = -1;

    // Must set max to -1 so we immediately realize we need to step to the next
    // transition and then pop this frame:
    transition->max = -1;
  }
}

void IntersectTermsEnumFrame::load(shared_ptr<BytesRef> frameIndexData) throw(
    IOException)
{
  if (frameIndexData != nullptr) {
    floorDataReader->reset(frameIndexData->bytes, frameIndexData->offset,
                           frameIndexData->length);
    // Skip first long -- has redundant fp, hasTerms
    // flag, isFloor flag
    constexpr int64_t code = floorDataReader->readVLong();
    if ((code & BlockTreeTermsReader::OUTPUT_FLAG_IS_FLOOR) != 0) {
      // Floor frame
      numFollowFloorBlocks = floorDataReader->readVInt();
      nextFloorLabel = floorDataReader->readByte() & 0xff;

      // If current state is not accept, and has transitions, we must process
      // first block in case it has empty suffix:
      if (ite->runAutomaton->isAccept(state) == false && transitionCount != 0) {
        // Maybe skip floor blocks:
        assert((transitionIndex == 0,
                L"transitionIndex=" + to_wstring(transitionIndex)));
        while (numFollowFloorBlocks != 0 && nextFloorLabel <= transition->min) {
          fp = fpOrig +
               (static_cast<int64_t>(static_cast<uint64_t>(
                                           floorDataReader->readVLong()) >>
                                       1));
          numFollowFloorBlocks--;
          if (numFollowFloorBlocks != 0) {
            nextFloorLabel = floorDataReader->readByte() & 0xff;
          } else {
            nextFloorLabel = 256;
          }
        }
      }
    }
  }

  ite->in_->seek(fp);
  int code = ite->in_->readVInt();
  entCount = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  assert(entCount > 0);
  isLastInFloor = (code & 1) != 0;

  // term suffixes:
  code = ite->in_->readVInt();
  isLeafBlock = (code & 1) != 0;
  int numBytes = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  if (suffixBytes.size() < numBytes) {
    suffixBytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ite->in_->readBytes(suffixBytes, 0, numBytes);
  suffixesReader->reset(suffixBytes, 0, numBytes);

  // stats
  numBytes = ite->in_->readVInt();
  if (statBytes.size() < numBytes) {
    statBytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ite->in_->readBytes(statBytes, 0, numBytes);
  statsReader->reset(statBytes, 0, numBytes);
  metaDataUpto = 0;

  termState->termBlockOrd = 0;
  nextEnt = 0;

  // metadata
  numBytes = ite->in_->readVInt();
  if (bytes.size() < numBytes) {
    bytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ite->in_->readBytes(bytes, 0, numBytes);
  bytesReader->reset(bytes, 0, numBytes);

  if (!isLastInFloor) {
    // Sub-blocks of a single floor block are always
    // written one after another -- tail recurse:
    fpEnd = ite->in_->getFilePointer();
  }

  // Necessary in case this ord previously was an auto-prefix
  // term but now we recurse to a new leaf block
  isAutoPrefixTerm = false;
}

bool IntersectTermsEnumFrame::next()
{
  if (isLeafBlock) {
    nextLeaf();
    return false;
  } else {
    return nextNonLeaf();
  }
}

void IntersectTermsEnumFrame::nextLeaf()
{
  assert((nextEnt != -1 && nextEnt < entCount,
          L"nextEnt=" + to_wstring(nextEnt) + L" entCount=" +
              to_wstring(entCount) + L" fp=" + to_wstring(fp)));
  nextEnt++;
  suffix = suffixesReader->readVInt();
  startBytePos = suffixesReader->getPosition();
  suffixesReader->skipBytes(suffix);
}

bool IntersectTermsEnumFrame::nextNonLeaf()
{
  assert((nextEnt != -1 && nextEnt < entCount,
          L"nextEnt=" + to_wstring(nextEnt) + L" entCount=" +
              to_wstring(entCount) + L" fp=" + to_wstring(fp)));
  nextEnt++;
  constexpr int code = suffixesReader->readVInt();
  suffix = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  startBytePos = suffixesReader->getPosition();
  suffixesReader->skipBytes(suffix);
  if ((code & 1) == 0) {
    // A normal term
    termState->termBlockOrd++;
    return false;
  } else {
    // A sub-block; make sub-FP absolute:
    lastSubFP = fp - suffixesReader->readVLong();
    return true;
  }
}

int IntersectTermsEnumFrame::getTermBlockOrd()
{
  return isLeafBlock ? nextEnt : termState->termBlockOrd;
}

void IntersectTermsEnumFrame::decodeMetaData() 
{

  // lazily catch up on metadata decode:
  constexpr int limit = getTermBlockOrd();
  bool absolute = metaDataUpto == 0;
  assert(limit > 0);

  // TODO: better API would be "jump straight to term=N"???
  while (metaDataUpto < limit) {

    // TODO: we could make "tiers" of metadata, ie,
    // decode docFreq/totalTF but don't decode postings
    // metadata; this way caller could get
    // docFreq/totalTF w/o paying decode cost for
    // postings

    // TODO: if docFreq were bulk decoded we could
    // just skipN here:

    // stats
    termState->docFreq = statsReader->readVInt();
    if (ite->fr->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
      termState->totalTermFreq = termState->docFreq + statsReader->readVLong();
    }
    // metadata
    for (int i = 0; i < ite->fr->longsSize; i++) {
      longs[i] = bytesReader->readVLong();
    }
    ite->fr->parent->postingsReader->decodeTerm(
        longs, bytesReader, ite->fr->fieldInfo, termState, absolute);

    metaDataUpto++;
    absolute = false;
  }
  termState->termBlockOrd = metaDataUpto;
}
} // namespace org::apache::lucene::codecs::blocktree