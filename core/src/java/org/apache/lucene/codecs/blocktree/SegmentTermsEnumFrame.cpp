using namespace std;

#include "SegmentTermsEnumFrame.h"
#include "../../index/IndexOptions.h"
#include "../../store/ByteArrayDataInput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../BlockTermState.h"
#include "SegmentTermsEnum.h"

namespace org::apache::lucene::codecs::blocktree
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using FST = org::apache::lucene::util::fst::FST;

SegmentTermsEnumFrame::SegmentTermsEnumFrame(shared_ptr<SegmentTermsEnum> ste,
                                             int ord) 
    : ord(ord), state(ste->fr->parent->postingsReader->newTermState()),
      longs(std::deque<int64_t>(ste->fr->longsSize)), ste(ste)
{
  this->state->totalTermFreq = -1;
}

void SegmentTermsEnumFrame::setFloorData(shared_ptr<ByteArrayDataInput> in_,
                                         shared_ptr<BytesRef> source)
{
  constexpr int numBytes =
      source->length - (in_->getPosition() - source->offset);
  if (numBytes > floorData.size()) {
    floorData = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  System::arraycopy(source->bytes, source->offset + in_->getPosition(),
                    floorData, 0, numBytes);
  floorDataReader->reset(floorData, 0, numBytes);
  numFollowFloorBlocks = floorDataReader->readVInt();
  nextFloorLabel = floorDataReader->readByte() & 0xff;
  // if (DEBUG) {
  // System.out.println("    setFloorData fpOrig=" + fpOrig + " bytes=" + new
  // BytesRef(source.bytes, source.offset + in.getPosition(), numBytes) + "
  // numFollowFloorBlocks=" + numFollowFloorBlocks + " nextFloorLabel=" +
  // toHex(nextFloorLabel));
  //}
}

int SegmentTermsEnumFrame::getTermBlockOrd()
{
  return isLeafBlock ? nextEnt : state->termBlockOrd;
}

void SegmentTermsEnumFrame::loadNextFloorBlock() 
{
  // if (DEBUG) {
  // System.out.println("    loadNextFloorBlock fp=" + fp + " fpEnd=" + fpEnd);
  //}
  assert((arc == nullptr || isFloor,
          L"arc=" + arc + L" isFloor=" + StringHelper::toString(isFloor)));
  fp = fpEnd;
  nextEnt = -1;
  loadBlock();
}

void SegmentTermsEnumFrame::loadBlock() 
{

  // Clone the IndexInput lazily, so that consumers
  // that just pull a TermsEnum to
  // seekExact(TermState) don't pay this cost:
  ste->initIndexInput();

  if (nextEnt != -1) {
    // Already loaded
    return;
  }
  // System.out.println("blc=" + blockLoadCount);

  ste->in_->seek(fp);
  int code = ste->in_->readVInt();
  entCount = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  assert(entCount > 0);
  isLastInFloor = (code & 1) != 0;

  assert((arc == nullptr || (isLastInFloor || isFloor),
          L"fp=" + to_wstring(fp) + L" arc=" + arc + L" isFloor=" +
              StringHelper::toString(isFloor) + L" isLastInFloor=" +
              StringHelper::toString(isLastInFloor)));

  // TODO: if suffixes were stored in random-access
  // array structure, then we could do binary search
  // instead of linear scan to find target term; eg
  // we could have simple array of offsets

  // term suffixes:
  code = ste->in_->readVInt();
  isLeafBlock = (code & 1) != 0;
  int numBytes = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  if (suffixBytes.size() < numBytes) {
    suffixBytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ste->in_->readBytes(suffixBytes, 0, numBytes);
  suffixesReader->reset(suffixBytes, 0, numBytes);

  /*if (DEBUG) {
    if (arc == null) {
    System.out.println("    loadBlock (next) fp=" + fp + " entCount=" + entCount
    + " prefixLen=" + prefix + " isLastInFloor=" + isLastInFloor + " leaf?=" +
    isLeafBlock); } else { System.out.println("    loadBlock (seek) fp=" + fp +
    " entCount=" + entCount + " prefixLen=" + prefix + " hasTerms?=" + hasTerms
    + " isFloor?=" + isFloor + " isLastInFloor=" + isLastInFloor + " leaf?=" +
    isLeafBlock);
    }
    }*/

  // stats
  numBytes = ste->in_->readVInt();
  if (statBytes.size() < numBytes) {
    statBytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ste->in_->readBytes(statBytes, 0, numBytes);
  statsReader->reset(statBytes, 0, numBytes);
  metaDataUpto = 0;

  state->termBlockOrd = 0;
  nextEnt = 0;
  lastSubFP = -1;

  // TODO: we could skip this if !hasTerms; but
  // that's rare so won't help much
  // metadata
  numBytes = ste->in_->readVInt();
  if (bytes.size() < numBytes) {
    bytes = std::deque<char>(ArrayUtil::oversize(numBytes, 1));
  }
  ste->in_->readBytes(bytes, 0, numBytes);
  bytesReader->reset(bytes, 0, numBytes);

  // Sub-blocks of a single floor block are always
  // written one after another -- tail recurse:
  fpEnd = ste->in_->getFilePointer();
  // if (DEBUG) {
  //   System.out.println("      fpEnd=" + fpEnd);
  // }
}

void SegmentTermsEnumFrame::rewind()
{

  // Force reload:
  fp = fpOrig;
  nextEnt = -1;
  hasTerms = hasTermsOrig;
  if (isFloor) {
    floorDataReader->rewind();
    numFollowFloorBlocks = floorDataReader->readVInt();
    assert(numFollowFloorBlocks > 0);
    nextFloorLabel = floorDataReader->readByte() & 0xff;
  }

  /*
  //System.out.println("rewind");
  // Keeps the block loaded, but rewinds its state:
  if (nextEnt > 0 || fp != fpOrig) {
  if (DEBUG) {
  System.out.println("      rewind frame ord=" + ord + " fpOrig=" + fpOrig + "
  fp=" + fp + " hasTerms?=" + hasTerms + " isFloor?=" + isFloor + " nextEnt=" +
  nextEnt + " prefixLen=" + prefix);
  }
  if (fp != fpOrig) {
  fp = fpOrig;
  nextEnt = -1;
  } else {
  nextEnt = 0;
  }
  hasTerms = hasTermsOrig;
  if (isFloor) {
  floorDataReader.rewind();
  numFollowFloorBlocks = floorDataReader.readVInt();
  nextFloorLabel = floorDataReader.readByte() & 0xff;
  }
  assert suffixBytes != null;
  suffixesReader.rewind();
  assert statBytes != null;
  statsReader.rewind();
  metaDataUpto = 0;
  state.termBlockOrd = 0;
  // TODO: skip this if !hasTerms?  Then postings
  // impl wouldn't have to write useless 0 byte
  postingsReader.resetTermsBlock(fieldInfo, state);
  lastSubFP = -1;
  } else if (DEBUG) {
  System.out.println("      skip rewind fp=" + fp + " fpOrig=" + fpOrig + "
  nextEnt=" + nextEnt + " ord=" + ord);
  }
  */
}

bool SegmentTermsEnumFrame::next() 
{
  if (isLeafBlock) {
    nextLeaf();
    return false;
  } else {
    return nextNonLeaf();
  }
}

void SegmentTermsEnumFrame::nextLeaf()
{
  // if (DEBUG) System.out.println("  frame.next ord=" + ord + " nextEnt=" +
  // nextEnt + " entCount=" + entCount);
  assert((nextEnt != -1 && nextEnt < entCount,
          L"nextEnt=" + to_wstring(nextEnt) + L" entCount=" +
              to_wstring(entCount) + L" fp=" + to_wstring(fp)));
  nextEnt++;
  suffix = suffixesReader->readVInt();
  startBytePos = suffixesReader->getPosition();
  ste->term_->setLength(prefix + suffix);
  ste->term_.grow(ste->term_->length());
  suffixesReader->readBytes(ste->term_.bytes(), prefix, suffix);
  ste->termExists = true;
}

bool SegmentTermsEnumFrame::nextNonLeaf() 
{
  // if (DEBUG) System.out.println("  stef.next ord=" + ord + " nextEnt=" +
  // nextEnt + " entCount=" + entCount + " fp=" + suffixesReader.getPosition());
  while (true) {
    if (nextEnt == entCount) {
      assert((arc == nullptr || (isFloor && isLastInFloor == false),
              L"isFloor=" + StringHelper::toString(isFloor) +
                  L" isLastInFloor=" + StringHelper::toString(isLastInFloor)));
      loadNextFloorBlock();
      if (isLeafBlock) {
        nextLeaf();
        return false;
      } else {
        continue;
      }
    }

    assert((nextEnt != -1 && nextEnt < entCount,
            L"nextEnt=" + to_wstring(nextEnt) + L" entCount=" +
                to_wstring(entCount) + L" fp=" + to_wstring(fp)));
    nextEnt++;
    constexpr int code = suffixesReader->readVInt();
    suffix = static_cast<int>(static_cast<unsigned int>(code) >> 1);
    startBytePos = suffixesReader->getPosition();
    ste->term_->setLength(prefix + suffix);
    ste->term_.grow(ste->term_->length());
    suffixesReader->readBytes(ste->term_.bytes(), prefix, suffix);
    if ((code & 1) == 0) {
      // A normal term
      ste->termExists = true;
      subCode = 0;
      state->termBlockOrd++;
      return false;
    } else {
      // A sub-block; make sub-FP absolute:
      ste->termExists = false;
      subCode = suffixesReader->readVLong();
      lastSubFP = fp - subCode;
      // if (DEBUG) {
      // System.out.println("    lastSubFP=" + lastSubFP);
      //}
      return true;
    }
  }
}

void SegmentTermsEnumFrame::scanToFloorFrame(shared_ptr<BytesRef> target)
{

  if (!isFloor || target->length <= prefix) {
    // if (DEBUG) {
    //   System.out.println("    scanToFloorFrame skip: isFloor=" + isFloor + "
    //   target.length=" + target.length + " vs prefix=" + prefix);
    // }
    return;
  }

  constexpr int targetLabel = target->bytes[target->offset + prefix] & 0xFF;

  // if (DEBUG) {
  //   System.out.println("    scanToFloorFrame fpOrig=" + fpOrig + "
  //   targetLabel=" + toHex(targetLabel) + " vs nextFloorLabel=" +
  //   toHex(nextFloorLabel) + " numFollowFloorBlocks=" + numFollowFloorBlocks);
  // }

  if (targetLabel < nextFloorLabel) {
    // if (DEBUG) {
    //   System.out.println("      already on correct block");
    // }
    return;
  }

  assert(numFollowFloorBlocks != 0);

  int64_t newFP = fpOrig;
  while (true) {
    constexpr int64_t code = floorDataReader->readVLong();
    newFP =
        fpOrig +
        (static_cast<int64_t>(static_cast<uint64_t>(code) >> 1));
    hasTerms = (code & 1) != 0;
    // if (DEBUG) {
    //   System.out.println("      label=" + toHex(nextFloorLabel) + " fp=" +
    //   newFP + " hasTerms?=" + hasTerms + " numFollowFloor=" +
    //   numFollowFloorBlocks);
    // }

    isLastInFloor = numFollowFloorBlocks == 1;
    numFollowFloorBlocks--;

    if (isLastInFloor) {
      nextFloorLabel = 256;
      // if (DEBUG) {
      //   System.out.println("        stop!  last block nextFloorLabel=" +
      //   toHex(nextFloorLabel));
      // }
      break;
    } else {
      nextFloorLabel = floorDataReader->readByte() & 0xff;
      if (targetLabel < nextFloorLabel) {
        // if (DEBUG) {
        //   System.out.println("        stop!  nextFloorLabel=" +
        //   toHex(nextFloorLabel));
        // }
        break;
      }
    }
  }

  if (newFP != fp) {
    // Force re-load of the block:
    // if (DEBUG) {
    //   System.out.println("      force switch to fp=" + newFP + " oldFP=" +
    //   fp);
    // }
    nextEnt = -1;
    fp = newFP;
  } else {
    // if (DEBUG) {
    //   System.out.println("      stay on same fp=" + newFP);
    // }
  }
}

void SegmentTermsEnumFrame::decodeMetaData() 
{

  // if (DEBUG) System.out.println("\nBTTR.decodeMetadata seg=" + segment + "
  // mdUpto=" + metaDataUpto + " vs termBlockOrd=" + state.termBlockOrd);

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
    state->docFreq = statsReader->readVInt();
    // if (DEBUG) System.out.println("    dF=" + state.docFreq);
    if (ste->fr->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
      state->totalTermFreq = state->docFreq + statsReader->readVLong();
      // if (DEBUG) System.out.println("    totTF=" + state.totalTermFreq);
    }
    // metadata
    for (int i = 0; i < ste->fr->longsSize; i++) {
      longs[i] = bytesReader->readVLong();
    }
    ste->fr->parent->postingsReader->decodeTerm(
        longs, bytesReader, ste->fr->fieldInfo, state, absolute);

    metaDataUpto++;
    absolute = false;
  }
  state->termBlockOrd = metaDataUpto;
}

bool SegmentTermsEnumFrame::prefixMatches(shared_ptr<BytesRef> target)
{
  for (int bytePos = 0; bytePos < prefix; bytePos++) {
    if (target->bytes[target->offset + bytePos] != ste->term_.byteAt(bytePos)) {
      return false;
    }
  }

  return true;
}

void SegmentTermsEnumFrame::scanToSubBlock(int64_t subFP)
{
  assert(!isLeafBlock);
  // if (DEBUG) System.out.println("  scanToSubBlock fp=" + fp + " subFP=" +
  // subFP + " entCount=" + entCount + " lastSubFP=" + lastSubFP); assert nextEnt
  // == 0;
  if (lastSubFP == subFP) {
    // if (DEBUG) System.out.println("    already positioned");
    return;
  }
  assert(
      (subFP < fp, L"fp=" + to_wstring(fp) + L" subFP=" + to_wstring(subFP)));
  constexpr int64_t targetSubCode = fp - subFP;
  // if (DEBUG) System.out.println("    targetSubCode=" + targetSubCode);
  while (true) {
    assert(nextEnt < entCount);
    nextEnt++;
    constexpr int code = suffixesReader->readVInt();
    suffixesReader->skipBytes(
        static_cast<int>(static_cast<unsigned int>(code) >> 1));
    if ((code & 1) != 0) {
      constexpr int64_t subCode = suffixesReader->readVLong();
      if (targetSubCode == subCode) {
        // if (DEBUG) System.out.println("        match!");
        lastSubFP = subFP;
        return;
      }
    } else {
      state->termBlockOrd++;
    }
  }
}

SeekStatus SegmentTermsEnumFrame::scanToTerm(shared_ptr<BytesRef> target,
                                             bool exactOnly) 
{
  return isLeafBlock ? scanToTermLeaf(target, exactOnly)
                     : scanToTermNonLeaf(target, exactOnly);
}

SeekStatus
SegmentTermsEnumFrame::scanToTermLeaf(shared_ptr<BytesRef> target,
                                      bool exactOnly) 
{

  // if (DEBUG) System.out.println("    scanToTermLeaf: block fp=" + fp + "
  // prefix=" + prefix + " nextEnt=" + nextEnt + " (of " + entCount + ")
  // target=" + brToString(target) + " term=" + brToString(term));

  assert(nextEnt != -1);

  ste->termExists = true;
  subCode = 0;

  if (nextEnt == entCount) {
    if (exactOnly) {
      fillTerm();
    }
    return SeekStatus::END;
  }

  assert(prefixMatches(target));

  // Loop over each entry (term or sub-block) in this block:
  // nextTerm: while(nextEnt < entCount) {
  while (true) {
    nextEnt++;

    suffix = suffixesReader->readVInt();

    // if (DEBUG) {
    //   BytesRef suffixBytesRef = new BytesRef();
    //   suffixBytesRef.bytes = suffixBytes;
    //   suffixBytesRef.offset = suffixesReader.getPosition();
    //   suffixBytesRef.length = suffix;
    //   System.out.println("      cycle: term " + (nextEnt-1) + " (of " +
    //   entCount + ") suffix=" + brToString(suffixBytesRef));
    // }

    constexpr int termLen = prefix + suffix;
    startBytePos = suffixesReader->getPosition();
    suffixesReader->skipBytes(suffix);

    constexpr int targetLimit =
        target->offset + (target->length < termLen ? target->length : termLen);
    int targetPos = target->offset + prefix;

    // Loop over bytes in the suffix, comparing to
    // the target
    int bytePos = startBytePos;
    while (true) {
      constexpr int cmp;
      constexpr bool stop;
      if (targetPos < targetLimit) {
        cmp = (suffixBytes[bytePos++] & 0xFF) -
              (target->bytes[targetPos++] & 0xFF);
        stop = false;
      } else {
        assert(targetPos == targetLimit);
        cmp = termLen - target->length;
        stop = true;
      }

      if (cmp < 0) {
        // Current entry is still before the target;
        // keep scanning

        if (nextEnt == entCount) {
          // We are done scanning this block
          goto nextTermBreak;
        } else {
          goto nextTermContinue;
        }
      } else if (cmp > 0) {

        // Done!  Current entry is after target --
        // return NOT_FOUND:
        fillTerm();

        // if (DEBUG) System.out.println("        not found");
        return SeekStatus::NOT_FOUND;
      } else if (stop) {
        // Exact match!

        // This cannot be a sub-block because we
        // would have followed the index to this
        // sub-block from the start:

        assert(ste->termExists);
        fillTerm();
        // if (DEBUG) System.out.println("        found!");
        return SeekStatus::FOUND;
      }
    }
  nextTermContinue:;
  }
nextTermBreak:

  // It is possible (and OK) that terms index pointed us
  // at this block, but, we scanned the entire block and
  // did not find the term to position to.  This happens
  // when the target is after the last term in the block
  // (but, before the next term in the index).  EG
  // target could be foozzz, and terms index pointed us
  // to the foo* block, but the last term in this block
  // was fooz (and, eg, first term in the next block will
  // bee fop).
  // if (DEBUG) System.out.println("      block end");
  if (exactOnly) {
    fillTerm();
  }

  // TODO: not consistent that in the
  // not-exact case we don't next() into the next
  // frame here
  return SeekStatus::END;
}

SeekStatus
SegmentTermsEnumFrame::scanToTermNonLeaf(shared_ptr<BytesRef> target,
                                         bool exactOnly) 
{

  // if (DEBUG) System.out.println("    scanToTermNonLeaf: block fp=" + fp + "
  // prefix=" + prefix + " nextEnt=" + nextEnt + " (of " + entCount + ") target="
  // + brToString(target) + " term=" + brToString(target));

  assert(nextEnt != -1);

  if (nextEnt == entCount) {
    if (exactOnly) {
      fillTerm();
      ste->termExists = subCode == 0;
    }
    return SeekStatus::END;
  }

  assert(prefixMatches(target));

  // Loop over each entry (term or sub-block) in this block:
  while (nextEnt < entCount) {

    nextEnt++;

    constexpr int code = suffixesReader->readVInt();
    suffix = static_cast<int>(static_cast<unsigned int>(code) >> 1);

    // if (DEBUG) {
    //  BytesRef suffixBytesRef = new BytesRef();
    //  suffixBytesRef.bytes = suffixBytes;
    //  suffixBytesRef.offset = suffixesReader.getPosition();
    //  suffixBytesRef.length = suffix;
    //  System.out.println("      cycle: " + ((code&1)==1 ? "sub-block" :
    //  "term") + " " + (nextEnt-1) + " (of " + entCount + ") suffix=" +
    //  brToString(suffixBytesRef));
    //}

    constexpr int termLen = prefix + suffix;
    startBytePos = suffixesReader->getPosition();
    suffixesReader->skipBytes(suffix);
    ste->termExists = (code & 1) == 0;
    if (ste->termExists) {
      state->termBlockOrd++;
      subCode = 0;
    } else {
      subCode = suffixesReader->readVLong();
      lastSubFP = fp - subCode;
    }

    constexpr int targetLimit =
        target->offset + (target->length < termLen ? target->length : termLen);
    int targetPos = target->offset + prefix;

    // Loop over bytes in the suffix, comparing to
    // the target
    int bytePos = startBytePos;
    while (true) {
      constexpr int cmp;
      constexpr bool stop;
      if (targetPos < targetLimit) {
        cmp = (suffixBytes[bytePos++] & 0xFF) -
              (target->bytes[targetPos++] & 0xFF);
        stop = false;
      } else {
        assert(targetPos == targetLimit);
        cmp = termLen - target->length;
        stop = true;
      }

      if (cmp < 0) {
        // Current entry is still before the target;
        // keep scanning
        goto nextTermContinue;
      } else if (cmp > 0) {

        // Done!  Current entry is after target --
        // return NOT_FOUND:
        fillTerm();

        // if (DEBUG) System.out.println("        maybe done exactOnly=" +
        // exactOnly + " ste.termExists=" + ste.termExists);

        if (!exactOnly && !ste->termExists) {
          // System.out.println("  now pushFrame");
          // TODO this
          // We are on a sub-block, and caller wants
          // us to position to the next term after
          // the target, so we must recurse into the
          // sub-frame(s):
          ste->currentFrame =
              ste->pushFrame(nullptr, ste->currentFrame->lastSubFP, termLen);
          ste->currentFrame->loadBlock();
          while (ste->currentFrame->next()) {
            ste->currentFrame = ste->pushFrame(
                nullptr, ste->currentFrame->lastSubFP, ste->term_->length());
            ste->currentFrame->loadBlock();
          }
        }

        // if (DEBUG) System.out.println("        not found");
        return SeekStatus::NOT_FOUND;
      } else if (stop) {
        // Exact match!

        // This cannot be a sub-block because we
        // would have followed the index to this
        // sub-block from the start:

        assert(ste->termExists);
        fillTerm();
        // if (DEBUG) System.out.println("        found!");
        return SeekStatus::FOUND;
      }
    }
  nextTermContinue:;
  }
nextTermBreak:

  // It is possible (and OK) that terms index pointed us
  // at this block, but, we scanned the entire block and
  // did not find the term to position to.  This happens
  // when the target is after the last term in the block
  // (but, before the next term in the index).  EG
  // target could be foozzz, and terms index pointed us
  // to the foo* block, but the last term in this block
  // was fooz (and, eg, first term in the next block will
  // bee fop).
  // if (DEBUG) System.out.println("      block end");
  if (exactOnly) {
    fillTerm();
  }

  // TODO: not consistent that in the
  // not-exact case we don't next() into the next
  // frame here
  return SeekStatus::END;
}

void SegmentTermsEnumFrame::fillTerm()
{
  constexpr int termLength = prefix + suffix;
  ste->term_->setLength(termLength);
  ste->term_.grow(termLength);
  System::arraycopy(suffixBytes, startBytePos, ste->term_.bytes(), prefix,
                    suffix);
}
} // namespace org::apache::lucene::codecs::blocktree