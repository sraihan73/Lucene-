using namespace std;

#include "IntersectTermsEnum.h"
#include "../../index/PostingsEnum.h"
#include "../../index/TermState.h"
#include "../../store/IndexInput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/RamUsageEstimator.h"
#include "../../util/StringHelper.h"
#include "../../util/automaton/Automaton.h"
#include "../../util/automaton/RunAutomaton.h"
#include "../../util/automaton/Transition.h"
#include "../../util/fst/FST.h"
#include "FieldReader.h"
#include "IntersectTermsEnumFrame.h"

namespace org::apache::lucene::codecs::blocktree
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;
using Transition = org::apache::lucene::util::automaton::Transition;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;
const shared_ptr<org::apache::lucene::util::fst::Outputs<
    std::shared_ptr<org::apache::lucene::util::BytesRef>>>
    IntersectTermsEnum::fstOutputs =
        org::apache::lucene::util::fst::ByteSequenceOutputs::getSingleton();

IntersectTermsEnum::IntersectTermsEnum(shared_ptr<FieldReader> fr,
                                       shared_ptr<Automaton> automaton,
                                       shared_ptr<RunAutomaton> runAutomaton,
                                       shared_ptr<BytesRef> commonSuffix,
                                       shared_ptr<BytesRef> startTerm,
                                       int sinkState) 
    : in_(fr->parent->termsIn->clone()), runAutomaton(runAutomaton),
      automaton(automaton), commonSuffix(commonSuffix),
      fstReader(fr->index->getBytesReader()),
      allowAutoPrefixTerms(sinkState != -1), fr(fr), sinkState(sinkState)
{

  assert(automaton != nullptr);
  assert(runAutomaton != nullptr);

  stack = std::deque<std::shared_ptr<IntersectTermsEnumFrame>>(5);
  for (int idx = 0; idx < stack.size(); idx++) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    stack[idx] = make_shared<IntersectTermsEnumFrame>(shared_from_this(), idx);
  }
  for (int arcIdx = 0; arcIdx < arcs.size(); arcIdx++) {
    arcs[arcIdx] = make_shared<FST::Arc<BytesRef>>();
  }

  // TODO: if the automaton is "smallish" we really
  // should use the terms index to seek at least to
  // the initial term and likely to subsequent terms
  // (or, maybe just fallback to ATE for such cases).
  // Else the seek cost of loading the frames will be
  // too costly.

  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> *const arc =
      fr->index->getFirstArc(arcs[0]);
  // Empty string prefix must have an output in the index!
  assert(arc->isFinal());

  // Special pushFrame since it's the first one:
  shared_ptr<IntersectTermsEnumFrame> *const f = stack[0];
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
  currentTransition = currentFrame->transition;
}

bool IntersectTermsEnum::setSavedStartTerm(shared_ptr<BytesRef> startTerm)
{
  savedStartTerm =
      startTerm == nullptr ? nullptr : BytesRef::deepCopyOf(startTerm);
  return true;
}

shared_ptr<TermState> IntersectTermsEnum::termState() 
{
  currentFrame->decodeMetaData();
  return currentFrame->termState->clone();
}

shared_ptr<IntersectTermsEnumFrame>
IntersectTermsEnum::getFrame(int ord) 
{
  if (ord >= stack.size()) {
    std::deque<std::shared_ptr<IntersectTermsEnumFrame>> next(
        ArrayUtil::oversize(1 + ord, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(stack, 0, next, 0, stack.size());
    for (int stackOrd = stack.size(); stackOrd < next.size(); stackOrd++) {
      next[stackOrd] =
          make_shared<IntersectTermsEnumFrame>(shared_from_this(), stackOrd);
    }
    stack = next;
  }
  assert(stack[ord]->ord == ord);
  return stack[ord];
}

shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>>
IntersectTermsEnum::getArc(int ord)
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

shared_ptr<IntersectTermsEnumFrame>
IntersectTermsEnum::pushFrame(int state) 
{
  assert(currentFrame != nullptr);

  shared_ptr<IntersectTermsEnumFrame> *const f =
      getFrame(currentFrame == nullptr ? 0 : 1 + currentFrame->ord);

  f->fp = f->fpOrig = currentFrame->lastSubFP;
  f->prefix = currentFrame->prefix + currentFrame->suffix;
  f->setState(state);

  // Walk the arc through the index -- we only
  // "bother" with this so we can get the floor data
  // from the index and skip floor blocks when
  // possible:
  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc = currentFrame->arc;
  int idx = currentFrame->prefix;
  assert(currentFrame->suffix > 0);
  shared_ptr<BytesRef> output = currentFrame->outputPrefix;
  while (idx < f->prefix) {
    constexpr int target = term_->bytes[idx] & 0xff;
    // TODO: we could be more efficient for the next()
    // case by using current arc as starting point,
    // passed to findTargetArc
    arc = fr->index->findTargetArc(target, arc, getArc(1 + idx), fstReader);
    assert(arc != nullptr);
    output = fstOutputs->add(output, arc->output);
    idx++;
  }

  f->arc = arc;
  f->outputPrefix = output;
  assert(arc->isFinal());
  f->load(fstOutputs->add(output, arc->nextFinalOutput));
  return f;
}

shared_ptr<BytesRef> IntersectTermsEnum::term() { return term_; }

int IntersectTermsEnum::docFreq() 
{
  currentFrame->decodeMetaData();
  return currentFrame->termState->docFreq;
}

int64_t IntersectTermsEnum::totalTermFreq() 
{
  currentFrame->decodeMetaData();
  return currentFrame->termState->totalTermFreq;
}

shared_ptr<PostingsEnum>
IntersectTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                             int flags) 
{
  currentFrame->decodeMetaData();
  return fr->parent->postingsReader->postings(
      fr->fieldInfo, currentFrame->termState, reuse, flags);
}

int IntersectTermsEnum::getState()
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

void IntersectTermsEnum::seekToStartTerm(shared_ptr<BytesRef> target) throw(
    IOException)
{
  assert(currentFrame->ord == 0);
  if (term_->length < target->length) {
    term_->bytes = ArrayUtil::grow(term_->bytes, target->length);
  }
  shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc = arcs[0];
  assert(arc == currentFrame->arc);

  for (int idx = 0; idx <= target->length; idx++) {

    while (true) {
      constexpr int savNextEnt = currentFrame->nextEnt;
      constexpr int savePos = currentFrame->suffixesReader->getPosition();
      constexpr int saveStartBytePos = currentFrame->startBytePos;
      constexpr int saveSuffix = currentFrame->suffix;
      constexpr int64_t saveLastSubFP = currentFrame->lastSubFP;
      constexpr int saveTermBlockOrd = currentFrame->termState->termBlockOrd;
      constexpr bool saveIsAutoPrefixTerm = currentFrame->isAutoPrefixTerm;

      constexpr bool isSubBlock = currentFrame->next();

      term_->length = currentFrame->prefix + currentFrame->suffix;
      if (term_->bytes.size() < term_->length) {
        term_->bytes = ArrayUtil::grow(term_->bytes, term_->length);
      }
      System::arraycopy(currentFrame->suffixBytes, currentFrame->startBytePos,
                        term_->bytes, currentFrame->prefix,
                        currentFrame->suffix);

      if (isSubBlock && StringHelper::startsWith(target, term_)) {
        // Recurse
        currentFrame = pushFrame(getState());
        break;
      } else {
        constexpr int cmp = term_->compareTo(target);
        if (cmp < 0) {
          if (currentFrame->nextEnt == currentFrame->entCount) {
            if (!currentFrame->isLastInFloor) {
              // Advance to next floor block
              currentFrame->loadNextFloorBlock();
              continue;
            } else {
              return;
            }
          }
          continue;
        } else if (cmp == 0) {
          if (allowAutoPrefixTerms == false && currentFrame->isAutoPrefixTerm) {
            continue;
          }
          return;
        } else if (allowAutoPrefixTerms ||
                   currentFrame->isAutoPrefixTerm == false) {
          // Fallback to prior entry: the semantics of
          // this method is that the first call to
          // next() will return the term after the
          // requested term
          currentFrame->nextEnt = savNextEnt;
          currentFrame->lastSubFP = saveLastSubFP;
          currentFrame->startBytePos = saveStartBytePos;
          currentFrame->suffix = saveSuffix;
          currentFrame->suffixesReader->setPosition(savePos);
          currentFrame->termState->termBlockOrd = saveTermBlockOrd;
          currentFrame->isAutoPrefixTerm = saveIsAutoPrefixTerm;
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

bool IntersectTermsEnum::popPushNext() 
{
  // Pop finished frames
  while (currentFrame->nextEnt == currentFrame->entCount) {
    if (!currentFrame->isLastInFloor) {
      // Advance to next floor block
      currentFrame->loadNextFloorBlock();
      break;
    } else {
      if (currentFrame->ord == 0) {
        throw NoMoreTermsException::INSTANCE;
      }
      constexpr int64_t lastFP = currentFrame->fpOrig;
      currentFrame = stack[currentFrame->ord - 1];
      currentTransition = currentFrame->transition;
      assert(currentFrame->lastSubFP == lastFP);
    }
  }

  return currentFrame->next();
}

bool IntersectTermsEnum::skipPastLastAutoPrefixTerm() 
{
  assert(currentFrame->isAutoPrefixTerm);
  useAutoPrefixTerm = false;

  // If we last returned an auto-prefix term, we must now skip all
  // actual terms sharing that prefix.  At most, that skipping
  // requires popping one frame, but it can also require simply
  // scanning ahead within the current frame.  This scanning will
  // skip sub-blocks that contain many terms, which is why the
  // optimization "works":
  int floorSuffixLeadEnd = currentFrame->floorSuffixLeadEnd;

  bool isSubBlock;

  if (floorSuffixLeadEnd == -1) {
    // An ordinary prefix, e.g. foo*
    int prefix = currentFrame->prefix;
    int suffix = currentFrame->suffix;
    if (suffix == 0) {

      // Easy case: the prefix term's suffix is the empty string,
      // meaning the prefix corresponds to all terms in the
      // current block, so we just pop this entire block:
      if (currentFrame->ord == 0) {
        throw NoMoreTermsException::INSTANCE;
      }
      currentFrame = stack[currentFrame->ord - 1];
      currentTransition = currentFrame->transition;

      return popPushNext();

    } else {

      // Just next() until we hit an entry that doesn't share this
      // prefix.  The first next should be a sub-block sharing the
      // same prefix, because if there are enough terms matching a
      // given prefix to warrant an auto-prefix term, then there
      // must also be enough to make a sub-block (assuming
      // minItemsInPrefix > minItemsInBlock):
      while (true) {
        if (currentFrame->nextEnt == currentFrame->entCount) {
          if (currentFrame->isLastInFloor == false) {
            currentFrame->loadNextFloorBlock();
          } else if (currentFrame->ord == 0) {
            throw NoMoreTermsException::INSTANCE;
          } else {
            // Pop frame, which also means we've moved beyond this
            // auto-prefix term:
            currentFrame = stack[currentFrame->ord - 1];
            currentTransition = currentFrame->transition;

            return popPushNext();
          }
        }
        isSubBlock = currentFrame->next();
        for (int i = 0; i < suffix; i++) {
          if (term_->bytes[prefix + i] !=
              currentFrame->suffixBytes[currentFrame->startBytePos + i]) {
            goto scanPrefixBreak;
          }
        }
      scanPrefixContinue:;
      }
    scanPrefixBreak:;
    }
  } else {
    // Floor'd auto-prefix term; in this case we must skip all
    // terms e.g. matching foo[a-m]*.  We are currently "on" fooa,
    // which the automaton accepted (fooa* through foom*), and
    // floorSuffixLeadEnd is m, so we must now scan to foon:
    int prefix = currentFrame->prefix;
    int suffix = currentFrame->suffix;

    if (currentFrame->floorSuffixLeadStart == -1) {
      suffix++;
    }

    if (suffix == 0) {

      // This means current frame is fooa*, so we have to first
      // pop the current frame, then scan in parent frame:
      if (currentFrame->ord == 0) {
        throw NoMoreTermsException::INSTANCE;
      }
      currentFrame = stack[currentFrame->ord - 1];
      currentTransition = currentFrame->transition;

      // Current (parent) frame is now foo*, so now we just scan
      // until the lead suffix byte is > floorSuffixLeadEnd
      // assert currentFrame.prefix == prefix-1;
      // prefix = currentFrame.prefix;

      // In case when we pop, and the parent block is not just prefix-1, e.g. in
      // block 417* on its first term = floor prefix term 41[7-9], popping to
      // block 4*:
      prefix = currentFrame->prefix;

      suffix = term_->length - currentFrame->prefix;
    } else {
      // No need to pop; just scan in currentFrame:
    }

    // Now we scan until the lead suffix byte is > floorSuffixLeadEnd
    while (true) {
      if (currentFrame->nextEnt == currentFrame->entCount) {
        if (currentFrame->isLastInFloor == false) {
          currentFrame->loadNextFloorBlock();
        } else if (currentFrame->ord == 0) {
          throw NoMoreTermsException::INSTANCE;
        } else {
          // Pop frame, which also means we've moved beyond this
          // auto-prefix term:
          currentFrame = stack[currentFrame->ord - 1];
          currentTransition = currentFrame->transition;

          return popPushNext();
        }
      }
      isSubBlock = currentFrame->next();
      for (int i = 0; i < suffix - 1; i++) {
        if (term_->bytes[prefix + i] !=
            currentFrame->suffixBytes[currentFrame->startBytePos + i]) {
          goto scanFloorBreak;
        }
      }
      if (currentFrame->suffix >= suffix &&
          (currentFrame->suffixBytes[currentFrame->startBytePos + suffix - 1] &
           0xff) > floorSuffixLeadEnd) {
        // Done scanning: we are now on the first term after all
        // terms matched by this auto-prefix term
        break;
      }
    scanFloorContinue:;
    }
  scanFloorBreak:;
  }

  return isSubBlock;
}

const shared_ptr<NoMoreTermsException>
    IntersectTermsEnum::NoMoreTermsException::INSTANCE =
        make_shared<NoMoreTermsException>();

IntersectTermsEnum::NoMoreTermsException::NoMoreTermsException() {}

runtime_error IntersectTermsEnum::NoMoreTermsException::fillInStackTrace()
{
  // Do nothing:
  return shared_from_this();
}

shared_ptr<BytesRef> IntersectTermsEnum::next() 
{
  try {
    return _next();
  } catch (const NoMoreTermsException &eoi) {
    // Provoke NPE if we are (illegally!) called again:
    currentFrame.reset();
    return nullptr;
  }
}

shared_ptr<BytesRef> IntersectTermsEnum::_next() 
{

  bool isSubBlock;

  if (useAutoPrefixTerm) {
    // If the current term was an auto-prefix term, we have to skip past it:
    isSubBlock = skipPastLastAutoPrefixTerm();
    assert(useAutoPrefixTerm == false);
  } else {
    isSubBlock = popPushNext();
  }

  while (true) {
    assert(currentFrame->transition == currentTransition);

    int state;
    int lastState;

    // NOTE: suffix == 0 can only happen on the first term in a block, when
    // there is a term exactly matching a prefix in the index.  If we
    // could somehow re-org the code so we only checked this case immediately
    // after pushing a frame...
    if (currentFrame->suffix != 0) {

      const std::deque<char> suffixBytes = currentFrame->suffixBytes;

      // This is the first byte of the suffix of the term we are now on:
      constexpr int label = suffixBytes[currentFrame->startBytePos] & 0xff;

      if (label < currentTransition->min) {
        // Common case: we are scanning terms in this block to "catch up" to
        // current transition in the automaton:
        int minTrans = currentTransition->min;
        while (currentFrame->nextEnt < currentFrame->entCount) {
          isSubBlock = currentFrame->next();
          if ((suffixBytes[currentFrame->startBytePos] & 0xff) >= minTrans) {
            goto nextTermContinue;
          }
        }

        // End of frame:
        isSubBlock = popPushNext();
        goto nextTermContinue;
      }

      // Advance where we are in the automaton to match this label:

      while (label > currentTransition->max) {
        if (currentFrame->transitionIndex >=
            currentFrame->transitionCount - 1) {
          // Pop this frame: no further matches are possible because
          // we've moved beyond what the max transition will allow
          if (currentFrame->ord == 0) {
            // Provoke NPE if we are (illegally!) called again:
            currentFrame.reset();
            return nullptr;
          }
          currentFrame = stack[currentFrame->ord - 1];
          currentTransition = currentFrame->transition;
          isSubBlock = popPushNext();
          goto nextTermContinue;
        }
        currentFrame->transitionIndex++;
        automaton->getNextTransition(currentTransition);

        if (label < currentTransition->min) {
          int minTrans = currentTransition->min;
          while (currentFrame->nextEnt < currentFrame->entCount) {
            isSubBlock = currentFrame->next();
            if ((suffixBytes[currentFrame->startBytePos] & 0xff) >= minTrans) {
              goto nextTermContinue;
            }
          }

          // End of frame:
          isSubBlock = popPushNext();
          goto nextTermContinue;
        }
      }

      if (commonSuffix != nullptr && !isSubBlock) {
        constexpr int termLen = currentFrame->prefix + currentFrame->suffix;
        if (termLen < commonSuffix->length) {
          // No match
          isSubBlock = popPushNext();
          goto nextTermContinue;
        }

        const std::deque<char> commonSuffixBytes = commonSuffix->bytes;

        constexpr int lenInPrefix = commonSuffix->length - currentFrame->suffix;
        assert(commonSuffix->offset == 0);
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
              isSubBlock = popPushNext();
              goto nextTermContinue;
            }
          }
          suffixBytesPos = currentFrame->startBytePos;
        } else {
          suffixBytesPos = currentFrame->startBytePos + currentFrame->suffix -
                           commonSuffix->length;
        }

        // Test overlapping suffix part:
        constexpr int commonSuffixBytesPosEnd = commonSuffix->length;
        while (commonSuffixBytesPos < commonSuffixBytesPosEnd) {
          if (suffixBytes[suffixBytesPos++] !=
              commonSuffixBytes[commonSuffixBytesPos++]) {
            isSubBlock = popPushNext();
            goto nextTermContinue;
          }
        }
      }

      // TODO: maybe we should do the same linear test
      // that AutomatonTermsEnum does, so that if we
      // reach a part of the automaton where .* is
      // "temporarily" accepted, we just blindly .next()
      // until the limit

      // See if the term suffix matches the automaton:

      // We know from above that the first byte in our suffix (label) matches
      // the current transition, so we step from the 2nd byte
      // in the suffix:
      lastState = currentFrame->state;
      state = currentTransition->dest;

      int end = currentFrame->startBytePos + currentFrame->suffix;
      for (int idx = currentFrame->startBytePos + 1; idx < end; idx++) {
        lastState = state;
        state = runAutomaton->step(state, suffixBytes[idx] & 0xff);
        if (state == -1) {
          // No match
          isSubBlock = popPushNext();
          goto nextTermContinue;
        }
      }
    } else {
      state = currentFrame->state;
      lastState = currentFrame->lastState;
    }

    if (isSubBlock) {
      // Match!  Recurse:
      copyTerm();
      currentFrame = pushFrame(state);
      currentTransition = currentFrame->transition;
      currentFrame->lastState = lastState;
    } else if (currentFrame->isAutoPrefixTerm) {
      // We are on an auto-prefix term, meaning this term was compiled
      // at indexing time, matching all terms sharing this prefix (or,
      // a floor'd subset of them if that count was too high).  A
      // prefix term represents a range of terms, so we now need to
      // test whether, from the current state in the automaton, it
      // accepts all terms in that range.  As long as it does, we can
      // use this term and then later skip ahead past all terms in
      // this range:
      if (allowAutoPrefixTerms) {

        if (currentFrame->floorSuffixLeadEnd == -1) {
          // Simple prefix case
          useAutoPrefixTerm = state == sinkState;
        } else {
          if (currentFrame->floorSuffixLeadStart == -1) {
            // Must also accept the empty string in this case
            if (automaton->isAccept(state)) {
              useAutoPrefixTerm = acceptsSuffixRange(
                  state, 0, currentFrame->floorSuffixLeadEnd);
            }
          } else {
            useAutoPrefixTerm = acceptsSuffixRange(
                lastState, currentFrame->floorSuffixLeadStart,
                currentFrame->floorSuffixLeadEnd);
          }
        }

        if (useAutoPrefixTerm) {
          // All suffixes of this auto-prefix term are accepted by the
          // automaton, so we can use it:
          copyTerm();
          return term_;
        } else {
          // We move onto the next term
        }
      } else {
        // We are not allowed to use auto-prefix terms, so we just skip it
      }
    } else if (runAutomaton->isAccept(state)) {
      copyTerm();
      assert((savedStartTerm == nullptr || term_->compareTo(savedStartTerm) > 0,
              L"saveStartTerm=" + savedStartTerm->utf8ToString() + L" term=" +
                  term_->utf8ToString()));
      return term_;
    } else {
      // This term is a prefix of a term accepted by the automaton, but is not
      // itself acceptd
    }

    isSubBlock = popPushNext();
  nextTermContinue:;
  }
nextTermBreak:;
}

bool IntersectTermsEnum::acceptsSuffixRange(int state, int start, int end)
{

  int count = automaton->initTransition(state, scratchTransition);
  for (int i = 0; i < count; i++) {
    automaton->getNextTransition(scratchTransition);
    if (start >= scratchTransition->min && end <= scratchTransition->max &&
        scratchTransition->dest == sinkState) {
      return true;
    }
  }

  return false;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
// brToString(org.apache.lucene.util.BytesRef b)
wstring IntersectTermsEnum::brToString(shared_ptr<BytesRef> b)
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

void IntersectTermsEnum::copyTerm()
{
  constexpr int len = currentFrame->prefix + currentFrame->suffix;
  if (term_->bytes.size() < len) {
    term_->bytes = ArrayUtil::grow(term_->bytes, len);
  }
  System::arraycopy(currentFrame->suffixBytes, currentFrame->startBytePos,
                    term_->bytes, currentFrame->prefix, currentFrame->suffix);
  term_->length = len;
}

bool IntersectTermsEnum::seekExact(shared_ptr<BytesRef> text)
{
  throw make_shared<UnsupportedOperationException>();
}

void IntersectTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t IntersectTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

SeekStatus IntersectTermsEnum::seekCeil(shared_ptr<BytesRef> text)
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::blocktree