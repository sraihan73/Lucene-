using namespace std;

#include "ConcatenateGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/LimitedFiniteStringsIterator.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Transition.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using LimitedFiniteStringsIterator =
    org::apache::lucene::util::automaton::LimitedFiniteStringsIterator;
using Operations = org::apache::lucene::util::automaton::Operations;
using TooComplexToDeterminizeException =
    org::apache::lucene::util::automaton::TooComplexToDeterminizeException;
using Transition = org::apache::lucene::util::automaton::Transition;
using Util = org::apache::lucene::util::fst::Util;

ConcatenateGraphFilter::ConcatenateGraphFilter(
    shared_ptr<TokenStream> inputTokenStream)
    : ConcatenateGraphFilter(inputTokenStream, DEFAULT_PRESERVE_SEP,
                             DEFAULT_PRESERVE_POSITION_INCREMENTS,
                             DEFAULT_MAX_GRAPH_EXPANSIONS)
{
}

ConcatenateGraphFilter::ConcatenateGraphFilter(
    shared_ptr<TokenStream> inputTokenStream, bool preserveSep,
    bool preservePositionIncrements, int maxGraphExpansions)
    : inputTokenStream(inputTokenStream), preserveSep(preserveSep),
      preservePositionIncrements(preservePositionIncrements),
      maxGraphExpansions(maxGraphExpansions)
{
  // Don't call the super(input) ctor - this is a true delegate and has a new
  // attribute source since we consume the input stream entirely in the first
  // call to incrementToken
}

void ConcatenateGraphFilter::reset() 
{
  TokenStream::reset();
  // we only capture this if we really need it to save the UTF-8 to UTF-16
  // conversion
  charTermAttribute =
      getAttribute(CharTermAttribute::typeid); // may return null
  wasReset = true;
}

bool ConcatenateGraphFilter::incrementToken() 
{
  if (finiteStrings == nullptr) {
    if (wasReset == false) {
      throw make_shared<IllegalStateException>(
          L"reset() missing before incrementToken");
    }
    // lazy init/consume
    shared_ptr<Automaton> automaton =
        toAutomaton(); // calls reset(), incrementToken() repeatedly, and end()
                       // on inputTokenStream
    finiteStrings = make_shared<LimitedFiniteStringsIterator>(
        automaton, maxGraphExpansions);
    // note: would be nice to know the startOffset but toAutomaton doesn't
    // capture it.  We'll assume 0
    endOffset =
        inputTokenStream->getAttribute(OffsetAttribute::typeid)->endOffset();
  }

  shared_ptr<IntsRef> string = finiteStrings->next();
  if (string == nullptr) {
    return false;
  }

  clearAttributes();

  if (finiteStrings->size() >
      1) { // if number of iterated strings so far is more than one...
    posIncrAtt->setPositionIncrement(0); // stacked
  }

  offsetAtt->setOffset(0, endOffset);

  Util::toBytesRef(string, bytesAtt->builder()); // now we have UTF-8
  if (charTermAttribute != nullptr) {
    charTermAttribute->setLength(0);
    charTermAttribute->append(bytesAtt->toUTF16());
  }

  return true;
}

void ConcatenateGraphFilter::end() 
{
  TokenStream::end();
  if (finiteStrings ==
      nullptr) {             // thus inputTokenStream hasn't yet received end()
    inputTokenStream->end(); // the input TS may really want to see "end()"
                             // called even if incrementToken hasn't.
  } // else we already eagerly consumed inputTokenStream including end()
  if (endOffset != -1) {
    offsetAtt->setOffset(0, endOffset);
  }
}

ConcatenateGraphFilter::~ConcatenateGraphFilter()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  // delegate lifecycle.  Note toAutomaton does not close the stream
  delete inputTokenStream;
  finiteStrings.reset();
  wasReset = false; // reset
  endOffset = -1;   // reset
}

shared_ptr<Automaton> ConcatenateGraphFilter::toAutomaton() 
{
  return toAutomaton(false);
}

shared_ptr<Automaton>
ConcatenateGraphFilter::toAutomaton(bool unicodeAware) 
{
  // TODO refactor this
  // maybe we could hook up a modified automaton from TermAutomatonQuery here?

  // Create corresponding automaton: labels are bytes
  // from each analyzed token, with byte 0 used as
  // separator between tokens:
  shared_ptr<TokenStreamToAutomaton> *const tsta;
  if (preserveSep) {
    tsta = make_shared<EscapingTokenStreamToAutomaton>(SEP_LABEL);
  } else {
    // When we're not preserving sep, we don't steal 0xff
    // byte, so we don't need to do any escaping:
    tsta = make_shared<TokenStreamToAutomaton>();
  }
  tsta->setPreservePositionIncrements(preservePositionIncrements);
  tsta->setUnicodeArcs(unicodeAware);

  shared_ptr<Automaton> automaton = tsta->toAutomaton(inputTokenStream);

  // TODO: we can optimize this somewhat by determinizing
  // while we convert
  automaton = replaceSep(automaton, preserveSep, SEP_LABEL);
  // This automaton should not blow up during determinize:
  return Operations::determinize(automaton, maxGraphExpansions);
}

ConcatenateGraphFilter::EscapingTokenStreamToAutomaton::
    EscapingTokenStreamToAutomaton(int sepLabel)
    : sepLabel(static_cast<char>(sepLabel))
{
  assert((sepLabel <= std, : numeric_limits<char>::max()));
}

shared_ptr<BytesRef>
ConcatenateGraphFilter::EscapingTokenStreamToAutomaton::changeToken(
    shared_ptr<BytesRef> in_)
{
  int upto = 0;
  for (int i = 0; i < in_->length; i++) {
    char b = in_->bytes[in_->offset + i];
    if (b == sepLabel) {
      spare->grow(upto + 2);
      spare->setByteAt(upto++, sepLabel);
      spare->setByteAt(upto++, b);
    } else {
      spare->grow(upto + 1);
      spare->setByteAt(upto++, b);
    }
  }
  spare->setLength(upto);
  return spare->get();
}

shared_ptr<Automaton>
ConcatenateGraphFilter::replaceSep(shared_ptr<Automaton> a, bool preserveSep,
                                   int sepLabel)
{

  shared_ptr<Automaton> result = make_shared<Automaton>();

  // Copy all states over
  int numStates = a->getNumStates();
  for (int s = 0; s < numStates; s++) {
    result->createState();
    result->setAccept(s, a->isAccept(s));
  }

  // Go in reverse topo sort so we know we only have to
  // make one pass:
  shared_ptr<Transition> t = make_shared<Transition>();
  std::deque<int> topoSortStates = Operations::topoSortStates(a);
  for (int i = 0; i < topoSortStates.size(); i++) {
    int state = topoSortStates[topoSortStates.size() - 1 - i];
    int count = a->initTransition(state, t);
    for (int j = 0; j < count; j++) {
      a->getNextTransition(t);
      if (t->min == TokenStreamToAutomaton::POS_SEP) {
        assert(t->max == TokenStreamToAutomaton::POS_SEP);
        if (preserveSep) {
          // Remap to SEP_LABEL:
          result->addTransition(state, t->dest, sepLabel);
        } else {
          result->addEpsilon(state, t->dest);
        }
      } else if (t->min == TokenStreamToAutomaton::HOLE) {
        assert(t->max == TokenStreamToAutomaton::HOLE);

        // Just remove the hole: there will then be two
        // SEP tokens next to each other, which will only
        // match another hole at search time.  Note that
        // it will also match an empty-string token ... if
        // that's somehow a problem we can always map_obj HOLE
        // to a dedicated byte (and escape it in the
        // input).
        result->addEpsilon(state, t->dest);
      } else {
        result->addTransition(state, t->dest, t->min, t->max);
      }
    }
  }

  result->finishState();

  return result;
}

ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::
    BytesRefBuilderTermAttributeImpl()
{
}

shared_ptr<BytesRefBuilder>
ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::builder()
{
  return bytes;
}

shared_ptr<BytesRef>
ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::getBytesRef()
{
  return bytes->get();
}

void ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::clear()
{
  bytes->clear();
}

void ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> target)
{
  shared_ptr<BytesRefBuilderTermAttributeImpl> other =
      std::static_pointer_cast<BytesRefBuilderTermAttributeImpl>(target);
  other->bytes->copyBytes(bytes);
}

shared_ptr<AttributeImpl>
ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::clone()
{
  shared_ptr<BytesRefBuilderTermAttributeImpl> other =
      make_shared<BytesRefBuilderTermAttributeImpl>();
  copyTo(other);
  return other;
}

void ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(TermToBytesRefAttribute::typeid, L"bytes", getBytesRef());
}

shared_ptr<std::wstring>
ConcatenateGraphFilter::BytesRefBuilderTermAttributeImpl::toUTF16()
{
  if (charsRef == nullptr) {
    charsRef = make_shared<CharsRefBuilder>();
  }
  charsRef->copyUTF8Bytes(getBytesRef());
  return charsRef->get();
}
} // namespace org::apache::lucene::analysis::miscellaneous