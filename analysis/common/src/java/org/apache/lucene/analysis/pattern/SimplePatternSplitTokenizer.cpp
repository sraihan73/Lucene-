using namespace std;

#include "SimplePatternSplitTokenizer.h"
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

SimplePatternSplitTokenizer::SimplePatternSplitTokenizer(const wstring &regexp)
    : SimplePatternSplitTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, regexp,
                                  Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

SimplePatternSplitTokenizer::SimplePatternSplitTokenizer(
    shared_ptr<Automaton> dfa)
    : SimplePatternSplitTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, dfa)
{
}

SimplePatternSplitTokenizer::SimplePatternSplitTokenizer(
    shared_ptr<AttributeFactory> factory, const wstring &regexp,
    int maxDeterminizedStates)
    : SimplePatternSplitTokenizer(factory, new RegExp(regexp).toAutomaton())
{
}

SimplePatternSplitTokenizer::SimplePatternSplitTokenizer(
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

void SimplePatternSplitTokenizer::fillToken(int offsetStart)
{
  termAtt->setLength(tokenUpto);
  offsetAtt->setOffset(correctOffset(offsetStart),
                       correctOffset(offsetStart + tokenUpto));
}

bool SimplePatternSplitTokenizer::incrementToken() 
{

  int offsetStart = offset;

  clearAttributes();

  tokenUpto = 0;

  while (true) {
    sepUpto = 0;

    // The runDFA operates in Unicode space, not UTF16 (java's char):
    int ch = nextCodePoint();
    if (ch == -1) {
      if (tokenUpto > 0) {
        fillToken(offsetStart);
        return true;
      } else {
        return false;
      }
    }
    int state = runDFA->step(0, ch);

    if (state != -1) {
      // a token separator just possibly started; keep scanning to see if the
      // token is accepted:
      int lastAcceptLength = -1;
      do {

        if (runDFA->isAccept(state)) {
          // record that the token separator matches here, but keep scanning in
          // case a longer match also works (greedy):
          lastAcceptLength = sepUpto;
        }

        ch = nextCodePoint();
        if (ch == -1) {
          break;
        }
        state = runDFA->step(state, ch);
      } while (state != -1);

      if (lastAcceptLength != -1) {
        // we found a token separator; strip the trailing separator we just
        // matched from the token:
        int extra = sepUpto - lastAcceptLength;
        if (extra != 0) {
          pushBack(extra);
        }
        tokenUpto -= lastAcceptLength;
        if (tokenUpto > 0) {
          fillToken(offsetStart);
          return true;
        } else {
          // we matched one token separator immediately after another
          offsetStart = offset;
        }
      } else if (ch == -1) {
        if (tokenUpto > 0) {
          fillToken(offsetStart);
          return true;
        } else {
          return false;
        }
      } else {
        // false alarm: there was no token separator here; push back all but the
        // first character we scanned
        pushBack(sepUpto - 1);
      }
    }
  }
}

void SimplePatternSplitTokenizer::end() 
{
  Tokenizer::end();
  constexpr int ofs = correctOffset(offset + pendingLimit - pendingUpto);
  offsetAtt->setOffset(ofs, ofs);
}

void SimplePatternSplitTokenizer::reset() 
{
  Tokenizer::reset();
  offset = 0;
  pendingUpto = 0;
  pendingLimit = 0;
  sepUpto = 0;
  bufferNextRead = 0;
  bufferLimit = 0;
}

void SimplePatternSplitTokenizer::pushBack(int count)
{
  tokenUpto -= count;
  assert(tokenUpto >= 0);
  if (pendingLimit == 0) {
    if (bufferLimit != -1 && bufferNextRead >= count) {
      // optimize common case when the chars we are pushing back are still in
      // the buffer
      bufferNextRead -= count;
    } else {
      if (count > pendingChars.size()) {
        pendingChars = ArrayUtil::grow(pendingChars, count);
      }
      System::arraycopy(termAtt->buffer(), tokenUpto, pendingChars, 0, count);
      pendingLimit = count;
    }
  } else {
    // we are pushing back what is already in our pending buffer
    pendingUpto -= count;
    assert(pendingUpto >= 0);
  }
  offset -= count;
}

void SimplePatternSplitTokenizer::appendToToken(wchar_t ch)
{
  std::deque<wchar_t> buffer = termAtt->buffer();
  if (tokenUpto == buffer.size()) {
    buffer = termAtt->resizeBuffer(tokenUpto + 1);
  }
  buffer[tokenUpto++] = ch;
  sepUpto++;
}

int SimplePatternSplitTokenizer::nextCodeUnit() 
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

int SimplePatternSplitTokenizer::nextCodePoint() 
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