using namespace std;

#include "TokenStreamToAutomaton.h"
#include "../util/BytesRef.h"
#include "../util/automaton/Automaton.h"
#include "TokenStream.h"
#include "tokenattributes/OffsetAttribute.h"
#include "tokenattributes/PositionIncrementAttribute.h"
#include "tokenattributes/PositionLengthAttribute.h"
#include "tokenattributes/TermToBytesRefAttribute.h"

namespace org::apache::lucene::analysis
{
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;
using Automaton = org::apache::lucene::util::automaton::Automaton;

TokenStreamToAutomaton::TokenStreamToAutomaton()
{
  this->preservePositionIncrements = true;
}

void TokenStreamToAutomaton::setPreservePositionIncrements(
    bool enablePositionIncrements)
{
  this->preservePositionIncrements = enablePositionIncrements;
}

void TokenStreamToAutomaton::setFinalOffsetGapAsHole(bool finalOffsetGapAsHole)
{
  this->finalOffsetGapAsHole = finalOffsetGapAsHole;
}

void TokenStreamToAutomaton::setUnicodeArcs(bool unicodeArcs)
{
  this->unicodeArcs = unicodeArcs;
}

void TokenStreamToAutomaton::Position::reset()
{
  arriving = -1;
  leaving = -1;
}

shared_ptr<Position> TokenStreamToAutomaton::Positions::newInstance()
{
  return make_shared<Position>();
}

shared_ptr<BytesRef>
TokenStreamToAutomaton::changeToken(shared_ptr<BytesRef> in_)
{
  return in_;
}

shared_ptr<Automaton> TokenStreamToAutomaton::toAutomaton(
    shared_ptr<TokenStream> in_) 
{
  shared_ptr<Automaton::Builder> *const builder =
      make_shared<Automaton::Builder>();
  builder->createState();

  shared_ptr<TermToBytesRefAttribute> *const termBytesAtt =
      in_->addAttribute(TermToBytesRefAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> *const posIncAtt =
      in_->addAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionLengthAttribute> *const posLengthAtt =
      in_->addAttribute(PositionLengthAttribute::typeid);
  shared_ptr<OffsetAttribute> *const offsetAtt =
      in_->addAttribute(OffsetAttribute::typeid);

  in_->reset();

  // Only temporarily holds states ahead of our current
  // position:

  shared_ptr<RollingBuffer<std::shared_ptr<Position>>> *const positions =
      make_shared<Positions>();

  int pos = -1;
  int freedPos = 0;
  shared_ptr<Position> posData = nullptr;
  int maxOffset = 0;
  while (in_->incrementToken()) {
    int posInc = posIncAtt->getPositionIncrement();
    if (preservePositionIncrements == false && posInc > 1) {
      posInc = 1;
    }
    assert(pos > -1 || posInc > 0);

    if (posInc > 0) {

      // New node:
      pos += posInc;

      posData = positions->get(pos);
      assert(posData->leaving == -1);

      if (posData->arriving == -1) {
        // No token ever arrived to this position
        if (pos == 0) {
          // OK: this is the first token
          posData->leaving = 0;
        } else {
          // This means there's a hole (eg, StopFilter
          // does this):
          posData->leaving = builder->createState();
          addHoles(builder, positions, pos);
        }
      } else {
        posData->leaving = builder->createState();
        builder->addTransition(posData->arriving, posData->leaving, POS_SEP);
        if (posInc > 1) {
          // A token spanned over a hole; add holes
          // "under" it:
          addHoles(builder, positions, pos);
        }
      }
      while (freedPos <= pos) {
        shared_ptr<Position> freePosData = positions->get(freedPos);
        // don't free this position yet if we may still need to fill holes over
        // it:
        if (freePosData->arriving == -1 || freePosData->leaving == -1) {
          break;
        }
        positions->freeBefore(freedPos);
        freedPos++;
      }
    }

    constexpr int endPos = pos + posLengthAtt->getPositionLength();

    shared_ptr<BytesRef> *const termUTF8 =
        changeToken(termBytesAtt->getBytesRef());
    std::deque<int> termUnicode;
    shared_ptr<Position> *const endPosData = positions->get(endPos);
    if (endPosData->arriving == -1) {
      endPosData->arriving = builder->createState();
    }

    int termLen;
    if (unicodeArcs) {
      const wstring utf16 = termUTF8->utf8ToString();
      termUnicode = std::deque<int>(utf16.codePointCount(0, utf16.length()));
      termLen = termUnicode.size();
      for (shared_ptr<int cp, > i = 0, j = 0; i < utf16.length();
           i += Character::charCount(cp)) {
        termUnicode[j++] = cp = utf16.codePointAt(i);
      }
    } else {
      termLen = termUTF8->length;
    }

    int state = posData->leaving;

    for (int byteIDX = 0; byteIDX < termLen; byteIDX++) {
      constexpr int nextState = byteIDX == termLen - 1 ? endPosData->arriving
                                                       : builder->createState();
      int c;
      if (unicodeArcs) {
        c = termUnicode[byteIDX];
      } else {
        c = termUTF8->bytes[termUTF8->offset + byteIDX] & 0xff;
      }
      builder->addTransition(state, nextState, c);
      state = nextState;
    }

    maxOffset = max(maxOffset, offsetAtt->endOffset());
  }

  in_->end();

  int endPosInc = posIncAtt->getPositionIncrement();
  if (endPosInc == 0 && finalOffsetGapAsHole &&
      offsetAtt->endOffset() > maxOffset) {
    endPosInc = 1;
  } else if (endPosInc > 0 && preservePositionIncrements == false) {
    endPosInc = 0;
  }

  int endState;
  if (endPosInc > 0) {
    // there were hole(s) after the last token
    endState = builder->createState();

    // add trailing holes now:
    int lastState = endState;
    while (true) {
      int state1 = builder->createState();
      builder->addTransition(lastState, state1, HOLE);
      endPosInc--;
      if (endPosInc == 0) {
        builder->setAccept(state1, true);
        break;
      }
      int state2 = builder->createState();
      builder->addTransition(state1, state2, POS_SEP);
      lastState = state2;
    }
  } else {
    endState = -1;
  }

  pos++;
  while (pos <= positions->getMaxPos()) {
    posData = positions->get(pos);
    if (posData->arriving != -1) {
      if (endState != -1) {
        builder->addTransition(posData->arriving, endState, POS_SEP);
      } else {
        builder->setAccept(posData->arriving, true);
      }
    }
    pos++;
  }

  return builder->finish();
}

void TokenStreamToAutomaton::addHoles(
    shared_ptr<Automaton::Builder> builder,
    shared_ptr<RollingBuffer<std::shared_ptr<Position>>> positions, int pos)
{
  shared_ptr<Position> posData = positions->get(pos);
  shared_ptr<Position> prevPosData = positions->get(pos - 1);

  while (posData->arriving == -1 || prevPosData->leaving == -1) {
    if (posData->arriving == -1) {
      posData->arriving = builder->createState();
      builder->addTransition(posData->arriving, posData->leaving, POS_SEP);
    }
    if (prevPosData->leaving == -1) {
      if (pos == 1) {
        prevPosData->leaving = 0;
      } else {
        prevPosData->leaving = builder->createState();
      }
      if (prevPosData->arriving != -1) {
        builder->addTransition(prevPosData->arriving, prevPosData->leaving,
                               POS_SEP);
      }
    }
    builder->addTransition(prevPosData->leaving, posData->arriving, HOLE);
    pos--;
    if (pos <= 0) {
      break;
    }
    posData = prevPosData;
    prevPosData = positions->get(pos - 1);
  }
}
} // namespace org::apache::lucene::analysis