using namespace std;

#include "OrdsIntersectTermsEnumFrame.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Transition.h"
#include "FSTOrdsOutputs.h"
#include "OrdsBlockTreeTermsWriter.h"
#include "OrdsIntersectTermsEnum.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using Transition = org::apache::lucene::util::automaton::Transition;
using FST = org::apache::lucene::util::fst::FST;

OrdsIntersectTermsEnumFrame::OrdsIntersectTermsEnumFrame(
    shared_ptr<OrdsIntersectTermsEnum> ite, int ord) 
    : ord(ord), termState(ite->fr->parent->postingsReader->newTermState()),
      ite(ite)
{
  this->termState->totalTermFreq = -1;
  this->longs = std::deque<int64_t>(ite->fr->longsSize);
}

void OrdsIntersectTermsEnumFrame::loadNextFloorBlock() 
{
  assert(numFollowFloorBlocks > 0);
  // if (DEBUG) System.out.println("    loadNextFoorBlock trans=" +
  // transitions[transitionIndex]);

  do {
    fp = fpOrig + (static_cast<int64_t>(static_cast<uint64_t>(
                                              floorDataReader->readVLong()) >>
                                          1));
    numFollowFloorBlocks--;
    // if (DEBUG) System.out.println("    skip floor block2!  nextFloorLabel=" +
    // (char) nextFloorLabel + " vs target=" + (char)
    // transitions[transitionIndex].getMin() + " newFP=" + fp + "
    // numFollowFloorBlocks=" + numFollowFloorBlocks);
    if (numFollowFloorBlocks != 0) {
      nextFloorLabel = floorDataReader->readByte() & 0xff;
      termOrd += floorDataReader->readVLong();
    } else {
      nextFloorLabel = 256;
    }
    // if (DEBUG) System.out.println("    nextFloorLabel=" + (char)
    // nextFloorLabel);
  } while (numFollowFloorBlocks != 0 && nextFloorLabel <= transition->min);

  load(nullptr);
}

void OrdsIntersectTermsEnumFrame::setState(int state)
{
  this->state = state;
  transitionIndex = 0;
  transitionCount = ite->compiledAutomaton->automaton->getNumTransitions(state);
  if (transitionCount != 0) {
    ite->compiledAutomaton->automaton->initTransition(state, transition);
    ite->compiledAutomaton->automaton->getNextTransition(transition);
    curTransitionMax = transition->max;
  } else {
    curTransitionMax = -1;
  }
}

void OrdsIntersectTermsEnumFrame::load(shared_ptr<Output> output) throw(
    IOException)
{

  // if (DEBUG) System.out.println("    load fp=" + fp + " fpOrig=" + fpOrig + "
  // frameIndexData=" + frameIndexData + " trans=" + (transitions.length != 0 ?
  // transitions[0] : "n/a" + " state=" + state));

  if (output != nullptr && output->bytes != nullptr && transitionCount != 0) {
    shared_ptr<BytesRef> frameIndexData = output->bytes;

    // Floor frame
    if (floorData.size() < frameIndexData->length) {
      this->floorData =
          std::deque<char>(ArrayUtil::oversize(frameIndexData->length, 1));
    }
    System::arraycopy(frameIndexData->bytes, frameIndexData->offset, floorData,
                      0, frameIndexData->length);
    floorDataReader->reset(floorData, 0, frameIndexData->length);
    constexpr int64_t code = floorDataReader->readVLong();
    if ((code & OrdsBlockTreeTermsWriter::OUTPUT_FLAG_IS_FLOOR) != 0) {
      numFollowFloorBlocks = floorDataReader->readVInt();
      nextFloorLabel = floorDataReader->readByte() & 0xff;

      termOrd = termOrdOrig + floorDataReader->readVLong();

      // if (DEBUG) System.out.println("    numFollowFloorBlocks=" +
      // numFollowFloorBlocks + " nextFloorLabel=" + nextFloorLabel);

      // If current state is accept, we must process
      // first block in case it has empty suffix:
      if (!ite->runAutomaton->isAccept(state)) {
        // Maybe skip floor blocks:
        assert((transitionIndex == 0,
                L"transitionIndex=" + to_wstring(transitionIndex)));
        while (numFollowFloorBlocks != 0 && nextFloorLabel <= transition->min) {
          fp = fpOrig +
               (static_cast<int64_t>(static_cast<uint64_t>(
                                           floorDataReader->readVLong()) >>
                                       1));
          numFollowFloorBlocks--;
          // if (DEBUG) System.out.println("    skip floor block!
          // nextFloorLabel=" + (char) nextFloorLabel + " vs target=" + (char)
          // transitions[0].getMin() + " newFP=" + fp + " numFollowFloorBlocks="
          // + numFollowFloorBlocks);
          if (numFollowFloorBlocks != 0) {
            nextFloorLabel = floorDataReader->readByte() & 0xff;
            termOrd += floorDataReader->readVLong();
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
  // if (DEBUG) System.out.println("      entCount=" + entCount + "
  // lastInFloor?=" + isLastInFloor + " leafBlock?=" + isLeafBlock + "
  // numSuffixBytes=" + numBytes);
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
  if (bytes.empty()) {
    bytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
    bytesReader = make_shared<ByteArrayDataInput>();
  } else if (bytes.size() < numBytes) {
    bytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ite->in_->readBytes(bytes, 0, numBytes);
  bytesReader->reset(bytes, 0, numBytes);

  if (!isLastInFloor) {
    // Sub-blocks of a single floor block are always
    // written one after another -- tail recurse:
    fpEnd = ite->in_->getFilePointer();
  }
}

bool OrdsIntersectTermsEnumFrame::next()
{
  return isLeafBlock ? nextLeaf() : nextNonLeaf();
}

bool OrdsIntersectTermsEnumFrame::nextLeaf()
{
  // if (DEBUG) System.out.println("  frame.next ord=" + ord + " nextEnt=" +
  // nextEnt + " entCount=" + entCount);
  assert((nextEnt != -1 && nextEnt < entCount,
          L"nextEnt=" + to_wstring(nextEnt) + L" entCount=" +
              to_wstring(entCount) + L" fp=" + to_wstring(fp)));
  nextEnt++;
  suffix = suffixesReader->readVInt();
  startBytePos = suffixesReader->getPosition();
  suffixesReader->skipBytes(suffix);
  return false;
}

bool OrdsIntersectTermsEnumFrame::nextNonLeaf()
{
  // if (DEBUG) System.out.println("  frame.next ord=" + ord + " nextEnt=" +
  // nextEnt + " entCount=" + entCount);
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
    // Skip term ord
    suffixesReader->readVLong();
    return true;
  }
}

int OrdsIntersectTermsEnumFrame::getTermBlockOrd()
{
  return isLeafBlock ? nextEnt : termState->termBlockOrd;
}

void OrdsIntersectTermsEnumFrame::decodeMetaData() 
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
    // if (DEBUG) System.out.println("    dF=" + state.docFreq);
    if (ite->fr->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
      termState->totalTermFreq = termState->docFreq + statsReader->readVLong();
      // if (DEBUG) System.out.println("    totTF=" + state.totalTermFreq);
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
} // namespace org::apache::lucene::codecs::blocktreeords