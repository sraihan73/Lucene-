using namespace std;

#include "ReversePathHierarchyTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::path
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer()
    : ReversePathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, DEFAULT_DELIMITER,
                                    DEFAULT_DELIMITER, DEFAULT_SKIP)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(int skip)
    : ReversePathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, DEFAULT_DELIMITER,
                                    DEFAULT_DELIMITER, skip)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(int bufferSize,
                                                             wchar_t delimiter)
    : ReversePathHierarchyTokenizer(bufferSize, delimiter, delimiter,
                                    DEFAULT_SKIP)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(
    wchar_t delimiter, wchar_t replacement)
    : ReversePathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, delimiter, replacement,
                                    DEFAULT_SKIP)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(
    int bufferSize, wchar_t delimiter, wchar_t replacement)
    : ReversePathHierarchyTokenizer(bufferSize, delimiter, replacement,
                                    DEFAULT_SKIP)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(wchar_t delimiter,
                                                             int skip)
    : ReversePathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, delimiter, delimiter,
                                    skip)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(
    wchar_t delimiter, wchar_t replacement, int skip)
    : ReversePathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, delimiter, replacement,
                                    skip)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(
    shared_ptr<AttributeFactory> factory, wchar_t delimiter,
    wchar_t replacement, int skip)
    : ReversePathHierarchyTokenizer(factory, DEFAULT_BUFFER_SIZE, delimiter,
                                    replacement, skip)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(
    int bufferSize, wchar_t delimiter, wchar_t replacement, int skip)
    : ReversePathHierarchyTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, bufferSize,
                                    delimiter, replacement, skip)
{
}

ReversePathHierarchyTokenizer::ReversePathHierarchyTokenizer(
    shared_ptr<AttributeFactory> factory, int bufferSize, wchar_t delimiter,
    wchar_t replacement, int skip)
    : org::apache::lucene::analysis::Tokenizer(factory), delimiter(delimiter),
      replacement(replacement), skip(skip)
{
  if (bufferSize < 0) {
    throw invalid_argument(L"bufferSize cannot be negative");
  }
  if (skip < 0) {
    throw invalid_argument(L"skip cannot be negative");
  }
  termAtt->resizeBuffer(bufferSize);
  resultToken = make_shared<StringBuilder>(bufferSize);
  resultTokenBuffer = std::deque<wchar_t>(bufferSize);
  delimiterPositions = deque<>(bufferSize / 10);
}

bool ReversePathHierarchyTokenizer::incrementToken() 
{
  clearAttributes();
  if (delimitersCount == -1) {
    int length = 0;
    delimiterPositions.push_back(0);
    while (true) {
      int c = input->read();
      if (c < 0) {
        break;
      }
      length++;
      if (c == delimiter) {
        delimiterPositions.push_back(length);
        resultToken->append(replacement);
      } else {
        resultToken->append(static_cast<wchar_t>(c));
      }
    }
    delimitersCount = delimiterPositions.size();
    if (delimiterPositions[delimitersCount - 1] < length) {
      delimiterPositions.push_back(length);
      delimitersCount++;
    }
    if (resultTokenBuffer.size() < resultToken->length()) {
      resultTokenBuffer = std::deque<wchar_t>(resultToken->length());
    }
    resultToken->getChars(0, resultToken->length(), resultTokenBuffer, 0);
    resultToken->setLength(0);
    int idx = delimitersCount - 1 - skip;
    if (idx >= 0) {
      // otherwise it's ok, because we will skip and return false
      endPosition = delimiterPositions[idx];
    }
    finalOffset = correctOffset(length);
    posAtt->setPositionIncrement(1);
  } else {
    posAtt->setPositionIncrement(0);
  }

  while (skipped < delimitersCount - skip - 1) {
    int start = delimiterPositions[skipped];
    termAtt->copyBuffer(resultTokenBuffer, start, endPosition - start);
    offsetAtt->setOffset(correctOffset(start), correctOffset(endPosition));
    skipped++;
    return true;
  }

  return false;
}

void ReversePathHierarchyTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  offsetAtt->setOffset(finalOffset, finalOffset);
}

void ReversePathHierarchyTokenizer::reset() 
{
  Tokenizer::reset();
  resultToken->setLength(0);
  finalOffset = 0;
  endPosition = 0;
  skipped = 0;
  delimitersCount = -1;
  delimiterPositions.clear();
}
} // namespace org::apache::lucene::analysis::path