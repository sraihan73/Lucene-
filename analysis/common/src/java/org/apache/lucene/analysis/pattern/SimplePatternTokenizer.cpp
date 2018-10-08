using namespace std;

#include "SimplePatternTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"

namespace org::apache::lucene::analysis::pattern
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;

SimplePatternTokenizer::SimplePatternTokenizer(const wstring &regexp)
    : SimplePatternTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, regexp,
                             Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

SimplePatternTokenizer::SimplePatternTokenizer(shared_ptr<Automaton> dfa)
    : SimplePatternTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, dfa)
{
}

SimplePatternTokenizer::SimplePatternTokenizer(
    shared_ptr<AttributeFactory> factory, const wstring &regexp,
    int maxDeterminizedStates)
    : SimplePatternTokenizer(factory, new RegExp(regexp).toAutomaton())
{
}

SimplePatternTokenizer::SimplePatternTokenizer(
    shared_ptr<AttributeFactory> factory, shared_ptr<Automaton> dfa)
    : org::apache::lucene::analysis::Tokenizer(factory),
      runDFA(make_shared<CharacterRunAutomaton>(
          dfa, Operations::DEFAULT_MAX_DETERMINIZED_STATES))
{

  // we require user to do this up front because it is a possibly very costly
  // operation, and user may be creating us frequently, not realizing this ctor
  // is otherwise trappy
  if (dfa->isDeterministic() == false) {
    throw invalid_argument(L"please determinize the incoming automaton first");
  }
}

bool SimplePatternTokenizer::incrementToken() 
{

  clearAttributes();
  tokenUpto = 0;

  while (true) {

    int offsetStart = offset;

    // The runDFA operates in Unicode space, not UTF16 (java's char):

    int ch = nextCodePoint();
    if (ch == -1) {
      return false;
    }

    int state = runDFA->step(0, ch);

    if (state != -1) {
      // a token just possibly started; keep scanning to see if the token is
      // accepted:
      int lastAcceptLength = -1;
      do {

        if (runDFA->isAccept(state)) {
          // record that the token matches here, but keep scanning in case a
          // longer match also works (greedy):
          lastAcceptLength = tokenUpto;
        }

        ch = nextCodePoint();
        if (ch == -1) {
          break;
        }
        state = runDFA->step(state, ch);
      } while (state != -1);

      if (lastAcceptLength != -1) {
        // we found a token
        int extra = tokenUpto - lastAcceptLength;
        if (extra != 0) {
          pushBack(extra);
        }
        termAtt->setLength(lastAcceptLength);
        offsetAtt->setOffset(correctOffset(offsetStart),
                             correctOffset(offsetStart + lastAcceptLength));
        return true;
      } else if (ch == -1) {
        return false;
      } else {
        // false alarm: there was no token here; push back all but the first
        // character we scanned
        pushBack(tokenUpto - 1);
        tokenUpto = 0;
      }
    } else {
      tokenUpto = 0;
    }
  }
}

void SimplePatternTokenizer::end() 
{
  Tokenizer::end();
  constexpr int ofs = correctOffset(offset + pendingLimit - pendingUpto);
  offsetAtt->setOffset(ofs, ofs);
}

void SimplePatternTokenizer::reset() 
{
  Tokenizer::reset();
  offset = 0;
  pendingUpto = 0;
  pendingLimit = 0;
  tokenUpto = 0;
  bufferNextRead = 0;
  bufferLimit = 0;
}

void SimplePatternTokenizer::pushBack(int count)
{

  if (pendingLimit == 0) {
    if (bufferLimit != -1 && bufferNextRead >= count) {
      // optimize common case when the chars we are pushing back are still in
      // the buffer
      bufferNextRead -= count;
    } else {
      if (count > pendingChars.size()) {
        pendingChars = ArrayUtil::grow(pendingChars, count);
      }
      System::arraycopy(termAtt->buffer(), tokenUpto - count, pendingChars, 0,
                        count);
      pendingLimit = count;
    }
  } else {
    // we are pushing back what is already in our pending buffer
    pendingUpto -= count;
    assert(pendingUpto >= 0);
  }
  offset -= count;
}

void SimplePatternTokenizer::appendToToken(wchar_t ch)
{
  std::deque<wchar_t> buffer = termAtt->buffer();
  if (tokenUpto == buffer.size()) {
    buffer = termAtt->resizeBuffer(tokenUpto + 1);
  }
  buffer[tokenUpto++] = ch;
}

int SimplePatternTokenizer::nextCodeUnit() 
{
  int result;
  if (pendingUpto < pendingLimit) {
    result = pendingChars[pendingUpto++];
    if (pendingUpto == pendingLimit) {
      // We used up the pending buffer
      pendingUpto = 0;
      pendingLimit = 0;
    }
    appendToToken(static_cast<wchar_t>(result));
    offset++;
  } else if (bufferLimit == -1) {
    return -1;
  } else {
    assert((bufferNextRead <= bufferLimit,
            L"bufferNextRead=" + to_wstring(bufferNextRead) + L" bufferLimit=" +
                to_wstring(bufferLimit)));
    if (bufferNextRead == bufferLimit) {
      bufferLimit = input->read(buffer, 0, buffer.size());
      if (bufferLimit == -1) {
        return -1;
      }
      bufferNextRead = 0;
    }
    result = buffer[bufferNextRead++];
    offset++;
    appendToToken(static_cast<wchar_t>(result));
  }
  return result;
}

int SimplePatternTokenizer::nextCodePoint() 
{

  int ch = nextCodeUnit();
  if (ch == -1) {
    return ch;
  }
  if (Character::isHighSurrogate(static_cast<wchar_t>(ch))) {
    return Character::toCodePoint(static_cast<wchar_t>(ch),
                                  static_cast<wchar_t>(nextCodeUnit()));
  } else {
    return ch;
  }
}
} // namespace org::apache::lucene::analysis::pattern