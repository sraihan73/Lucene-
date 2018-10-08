using namespace std;

#include "NGramTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharacterUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::ngram
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

NGramTokenizer::NGramTokenizer(int minGram, int maxGram, bool edgesOnly)
{
  init(minGram, maxGram, edgesOnly);
}

NGramTokenizer::NGramTokenizer(int minGram, int maxGram)
    : NGramTokenizer(minGram, maxGram, false)
{
}

NGramTokenizer::NGramTokenizer(shared_ptr<AttributeFactory> factory,
                               int minGram, int maxGram, bool edgesOnly)
    : org::apache::lucene::analysis::Tokenizer(factory)
{
  init(minGram, maxGram, edgesOnly);
}

NGramTokenizer::NGramTokenizer(shared_ptr<AttributeFactory> factory,
                               int minGram, int maxGram)
    : NGramTokenizer(factory, minGram, maxGram, false)
{
}

NGramTokenizer::NGramTokenizer()
    : NGramTokenizer(DEFAULT_MIN_NGRAM_SIZE, DEFAULT_MAX_NGRAM_SIZE)
{
}

void NGramTokenizer::init(int minGram, int maxGram, bool edgesOnly)
{
  if (minGram < 1) {
    throw invalid_argument(L"minGram must be greater than zero");
  }
  if (minGram > maxGram) {
    throw invalid_argument(L"minGram must not be greater than maxGram");
  }
  this->minGram = minGram;
  this->maxGram = maxGram;
  this->edgesOnly = edgesOnly;
  charBuffer = CharacterUtils::newCharacterBuffer(
      2 * maxGram +
      1024); // 2 * maxGram in case all code points require 2 chars and + 1024
             // for buffering to not keep polling the Reader
  buffer = std::deque<int>(charBuffer->getBuffer().size());
  // Make the term att large enough
  termAtt->resizeBuffer(2 * maxGram);
}

bool NGramTokenizer::incrementToken() 
{
  clearAttributes();

  // termination of this loop is guaranteed by the fact that every iteration
  // either advances the buffer (calls consumes()) or increases gramSize
  while (true) {
    // compact
    if (bufferStart >= bufferEnd - maxGram - 1 && !exhausted) {
      System::arraycopy(buffer, bufferStart, buffer, 0,
                        bufferEnd - bufferStart);
      bufferEnd -= bufferStart;
      lastCheckedChar -= bufferStart;
      lastNonTokenChar -= bufferStart;
      bufferStart = 0;

      // fill in remaining space
      exhausted =
          !CharacterUtils::fill(charBuffer, input, buffer.size() - bufferEnd);
      // convert to code points
      bufferEnd += CharacterUtils::toCodePoints(charBuffer->getBuffer(), 0,
                                                charBuffer->getLength(), buffer,
                                                bufferEnd);
    }

    // should we go to the next offset?
    if (gramSize > maxGram || (bufferStart + gramSize) > bufferEnd) {
      if (bufferStart + 1 + minGram > bufferEnd) {
        assert(exhausted);
        return false;
      }
      consume();
      gramSize = minGram;
    }

    updateLastNonTokenChar();

    // retry if the token to be emitted was going to not only contain token
    // chars
    constexpr bool termContainsNonTokenChar =
        lastNonTokenChar >= bufferStart &&
        lastNonTokenChar < (bufferStart + gramSize);
    constexpr bool isEdgeAndPreviousCharIsTokenChar =
        edgesOnly && lastNonTokenChar != bufferStart - 1;
    if (termContainsNonTokenChar || isEdgeAndPreviousCharIsTokenChar) {
      consume();
      gramSize = minGram;
      continue;
    }

    constexpr int length = CharacterUtils::toChars(
        buffer, bufferStart, gramSize, termAtt->buffer(), 0);
    termAtt->setLength(length);
    posIncAtt->setPositionIncrement(1);
    posLenAtt->setPositionLength(1);
    offsetAtt->setOffset(correctOffset(offset), correctOffset(offset + length));
    ++gramSize;
    return true;
  }
}

void NGramTokenizer::updateLastNonTokenChar()
{
  constexpr int termEnd = bufferStart + gramSize - 1;
  if (termEnd > lastCheckedChar) {
    for (int i = termEnd; i > lastCheckedChar; --i) {
      if (!isTokenChar(buffer[i])) {
        lastNonTokenChar = i;
        break;
      }
    }
    lastCheckedChar = termEnd;
  }
}

void NGramTokenizer::consume()
{
  offset += Character::charCount(buffer[bufferStart++]);
}

bool NGramTokenizer::isTokenChar(int chr) { return true; }

void NGramTokenizer::end() 
{
  Tokenizer::end();
  assert(bufferStart <= bufferEnd);
  int endOffset = offset;
  for (int i = bufferStart; i < bufferEnd; ++i) {
    endOffset += Character::charCount(buffer[i]);
  }
  endOffset = correctOffset(endOffset);
  // set final offset
  offsetAtt->setOffset(endOffset, endOffset);
}

void NGramTokenizer::reset() 
{
  Tokenizer::reset();
  bufferStart = bufferEnd = buffer.size();
  lastNonTokenChar = lastCheckedChar = bufferStart - 1;
  offset = 0;
  gramSize = minGram;
  exhausted = false;
  charBuffer->reset();
}
} // namespace org::apache::lucene::analysis::ngram