using namespace std;

#include "Util.h"

namespace org::apache::lucene::util::fst
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;

Util::Util() {}

template <typename T>
T Util::get(shared_ptr<FST<T>> fst,
            shared_ptr<IntsRef> input) 
{

  // TODO: would be nice not to alloc this on every lookup
  shared_ptr<FST::Arc<T>> *const arc =
      fst->getFirstArc(make_shared<FST::Arc<T>>());

  shared_ptr<BytesReader> *const fstReader = fst->getBytesReader();

  // Accumulate output as we go
  T output = fst->outputs.getNoOutput();
  for (int i = 0; i < input->length; i++) {
    if (fst->findTargetArc(input->ints[input->offset + i], arc, arc,
                           fstReader) == nullptr) {
      return nullptr;
    }
    output = fst->outputs->add(output, arc->output);
  }

  if (arc->isFinal()) {
    return fst->outputs->add(output, arc->nextFinalOutput);
  } else {
    return nullptr;
  }
}

template <typename T>
T Util::get(shared_ptr<FST<T>> fst,
            shared_ptr<BytesRef> input) 
{
  assert(fst->inputType == FST::INPUT_TYPE::BYTE1);

  shared_ptr<BytesReader> *const fstReader = fst->getBytesReader();

  // TODO: would be nice not to alloc this on every lookup
  shared_ptr<FST::Arc<T>> *const arc =
      fst->getFirstArc(make_shared<FST::Arc<T>>());

  // Accumulate output as we go
  T output = fst->outputs.getNoOutput();
  for (int i = 0; i < input->length; i++) {
    if (fst->findTargetArc(input->bytes[i + input->offset] & 0xFF, arc, arc,
                           fstReader) == nullptr) {
      return nullptr;
    }
    output = fst->outputs->add(output, arc->output);
  }

  if (arc->isFinal()) {
    return fst->outputs->add(output, arc->nextFinalOutput);
  } else {
    return nullptr;
  }
}

shared_ptr<IntsRef> Util::getByOutput(shared_ptr<FST<int64_t>> fst,
                                      int64_t targetOutput) 
{

  shared_ptr<BytesReader> *const in_ = fst->getBytesReader();

  // TODO: would be nice not to alloc this on every lookup
  shared_ptr<FST::Arc<int64_t>> arc =
      fst->getFirstArc(make_shared<FST::Arc<int64_t>>());

  shared_ptr<FST::Arc<int64_t>> scratchArc =
      make_shared<FST::Arc<int64_t>>();

  shared_ptr<IntsRefBuilder> *const result = make_shared<IntsRefBuilder>();
  return getByOutput(fst, targetOutput, in_, arc, scratchArc, result);
}

shared_ptr<IntsRef>
Util::getByOutput(shared_ptr<FST<int64_t>> fst, int64_t targetOutput,
                  shared_ptr<BytesReader> in_, shared_ptr<Arc<int64_t>> arc,
                  shared_ptr<Arc<int64_t>> scratchArc,
                  shared_ptr<IntsRefBuilder> result) 
{
  int64_t output = arc->output;
  int upto = 0;

  // System.out.println("reverseLookup output=" + targetOutput);

  while (true) {
    // System.out.println("loop: output=" + output + " upto=" + upto + " arc=" +
    // arc);
    if (arc->isFinal()) {
      constexpr int64_t finalOutput = output + arc->nextFinalOutput;
      // System.out.println("  isFinal finalOutput=" + finalOutput);
      if (finalOutput == targetOutput) {
        result->setLength(upto);
        // System.out.println("    found!");
        return result->get();
      } else if (finalOutput > targetOutput) {
        // System.out.println("    not found!");
        return nullptr;
      }
    }

    if (FST::targetHasArcs(arc)) {
      // System.out.println("  targetHasArcs");
      result->grow(1 + upto);

      fst->readFirstRealTargetArc(arc->target, arc, in_);

      if (arc->bytesPerArc != 0) {

        int low = 0;
        int high = arc->numArcs - 1;
        int mid = 0;
        // System.out.println("bsearch: numArcs=" + arc.numArcs + " target=" +
        // targetOutput + " output=" + output);
        bool exact = false;
        while (low <= high) {
          mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
          in_->setPosition(arc->posArcsStart);
          in_->skipBytes(arc->bytesPerArc * mid);
          constexpr char flags = in_->readByte();
          fst->readLabel(in_);
          constexpr int64_t minArcOutput;
          if ((flags & FST::BIT_ARC_HAS_OUTPUT) != 0) {
            constexpr int64_t arcOutput = fst->outputs.read(in_);
            minArcOutput = output + arcOutput;
          } else {
            minArcOutput = output;
          }
          // System.out.println("  cycle mid=" + mid + " label=" + (char) label
          // + " output=" + minArcOutput);
          if (minArcOutput == targetOutput) {
            exact = true;
            break;
          } else if (minArcOutput < targetOutput) {
            low = mid + 1;
          } else {
            high = mid - 1;
          }
        }

        if (high == -1) {
          return nullptr;
        } else if (exact) {
          arc->arcIdx = mid - 1;
        } else {
          arc->arcIdx = low - 2;
        }

        fst->readNextRealArc(arc, in_);
        result->setIntAt(upto++, arc->label);
        output += arc->output;

      } else {

        shared_ptr<FST::Arc<int64_t>> prevArc = nullptr;

        while (true) {
          // System.out.println("    cycle label=" + arc.label + " output=" +
          // arc.output);

          // This is the min output we'd hit if we follow
          // this arc:
          constexpr int64_t minArcOutput = output + arc->output;

          if (minArcOutput == targetOutput) {
            // Recurse on this arc:
            // System.out.println("  match!  break");
            output = minArcOutput;
            result->setIntAt(upto++, arc->label);
            break;
          } else if (minArcOutput > targetOutput) {
            if (prevArc == nullptr) {
              // Output doesn't exist
              return nullptr;
            } else {
              // Recurse on previous arc:
              arc->copyFrom(prevArc);
              result->setIntAt(upto++, arc->label);
              output += arc->output;
              // System.out.println("    recurse prev label=" + (char) arc.label
              // + " output=" + output);
              break;
            }
          } else if (arc->isLast()) {
            // Recurse on this arc:
            output = minArcOutput;
            // System.out.println("    recurse last label=" + (char) arc.label +
            // " output=" + output);
            result->setIntAt(upto++, arc->label);
            break;
          } else {
            // Read next arc in this node:
            prevArc = scratchArc;
            prevArc->copyFrom(arc);
            // System.out.println("      after copy label=" + (char)
            // prevArc.label + " vs " + (char) arc.label);
            fst->readNextRealArc(arc, in_);
          }
        }
      }
    } else {
      // System.out.println("  no target arcs; not found!");
      return nullptr;
    }
  }
}

template <typename T>
shared_ptr<TopResults<T>>
Util::shortestPaths(shared_ptr<FST<T>> fst, shared_ptr<FST::Arc<T>> fromNode,
                    T startOutput, shared_ptr<Comparator<T>> comparator,
                    int topN, bool allowEmptyString) 
{

  // All paths are kept, so we can pass topN for
  // maxQueueDepth and the pruning is admissible:
  shared_ptr<TopNSearcher<T>> searcher =
      make_shared<TopNSearcher<T>>(fst, topN, topN, comparator);

  // since this search is initialized with a single start node
  // it is okay to start with an empty input path here
  searcher->addStartPaths(fromNode, startOutput, allowEmptyString,
                          make_shared<IntsRefBuilder>());
  return searcher->search();
}

template <typename T>
void Util::toDot(shared_ptr<FST<T>> fst, shared_ptr<Writer> out, bool sameRank,
                 bool labelStates) 
{
  const wstring expandedNodeColor = L"blue";

  // This is the start arc in the automaton (from the epsilon state to the first
  // state with outgoing transitions.
  shared_ptr<FST::Arc<T>> *const startArc =
      fst->getFirstArc(make_shared<FST::Arc<T>>());

  // A queue of transitions to consider for the next level.
  const deque<FST::Arc<T>> thisLevelQueue = deque<FST::Arc<T>>();

  // A queue of transitions to consider when processing the next level.
  const deque<FST::Arc<T>> nextLevelQueue = deque<FST::Arc<T>>();
  nextLevelQueue.push_back(startArc);
  // System.out.println("toDot: startArc: " + startArc);

  // A deque of states on the same level (for ranking).
  const deque<int> sameLevelStates = deque<int>();

  // A bitset of already seen states (target offset).
  shared_ptr<BitSet> *const seen = make_shared<BitSet>();
  seen->set(static_cast<int>(startArc->target));

  // Shape for states.
  const wstring stateShape = L"circle";
  const wstring finalStateShape = L"doublecircle";

  // Emit DOT prologue.
  out->write(L"digraph FST {\n");
  out->write(L"  rankdir = LR; splines=true; concentrate=true; ordering=out; "
             L"ranksep=2.5; \n");

  if (!labelStates) {
    out->write(L"  node [shape=circle, width=.2, height=.2, style=filled]\n");
  }

  emitDotState(out, L"initial", L"point", L"white", L"");

  constexpr T NO_OUTPUT = fst->outputs.getNoOutput();
  shared_ptr<BytesReader> *const r = fst->getBytesReader();

  // final FST.Arc<T> scratchArc = new FST.Arc<>();

  {
    const wstring stateColor;
    if (fst->isExpandedTarget(startArc, r)) {
      stateColor = expandedNodeColor;
    } else {
      stateColor = L"";
    }

    constexpr bool isFinal;
    constexpr T finalOutput;
    if (startArc->isFinal()) {
      isFinal = true;
      finalOutput = startArc->nextFinalOutput == NO_OUTPUT
                        ? nullptr
                        : startArc->nextFinalOutput;
    } else {
      isFinal = false;
      finalOutput = nullptr;
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    emitDotState(out, Long::toString(startArc->target),
                 isFinal ? finalStateShape : stateShape, stateColor,
                 finalOutput == nullptr
                     ? L""
                     : fst->outputs.outputToString(finalOutput));
  }

  out->write(L"  initial -> " + to_wstring(startArc->target) + L"\n");

  int level = 0;

  while (!nextLevelQueue.empty()) {
    // we could double buffer here, but it doesn't matter probably.
    // System.out.println("next level=" + level);
    thisLevelQueue.insert(thisLevelQueue.end(), nextLevelQueue.begin(),
                          nextLevelQueue.end());
    nextLevelQueue.clear();

    level++;
    out->write(L"\n  // Transitions and states at level: " + to_wstring(level) +
               L"\n");
    while (!thisLevelQueue.empty()) {
      shared_ptr<FST::Arc<T>> *const arc = thisLevelQueue.pop_back();
      // System.out.println("  pop: " + arc);
      if (FST::targetHasArcs(arc)) {
        // scan all target arcs
        // System.out.println("  readFirstTarget...");

        constexpr int64_t node = arc->target;

        fst->readFirstRealTargetArc(arc->target, arc, r);

        // System.out.println("    firstTarget: " + arc);

        while (true) {

          // System.out.println("  cycle arc=" + arc);
          // Emit the unseen state and add it to the queue for the next level.
          if (arc->target >= 0 && !seen->get(static_cast<int>(arc->target))) {

            /*
            bool isFinal = false;
            T finalOutput = null;
            fst.readFirstTargetArc(arc, scratchArc);
            if (scratchArc.isFinal() && fst.targetHasArcs(scratchArc)) {
              // target is final
              isFinal = true;
              finalOutput = scratchArc.output == NO_OUTPUT ? null :
            scratchArc.output; System.out.println("dot hit final label=" +
            (char) scratchArc.label);
            }
            */
            const wstring stateColor;
            if (fst->isExpandedTarget(arc, r)) {
              stateColor = expandedNodeColor;
            } else {
              stateColor = L"";
            }

            const wstring finalOutput;
            if (arc->nextFinalOutput != nullptr &&
                arc->nextFinalOutput != NO_OUTPUT) {
              finalOutput = fst->outputs.outputToString(arc->nextFinalOutput);
            } else {
              finalOutput = L"";
            }

            // C++ TODO: There is no native C++ equivalent to 'toString':
            emitDotState(out, Long::toString(arc->target), stateShape,
                         stateColor, finalOutput);
            // To see the node address, use this instead:
            // emitDotState(out, Integer.toString(arc.target), stateShape,
            // stateColor, std::wstring.valueOf(arc.target));
            seen->set(static_cast<int>(arc->target));
            nextLevelQueue.push_back(
                (make_shared<FST::Arc<T>>())->copyFrom(arc));
            sameLevelStates.push_back(static_cast<int>(arc->target));
          }

          wstring outs;
          if (arc->output != NO_OUTPUT) {
            outs = L"/" + fst->outputs.outputToString(arc->output);
          } else {
            outs = L"";
          }

          if (!FST::targetHasArcs(arc) && arc->isFinal() &&
              arc->nextFinalOutput != NO_OUTPUT) {
            // Tricky special case: sometimes, due to
            // pruning, the builder can [sillily] produce
            // an FST with an arc into the final end state
            // (-1) but also with a next final output; in
            // this case we pull that output up onto this
            // arc
            outs = outs + L"/[" +
                   fst->outputs.outputToString(arc->nextFinalOutput) + L"]";
          }

          const wstring arcColor;
          if (arc->flag(FST::BIT_TARGET_NEXT)) {
            arcColor = L"red";
          } else {
            arcColor = L"black";
          }

          assert(arc->label != FST::END_LABEL);
          out->write(L"  " + to_wstring(node) + L" -> " +
                     to_wstring(arc->target) + L" [label=\"" +
                     printableLabel(arc->label) + outs + L"\"" +
                     (arc->isFinal() ? L" style=\"bold\"" : L"") +
                     L" color=\"" + arcColor + L"\"]\n");

          // Break the loop if we're on the last arc of this state.
          if (arc->isLast()) {
            // System.out.println("    break");
            break;
          }
          fst->readNextRealArc(arc, r);
        }
      }
    }

    // Emit state ranking information.
    if (sameRank && sameLevelStates.size() > 1) {
      out->write(L"  {rank=same; ");
      for (auto state : sameLevelStates) {
        out->write(to_wstring(state) + L"; ");
      }
      out->write(L" }\n");
    }
    sameLevelStates.clear();
  }

  // Emit terminating state (always there anyway).
  out->write(
      L"  -1 [style=filled, color=black, shape=doublecircle, label=\"\"]\n\n");
  out->write(L"  {rank=sink; -1 }\n");

  out->write(L"}\n");
  out->flush();
}

void Util::emitDotState(shared_ptr<Writer> out, const wstring &name,
                        const wstring &shape, const wstring &color,
                        const wstring &label) 
{
  out->write(
      L"  " + name + L" [" +
      StringHelper::toString(shape != L"" ? L"shape=" + shape : L"") + L" " +
      StringHelper::toString(color != L"" ? L"color=" + color : L"") + L" " +
      StringHelper::toString(label != L"" ? L"label=\"" + label + L"\""
                                          : L"label=\"\"") +
      L" " + L"]\n");
}

wstring Util::printableLabel(int label)
{
  // Any ordinary ascii character, except for " or \, are
  // printed as the character; else, as a hex string:
  if (label >= 0x20 && label <= 0x7d && label != 0x22 &&
      label != 0x5c) { // " OR \
//C++ TODO: There is no native C++ equivalent to 'toString':
    return Character::toString(static_cast<wchar_t>(label));
  }
  return L"0x" + Integer::toHexString(label);
}

shared_ptr<IntsRef> Util::toUTF16(shared_ptr<std::wstring> s,
                                  shared_ptr<IntsRefBuilder> scratch)
{
  constexpr int charLimit = s->length();
  scratch->setLength(charLimit);
  scratch->grow(charLimit);
  for (int idx = 0; idx < charLimit; idx++) {
    scratch->setIntAt(idx, static_cast<int>(s->charAt(idx)));
  }
  return scratch->get();
}

shared_ptr<IntsRef> Util::toUTF32(shared_ptr<std::wstring> s,
                                  shared_ptr<IntsRefBuilder> scratch)
{
  int charIdx = 0;
  int intIdx = 0;
  constexpr int charLimit = s->length();
  while (charIdx < charLimit) {
    scratch->grow(intIdx + 1);
    constexpr int utf32 = Character::codePointAt(s, charIdx);
    scratch->setIntAt(intIdx, utf32);
    charIdx += Character::charCount(utf32);
    intIdx++;
  }
  scratch->setLength(intIdx);
  return scratch->get();
}

shared_ptr<IntsRef> Util::toUTF32(std::deque<wchar_t> &s, int offset,
                                  int length,
                                  shared_ptr<IntsRefBuilder> scratch)
{
  int charIdx = offset;
  int intIdx = 0;
  constexpr int charLimit = offset + length;
  while (charIdx < charLimit) {
    scratch->grow(intIdx + 1);
    constexpr int utf32 = Character::codePointAt(s, charIdx, charLimit);
    scratch->setIntAt(intIdx, utf32);
    charIdx += Character::charCount(utf32);
    intIdx++;
  }
  scratch->setLength(intIdx);
  return scratch->get();
}

shared_ptr<IntsRef> Util::toIntsRef(shared_ptr<BytesRef> input,
                                    shared_ptr<IntsRefBuilder> scratch)
{
  scratch->clear();
  for (int i = 0; i < input->length; i++) {
    scratch->append(input->bytes[i + input->offset] & 0xFF);
  }
  return scratch->get();
}

shared_ptr<BytesRef> Util::toBytesRef(shared_ptr<IntsRef> input,
                                      shared_ptr<BytesRefBuilder> scratch)
{
  scratch->grow(input->length);
  for (int i = 0; i < input->length; i++) {
    int value = input->ints[i + input->offset];
    // NOTE: we allow -128 to 255
    assert((value >= std,
            : numeric_limits<char>::min() && value <= 255
            : L"value " + to_wstring(value) + L" doesn't fit into byte"));
    scratch->setByteAt(i, static_cast<char>(value));
  }
  scratch->setLength(input->length);
  return scratch->get();
}

template <typename T>
shared_ptr<Arc<T>>
Util::readCeilArc(int label, shared_ptr<FST<T>> fst, shared_ptr<Arc<T>> follow,
                  shared_ptr<Arc<T>> arc,
                  shared_ptr<BytesReader> in_) 
{
  // TODO maybe this is a useful in the FST class - we could simplify some other
  // code like FSTEnum?
  if (label == FST::END_LABEL) {
    if (follow->isFinal()) {
      if (follow->target <= 0) {
        arc->flags = FST::BIT_LAST_ARC;
      } else {
        arc->flags = 0;
        // NOTE: nextArc is a node (not an address!) in this case:
        arc->nextArc = follow->target;
      }
      arc->output = follow->nextFinalOutput;
      arc->label = FST::END_LABEL;
      return arc;
    } else {
      return nullptr;
    }
  }

  if (!FST::targetHasArcs(follow)) {
    return nullptr;
  }
  fst->readFirstTargetArc(follow, arc, in_);
  if (arc->bytesPerArc != 0 && arc->label != FST::END_LABEL) {
    // Arcs are fixed array -- use binary search to find
    // the target.

    int low = arc->arcIdx;
    int high = arc->numArcs - 1;
    int mid = 0;
    // System.out.println("do arc array low=" + low + " high=" + high +
    // " targetLabel=" + targetLabel);
    while (low <= high) {
      mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
      in_->setPosition(arc->posArcsStart);
      in_->skipBytes(arc->bytesPerArc * mid + 1);
      constexpr int midLabel = fst->readLabel(in_);
      constexpr int cmp = midLabel - label;
      // System.out.println("  cycle low=" + low + " high=" + high + " mid=" +
      // mid + " midLabel=" + midLabel + " cmp=" + cmp);
      if (cmp < 0) {
        low = mid + 1;
      } else if (cmp > 0) {
        high = mid - 1;
      } else {
        arc->arcIdx = mid - 1;
        return fst->readNextRealArc(arc, in_);
      }
    }
    if (low == arc->numArcs) {
      // DEAD END!
      return nullptr;
    }

    arc->arcIdx = (low > high ? high : low);
    return fst->readNextRealArc(arc, in_);
  }

  // Linear scan
  fst->readFirstRealTargetArc(follow->target, arc, in_);

  while (true) {
    // System.out.println("  non-bs cycle");
    // TODO: we should fix this code to not have to create
    // object for the output of every arc we scan... only
    // for the matching arc, if found
    if (arc->label >= label) {
      // System.out.println("    found!");
      return arc;
    } else if (arc->isLast()) {
      return nullptr;
    } else {
      fst->readNextRealArc(arc, in_);
    }
  }
}
} // namespace org::apache::lucene::util::fst