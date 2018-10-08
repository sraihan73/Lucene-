using namespace std;

#include "OrdsSegmentTermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "FSTOrdsOutputs.h"
#include "OrdsBlockTreeTermsWriter.h"
#include "OrdsFieldReader.h"
#include "OrdsSegmentTermsEnumFrame.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;

OrdsSegmentTermsEnum::OrdsSegmentTermsEnum(
    shared_ptr<OrdsFieldReader> fr) 
    : staticFrame(
          make_shared<OrdsSegmentTermsEnumFrame>(shared_from_this(), -1)),
      fr(fr)
{

  // if (DEBUG) System.out.println("BTTR.init seg=" + segment);
  stack = std::deque<std::shared_ptr<OrdsSegmentTermsEnumFrame>>(0);

  // Used to hold seek by TermState, or cached seek

  if (fr->index == nullptr) {
    fstReader.reset();
  } else {
    fstReader = fr->index->getBytesReader();
  }

  // Init w/ root block; don't use index since it may
  // not (and need not) have been loaded
  for (int arcIdx = 0; arcIdx < arcs.size(); arcIdx++) {
    arcs[arcIdx] = make_shared<FST::Arc<Output>>();
  }

  currentFrame = staticFrame;
  shared_ptr<FST::Arc<std::shared_ptr<Output>>> *const arc;
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

void OrdsSegmentTermsEnum::initIndexInput()
{
  if (this->in_ == nullptr) {
    this->in_ = fr->parent->in_->clone();
  }
}

shared_ptr<OrdsSegmentTermsEnumFrame>
OrdsSegmentTermsEnum::getFrame(int ord) 
{
  if (ord >= stack.size()) {
    std::deque<std::shared_ptr<OrdsSegmentTermsEnumFrame>> next(
        ArrayUtil::oversize(1 + ord, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(stack, 0, next, 0, stack.size());
    for (int stackOrd = stack.size(); stackOrd < next.size(); stackOrd++) {
      next[stackOrd] =
          make_shared<OrdsSegmentTermsEnumFrame>(shared_from_this(), stackOrd);
    }
    stack = next;
  }
  assert(stack[ord]->ord == ord);
  return stack[ord];
}

shared_ptr<FST::Arc<std::shared_ptr<Output>>>
OrdsSegmentTermsEnum::getArc(int ord)
{
  if (ord >= arcs.size()) {
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
    // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.codecs.blocktreeords.FSTOrdsOutputs.Output>[]
    // next = new
    // org.apache.lucene.util.fst.FST.Arc[org.apache.lucene.util.ArrayUtil.oversize(1+ord,
    // org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF)];
    std::deque<FST::Arc<std::shared_ptr<Output>>> next =
        std::deque<std::shared_ptr<FST::Arc>>(ArrayUtil::oversize(
            1 + ord, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(arcs, 0, next, 0, arcs.size());
    for (int arcOrd = arcs.size(); arcOrd < next.size(); arcOrd++) {
      next[arcOrd] = make_shared<FST::Arc<Output>>();
    }
    arcs = next;
  }
  return arcs[ord];
}

shared_ptr<OrdsSegmentTermsEnumFrame> OrdsSegmentTermsEnum::pushFrame(
    shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc,
    shared_ptr<Output> frameData, int length) 
{
  scratchReader->reset(frameData->bytes->bytes, frameData->bytes->offset,
                       frameData->bytes->length);
  constexpr int64_t code = scratchReader->readVLong();
  constexpr int64_t fpSeek =
      static_cast<int64_t>(static_cast<uint64_t>(code) >>
                             OrdsBlockTreeTermsWriter::OUTPUT_FLAGS_NUM_BITS);
  // System.out.println("    fpSeek=" + fpSeek);
  shared_ptr<OrdsSegmentTermsEnumFrame> *const f =
      getFrame(1 + currentFrame->ord);
  f->hasTerms = (code & OrdsBlockTreeTermsWriter::OUTPUT_FLAG_HAS_TERMS) != 0;
  f->hasTermsOrig = f->hasTerms;
  f->isFloor = (code & OrdsBlockTreeTermsWriter::OUTPUT_FLAG_IS_FLOOR) != 0;

  // Must setFloorData before pushFrame in case pushFrame tries to rewind:
  if (f->isFloor) {
    f->termOrdOrig = frameData->startOrd;
    f->setFloorData(scratchReader, frameData->bytes);
  }

  pushFrame(arc, fpSeek, length, frameData->startOrd);

  return f;
}

shared_ptr<OrdsSegmentTermsEnumFrame> OrdsSegmentTermsEnum::pushFrame(
    shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc, int64_t fp, int length,
    int64_t termOrd) 
{
  shared_ptr<OrdsSegmentTermsEnumFrame> *const f =
      getFrame(1 + currentFrame->ord);
  f->arc = arc;
  // System.out.println("pushFrame termOrd= " + termOrd + " fpOrig=" + f.fpOrig
  // + " fp=" + fp + " nextEnt=" + f.nextEnt);
  if (f->fpOrig == fp && f->nextEnt != -1) {
    // if (DEBUG) System.out.println("      push reused frame ord=" + f.ord + "
    // fp=" + f.fp + " isFloor?=" + f.isFloor + " hasTerms=" + f.hasTerms + "
    // pref=" + term + " nextEnt=" + f.nextEnt + " targetBeforeCurrentLength=" +
    // targetBeforeCurrentLength + " term.length=" + term.length + " vs prefix="
    // + f.prefix);
    if (f->prefix > targetBeforeCurrentLength) {
      // System.out.println("        do rewind!");
      f->rewind();
    } else {
      // if (DEBUG) {
      // System.out.println("        skip rewind!");
      // }
    }
    assert(length == f->prefix);
    assert(termOrd == f->termOrdOrig);
  } else {
    f->nextEnt = -1;
    f->prefix = length;
    f->state->termBlockOrd = 0;
    f->termOrdOrig = termOrd;
    // System.out.println("set termOrdOrig=" + termOrd);
    f->termOrd = termOrd;
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

bool OrdsSegmentTermsEnum::clearEOF()
{
  eof = false;
  return true;
}

bool OrdsSegmentTermsEnum::setEOF()
{
  eof = true;
  return true;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
// brToString(org.apache.lucene.util.BytesRef b)
wstring OrdsSegmentTermsEnum::brToString(shared_ptr<BytesRef> b)
{
  try {
    return b->utf8ToString() + L" " + b;
  } catch (const runtime_error &t) {
    // If BytesRef isn't actually UTF8, or it's eg a
    // prefix of UTF8 that ends mid-unicode-char, we
    // fallback to hex:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return b->toString();
  }
}

bool OrdsSegmentTermsEnum::seekExact(shared_ptr<BytesRef> target) throw(
    IOException)
{

  if (fr->index == nullptr) {
    throw make_shared<IllegalStateException>(L"terms index was not loaded");
  }

  term_->grow(1 + target->length);

  assert(clearEOF());

  /*
  if (DEBUG) {
    System.out.println("\nBTTR.seekExact seg=" + fr.parent.segment + " target="
  + fr.fieldInfo.name + ":" + brToString(target) + " current=" +
  brToString(term) + " (exists?=" + termExists + ") validIndexPrefix=" +
  validIndexPrefix); printSeekState(System.out);
  }
  */

  shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc;
  int targetUpto;
  shared_ptr<Output> output;

  targetBeforeCurrentLength = currentFrame->ord;

  if (positioned && currentFrame != staticFrame) {

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

    shared_ptr<OrdsSegmentTermsEnumFrame> lastFrame = stack[0];
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
      if (arc->output != OrdsBlockTreeTermsWriter::NO_OUTPUT) {
        output =
            OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);
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
    currentFrame = pushFrame(arc,
                             OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(
                                 output, arc->nextFinalOutput),
                             0);
  }

  positioned = true;

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

    shared_ptr<FST::Arc<std::shared_ptr<Output>>> *const nextArc =
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
      if (arc->output != OrdsBlockTreeTermsWriter::NO_OUTPUT) {
        output =
            OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);
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
                                 OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(
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
OrdsSegmentTermsEnum::seekCeil(shared_ptr<BytesRef> target) 
{
  if (fr->index == nullptr) {
    throw make_shared<IllegalStateException>(L"terms index was not loaded");
  }

  term_->grow(1 + target->length);

  assert(clearEOF());

  // if (DEBUG) {
  // System.out.println("\nBTTR.seekCeil seg=" + segment + " target=" +
  // fieldInfo.name + ":" + target.utf8ToString() + " " + target + " current=" +
  // brToString(term) + " (exists?=" + termExists + ") validIndexPrefix=  " +
  // validIndexPrefix); printSeekState();
  //}

  shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc;
  int targetUpto;
  shared_ptr<Output> output;

  targetBeforeCurrentLength = currentFrame->ord;

  if (positioned && currentFrame != staticFrame) {

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

    shared_ptr<OrdsSegmentTermsEnumFrame> lastFrame = stack[0];
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
      // (term.bytes[targetUpto]) + ")"   + " arc.output=" + arc.output + "
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
      if (arc->output != OrdsBlockTreeTermsWriter::NO_OUTPUT) {
        output =
            OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);
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
        // (term.bytes[targetUpto]) + ")");
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
    currentFrame = pushFrame(arc,
                             OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(
                                 output, arc->nextFinalOutput),
                             0);
  }

  positioned = true;

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

    shared_ptr<FST::Arc<std::shared_ptr<Output>>> *const nextArc =
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

      currentFrame->loadBlock();

      constexpr SeekStatus result = currentFrame->scanToTerm(target, false);
      if (result == SeekStatus::END) {
        term_->copyBytes(target);
        termExists = false;

        if (next() != nullptr) {
          // if (DEBUG) {
          // System.out.println("  return NOT_FOUND term=" + brToString(term) +
          // " " + term);
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
        // System.out.println("  return " + result + " term=" + brToString(term)
        // + " " + term);
        //}
        return result;
      }
    } else {
      // Follow this arc
      term_->setByteAt(targetUpto, static_cast<char>(targetLabel));
      arc = nextArc;
      // Aggregate output as we go:
      assert(arc->output != nullptr);
      if (arc->output != OrdsBlockTreeTermsWriter::NO_OUTPUT) {
        output =
            OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);
      }

      // if (DEBUG) {
      // System.out.println("    index: follow label=" +
      // toHex(target.bytes[target.offset + targetUpto]&0xff) + " arc.output=" +
      // arc.output + " arc.nfo=" + arc.nextFinalOutput);
      //}
      targetUpto++;

      if (arc->isFinal()) {
        // if (DEBUG) System.out.println("    arc is final!");
        currentFrame = pushFrame(arc,
                                 OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(
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
      // System.out.println("  return NOT_FOUND term=" + term.utf8ToString() + "
      // " + term);
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
void OrdsSegmentTermsEnum::printSeekState(shared_ptr<PrintStream> out) throw(
    IOException)
{
  if (currentFrame == staticFrame) {
    out->println(L"  no prior seek");
  } else {
    out->println(L"  prior seek state:");
    int ord = 0;
    bool isSeekFrame = true;
    while (true) {
      shared_ptr<OrdsSegmentTermsEnumFrame> f = getFrame(ord);
      assert(f != nullptr);
      shared_ptr<BytesRef> *const prefix =
          make_shared<BytesRef>(term_->bytes(), 0, f->prefix);
      if (f->nextEnt == -1) {
        out->println(
            L"    frame " + (isSeekFrame ? L"(seek)" : L"(next)") + L" ord=" +
            to_wstring(ord) + L" fp=" + to_wstring(f->fp) +
            (f->isFloor ? (L" (fpOrig=" + to_wstring(f->fpOrig) + L")") : L"") +
            L" prefixLen=" + to_wstring(f->prefix) + L" prefix=" +
            brToString(prefix) +
            StringHelper::toString(
                f->nextEnt == -1
                    ? L""
                    : (L" (of " + to_wstring(f->entCount) + L")")) +
            L" hasTerms=" + StringHelper::toString(f->hasTerms) + L" isFloor=" +
            StringHelper::toString(f->isFloor) + L" code=" +
            to_wstring(
                (f->fp << OrdsBlockTreeTermsWriter::OUTPUT_FLAGS_NUM_BITS) +
                (f->hasTerms ? OrdsBlockTreeTermsWriter::OUTPUT_FLAG_HAS_TERMS
                             : 0) +
                (f->isFloor ? OrdsBlockTreeTermsWriter::OUTPUT_FLAG_IS_FLOOR
                            : 0)) +
            L" isLastInFloor=" + StringHelper::toString(f->isLastInFloor) +
            L" mdUpto=" + to_wstring(f->metaDataUpto) + L" tbOrd=" +
            to_wstring(f->getTermBlockOrd()) + L" termOrd=" +
            to_wstring(f->termOrd));
      } else {
        out->println(
            L"    frame " +
            (isSeekFrame ? L"(seek, loaded)" : L"(next, loaded)") + L" ord=" +
            to_wstring(ord) + L" fp=" + to_wstring(f->fp) +
            (f->isFloor ? (L" (fpOrig=" + to_wstring(f->fpOrig) + L")") : L"") +
            L" prefixLen=" + to_wstring(f->prefix) + L" prefix=" +
            brToString(prefix) + L" nextEnt=" + to_wstring(f->nextEnt) +
            StringHelper::toString(
                f->nextEnt == -1
                    ? L""
                    : (L" (of " + to_wstring(f->entCount) + L")")) +
            L" hasTerms=" + StringHelper::toString(f->hasTerms) + L" isFloor=" +
            StringHelper::toString(f->isFloor) + L" code=" +
            to_wstring(
                (f->fp << OrdsBlockTreeTermsWriter::OUTPUT_FLAGS_NUM_BITS) +
                (f->hasTerms ? OrdsBlockTreeTermsWriter::OUTPUT_FLAG_HAS_TERMS
                             : 0) +
                (f->isFloor ? OrdsBlockTreeTermsWriter::OUTPUT_FLAG_IS_FLOOR
                            : 0)) +
            L" lastSubFP=" + to_wstring(f->lastSubFP) + L" isLastInFloor=" +
            StringHelper::toString(f->isLastInFloor) + L" mdUpto=" +
            to_wstring(f->metaDataUpto) + L" tbOrd=" +
            to_wstring(f->getTermBlockOrd()) + L" termOrd=" +
            to_wstring(f->termOrd));
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
        shared_ptr<Output> output = Util::get(fr->index, prefix);
        if (output == nullptr) {
          out->println(
              L"      broken seek state: prefix is not final in index");
          throw runtime_error(L"seek state is broken");
        } else if (isSeekFrame && !f->isFloor) {
          shared_ptr<ByteArrayDataInput> *const reader =
              make_shared<ByteArrayDataInput>(output->bytes->bytes,
                                              output->bytes->offset,
                                              output->bytes->length);
          constexpr int64_t codeOrig = reader->readVLong();
          constexpr int64_t code =
              (f->fp << OrdsBlockTreeTermsWriter::OUTPUT_FLAGS_NUM_BITS) |
              (f->hasTerms ? OrdsBlockTreeTermsWriter::OUTPUT_FLAG_HAS_TERMS
                           : 0) |
              (f->isFloor ? OrdsBlockTreeTermsWriter::OUTPUT_FLAG_IS_FLOOR : 0);
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

shared_ptr<BytesRef> OrdsSegmentTermsEnum::next() 
{

  if (in_ == nullptr) {
    // Fresh TermsEnum; seek to first term:
    shared_ptr<FST::Arc<std::shared_ptr<Output>>> *const arc;
    if (fr->index != nullptr) {
      arc = fr->index->getFirstArc(arcs[0]);
      // Empty string prefix must have an output in the index!
      assert(arc->isFinal());
    } else {
      arc.reset();
    }
    currentFrame = pushFrame(arc, fr->rootCode, 0);
    currentFrame->loadBlock();
    positioned = true;
  }

  targetBeforeCurrentLength = currentFrame->ord;

  assert(!eof);
  // if (DEBUG) {
  // System.out.println("\nBTTR.next seg=" + segment + " term=" +
  // brToString(term) + " termExists?=" + termExists + " field=" + fieldInfo.name
  // + " termBlockOrd=" + currentFrame.state.termBlockOrd + " validIndexPrefix="
  // + validIndexPrefix); printSeekState();
  //}

  if (currentFrame == staticFrame || positioned == false) {
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
      currentFrame->loadNextFloorBlock();
    } else {
      // if (DEBUG) System.out.println("  pop frame");
      if (currentFrame->ord == 0) {
        // if (DEBUG) System.out.println("  return null");
        assert(setEOF());
        term_->setLength(0);
        validIndexPrefix = 0;
        currentFrame->rewind();
        termExists = false;
        positioned = false;
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
    int64_t prevTermOrd = currentFrame->termOrd;
    if (currentFrame->next()) {
      // Push to new block:
      // if (DEBUG) System.out.println("  push frame");
      currentFrame = pushFrame(nullptr, currentFrame->lastSubFP,
                               term_->length(), prevTermOrd);
      // This is a "next" frame -- even if it's
      // floor'd we must pretend it isn't so we don't
      // try to scan to the right floor frame:
      currentFrame->isFloor = false;
      // currentFrame.hasTerms = true;
      currentFrame->loadBlock();
    } else {
      // if (DEBUG) System.out.println("  return term=" + term.utf8ToString() +
      // " " + term + " currentFrame.ord=" + currentFrame.ord);
      positioned = true;
      return term_->get();
    }
  }
}

shared_ptr<BytesRef> OrdsSegmentTermsEnum::term()
{
  assert(!eof);
  return term_->get();
}

int64_t OrdsSegmentTermsEnum::ord()
{
  assert(!eof);
  assert(currentFrame->termOrd > 0);
  return currentFrame->termOrd - 1;
}

int OrdsSegmentTermsEnum::docFreq() 
{
  assert(!eof);
  // if (DEBUG) System.out.println("BTR.docFreq");
  currentFrame->decodeMetaData();
  // if (DEBUG) System.out.println("  return " + currentFrame.state.docFreq);
  return currentFrame->state->docFreq;
}

int64_t OrdsSegmentTermsEnum::totalTermFreq() 
{
  assert(!eof);
  currentFrame->decodeMetaData();
  return currentFrame->state->totalTermFreq;
}

shared_ptr<PostingsEnum>
OrdsSegmentTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
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

void OrdsSegmentTermsEnum::seekExact(shared_ptr<BytesRef> target,
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
    shared_ptr<BlockTermState> blockState =
        std::static_pointer_cast<BlockTermState>(otherState);
    currentFrame = staticFrame;
    currentFrame->state->copyFrom(otherState);
    term_->copyBytes(target);
    currentFrame->metaDataUpto = currentFrame->getTermBlockOrd();
    currentFrame->termOrd = blockState->ord + 1;
    assert(currentFrame->metaDataUpto > 0);
    validIndexPrefix = 0;
  } else {
    // if (DEBUG) {
    //   System.out.println("  skip seek: already on target state=" +
    //   currentFrame.state);
    // }
  }
  positioned = true;
}

shared_ptr<TermState> OrdsSegmentTermsEnum::termState() 
{
  assert(!eof);
  currentFrame->decodeMetaData();
  shared_ptr<BlockTermState> ts =
      std::static_pointer_cast<BlockTermState>(currentFrame->state->clone());
  assert(currentFrame->termOrd > 0);
  ts->ord = currentFrame->termOrd - 1;
  // if (DEBUG) System.out.println("BTTR.termState seg=" + segment + " state=" +
  // ts);
  return ts;
}

void OrdsSegmentTermsEnum::seekExact(int64_t targetOrd) 
{
  // System.out.println("seekExact targetOrd=" + targetOrd);
  if (targetOrd < 0 || targetOrd >= fr->numTerms) {
    throw invalid_argument(L"targetOrd out of bounds (got: " +
                           to_wstring(targetOrd) + L", numTerms=" +
                           to_wstring(fr->numTerms) + L")");
  }

  assert(clearEOF());

  // First do reverse lookup in the index to find the block that holds this
  // term:
  shared_ptr<InputOutput> io = getByOutput(targetOrd);
  term_->grow(io->input->length);

  Util::toBytesRef(io->input, term_);
  if (io->input->length == 0) {
    currentFrame = staticFrame;
  } else {
    currentFrame = getFrame(io->input->length - 1);
  }
  shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc = getArc(io->input->length);

  // Don't force rewind based on term length; we rewind below based on ord:
  targetBeforeCurrentLength = numeric_limits<int>::max();
  currentFrame = pushFrame(arc, io->output, io->input->length);
  if (currentFrame->termOrd > targetOrd) {
    // System.out.println("  do rewind: " + currentFrame.termOrd);
    currentFrame->rewind();
  }

  currentFrame->scanToFloorFrame(targetOrd);
  currentFrame->loadBlock();
  // System.out.println("  after loadBlock termOrd=" + currentFrame.termOrd + "
  // vs " + targetOrd);

  while (currentFrame->termOrd <= targetOrd) {
    currentFrame->next();
  }

  assert((currentFrame->termOrd == targetOrd + 1,
          L"currentFrame.termOrd=" + to_wstring(currentFrame->termOrd) +
              L" vs ord=" + to_wstring(targetOrd)));
  assert(termExists);

  // Leave enum fully unpositioned, because we didn't set frames for each byte
  // leading up to current term:
  validIndexPrefix = 0;
  positioned = false;
}

wstring OrdsSegmentTermsEnum::toString()
{
  return L"OrdsSegmentTermsEnum(seg=" + fr->parent + L")";
}

wstring OrdsSegmentTermsEnum::InputOutput::toString()
{
  return L"InputOutput(input=" + input + L" output=" + output + L")";
}

shared_ptr<InputOutput>
OrdsSegmentTermsEnum::getByOutput(int64_t targetOrd) 
{

  shared_ptr<IntsRefBuilder> *const result = make_shared<IntsRefBuilder>();

  fr->index->getFirstArc(arc);
  shared_ptr<Output> output = arc->output;
  int upto = 0;

  int bestUpto = 0;
  shared_ptr<Output> bestOutput = nullptr;

  /*
  Writer w = new OutputStreamWriter(new FileOutputStream("/tmp/out.dot"));
  Util.toDot(fr.index, w, true, true);
  w.close();
  */

  // System.out.println("reverseLookup seg=" + fr.parent.segment + " output=" +
  // targetOrd);

  while (true) {
    // System.out.println("  loop: output=" + output.startOrd + "-" +
    // (Long.MAX_VALUE-output.endOrd) + " upto=" + upto + " arc=" + arc + "
    // final?=" + arc.isFinal());
    if (arc->isFinal()) {
      shared_ptr<Output> *const finalOutput =
          OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output,
                                                     arc->nextFinalOutput);
      // System.out.println("  isFinal: " + finalOutput.startOrd + "-" +
      // (Long.MAX_VALUE-finalOutput.endOrd));
      if (targetOrd >= finalOutput->startOrd &&
          targetOrd <= numeric_limits<int64_t>::max() - finalOutput->endOrd) {
        // Only one range should match across all arc leaving this node
        // assert bestOutput == null;
        bestOutput = finalOutput;
        bestUpto = upto;
      }
    }

    if (FST::targetHasArcs(arc)) {
      // System.out.println("  targetHasArcs");
      result->grow(1 + upto);

      fr->index->readFirstRealTargetArc(arc->target, arc, fstReader);

      if (arc->bytesPerArc != 0) {
        // System.out.println("  array arcs");

        int low = 0;
        int high = arc->numArcs - 1;
        int mid = 0;
        // System.out.println("bsearch: numArcs=" + arc.numArcs + " target=" +
        // targetOutput + " output=" + output);
        bool found = false;
        while (low <= high) {
          mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
          fstReader->setPosition(arc->posArcsStart);
          fstReader->skipBytes(arc->bytesPerArc * mid);
          constexpr char flags = fstReader->readByte();
          fr->index->readLabel(fstReader);
          shared_ptr<Output> *const minArcOutput;
          if ((flags & FST::BIT_ARC_HAS_OUTPUT) != 0) {
            minArcOutput = OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(
                output, OrdsBlockTreeTermsWriter::FST_OUTPUTS->read(fstReader));
          } else {
            minArcOutput = output;
          }
          // System.out.println("  cycle mid=" + mid + " targetOrd=" + targetOrd
          // + " output=" + minArcOutput.startOrd + "-" +
          // (Long.MAX_VALUE-minArcOutput.endOrd));
          if (targetOrd >
              numeric_limits<int64_t>::max() - minArcOutput->endOrd) {
            low = mid + 1;
          } else if (targetOrd < minArcOutput->startOrd) {
            high = mid - 1;
          } else {
            // System.out.println("    found!!");
            found = true;
            break;
          }
        }

        if (found) {
          // Keep recursing
          arc->arcIdx = mid - 1;
        } else {
          result->setLength(bestUpto);
          shared_ptr<InputOutput> io = make_shared<InputOutput>();
          io->input = result->get();
          io->output = bestOutput;
          // System.out.println("  ret0=" + io);
          return io;
        }

        fr->index->readNextRealArc(arc, fstReader);

        // Recurse on this arc:
        result->setIntAt(upto++, arc->label);
        output =
            OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);

      } else {
        // System.out.println("    non-array arc");

        while (true) {
          // System.out.println("    cycle label=" + arc.label + " output=" +
          // arc.output);

          // This is the min output we'd hit if we follow
          // this arc:
          shared_ptr<Output> *const minArcOutput =
              OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);
          int64_t endOrd =
              numeric_limits<int64_t>::max() - minArcOutput->endOrd;
          // System.out.println("    endOrd=" + endOrd + " targetOrd=" +
          // targetOrd);

          if (targetOrd >= minArcOutput->startOrd && targetOrd <= endOrd) {
            // Recurse on this arc:
            output = minArcOutput;
            result->setIntAt(upto++, arc->label);
            break;
          } else if (targetOrd < endOrd || arc->isLast()) {
            result->setLength(bestUpto);
            shared_ptr<InputOutput> io = make_shared<InputOutput>();
            io->input = result->get();
            assert(bestOutput != nullptr);
            io->output = bestOutput;
            // System.out.println("  ret2=" + io);
            return io;
          } else {
            // System.out.println("  next arc");
            // Read next arc in this node:
            fr->index->readNextRealArc(arc, fstReader);
          }
        }
      }
    } else {
      result->setLength(bestUpto);
      shared_ptr<InputOutput> io = make_shared<InputOutput>();
      io->input = result->get();
      io->output = bestOutput;
      // System.out.println("  ret3=" + io);
      return io;
    }
  }
}
} // namespace org::apache::lucene::codecs::blocktreeords