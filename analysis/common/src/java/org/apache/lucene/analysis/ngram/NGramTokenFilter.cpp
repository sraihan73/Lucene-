using namespace std;

#include "NGramTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::ngram
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

NGramTokenFilter::NGramTokenFilter(shared_ptr<TokenStream> input, int minGram,
                                   int maxGram, bool preserveOriginal)
    : org::apache::lucene::analysis::TokenFilter(input), minGram(minGram),
      maxGram(maxGram), preserveOriginal(preserveOriginal)
{
  if (minGram < 1) {
    throw invalid_argument(L"minGram must be greater than zero");
  }
  if (minGram > maxGram) {
    throw invalid_argument(L"minGram must not be greater than maxGram");
  }
}

NGramTokenFilter::NGramTokenFilter(shared_ptr<TokenStream> input, int gramSize)
    : NGramTokenFilter(input, gramSize, gramSize, DEFAULT_PRESERVE_ORIGINAL)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public
// NGramTokenFilter(org.apache.lucene.analysis.TokenStream input, int minGram,
// int maxGram)
NGramTokenFilter::NGramTokenFilter(shared_ptr<TokenStream> input, int minGram,
                                   int maxGram)
    : NGramTokenFilter(input, minGram, maxGram, DEFAULT_PRESERVE_ORIGINAL)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public
// NGramTokenFilter(org.apache.lucene.analysis.TokenStream input)
NGramTokenFilter::NGramTokenFilter(shared_ptr<TokenStream> input)
    : NGramTokenFilter(input, DEFAULT_MIN_NGRAM_SIZE, DEFAULT_MAX_NGRAM_SIZE,
                       DEFAULT_PRESERVE_ORIGINAL)
{
}

bool NGramTokenFilter::incrementToken() 
{
  while (true) {
    if (curTermBuffer.empty()) {
      if (!input->incrementToken()) {
        return false;
      }
      state = captureState();

      curTermLength = termAtt->length();
      curTermCodePointCount =
          Character::codePointCount(termAtt, 0, termAtt->length());
      curPosIncr += posIncrAtt->getPositionIncrement();
      curPos = 0;

      if (preserveOriginal && curTermCodePointCount < minGram) {
        // Token is shorter than minGram, but we'd still like to keep it.
        posIncrAtt->setPositionIncrement(curPosIncr);
        curPosIncr = 0;
        return true;
      }

      curTermBuffer = termAtt->buffer().clone();
      curGramSize = minGram;
    }

    if (curGramSize > maxGram ||
        (curPos + curGramSize) > curTermCodePointCount) {
      ++curPos;
      curGramSize = minGram;
    }
    if ((curPos + curGramSize) <= curTermCodePointCount) {
      restoreState(state);
      constexpr int start = Character::offsetByCodePoints(
          curTermBuffer, 0, curTermLength, 0, curPos);
      constexpr int end = Character::offsetByCodePoints(
          curTermBuffer, 0, curTermLength, start, curGramSize);
      termAtt->copyBuffer(curTermBuffer, start, end - start);
      posIncrAtt->setPositionIncrement(curPosIncr);
      curPosIncr = 0;
      curGramSize++;
      return true;
    } else if (preserveOriginal && curTermCodePointCount > maxGram) {
      // Token is longer than maxGram, but we'd still like to keep it.
      restoreState(state);
      posIncrAtt->setPositionIncrement(0);
      termAtt->copyBuffer(curTermBuffer, 0, curTermLength);
      curTermBuffer.clear();
      return true;
    }

    // Done with this input token, get next token on next iteration.
    curTermBuffer.clear();
  }
}

void NGramTokenFilter::reset() 
{
  TokenFilter::reset();
  curTermBuffer.clear();
  curPosIncr = 0;
}

void NGramTokenFilter::end() 
{
  TokenFilter::end();
  posIncrAtt->setPositionIncrement(curPosIncr);
}
} // namespace org::apache::lucene::analysis::ngram