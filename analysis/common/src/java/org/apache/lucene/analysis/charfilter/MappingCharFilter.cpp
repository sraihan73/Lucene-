using namespace std;

#include "MappingCharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../util/RollingCharBuffer.h"
#include "NormalizeCharMap.h"

namespace org::apache::lucene::analysis::charfilter
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
using RollingCharBuffer =
    org::apache::lucene::analysis::util::RollingCharBuffer;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharSequenceOutputs = org::apache::lucene::util::fst::CharSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;

MappingCharFilter::MappingCharFilter(shared_ptr<NormalizeCharMap> normMap,
                                     shared_ptr<Reader> in_)
    : BaseCharFilter(in_), map_obj(normMap->map_obj),
      cachedRootArcs(normMap->cachedRootArcs)
{
  buffer->reset(in_);

  if (map_obj != nullptr) {
    fstReader = map_obj->getBytesReader();
  } else {
    fstReader.reset();
  }
}

void MappingCharFilter::reset() 
{
  input->reset();
  buffer->reset(input);
  replacement.reset();
  inputOff = 0;
}

int MappingCharFilter::read() 
{

  // System.out.println("\nread");
  while (true) {

    if (replacement != nullptr && replacementPointer < replacement->length_) {
      // System.out.println("  return repl[" + replacementPointer + "]=" +
      // replacement.chars[replacement.offset + replacementPointer]);
      return replacement->chars[replacement->offset + replacementPointer++];
    }

    // TODO: a more efficient approach would be Aho/Corasick's
    // algorithm
    // (http://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_string_matching_algorithm)
    // or this generalizatio:
    // www.cis.uni-muenchen.de/people/Schulz/Pub/dictle5.ps
    //
    // I think this would be (almost?) equivalent to 1) adding
    // epsilon arcs from all final nodes back to the init
    // node in the FST, 2) adding a .* (skip any char)
    // loop on the initial node, and 3) determinizing
    // that.  Then we would not have to restart matching
    // at each position.

    int lastMatchLen = -1;
    shared_ptr<CharsRef> lastMatch = nullptr;

    constexpr int firstCH = buffer->get(inputOff);
    if (firstCH != -1) {
      shared_ptr<FST::Arc<std::shared_ptr<CharsRef>>> arc =
          cachedRootArcs[static_cast<Character>(static_cast<wchar_t>(firstCH))];
      if (arc != nullptr) {
        if (!FST::targetHasArcs(arc)) {
          // Fast pass for single character match:
          assert(arc->isFinal());
          lastMatchLen = 1;
          lastMatch = arc->output;
        } else {
          int lookahead = 0;
          shared_ptr<CharsRef> output = arc->output;
          while (true) {
            lookahead++;

            if (arc->isFinal()) {
              // Match! (to node is final)
              lastMatchLen = lookahead;
              lastMatch = outputs->add(output, arc->nextFinalOutput);
              // Greedy: keep searching to see if there's a
              // longer match...
            }

            if (!FST::targetHasArcs(arc)) {
              break;
            }

            int ch = buffer->get(inputOff + lookahead);
            if (ch == -1) {
              break;
            }
            if ((arc = map_obj->findTargetArc(ch, arc, scratchArc, fstReader)) ==
                nullptr) {
              // Dead end
              break;
            }
            output = outputs->add(output, arc->output);
          }
        }
      }
    }

    if (lastMatch != nullptr) {
      inputOff += lastMatchLen;
      // System.out.println("  match!  len=" + lastMatchLen + " repl=" +
      // lastMatch);

      constexpr int diff = lastMatchLen - lastMatch->length_;

      if (diff != 0) {
        constexpr int prevCumulativeDiff = getLastCumulativeDiff();
        if (diff > 0) {
          // Replacement is shorter than matched input:
          addOffCorrectMap(inputOff - diff - prevCumulativeDiff,
                           prevCumulativeDiff + diff);
        } else {
          // Replacement is longer than matched input: remap
          // the "extra" chars all back to the same input
          // offset:
          constexpr int outputStart = inputOff - prevCumulativeDiff;
          for (int extraIDX = 0; extraIDX < -diff; extraIDX++) {
            addOffCorrectMap(outputStart + extraIDX,
                             prevCumulativeDiff - extraIDX - 1);
          }
        }
      }

      replacement = lastMatch;
      replacementPointer = 0;

    } else {
      constexpr int ret = buffer->get(inputOff);
      if (ret != -1) {
        inputOff++;
        buffer->freeBefore(inputOff);
      }
      return ret;
    }
  }
}

int MappingCharFilter::read(std::deque<wchar_t> &cbuf, int off,
                            int len) 
{
  int numRead = 0;
  for (int i = off; i < off + len; i++) {
    int c = read();
    if (c == -1) {
      break;
    }
    cbuf[i] = static_cast<wchar_t>(c);
    numRead++;
  }

  return numRead == 0 ? -1 : numRead;
}
} // namespace org::apache::lucene::analysis::charfilter