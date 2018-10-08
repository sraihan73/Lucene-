using namespace std;

#include "SegmentTermsEnum.h"
#include "../../index/PostingsEnum.h"
#include "../../index/TermState.h"
#include "../../store/ByteArrayDataInput.h"
#include "../../store/IndexInput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/BytesRefBuilder.h"
#include "../../util/RamUsageEstimator.h"
#include "../../util/fst/FST.h"
#include "../../util/fst/Util.h"
#include "../BlockTermState.h"
#include "BlockTreeTermsReader.h"
#include "FieldReader.h"
#include "SegmentTermsEnumFrame.h"
#include "Stats.h"

namespace org::apache::lucene::codecs::blocktree
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;

SegmentTermsEnum::SegmentTermsEnum(shared_ptr<FieldReader> fr) throw(
    IOException)
    : staticFrame(make_shared<SegmentTermsEnumFrame>(shared_from_this(), -1)),
      fr(fr)
{

  // if (DEBUG) {
  //   System.out.println("BTTR.init seg=" + fr.parent.segment);
  // }
  stack = std::deque<std::shared_ptr<SegmentTermsEnumFrame>>(0);

  // Used to hold seek by TermState, or cached seek

  if (fr->index == nullptr) {
    fstReader.reset();
  } else {
    fstReader = fr->index->getBytesReader();
  }

  // Init w/ root block; don't use index since it may
  // not (and need not) have been loaded
  for (int arcIdx = 0; arcIdx < arcs.size(); arcIdx++) {
    arcs[arcIdx] = make_shared<FST::Arc<BytesRef>>();
  }

  currentFrame = staticFrame;
  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> *const arc;
  if (fr->index != nullptr) {
    arc = fr->index->getFirstArc(arcs[0]);
    // Empty string prefix must have an output in the index!
    assert(arc->isFinal());
  } else {
    arc.reset();
  }
  // currentFrame = pushFrame(arc, rootCode, 0);
  // currentFrame.loadBlock();
  validIndexPrefix = 0;
  // if (DEBUG) {
  //   System.out.println("init frame state " + currentFrame.ord);
  //   printSeekState();
  // }

  // System.out.println();
  // computeBlockStats().print(System.out);
}

void SegmentTermsEnum::initIndexInput()
{
  if (this->in_ == nullptr) {
    this->in_ = fr->parent->termsIn->clone();
  }
}

shared_ptr<Stats> SegmentTermsEnum::computeBlockStats() 
{

  // TODO: add total auto-prefix term count

  shared_ptr<Stats> stats =
      make_shared<Stats>(fr->parent->segment, fr->fieldInfo->name);
  if (fr->index != nullptr) {
    stats->indexNumBytes = fr->index->ramBytesUsed();
  }

  currentFrame = staticFrame;
  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc;
  if (fr->index != nullptr) {
    arc = fr->index->getFirstArc(arcs[0]);
    // Empty string prefix must have an output in the index!
    assert(arc->isFinal());
  } else {
    arc.reset();
  }

  // Empty string prefix must have an output in the
  // index!
  currentFrame = pushFrame(arc, fr->rootCode, 0);
  currentFrame->fpOrig = currentFrame->fp;
  currentFrame->loadBlock();
  validIndexPrefix = 0;

  stats->startBlock(currentFrame, !currentFrame->isLastInFloor);

  while (true) {

    // Pop finished blocks
    while (currentFrame->nextEnt == currentFrame->entCount) {
      stats->endBlock(currentFrame);
      if (!currentFrame->isLastInFloor) {
        // Advance to next floor block
        currentFrame->loadNextFloorBlock();
        stats->startBlock(currentFrame, true);
        break;
      } else {
        if (currentFrame->ord == 0) {
          goto allTermsBreak;
        }
        constexpr int64_t lastFP = currentFrame->fpOrig;
        currentFrame = stack[currentFrame->ord - 1];
        assert(lastFP == currentFrame->lastSubFP);
        // if (DEBUG) {
        //   System.out.println("  reset validIndexPrefix=" + validIndexPrefix);
        // }
      }
    }

    while (true) {
      if (currentFrame->next()) {
        // Push to new block:
        currentFrame =
            pushFrame(nullptr, currentFrame->lastSubFP, term_->length());
        currentFrame->fpOrig = currentFrame->fp;
        // This is a "next" frame -- even if it's
        // floor'd we must pretend it isn't so we don't
        // try to scan to the right floor frame:
        currentFrame->loadBlock();
        stats->startBlock(currentFrame, !currentFrame->isLastInFloor);
      } else {
        stats->term(term_->get());
        break;
      }
    }
  allTermsContinue:;
  }
allTermsBreak:

  stats->finish();

  // Put root frame back:
  currentFrame = staticFrame;
  if (fr->index != nullptr) {
    arc = fr->index->getFirstArc(arcs[0]);
    // Empty string prefix must have an output in the index!
    assert(arc->isFinal());
  } else {
    arc.reset();
  }
  currentFrame = pushFrame(arc, fr->rootCode, 0);
  currentFrame->rewind();
  currentFrame->loadBlock();
  validIndexPrefix = 0;
  term_->clear();

  return stats;
}

shared_ptr<SegmentTermsEnumFrame>
SegmentTermsEnum::getFrame(int ord) 
{
  if (ord >= stack.size()) {
    std::deque<std::shared_ptr<SegmentTermsEnumFrame>> next(
        ArrayUtil::oversize(1 + ord, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(stack, 0, next, 0, stack.size());
    for (int stackOrd = stack.size(); stackOrd < next.size(); stackOrd++) {
      next[stackOrd] =
          make_shared<SegmentTermsEnumFrame>(shared_from_this(), stackOrd);
    }
    stack = next;
  }
  assert(stack[ord]->ord == ord);
  return stack[ord];
}

shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>>
SegmentTermsEnum::getArc(int ord)
{
  if (ord >= arcs.size()) {
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
    // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.util.BytesRef>[] next
    // = new
    // org.apache.lucene.util.fst.FST.Arc[org.apache.lucene.util.ArrayUtil.oversize(1+ord,
    // org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF)];
    std::deque<FST::Arc<std::shared_ptr<BytesRef>>> next =
        std::deque<std::shared_ptr<FST::Arc>>(ArrayUtil::oversize(
            1 + ord, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(arcs, 0, next, 0, arcs.size());
    for (int arcOrd = arcs.size(); arcOrd < next.size(); arcOrd++) {
      next[arcOrd] = make_shared<FST::Arc<BytesRef>>();
    }
    arcs = next;
  }
  return arcs[ord];
}

shared_ptr<SegmentTermsEnumFrame>
SegmentTermsEnum::pushFrame(shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc,
                            shared_ptr<BytesRef> frameData,
                            int length) 
{
  scratchReader->reset(frameData->bytes, frameData->offset, frameData->length);
  constexpr int64_t code = scratchReader->readVLong();
  constexpr int64_t fpSeek =
      static_cast<int64_t>(static_cast<uint64_t>(code) >>
                             BlockTreeTermsReader::OUTPUT_FLAGS_NUM_BITS);
  shared_ptr<SegmentTermsEnumFrame> *const f = getFrame(1 + currentFrame->ord);
  f->hasTerms = (code & BlockTreeTermsReader::OUTPUT_FLAG_HAS_TERMS) != 0;
  f->hasTermsOrig = f->hasTerms;
  f->isFloor = (code & BlockTreeTermsReader::OUTPUT_FLAG_IS_FLOOR) != 0;
  if (f->isFloor) {
    f->setFloorData(scratchReader, frameData);
  }
  pushFrame(arc, fpSeek, length);

  return f;
}

shared_ptr<SegmentTermsEnumFrame>
SegmentTermsEnum::pushFrame(shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc,
                            int64_t fp, int length) 
{
  shared_ptr<SegmentTermsEnumFrame> *const f = getFrame(1 + currentFrame->ord);
  f->arc = arc;
  if (f->fpOrig == fp && f->nextEnt != -1) {
    // if (DEBUG) System.out.println("      push reused frame ord=" + f.ord + "
    // fp=" + f.fp + " isFloor?=" + f.isFloor + " hasTerms=" + f.hasTerms + "
    // pref=" + term + " nextEnt=" + f.nextEnt + " targetBeforeCurrentLength=" +
    // targetBeforeCurrentLength + " term.length=" + term.length + " vs prefix="
    // + f.prefix); if (f.prefix > targetBeforeCurrentLength) {
    if (f->ord > targetBeforeCurrentLength) {
      f->rewind();
    } else {
      // if (DEBUG) {
      //   System.out.println("        skip rewind!");
      // }
    }
    assert(length == f->prefix);
  } else {
    f->nextEnt = -1;
    f->prefix = length;
    f->state->termBlockOrd = 0;
    f->fpOrig = f->fp = fp;
    f->lastSubFP = -1;
    // if (DEBUG) {
    //   final int sav = term.length;
    //   term.length = length;
    //   System.out.println("      push new frame ord=" + f.ord + " fp=" + f.fp
    //   + " hasTerms=" + f.hasTerms + " isFloor=" + f.isFloor + " pref=" +
    //   brToString(term)); term.length = sav;
    // }
  }

  return f;
}

bool SegmentTermsEnum::clearEOF()
{
  eof = false;
  return true;
}

bool SegmentTermsEnum::setEOF()
{
  eof = true;
  return true;
}

bool SegmentTermsEnum::seekExact(shared_ptr<BytesRef> target) 
{

  if (fr->index == nullptr) {
    throw make_shared<IllegalStateException>(L"terms index was not loaded");
  }

  term_->grow(1 + target->length);

  assert(clearEOF());

  // if (DEBUG) {
  //   System.out.println("\nBTTR.seekExact seg=" + fr.parent.segment + "
  //   target=" + fr.fieldInfo.name + ":" + brToString(target) + " current=" +
  //   brToString(term) + " (exists?=" + termExists + ") validIndexPrefix=" +
  //   validIndexPrefix); printSeekState(System.out);
  // }

  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc;
  int targetUpto;
  shared_ptr<BytesRef> output;

  targetBeforeCurrentLength = currentFrame->ord;

  if (currentFrame != staticFrame) {

    // We are already seek'd; find the common
    // prefix of new seek term vs current term and
    // re-use the corresponding seek state.  For
    // example, if app first seeks to foobar, then
    // seeks to foobaz, we can re-use the seek state
    // for the first 5 bytes.

    // if (DEBUG) {
    //   System.out.println("  re-use current seek state validIndexPrefix=" +
    //   validIndexPrefix);
    // }

    arc = arcs[0];
    assert(arc->isFinal());
    output = arc->output;
    targetUpto = 0;

    shared_ptr<SegmentTermsEnumFrame> lastFrame = stack[0];
    assert(validIndexPrefix <= term_->length());

    constexpr int targetLimit = min(target->length, validIndexPrefix);

    int cmp = 0;

    // TODO: reverse vLong byte order for better FST
    // prefix output sharing

    // First compare up to valid seek frames:
    while (targetUpto < targetLimit) {
      cmp = (term_->byteAt(targetUpto) & 0xFF) -
            (target->bytes[target->offset + targetUpto] & 0xFF);
      // if (DEBUG) {
      //    System.out.println("    cycle targetUpto=" + targetUpto + " (vs
      //    limit=" + targetLimit + ") cmp=" + cmp + " (targetLabel=" + (char)
      //    (target.bytes[target.offset + targetUpto]) + " vs termLabel=" +
      //    (char) (term.bytes[targetUpto]) + ")"   + " arc.output=" +
      //    arc.output + " output=" + output);
      // }
      if (cmp != 0) {
        break;
      }
      arc = arcs[1 + targetUpto];
      assert((arc->label == (target->bytes[target->offset + targetUpto] & 0xFF),
              L"arc.label=" +
                  StringHelper::toString(static_cast<wchar_t>(arc->label)) +
                  L" targetLabel=" +
                  StringHelper::toString(static_cast<wchar_t>(
                      target->bytes[target->offset + targetUpto] & 0xFF))));
      if (arc->output != BlockTreeTermsReader::NO_OUTPUT) {
        output = BlockTreeTermsReader::FST_OUTPUTS->add(output, arc->output);
      }
      if (arc->isFinal()) {
        lastFrame = stack[1 + lastFrame->ord];
      }
      targetUpto++;
    }

    if (cmp == 0) {
      constexpr int targetUptoMid = targetUpto;

      // Second compare the rest of the term, but
      // don't save arc/output/frame; we only do this
      // to find out if the target term is before,
      // equal or after the current term
      constexpr int targetLimit2 = min(target->length, term_->length());
      while (targetUpto < targetLimit2) {
        cmp = (term_->byteAt(targetUpto) & 0xFF) -
              (target->bytes[target->offset + targetUpto] & 0xFF);
        // if (DEBUG) {
        //    System.out.println("    cycle2 targetUpto=" + targetUpto + " (vs
        //    limit=" + targetLimit + ") cmp=" + cmp + " (targetLabel=" + (char)
        //    (target.bytes[target.offset + targetUpto]) + " vs termLabel=" +
        //    (char) (term.bytes[targetUpto]) + ")");
        // }
        if (cmp != 0) {
          break;
        }
        targetUpto++;
      }

      if (cmp == 0) {
        cmp = term_->length() - target->length;
      }
      targetUpto = targetUptoMid;
    }

    if (cmp < 0) {
      // Common case: target term is after current
      // term, ie, app is seeking multiple terms
      // in sorted order
      // if (DEBUG) {
      //   System.out.println("  target is after current (shares prefixLen=" +
      //   targetUpto + "); frame.ord=" + lastFrame.ord);
      // }
      currentFrame = lastFrame;

    } else if (cmp > 0) {
      // Uncommon case: target term
      // is before current term; this means we can
      // keep the currentFrame but we must rewind it
      // (so we scan from the start)
      targetBeforeCurrentLength = lastFrame->ord;
      // if (DEBUG) {
      //   System.out.println("  target is before current (shares prefixLen=" +
      //   targetUpto + "); rewind frame ord=" + lastFrame.ord);
      // }
      currentFrame = lastFrame;
      currentFrame->rewind();
    } else {
      // Target is exactly the same as current term
      assert(term_->length() == target->length);
      if (termExists) {
        // if (DEBUG) {
        //   System.out.println("  target is same as current; return true");
        // }
        return true;
      } else {
        // if (DEBUG) {
        //   System.out.println("  target is same as current but term doesn't
        //   exist");
        // }
      }
      // validIndexPrefix = currentFrame.depth;
      // term.length = target.length;
      // return termExists;
    }

  } else {

    targetBeforeCurrentLength = -1;
    arc = fr->index->getFirstArc(arcs[0]);

    // Empty string prefix must have an output (block) in the index!
    assert(arc->isFinal());
    assert(arc->output != nullptr);

    // if (DEBUG) {
    //   System.out.println("    no seek state; push root frame");
    // }

    output = arc->output;

    currentFrame = staticFrame;

    // term.length = 0;
    targetUpto = 0;
    currentFrame = pushFrame(
        arc,
        BlockTreeTermsReader::FST_OUTPUTS->add(output, arc->nextFinalOutput),
        0);
  }

  // if (DEBUG) {
  //   System.out.println("  start index loop targetUpto=" + targetUpto + "
  //   output=" + output + " currentFrame.ord=" + currentFrame.ord + "
  //   targetBeforeCurrentLength=" + targetBeforeCurrentLength);
  // }

  // We are done sharing the common prefix with the incoming target and where we
  // are currently seek'd; now continue walking the index:
  while (targetUpto < target->length) {

    constexpr int targetLabel =
        target->bytes[target->offset + targetUpto] & 0xFF;

    shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> *const nextArc =
        fr->index->findTargetArc(targetLabel, arc, getArc(1 + targetUpto),
                                 fstReader);

    if (nextArc == nullptr) {

      // Index is exhausted
      // if (DEBUG) {
      //   System.out.println("    index: index exhausted label=" + ((char)
      //   targetLabel) + " " + toHex(targetLabel));
      // }

      validIndexPrefix = currentFrame->prefix;
      // validIndexPrefix = targetUpto;

      currentFrame->scanToFloorFrame(target);

      if (!currentFrame->hasTerms) {
        termExists = false;
        term_->setByteAt(targetUpto, static_cast<char>(targetLabel));
        term_->setLength(1 + targetUpto);
        // if (DEBUG) {
        //   System.out.println("  FAST NOT_FOUND term=" + brToString(term));
        // }
        return false;
      }

      currentFrame->loadBlock();

      constexpr SeekStatus result = currentFrame->scanToTerm(target, true);
      if (result == SeekStatus::FOUND) {
        // if (DEBUG) {
        //   System.out.println("  return FOUND term=" + term.utf8ToString() + "
        //   " + term);
        // }
        return true;
      } else {
        // if (DEBUG) {
        //   System.out.println("  got " + result + "; return NOT_FOUND term=" +
        //   brToString(term));
        // }
        return false;
      }
    } else {
      // Follow this arc
      arc = nextArc;
      term_->setByteAt(targetUpto, static_cast<char>(targetLabel));
      // Aggregate output as we go:
      assert(arc->output != nullptr);
      if (arc->output != BlockTreeTermsReader::NO_OUTPUT) {
        output = BlockTreeTermsReader::FST_OUTPUTS->add(output, arc->output);
      }

      // if (DEBUG) {
      //   System.out.println("    index: follow label=" +
      //   toHex(target.bytes[target.offset + targetUpto]&0xff) + " arc.output="
      //   + arc.output + " arc.nfo=" + arc.nextFinalOutput);
      // }
      targetUpto++;

      if (arc->isFinal()) {
        // if (DEBUG) System.out.println("    arc is final!");
        currentFrame = pushFrame(arc,
                                 BlockTreeTermsReader::FST_OUTPUTS->add(
                                     output, arc->nextFinalOutput),
                                 targetUpto);
        // if (DEBUG) System.out.println("    curFrame.ord=" + currentFrame.ord
        // + " hasTerms=" + currentFrame.hasTerms);
      }
    }
  }

  // validIndexPrefix = targetUpto;
  validIndexPrefix = currentFrame->prefix;

  currentFrame->scanToFloorFrame(target);

  // Target term is entirely contained in the index:
  if (!currentFrame->hasTerms) {
    termExists = false;
    term_->setLength(targetUpto);
    // if (DEBUG) {
    //   System.out.println("  FAST NOT_FOUND term=" + brToString(term));
    // }
    return false;
  }

  currentFrame->loadBlock();

  constexpr SeekStatus result = currentFrame->scanToTerm(target, true);
  if (result == SeekStatus::FOUND) {
    // if (DEBUG) {
    //   System.out.println("  return FOUND term=" + term.utf8ToString() + " " +
    //   term);
    // }
    return true;
  } else {
    // if (DEBUG) {
    //   System.out.println("  got result " + result + "; return NOT_FOUND
    //   term=" + term.utf8ToString());
    // }

    return false;
  }
}

SeekStatus
SegmentTermsEnum::seekCeil(shared_ptr<BytesRef> target) 
{

  if (fr->index == nullptr) {
    throw make_shared<IllegalStateException>(L"terms index was not loaded");
  }

  term_->grow(1 + target->length);

  assert(clearEOF());

  // if (DEBUG) {
  //   System.out.println("\nBTTR.seekCeil seg=" + fr.parent.segment + "
  //   target=" + fr.fieldInfo.name + ":" + brToString(target) + " " + target +
  //   " current=" + brToString(term) + " (exists?=" + termExists + ")
  //   validIndexPrefix=  " + validIndexPrefix); printSeekState(System.out);
  // }

  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc;
  int targetUpto;
  shared_ptr<BytesRef> output;

  targetBeforeCurrentLength = currentFrame->ord;

  if (currentFrame != staticFrame) {

    // We are already seek'd; find the common
    // prefix of new seek term vs current term and
    // re-use the corresponding seek state.  For
    // example, if app first seeks to foobar, then
    // seeks to foobaz, we can re-use the seek state
    // for the first 5 bytes.

    // if (DEBUG) {
    // System.out.println("  re-use current seek state validIndexPrefix=" +
    // validIndexPrefix);
    //}

    arc = arcs[0];
    assert(arc->isFinal());
    output = arc->output;
    targetUpto = 0;

    shared_ptr<SegmentTermsEnumFrame> lastFrame = stack[0];
    assert(validIndexPrefix <= term_->length());

    constexpr int targetLimit = min(target->length, validIndexPrefix);

    int cmp = 0;

    // TODO: we should write our vLong backwards (MSB
    // first) to get better sharing from the FST

    // First compare up to valid seek frames:
    while (targetUpto < targetLimit) {
      cmp = (term_->byteAt(targetUpto) & 0xFF) -
            (target->bytes[target->offset + targetUpto] & 0xFF);
      // if (DEBUG) {
      // System.out.println("    cycle targetUpto=" + targetUpto + " (vs limit="
      // + targetLimit + ") cmp=" + cmp + " (targetLabel=" + (char)
      // (target.bytes[target.offset + targetUpto]) + " vs termLabel=" + (char)
      // (term.byteAt(targetUpto)) + ")"   + " arc.output=" + arc.output + "
      // output=" + output);
      //}
      if (cmp != 0) {
        break;
      }
      arc = arcs[1 + targetUpto];
      assert((arc->label == (target->bytes[target->offset + targetUpto] & 0xFF),
              L"arc.label=" +
                  StringHelper::toString(static_cast<wchar_t>(arc->label)) +
                  L" targetLabel=" +
                  StringHelper::toString(static_cast<wchar_t>(
                      target->bytes[target->offset + targetUpto] & 0xFF))));
      // TODO: we could save the outputs in local
      // byte[][] instead of making new objs ever
      // seek; but, often the FST doesn't have any
      // shared bytes (but this could change if we
      // reverse vLong byte order)
      if (arc->output != BlockTreeTermsReader::NO_OUTPUT) {
        output = BlockTreeTermsReader::FST_OUTPUTS->add(output, arc->output);
      }
      if (arc->isFinal()) {
        lastFrame = stack[1 + lastFrame->ord];
      }
      targetUpto++;
    }

    if (cmp == 0) {
      constexpr int targetUptoMid = targetUpto;
      // Second compare the rest of the term, but
      // don't save arc/output/frame:
      constexpr int targetLimit2 = min(target->length, term_->length());
      while (targetUpto < targetLimit2) {
        cmp = (term_->byteAt(targetUpto) & 0xFF) -
              (target->bytes[target->offset + targetUpto] & 0xFF);
        // if (DEBUG) {
        // System.out.println("    cycle2 targetUpto=" + targetUpto + " (vs
        // limit=" + targetLimit + ") cmp=" + cmp + " (targetLabel=" + (char)
        // (target.bytes[target.offset + targetUpto]) + " vs termLabel=" + (char)
        // (term.byteAt(targetUpto)) + ")");
        //}
        if (cmp != 0) {
          break;
        }
        targetUpto++;
      }

      if (cmp == 0) {
        cmp = term_->length() - target->length;
      }
      targetUpto = targetUptoMid;
    }

    if (cmp < 0) {
      // Common case: target term is after current
      // term, ie, app is seeking multiple terms
      // in sorted order
      // if (DEBUG) {
      // System.out.println("  target is after current (shares prefixLen=" +
      // targetUpto + "); clear frame.scanned ord=" + lastFrame.ord);
      //}
      currentFrame = lastFrame;

    } else if (cmp > 0) {
      // Uncommon case: target term
      // is before current term; this means we can
      // keep the currentFrame but we must rewind it
      // (so we scan from the start)
      targetBeforeCurrentLength = 0;
      // if (DEBUG) {
      // System.out.println("  target is before current (shares prefixLen=" +
      // targetUpto + "); rewind frame ord=" + lastFrame.ord);
      //}
      currentFrame = lastFrame;
      currentFrame->rewind();
    } else {
      // Target is exactly the same as current term
      assert(term_->length() == target->length);
      if (termExists) {
        // if (DEBUG) {
        // System.out.println("  target is same as current; return FOUND");
        //}
        return SeekStatus::FOUND;
      } else {
        // if (DEBUG) {
        // System.out.println("  target is same as current but term doesn't
        // exist");
        //}
      }
    }

  } else {

    targetBeforeCurrentLength = -1;
    arc = fr->index->getFirstArc(arcs[0]);

    // Empty string prefix must have an output (block) in the index!
    assert(arc->isFinal());
    assert(arc->output != nullptr);

    // if (DEBUG) {
    // System.out.println("    no seek state; push root frame");
    //}

    output = arc->output;

    currentFrame = staticFrame;

    // term.length = 0;
    targetUpto = 0;
    currentFrame = pushFrame(
        arc,
        BlockTreeTermsReader::FST_OUTPUTS->add(output, arc->nextFinalOutput),
        0);
  }

  // if (DEBUG) {
  // System.out.println("  start index loop targetUpto=" + targetUpto + "
  // output=" + output + " currentFrame.ord+1=" + currentFrame.ord + "
  // targetBeforeCurrentLength=" + targetBeforeCurrentLength);
  //}

  // We are done sharing the common prefix with the incoming target and where we
  // are currently seek'd; now continue walking the index:
  while (targetUpto < target->length) {

    constexpr int targetLabel =
        target->bytes[target->offset + targetUpto] & 0xFF;

    shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> *const nextArc =
        fr->index->findTargetArc(targetLabel, arc, getArc(1 + targetUpto),
                                 fstReader);

    if (nextArc == nullptr) {

      // Index is exhausted
      // if (DEBUG) {
      //   System.out.println("    index: index exhausted label=" + ((char)
      //   targetLabel) + " " + targetLabel);
      // }

      validIndexPrefix = currentFrame->prefix;
      // validIndexPrefix = targetUpto;

      currentFrame->scanToFloorFrame(target);

      currentFrame->loadBlock();

      // if (DEBUG) System.out.println("  now scanToTerm");
      constexpr SeekStatus result = currentFrame->scanToTerm(target, false);
      if (result == SeekStatus::END) {
        term_->copyBytes(target);
        termExists = false;

        if (next() != nullptr) {
          // if (DEBUG) {
          // System.out.println("  return NOT_FOUND term=" + brToString(term));
          //}
          return SeekStatus::NOT_FOUND;
        } else {
          // if (DEBUG) {
          // System.out.println("  return END");
          //}
          return SeekStatus::END;
        }
      } else {
        // if (DEBUG) {
        // System.out.println("  return " + result + " term=" +
        // brToString(term));
        //}
        return result;
      }
    } else {
      // Follow this arc
      term_->setByteAt(targetUpto, static_cast<char>(targetLabel));
      arc = nextArc;
      // Aggregate output as we go:
      assert(arc->output != nullptr);
      if (arc->output != BlockTreeTermsReader::NO_OUTPUT) {
        output = BlockTreeTermsReader::FST_OUTPUTS->add(output, arc->output);
      }

      // if (DEBUG) {
      // System.out.println("    index: follow label=" +
      // (target.bytes[target.offset + targetUpto]&0xff) + " arc.output=" +
      // arc.output + " arc.nfo=" + arc.nextFinalOutput);
      //}
      targetUpto++;

      if (arc->isFinal()) {
        // if (DEBUG) System.out.println("    arc is final!");
        currentFrame = pushFrame(arc,
                                 BlockTreeTermsReader::FST_OUTPUTS->add(
                                     output, arc->nextFinalOutput),
                                 targetUpto);
        // if (DEBUG) System.out.println("    curFrame.ord=" + currentFrame.ord
        // + " hasTerms=" + currentFrame.hasTerms);
      }
    }
  }

  // validIndexPrefix = targetUpto;
  validIndexPrefix = currentFrame->prefix;

  currentFrame->scanToFloorFrame(target);

  currentFrame->loadBlock();

  constexpr SeekStatus result = currentFrame->scanToTerm(target, false);

  if (result == SeekStatus::END) {
    term_->copyBytes(target);
    termExists = false;
    if (next() != nullptr) {
      // if (DEBUG) {
      // System.out.println("  return NOT_FOUND term=" +
      // term.get().utf8ToString() + " " + term);
      //}
      return SeekStatus::NOT_FOUND;
    } else {
      // if (DEBUG) {
      // System.out.println("  return END");
      //}
      return SeekStatus::END;
    }
  } else {
    return result;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") private void
// printSeekState(java.io.PrintStream out) throws java.io.IOException
void SegmentTermsEnum::printSeekState(shared_ptr<PrintStream> out) throw(
    IOException)
{
  if (currentFrame == staticFrame) {
    out->println(L"  no prior seek");
  } else {
    out->println(L"  prior seek state:");
    int ord = 0;
    bool isSeekFrame = true;
    while (true) {
      shared_ptr<SegmentTermsEnumFrame> f = getFrame(ord);
      assert(f != nullptr);
      shared_ptr<BytesRef> *const prefix =
          make_shared<BytesRef>(term_->get().bytes, 0, f->prefix);
      if (f->nextEnt == -1) {
        out->println(
            L"    frame " + (isSeekFrame ? L"(seek)" : L"(next)") + L" ord=" +
            to_wstring(ord) + L" fp=" + to_wstring(f->fp) +
            (f->isFloor ? (L" (fpOrig=" + to_wstring(f->fpOrig) + L")") : L"") +
            L" prefixLen=" + to_wstring(f->prefix) + L" prefix=" + prefix +
            StringHelper::toString(
                f->nextEnt == -1
                    ? L""
                    : (L" (of " + to_wstring(f->entCount) + L")")) +
            L" hasTerms=" + StringHelper::toString(f->hasTerms) + L" isFloor=" +
            StringHelper::toString(f->isFloor) + L" code=" +
            to_wstring(
                (f->fp << BlockTreeTermsReader::OUTPUT_FLAGS_NUM_BITS) +
                (f->hasTerms ? BlockTreeTermsReader::OUTPUT_FLAG_HAS_TERMS
                             : 0) +
                (f->isFloor ? BlockTreeTermsReader::OUTPUT_FLAG_IS_FLOOR : 0)) +
            L" isLastInFloor=" + StringHelper::toString(f->isLastInFloor) +
            L" mdUpto=" + to_wstring(f->metaDataUpto) + L" tbOrd=" +
            to_wstring(f->getTermBlockOrd()));
      } else {
        out->println(
            L"    frame " +
            (isSeekFrame ? L"(seek, loaded)" : L"(next, loaded)") + L" ord=" +
            to_wstring(ord) + L" fp=" + to_wstring(f->fp) +
            (f->isFloor ? (L" (fpOrig=" + to_wstring(f->fpOrig) + L")") : L"") +
            L" prefixLen=" + to_wstring(f->prefix) + L" prefix=" + prefix +
            L" nextEnt=" + to_wstring(f->nextEnt) +
            StringHelper::toString(
                f->nextEnt == -1
                    ? L""
                    : (L" (of " + to_wstring(f->entCount) + L")")) +
            L" hasTerms=" + StringHelper::toString(f->hasTerms) + L" isFloor=" +
            StringHelper::toString(f->isFloor) + L" code=" +
            to_wstring(
                (f->fp << BlockTreeTermsReader::OUTPUT_FLAGS_NUM_BITS) +
                (f->hasTerms ? BlockTreeTermsReader::OUTPUT_FLAG_HAS_TERMS
                             : 0) +
                (f->isFloor ? BlockTreeTermsReader::OUTPUT_FLAG_IS_FLOOR : 0)) +
            L" lastSubFP=" + to_wstring(f->lastSubFP) + L" isLastInFloor=" +
            StringHelper::toString(f->isLastInFloor) + L" mdUpto=" +
            to_wstring(f->metaDataUpto) + L" tbOrd=" +
            to_wstring(f->getTermBlockOrd()));
      }
      if (fr->index != nullptr) {
        assert((!isSeekFrame || f->arc != nullptr,
                L"isSeekFrame=" + StringHelper::toString(isSeekFrame) +
                    L" f.arc=" + f->arc));
        if (f->prefix > 0 && isSeekFrame &&
            f->arc->label != (term_->byteAt(f->prefix - 1) & 0xFF)) {
          out->println(
              L"      broken seek state: arc.label=" +
              StringHelper::toString(static_cast<wchar_t>(f->arc->label)) +
              L" vs term byte=" +
              StringHelper::toString(
                  static_cast<wchar_t>(term_->byteAt(f->prefix - 1) & 0xFF)));
          throw runtime_error(L"seek state is broken");
        }
        shared_ptr<BytesRef> output = Util::get(fr->index, prefix);
        if (output == nullptr) {
          out->println(
              L"      broken seek state: prefix is not final in index");
          throw runtime_error(L"seek state is broken");
        } else if (isSeekFrame && !f->isFloor) {
          shared_ptr<ByteArrayDataInput> *const reader =
              make_shared<ByteArrayDataInput>(output->bytes, output->offset,
                                              output->length);
          constexpr int64_t codeOrig = reader->readVLong();
          constexpr int64_t code =
              (f->fp << BlockTreeTermsReader::OUTPUT_FLAGS_NUM_BITS) |
              (f->hasTerms ? BlockTreeTermsReader::OUTPUT_FLAG_HAS_TERMS : 0) |
              (f->isFloor ? BlockTreeTermsReader::OUTPUT_FLAG_IS_FLOOR : 0);
          if (codeOrig != code) {
            out->println(L"      broken seek state: output code=" +
                         to_wstring(codeOrig) + L" doesn't match frame code=" +
                         to_wstring(code));
            throw runtime_error(L"seek state is broken");
          }
        }
      }
      if (f == currentFrame) {
        break;
      }
      if (f->prefix == validIndexPrefix) {
        isSeekFrame = false;
      }
      ord++;
    }
  }
}

shared_ptr<BytesRef> SegmentTermsEnum::next() 
{
  if (in_ == nullptr) {
    // Fresh TermsEnum; seek to first term:
    shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> *const arc;
    if (fr->index != nullptr) {
      arc = fr->index->getFirstArc(arcs[0]);
      // Empty string prefix must have an output in the index!
      assert(arc->isFinal());
    } else {
      arc.reset();
    }
    currentFrame = pushFrame(arc, fr->rootCode, 0);
    currentFrame->loadBlock();
  }

  targetBeforeCurrentLength = currentFrame->ord;

  assert(!eof);
  // if (DEBUG) {
  //   System.out.println("\nBTTR.next seg=" + fr.parent.segment + " term=" +
  //   brToString(term) + " termExists?=" + termExists + " field=" +
  //   fr.fieldInfo.name + " termBlockOrd=" + currentFrame.state.termBlockOrd +
  //   " validIndexPrefix=" + validIndexPrefix); printSeekState(System.out);
  // }

  if (currentFrame == staticFrame) {
    // If seek was previously called and the term was
    // cached, or seek(TermState) was called, usually
    // caller is just going to pull a D/&PEnum or get
    // docFreq, etc.  But, if they then call next(),
    // this method catches up all internal state so next()
    // works properly:
    // if (DEBUG) System.out.println("  re-seek to pending term=" +
    // term.utf8ToString() + " " + term);
    constexpr bool result = seekExact(term_->get());
    assert(result);
  }

  // Pop finished blocks
  while (currentFrame->nextEnt == currentFrame->entCount) {
    if (!currentFrame->isLastInFloor) {
      // Advance to next floor block
      currentFrame->loadNextFloorBlock();
      break;
    } else {
      // if (DEBUG) System.out.println("  pop frame");
      if (currentFrame->ord == 0) {
        // if (DEBUG) System.out.println("  return null");
        assert(setEOF());
        term_->clear();
        validIndexPrefix = 0;
        currentFrame->rewind();
        termExists = false;
        return nullptr;
      }
      constexpr int64_t lastFP = currentFrame->fpOrig;
      currentFrame = stack[currentFrame->ord - 1];

      if (currentFrame->nextEnt == -1 || currentFrame->lastSubFP != lastFP) {
        // We popped into a frame that's not loaded
        // yet or not scan'd to the right entry
        currentFrame->scanToFloorFrame(term_->get());
        currentFrame->loadBlock();
        currentFrame->scanToSubBlock(lastFP);
      }

      // Note that the seek state (last seek) has been
      // invalidated beyond this depth
      validIndexPrefix = min(validIndexPrefix, currentFrame->prefix);
      // if (DEBUG) {
      // System.out.println("  reset validIndexPrefix=" + validIndexPrefix);
      //}
    }
  }

  while (true) {
    if (currentFrame->next()) {
      // Push to new block:
      // if (DEBUG) System.out.println("  push frame");
      currentFrame =
          pushFrame(nullptr, currentFrame->lastSubFP, term_->length());
      // This is a "next" frame -- even if it's
      // floor'd we must pretend it isn't so we don't
      // try to scan to the right floor frame:
      currentFrame->loadBlock();
    } else {
      // if (DEBUG) System.out.println("  return term=" + brToString(term) + "
      // currentFrame.ord=" + currentFrame.ord);
      return term_->get();
    }
  }
}

shared_ptr<BytesRef> SegmentTermsEnum::term()
{
  assert(!eof);
  return term_->get();
}

int SegmentTermsEnum::docFreq() 
{
  assert(!eof);
  // if (DEBUG) System.out.println("BTR.docFreq");
  currentFrame->decodeMetaData();
  // if (DEBUG) System.out.println("  return " + currentFrame.state.docFreq);
  return currentFrame->state->docFreq;
}

int64_t SegmentTermsEnum::totalTermFreq() 
{
  assert(!eof);
  currentFrame->decodeMetaData();
  return currentFrame->state->totalTermFreq;
}

shared_ptr<PostingsEnum>
SegmentTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                           int flags) 
{
  assert(!eof);
  // if (DEBUG) {
  // System.out.println("BTTR.docs seg=" + segment);
  //}
  currentFrame->decodeMetaData();
  // if (DEBUG) {
  // System.out.println("  state=" + currentFrame.state);
  //}
  return fr->parent->postingsReader->postings(
      fr->fieldInfo, currentFrame->state, reuse, flags);
}

void SegmentTermsEnum::seekExact(shared_ptr<BytesRef> target,
                                 shared_ptr<TermState> otherState)
{
  // if (DEBUG) {
  //   System.out.println("BTTR.seekExact termState seg=" + segment + " target="
  //   + target.utf8ToString() + " " + target + " state=" + otherState);
  // }
  assert(clearEOF());
  if (target->compareTo(term_->get()) != 0 || !termExists) {
    assert(otherState != nullptr &&
           std::dynamic_pointer_cast<BlockTermState>(otherState) != nullptr);
    currentFrame = staticFrame;
    currentFrame->state->copyFrom(otherState);
    term_->copyBytes(target);
    currentFrame->metaDataUpto = currentFrame->getTermBlockOrd();
    assert(currentFrame->metaDataUpto > 0);
    validIndexPrefix = 0;
  } else {
    // if (DEBUG) {
    //   System.out.println("  skip seek: already on target state=" +
    //   currentFrame.state);
    // }
  }
}

shared_ptr<TermState> SegmentTermsEnum::termState() 
{
  assert(!eof);
  currentFrame->decodeMetaData();
  shared_ptr<TermState> ts = currentFrame->state->clone();
  // if (DEBUG) System.out.println("BTTR.termState seg=" + segment + " state=" +
  // ts);
  return ts;
}

void SegmentTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t SegmentTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::blocktree