using namespace std;

#include "OrdsIntersectTermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/RunAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "FSTOrdsOutputs.h"
#include "OrdsBlockTreeTermsWriter.h"
#include "OrdsFieldReader.h"
#include "OrdsIntersectTermsEnumFrame.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;
using FST = org::apache::lucene::util::fst::FST;

OrdsIntersectTermsEnum::OrdsIntersectTermsEnum(
    shared_ptr<OrdsFieldReader> fr, shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
    : in_(fr->parent->in_->clone()), runAutomaton(compiled->runAutomaton),
      compiledAutomaton(compiled), fr(fr)
{
  // if (DEBUG) {
  //   System.out.println("\nintEnum.init seg=" + segment + " commonSuffix=" +
  //   brToString(compiled.commonSuffixRef));
  // }
  stack = std::deque<std::shared_ptr<OrdsIntersectTermsEnumFrame>>(5);
  for (int idx = 0; idx < stack.size(); idx++) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    stack[idx] =
        make_shared<OrdsIntersectTermsEnumFrame>(shared_from_this(), idx);
  }
  for (int arcIdx = 0; arcIdx < arcs.size(); arcIdx++) {
    arcs[arcIdx] = make_shared<FST::Arc<Output>>();
  }

  if (fr->index == nullptr) {
    fstReader.reset();
  } else {
    fstReader = fr->index->getBytesReader();
  }

  // TODO: if the automaton is "smallish" we really
  // should use the terms index to seek at least to
  // the initial term and likely to subsequent terms
  // (or, maybe just fallback to ATE for such cases).
  // Else the seek cost of loading the frames will be
  // too costly.

  shared_ptr<FST::Arc<std::shared_ptr<Output>>> *const arc =
      fr->index->getFirstArc(arcs[0]);
  // Empty string prefix must have an output in the index!
  assert(arc->isFinal());

  // Special pushFrame since it's the first one:
  shared_ptr<OrdsIntersectTermsEnumFrame> *const f = stack[0];
  f->fp = f->fpOrig = fr->rootBlockFP;
  f->prefix = 0;
  f->setState(0);
  f->arc = arc;
  f->outputPrefix = arc->output;
  f->load(fr->rootCode);

  // for assert:
  assert(setSavedStartTerm(startTerm));

  currentFrame = f;
  if (startTerm != nullptr) {
    seekToStartTerm(startTerm);
  }
}

bool OrdsIntersectTermsEnum::setSavedStartTerm(shared_ptr<BytesRef> startTerm)
{
  savedStartTerm =
      startTerm == nullptr ? nullptr : BytesRef::deepCopyOf(startTerm);
  return true;
}

shared_ptr<TermState> OrdsIntersectTermsEnum::termState() 
{
  currentFrame->decodeMetaData();
  return currentFrame->termState->clone();
}

shared_ptr<OrdsIntersectTermsEnumFrame>
OrdsIntersectTermsEnum::getFrame(int ord) 
{
  if (ord >= stack.size()) {
    std::deque<std::shared_ptr<OrdsIntersectTermsEnumFrame>> next(
        ArrayUtil::oversize(1 + ord, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(stack, 0, next, 0, stack.size());
    for (int stackOrd = stack.size(); stackOrd < next.size(); stackOrd++) {
      next[stackOrd] = make_shared<OrdsIntersectTermsEnumFrame>(
          shared_from_this(), stackOrd);
    }
    stack = next;
  }
  assert(stack[ord]->ord == ord);
  return stack[ord];
}

shared_ptr<FST::Arc<std::shared_ptr<Output>>>
OrdsIntersectTermsEnum::getArc(int ord)
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

shared_ptr<OrdsIntersectTermsEnumFrame>
OrdsIntersectTermsEnum::pushFrame(int state) 
{
  shared_ptr<OrdsIntersectTermsEnumFrame> *const f =
      getFrame(currentFrame == nullptr ? 0 : 1 + currentFrame->ord);

  f->fp = f->fpOrig = currentFrame->lastSubFP;
  f->prefix = currentFrame->prefix + currentFrame->suffix;
  // if (DEBUG) System.out.println("    pushFrame state=" + state + " prefix=" +
  // f.prefix);
  f->setState(state);

  // Walk the arc through the index -- we only
  // "bother" with this so we can get the floor data
  // from the index and skip floor blocks when
  // possible:
  shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc = currentFrame->arc;
  int idx = currentFrame->prefix;
  assert(currentFrame->suffix > 0);
  shared_ptr<Output> output = currentFrame->outputPrefix;
  while (idx < f->prefix) {
    constexpr int target = term_->bytes[idx] & 0xff;
    // TODO: we could be more efficient for the next()
    // case by using current arc as starting point,
    // passed to findTargetArc
    arc = fr->index->findTargetArc(target, arc, getArc(1 + idx), fstReader);
    assert(arc != nullptr);
    output = OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->output);
    idx++;
  }

  f->arc = arc;
  f->outputPrefix = output;
  assert(arc->isFinal());
  f->load(
      OrdsBlockTreeTermsWriter::FST_OUTPUTS->add(output, arc->nextFinalOutput));
  return f;
}

shared_ptr<BytesRef> OrdsIntersectTermsEnum::term() { return term_; }

int OrdsIntersectTermsEnum::docFreq() 
{
  // if (DEBUG) System.out.println("BTIR.docFreq");
  currentFrame->decodeMetaData();
  // if (DEBUG) System.out.println("  return " +
  // currentFrame.termState.docFreq);
  return currentFrame->termState->docFreq;
}

int64_t OrdsIntersectTermsEnum::totalTermFreq() 
{
  currentFrame->decodeMetaData();
  return currentFrame->termState->totalTermFreq;
}

shared_ptr<PostingsEnum>
OrdsIntersectTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                 int flags) 
{
  currentFrame->decodeMetaData();
  return fr->parent->postingsReader->postings(
      fr->fieldInfo, currentFrame->termState, reuse, flags);
}

int OrdsIntersectTermsEnum::getState()
{
  int state = currentFrame->state;
  for (int idx = 0; idx < currentFrame->suffix; idx++) {
    state = runAutomaton->step(
        state,
        currentFrame->suffixBytes[currentFrame->startBytePos + idx] & 0xff);
    assert(state != -1);
  }
  return state;
}

void OrdsIntersectTermsEnum::seekToStartTerm(shared_ptr<BytesRef> target) throw(
    IOException)
{
  // if (DEBUG) System.out.println("seek to startTerm=" +
  // target.utf8ToString());
  assert(currentFrame->ord == 0);
  if (term_->length < target->length) {
    term_->bytes = ArrayUtil::grow(term_->bytes, target->length);
  }
  shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc = arcs[0];
  assert(arc == currentFrame->arc);

  for (int idx = 0; idx <= target->length; idx++) {

    while (true) {
      constexpr int savePos = currentFrame->suffixesReader->getPosition();
      constexpr int saveStartBytePos = currentFrame->startBytePos;
      constexpr int saveSuffix = currentFrame->suffix;
      constexpr int64_t saveLastSubFP = currentFrame->lastSubFP;
      constexpr int saveTermBlockOrd = currentFrame->termState->termBlockOrd;

      constexpr bool isSubBlock = currentFrame->next();

      // if (DEBUG) System.out.println("    cycle ent=" + currentFrame.nextEnt +
      // " (of " + currentFrame.entCount + ") prefix=" + currentFrame.prefix + "
      // suffix=" + currentFrame.suffix + " isBlock=" + isSubBlock + "
      // firstLabel=" + (currentFrame.suffix == 0 ? "" :
      // (currentFrame.suffixBytes[currentFrame.startBytePos])&0xff));
      term_->length = currentFrame->prefix + currentFrame->suffix;
      if (term_->bytes.size() < term_->length) {
        term_->bytes = ArrayUtil::grow(term_->bytes, term_->length);
      }
      System::arraycopy(currentFrame->suffixBytes, currentFrame->startBytePos,
                        term_->bytes, currentFrame->prefix,
                        currentFrame->suffix);

      if (isSubBlock && StringHelper::startsWith(target, term_)) {
        // Recurse
        // if (DEBUG) System.out.println("      recurse!");
        currentFrame = pushFrame(getState());
        break;
      } else {
        constexpr int cmp = term_->compareTo(target);
        if (cmp < 0) {
          if (currentFrame->nextEnt == currentFrame->entCount) {
            if (!currentFrame->isLastInFloor) {
              // if (DEBUG) System.out.println("  load floorBlock");
              currentFrame->loadNextFloorBlock();
              continue;
            } else {
              // if (DEBUG) System.out.println("  return term=" +
              // brToString(term));
              return;
            }
          }
          continue;
        } else if (cmp == 0) {
          // if (DEBUG) System.out.println("  return term=" + brToString(term));
          return;
        } else {
          // Fallback to prior entry: the semantics of
          // this method is that the first call to
          // next() will return the term after the
          // requested term
          currentFrame->nextEnt--;
          currentFrame->lastSubFP = saveLastSubFP;
          currentFrame->startBytePos = saveStartBytePos;
          currentFrame->suffix = saveSuffix;
          currentFrame->suffixesReader->setPosition(savePos);
          currentFrame->termState->termBlockOrd = saveTermBlockOrd;
          System::arraycopy(currentFrame->suffixBytes,
                            currentFrame->startBytePos, term_->bytes,
                            currentFrame->prefix, currentFrame->suffix);
          term_->length = currentFrame->prefix + currentFrame->suffix;
          // If the last entry was a block we don't
          // need to bother recursing and pushing to
          // the last term under it because the first
          // next() will simply skip the frame anyway
          return;
        }
      }
    }
  }

  assert(false);
}

shared_ptr<BytesRef> OrdsIntersectTermsEnum::next() 
{

  // if (DEBUG) {
  //   System.out.println("\nintEnum.next seg=" + segment);
  //   System.out.println("  frame ord=" + currentFrame.ord + " prefix=" +
  //   brToString(new BytesRef(term.bytes, term.offset, currentFrame.prefix)) +
  //   " state=" + currentFrame.state + " lastInFloor?=" +
  //   currentFrame.isLastInFloor + " fp=" + currentFrame.fp + " trans=" +
  //   (currentFrame.transitions.length == 0 ? "n/a" :
  //   currentFrame.transitions[currentFrame.transitionIndex]) + "
  //   outputPrefix=" + currentFrame.outputPrefix);
  // }

  while (true) {
    // Pop finished frames
    while (currentFrame->nextEnt == currentFrame->entCount) {
      if (!currentFrame->isLastInFloor) {
        // if (DEBUG) System.out.println("    next-floor-block");
        currentFrame->loadNextFloorBlock();
        // if (DEBUG) System.out.println("\n  frame ord=" + currentFrame.ord + "
        // prefix=" + brToString(new BytesRef(term.bytes, term.offset,
        // currentFrame.prefix)) + " state=" + currentFrame.state + "
        // lastInFloor?=" + currentFrame.isLastInFloor + " fp=" + currentFrame.fp
        // + " trans=" + (currentFrame.transitions.length == 0 ? "n/a" :
        // currentFrame.transitions[currentFrame.transitionIndex]) + "
        // outputPrefix=" + currentFrame.outputPrefix);
      } else {
        // if (DEBUG) System.out.println("  pop frame");
        if (currentFrame->ord == 0) {
          return nullptr;
        }
        constexpr int64_t lastFP = currentFrame->fpOrig;
        currentFrame = stack[currentFrame->ord - 1];
        assert(currentFrame->lastSubFP == lastFP);
        // if (DEBUG) System.out.println("\n  frame ord=" + currentFrame.ord + "
        // prefix=" + brToString(new BytesRef(term.bytes, term.offset,
        // currentFrame.prefix)) + " state=" + currentFrame.state + "
        // lastInFloor?=" + currentFrame.isLastInFloor + " fp=" + currentFrame.fp
        // + " trans=" + (currentFrame.transitions.length == 0 ? "n/a" :
        // currentFrame.transitions[currentFrame.transitionIndex]) + "
        // outputPrefix=" + currentFrame.outputPrefix);
      }
    }

    constexpr bool isSubBlock = currentFrame->next();
    // if (DEBUG) {
    //   final BytesRef suffixRef = new BytesRef();
    //   suffixRef.bytes = currentFrame.suffixBytes;
    //   suffixRef.offset = currentFrame.startBytePos;
    //   suffixRef.length = currentFrame.suffix;
    //   System.out.println("    " + (isSubBlock ? "sub-block" : "term") + " " +
    //   currentFrame.nextEnt + " (of " + currentFrame.entCount + ") suffix=" +
    //   brToString(suffixRef));
    // }

    if (currentFrame->suffix != 0) {
      constexpr int label =
          currentFrame->suffixBytes[currentFrame->startBytePos] & 0xff;
      while (label > currentFrame->curTransitionMax) {
        if (currentFrame->transitionIndex >=
            currentFrame->transitionCount - 1) {
          // Stop processing this frame -- no further
          // matches are possible because we've moved
          // beyond what the max transition will allow
          // if (DEBUG) System.out.println("      break: trans=" +
          // (currentFrame.transitions.length == 0 ? "n/a" :
          // currentFrame.transitions[currentFrame.transitionIndex]));

          // sneaky!  forces a pop above
          currentFrame->isLastInFloor = true;
          currentFrame->nextEnt = currentFrame->entCount;
          goto nextTermContinue;
        }
        currentFrame->transitionIndex++;
        compiledAutomaton->automaton->getNextTransition(
            currentFrame->transition);
        currentFrame->curTransitionMax = currentFrame->transition->max;
        // if (DEBUG) System.out.println("      next trans=" +
        // currentFrame.transitions[currentFrame.transitionIndex]);
      }
    }

    // First test the common suffix, if set:
    if (compiledAutomaton->commonSuffixRef != nullptr && !isSubBlock) {
      constexpr int termLen = currentFrame->prefix + currentFrame->suffix;
      if (termLen < compiledAutomaton->commonSuffixRef->length) {
        // No match
        // if (DEBUG) {
        //   System.out.println("      skip: common suffix length");
        // }
        goto nextTermContinue;
      }

      const std::deque<char> suffixBytes = currentFrame->suffixBytes;
      const std::deque<char> commonSuffixBytes =
          compiledAutomaton->commonSuffixRef->bytes;

      constexpr int lenInPrefix =
          compiledAutomaton->commonSuffixRef->length - currentFrame->suffix;
      assert(compiledAutomaton->commonSuffixRef->offset == 0);
      int suffixBytesPos;
      int commonSuffixBytesPos = 0;

      if (lenInPrefix > 0) {
        // A prefix of the common suffix overlaps with
        // the suffix of the block prefix so we first
        // test whether the prefix part matches:
        const std::deque<char> termBytes = term_->bytes;
        int termBytesPos = currentFrame->prefix - lenInPrefix;
        assert(termBytesPos >= 0);
        constexpr int termBytesPosEnd = currentFrame->prefix;
        while (termBytesPos < termBytesPosEnd) {
          if (termBytes[termBytesPos++] !=
              commonSuffixBytes[commonSuffixBytesPos++]) {
            // if (DEBUG) {
            //   System.out.println("      skip: common suffix mismatch (in
            //   prefix)");
            // }
            goto nextTermContinue;
          }
        }
        suffixBytesPos = currentFrame->startBytePos;
      } else {
        suffixBytesPos = currentFrame->startBytePos + currentFrame->suffix -
                         compiledAutomaton->commonSuffixRef->length;
      }

      // Test overlapping suffix part:
      constexpr int commonSuffixBytesPosEnd =
          compiledAutomaton->commonSuffixRef->length;
      while (commonSuffixBytesPos < commonSuffixBytesPosEnd) {
        if (suffixBytes[suffixBytesPos++] !=
            commonSuffixBytes[commonSuffixBytesPos++]) {
          // if (DEBUG) {
          //   System.out.println("      skip: common suffix mismatch");
          // }
          goto nextTermContinue;
        }
      }
    }

    // TODO: maybe we should do the same linear test
    // that AutomatonTermsEnum does, so that if we
    // reach a part of the automaton where .* is
    // "temporarily" accepted, we just blindly .next()
    // until the limit

    // See if the term prefix matches the automaton:
    int state = currentFrame->state;
    for (int idx = 0; idx < currentFrame->suffix; idx++) {
      state = runAutomaton->step(
          state,
          currentFrame->suffixBytes[currentFrame->startBytePos + idx] & 0xff);
      if (state == -1) {
        // No match
        // System.out.println("    no s=" + state);
        goto nextTermContinue;
      } else {
        // System.out.println("    c s=" + state);
      }
    }

    if (isSubBlock) {
      // Match!  Recurse:
      // if (DEBUG) System.out.println("      sub-block match to state=" + state
      // + "; recurse fp=" + currentFrame.lastSubFP);
      copyTerm();
      currentFrame = pushFrame(state);
      // if (DEBUG) System.out.println("\n  frame ord=" + currentFrame.ord + "
      // prefix=" + brToString(new BytesRef(term.bytes, term.offset,
      // currentFrame.prefix)) + " state=" + currentFrame.state + "
      // lastInFloor?=" + currentFrame.isLastInFloor + " fp=" + currentFrame.fp +
      // " trans=" + (currentFrame.transitions.length == 0 ? "n/a" :
      // currentFrame.transitions[currentFrame.transitionIndex]) + "
      // outputPrefix=" + currentFrame.outputPrefix);
    } else if (runAutomaton->isAccept(state)) {
      copyTerm();
      // if (DEBUG) System.out.println("      term match to state=" + state + ";
      // return term=" + brToString(term));
      assert((savedStartTerm == nullptr || term_->compareTo(savedStartTerm) > 0,
              L"saveStartTerm=" + savedStartTerm->utf8ToString() + L" term=" +
                  term_->utf8ToString()));
      return term_;
    } else {
      // System.out.println("    no s=" + state);
    }
  nextTermContinue:;
  }
nextTermBreak:;
}

void OrdsIntersectTermsEnum::copyTerm()
{
  // System.out.println("      copyTerm cur.prefix=" + currentFrame.prefix + "
  // cur.suffix=" + currentFrame.suffix + " first=" + (char)
  // currentFrame.suffixBytes[currentFrame.startBytePos]);
  constexpr int len = currentFrame->prefix + currentFrame->suffix;
  if (term_->bytes.size() < len) {
    term_->bytes = ArrayUtil::grow(term_->bytes, len);
  }
  System::arraycopy(currentFrame->suffixBytes, currentFrame->startBytePos,
                    term_->bytes, currentFrame->prefix, currentFrame->suffix);
  term_->length = len;
}

bool OrdsIntersectTermsEnum::seekExact(shared_ptr<BytesRef> text)
{
  throw make_shared<UnsupportedOperationException>();
}

void OrdsIntersectTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t OrdsIntersectTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

SeekStatus OrdsIntersectTermsEnum::seekCeil(shared_ptr<BytesRef> text)
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::blocktreeords