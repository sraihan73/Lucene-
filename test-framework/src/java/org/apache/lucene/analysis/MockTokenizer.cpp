using namespace std;

#include "MockTokenizer.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using com::carrotsearch::randomizedtesting::RandomizedContext;
const shared_ptr<org::apache::lucene::util::automaton::CharacterRunAutomaton>
    MockTokenizer::WHITESPACE = make_shared<
        org::apache::lucene::util::automaton::CharacterRunAutomaton>(
        (make_shared<org::apache::lucene::util::automaton::RegExp>(
             L"[^ \t\r\n]+"))
            ->toAutomaton());
const shared_ptr<org::apache::lucene::util::automaton::CharacterRunAutomaton>
    MockTokenizer::KEYWORD = make_shared<
        org::apache::lucene::util::automaton::CharacterRunAutomaton>(
        (make_shared<org::apache::lucene::util::automaton::RegExp>(L".*"))
            ->toAutomaton());
const shared_ptr<org::apache::lucene::util::automaton::CharacterRunAutomaton>
    MockTokenizer::SIMPLE = make_shared<
        org::apache::lucene::util::automaton::CharacterRunAutomaton>(
        (make_shared<org::apache::lucene::util::automaton::RegExp>(
             L"[A-Za-zªµºÀ-ÖØ-öø-ˁ一-鿌]+"))
            ->toAutomaton());

MockTokenizer::MockTokenizer(shared_ptr<AttributeFactory> factory,
                             shared_ptr<CharacterRunAutomaton> runAutomaton,
                             bool lowerCase, int maxTokenLength)
    : Tokenizer(factory), runAutomaton(runAutomaton), lowerCase(lowerCase),
      maxTokenLength(maxTokenLength)
{
  this->state = 0;
}

MockTokenizer::MockTokenizer(shared_ptr<CharacterRunAutomaton> runAutomaton,
                             bool lowerCase, int maxTokenLength)
    : MockTokenizer(BaseTokenStreamTestCase::newAttributeFactory(),
                    runAutomaton, lowerCase, maxTokenLength)
{
}

MockTokenizer::MockTokenizer(shared_ptr<CharacterRunAutomaton> runAutomaton,
                             bool lowerCase)
    : MockTokenizer(runAutomaton, lowerCase, DEFAULT_MAX_TOKEN_LENGTH)
{
}

MockTokenizer::MockTokenizer() : MockTokenizer(WHITESPACE, true) {}

MockTokenizer::MockTokenizer(shared_ptr<AttributeFactory> factory,
                             shared_ptr<CharacterRunAutomaton> runAutomaton,
                             bool lowerCase)
    : MockTokenizer(factory, runAutomaton, lowerCase, DEFAULT_MAX_TOKEN_LENGTH)
{
}

MockTokenizer::MockTokenizer(shared_ptr<AttributeFactory> factory)
    : MockTokenizer(factory, WHITESPACE, true)
{
}

void MockTokenizer::fail(const wstring &message)
{
  if (enableChecks) {
    throw make_shared<IllegalStateException>(message);
  }
}

void MockTokenizer::failAlways(const wstring &message)
{
  throw make_shared<IllegalStateException>(message);
}

bool MockTokenizer::incrementToken() 
{
  if (streamState != State::RESET && streamState != State::INCREMENT) {
    fail(L"incrementToken() called while in wrong state: " + streamState);
  }

  clearAttributes();
  for (;;) {
    int startOffset;
    int cp;
    if (bufferedCodePoint >= 0) {
      cp = bufferedCodePoint;
      startOffset = bufferedOff;
      bufferedCodePoint = -1;
    } else {
      startOffset = off;
      cp = readCodePoint();
    }
    if (cp < 0) {
      break;
    } else if (isTokenChar(cp)) {
      int endOffset;
      do {
        std::deque<wchar_t> chars = Character::toChars(normalize(cp));
        for (int i = 0; i < chars.size(); i++) {
          termAtt->append(chars[i]);
        }
        endOffset = off;
        if (termAtt->length() >= maxTokenLength) {
          break;
        }
        cp = readCodePoint();
      } while (cp >= 0 && isTokenChar(cp));

      if (termAtt->length() < maxTokenLength) {
        // buffer up, in case the "rejected" char can start a new word of its
        // own
        bufferedCodePoint = cp;
        bufferedOff = endOffset;
      } else {
        // otherwise, it's because we hit term limit.
        bufferedCodePoint = -1;
      }
      int correctedStartOffset = correctOffset(startOffset);
      int correctedEndOffset = correctOffset(endOffset);
      if (correctedStartOffset < 0) {
        failAlways(L"invalid start offset: " +
                   to_wstring(correctedStartOffset) + L", before correction: " +
                   to_wstring(startOffset));
      }
      if (correctedEndOffset < 0) {
        failAlways(L"invalid end offset: " + to_wstring(correctedEndOffset) +
                   L", before correction: " + to_wstring(endOffset));
      }
      if (correctedStartOffset < lastOffset) {
        failAlways(L"start offset went backwards: " +
                   to_wstring(correctedStartOffset) + L", before correction: " +
                   to_wstring(startOffset) + L", lastOffset: " +
                   to_wstring(lastOffset));
      }
      lastOffset = correctedStartOffset;
      if (correctedEndOffset < correctedStartOffset) {
        failAlways(L"end offset: " + to_wstring(correctedEndOffset) +
                   L" is before start offset: " +
                   to_wstring(correctedStartOffset));
      }
      offsetAtt->setOffset(correctedStartOffset, correctedEndOffset);
      if (state == -1 || runAutomaton->isAccept(state)) {
        // either we hit a reject state (longest match), or end-of-text, but in
        // an accept state
        streamState = State::INCREMENT;
        return true;
      }
    }
  }
  streamState = State::INCREMENT_FALSE;
  return false;
}

int MockTokenizer::readCodePoint() 
{
  int ch = readChar();
  if (ch < 0) {
    return ch;
  } else {
    if (Character::isLowSurrogate(static_cast<wchar_t>(ch))) {
      failAlways(L"unpaired low surrogate: " + Integer::toHexString(ch));
    }
    off++;
    if (Character::isHighSurrogate(static_cast<wchar_t>(ch))) {
      int ch2 = readChar();
      if (ch2 >= 0) {
        off++;
        if (!Character::isLowSurrogate(static_cast<wchar_t>(ch2))) {
          failAlways(L"unpaired high surrogate: " + Integer::toHexString(ch) +
                     L", followed by: " + Integer::toHexString(ch2));
        }
        return Character::toCodePoint(static_cast<wchar_t>(ch),
                                      static_cast<wchar_t>(ch2));
      } else {
        failAlways(L"stream ends with unpaired high surrogate: " +
                   Integer::toHexString(ch));
      }
    }
    return ch;
  }
}

int MockTokenizer::readChar() 
{
  switch (random->nextInt(10)) {
  case 0: {
    // read(char[])
    std::deque<wchar_t> c(1);
    int ret = input->read(c);
    return ret < 0 ? ret : c[0];
  }
  case 1: {
    // read(char[], int, int)
    std::deque<wchar_t> c(2);
    int ret = input->read(c, 1, 1);
    return ret < 0 ? ret : c[1];
  }
  case 2: {
    // read(CharBuffer)
    std::deque<wchar_t> c(1);
    shared_ptr<CharBuffer> cb = CharBuffer::wrap(c);
    int ret = input->read(cb);
    return ret < 0 ? ret : c[0];
  }
  default:
    // read()
    return input->read();
  }
}

bool MockTokenizer::isTokenChar(int c)
{
  if (state < 0) {
    state = 0;
  }
  state = runAutomaton->step(state, c);
  if (state < 0) {
    return false;
  } else {
    return true;
  }
}

int MockTokenizer::normalize(int c) { return lowerCase ? towlower(c) : c; }

void MockTokenizer::reset() 
{
  try {
    Tokenizer::reset();
    state = 0;
    lastOffset = off = 0;
    bufferedCodePoint = -1;
    if (streamState == State::RESET) {
      fail(L"double reset()");
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    streamState = State::RESET;
  }
}

MockTokenizer::~MockTokenizer()
{
  try {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
    // in some exceptional cases (e.g. TestIndexWriterExceptions) a test can
    // prematurely close() these tests should disable this check, by default we
    // check the normal workflow.
    // TODO: investigate the CachingTokenFilter "double-close"... for now we
    // ignore this
    if (!(streamState == State::END || streamState == State::CLOSE)) {
      fail(L"close() called in wrong state: " + streamState);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    streamState = State::CLOSE;
  }
}

void MockTokenizer::setReaderTestPoint()
{
  try {
    if (streamState != State::CLOSE) {
      fail(L"setReader() called in wrong state: " + streamState);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    streamState = State::SETREADER;
  }
}

void MockTokenizer::end() 
{
  try {
    Tokenizer::end();
    int finalOffset = correctOffset(off);
    offsetAtt->setOffset(finalOffset, finalOffset);
    // some tokenizers, such as limiting tokenizers, call end() before
    // incrementToken() returns false. these tests should disable this check (in
    // general you should consume the entire stream)
    if (streamState != State::INCREMENT_FALSE) {
      fail(L"end() called in wrong state=" + streamState + L"!");
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    streamState = State::END;
  }
}

void MockTokenizer::setEnableChecks(bool enableChecks)
{
  this->enableChecks = enableChecks;
}
} // namespace org::apache::lucene::analysis