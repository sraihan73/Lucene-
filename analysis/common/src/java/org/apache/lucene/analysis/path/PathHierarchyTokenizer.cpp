using namespace std;

#include "PathHierarchyTokenizer.h"
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

PathHierarchyTokenizer::PathHierarchyTokenizer()
    : PathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, DEFAULT_DELIMITER,
                             DEFAULT_DELIMITER, DEFAULT_SKIP)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(int skip)
    : PathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, DEFAULT_DELIMITER,
                             DEFAULT_DELIMITER, skip)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(int bufferSize,
                                               wchar_t delimiter)
    : PathHierarchyTokenizer(bufferSize, delimiter, delimiter, DEFAULT_SKIP)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(wchar_t delimiter,
                                               wchar_t replacement)
    : PathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, delimiter, replacement,
                             DEFAULT_SKIP)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(wchar_t delimiter,
                                               wchar_t replacement, int skip)
    : PathHierarchyTokenizer(DEFAULT_BUFFER_SIZE, delimiter, replacement, skip)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(
    shared_ptr<AttributeFactory> factory, wchar_t delimiter,
    wchar_t replacement, int skip)
    : PathHierarchyTokenizer(factory, DEFAULT_BUFFER_SIZE, delimiter,
                             replacement, skip)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(int bufferSize,
                                               wchar_t delimiter,
                                               wchar_t replacement, int skip)
    : PathHierarchyTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, bufferSize,
                             delimiter, replacement, skip)
{
}

PathHierarchyTokenizer::PathHierarchyTokenizer(
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
}

bool PathHierarchyTokenizer::incrementToken() 
{
  clearAttributes();
  termAtt->append(resultToken);
  if (resultToken->length() == 0) {
    posAtt->setPositionIncrement(1);
  } else {
    posAtt->setPositionIncrement(0);
  }
  int length = 0;
  bool added = false;
  if (endDelimiter) {
    termAtt->append(replacement);
    length++;
    endDelimiter = false;
    added = true;
  }

  while (true) {
    int c = input->read();
    if (c >= 0) {
      charsRead++;
    } else {
      if (skipped > skip) {
        length += resultToken->length();
        termAtt->setLength(length);
        offsetAtt->setOffset(correctOffset(startPosition),
                             correctOffset(startPosition + length));
        if (added) {
          resultToken->setLength(0);
          resultToken->append(termAtt->buffer(), 0, length);
        }
        return added;
      } else {
        return false;
      }
    }
    if (!added) {
      added = true;
      skipped++;
      if (skipped > skip) {
        termAtt->append(c == delimiter ? replacement : static_cast<wchar_t>(c));
        length++;
      } else {
        startPosition++;
      }
    } else {
      if (c == delimiter) {
        if (skipped > skip) {
          endDelimiter = true;
          break;
        }
        skipped++;
        if (skipped > skip) {
          termAtt->append(replacement);
          length++;
        } else {
          startPosition++;
        }
      } else {
        if (skipped > skip) {
          termAtt->append(static_cast<wchar_t>(c));
          length++;
        } else {
          startPosition++;
        }
      }
    }
  }
  length += resultToken->length();
  termAtt->setLength(length);
  offsetAtt->setOffset(correctOffset(startPosition),
                       correctOffset(startPosition + length));
  resultToken->setLength(0);
  resultToken->append(termAtt->buffer(), 0, length);
  return true;
}

void PathHierarchyTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  int finalOffset = correctOffset(charsRead);
  offsetAtt->setOffset(finalOffset, finalOffset);
}

void PathHierarchyTokenizer::reset() 
{
  Tokenizer::reset();
  resultToken->setLength(0);
  charsRead = 0;
  endDelimiter = false;
  skipped = 0;
  startPosition = 0;
}
} // namespace org::apache::lucene::analysis::path