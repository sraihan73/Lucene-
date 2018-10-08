using namespace std;

#include "CommonGramsFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

namespace org::apache::lucene::analysis::commongrams
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
const wstring CommonGramsFilter::GRAM_TYPE = L"gram";

CommonGramsFilter::CommonGramsFilter(shared_ptr<TokenStream> input,
                                     shared_ptr<CharArraySet> commonWords)
    : org::apache::lucene::analysis::TokenFilter(input),
      commonWords(commonWords)
{
}

bool CommonGramsFilter::incrementToken() 
{
  // get the next piece of input
  if (savedState != nullptr) {
    restoreState(savedState);
    savedState.reset();
    saveTermBuffer();
    return true;
  } else if (!input->incrementToken()) {
    return false;
  }

  /* We build n-grams before and after stopwords.
   * When valid, the buffer always contains at least the separator.
   * If it's empty, there is nothing before this stopword.
   */
  if (lastWasCommon || (isCommon() && buffer->length() > 0)) {
    savedState = captureState();
    gramToken();
    return true;
  }

  saveTermBuffer();
  return true;
}

void CommonGramsFilter::reset() 
{
  TokenFilter::reset();
  lastWasCommon = false;
  savedState.reset();
  buffer->setLength(0);
}

bool CommonGramsFilter::isCommon()
{
  return commonWords != nullptr &&
         commonWords->contains(termAttribute->buffer(), 0,
                               termAttribute->length());
}

void CommonGramsFilter::saveTermBuffer()
{
  buffer->setLength(0);
  buffer->append(termAttribute->buffer(), 0, termAttribute->length());
  buffer->append(SEPARATOR);
  lastStartOffset = offsetAttribute->startOffset();
  lastWasCommon = isCommon();
}

void CommonGramsFilter::gramToken()
{
  buffer->append(termAttribute->buffer(), 0, termAttribute->length());
  int endOffset = offsetAttribute->endOffset();

  clearAttributes();

  int length = buffer->length();
  std::deque<wchar_t> termText = termAttribute->buffer();
  if (length > termText.size()) {
    termText = termAttribute->resizeBuffer(length);
  }

  buffer->getChars(0, length, termText, 0);
  termAttribute->setLength(length);
  posIncAttribute->setPositionIncrement(0);
  posLenAttribute->setPositionLength(2); // bigram
  offsetAttribute->setOffset(lastStartOffset, endOffset);
  typeAttribute->setType(GRAM_TYPE);
  buffer->setLength(0);
}
} // namespace org::apache::lucene::analysis::commongrams