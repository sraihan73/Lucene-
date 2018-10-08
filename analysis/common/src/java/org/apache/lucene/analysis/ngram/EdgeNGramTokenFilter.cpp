using namespace std;

#include "EdgeNGramTokenFilter.h"
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

EdgeNGramTokenFilter::EdgeNGramTokenFilter(shared_ptr<TokenStream> input,
                                           int minGram, int maxGram,
                                           bool preserveOriginal)
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

EdgeNGramTokenFilter::EdgeNGramTokenFilter(shared_ptr<TokenStream> input,
                                           int gramSize)
    : EdgeNGramTokenFilter(input, gramSize, gramSize, DEFAULT_PRESERVE_ORIGINAL)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public
// EdgeNGramTokenFilter(org.apache.lucene.analysis.TokenStream input, int
// minGram, int maxGram)
EdgeNGramTokenFilter::EdgeNGramTokenFilter(shared_ptr<TokenStream> input,
                                           int minGram, int maxGram)
    : EdgeNGramTokenFilter(input, minGram, maxGram, DEFAULT_PRESERVE_ORIGINAL)
{
}

bool EdgeNGramTokenFilter::incrementToken() 
{
  while (true) {
    if (curTermBuffer.empty()) {
      if (!input->incrementToken()) {
        return false;
      }
      state = captureState();

      curTermLength = termAtt->length();
      curTermCodePointCount =
          Character::codePointCount(termAtt, 0, curTermLength);
      curPosIncr += posIncrAtt->getPositionIncrement();

      if (preserveOriginal && curTermCodePointCount < minGram) {
        // Token is shorter than minGram, but we'd still like to keep it.
        posIncrAtt->setPositionIncrement(curPosIncr);
        curPosIncr = 0;
        return true;
      }

      curTermBuffer = termAtt->buffer().clone();
      curGramSize = minGram;
    }

    if (curGramSize <= curTermCodePointCount) {
      if (curGramSize <=
          maxGram) { // curGramSize is between minGram and maxGram
        restoreState(state);
        // first ngram gets increment, others don't
        posIncrAtt->setPositionIncrement(curPosIncr);
        curPosIncr = 0;

        constexpr int charLength = Character::offsetByCodePoints(
            curTermBuffer, 0, curTermLength, 0, curGramSize);
        termAtt->copyBuffer(curTermBuffer, 0, charLength);
        curGramSize++;
        return true;
      } else if (preserveOriginal) {
        // Token is longer than maxGram, but we'd still like to keep it.
        restoreState(state);
        posIncrAtt->setPositionIncrement(0);
        termAtt->copyBuffer(curTermBuffer, 0, curTermLength);
        curTermBuffer.clear();
        return true;
      }
    }
    // Done with this input token, get next token on the next iteration.
    curTermBuffer.clear();
  }
}

void EdgeNGramTokenFilter::reset() 
{
  TokenFilter::reset();
  curTermBuffer.clear();
  curPosIncr = 0;
}

void EdgeNGramTokenFilter::end() 
{
  TokenFilter::end();
  posIncrAtt->setPositionIncrement(curPosIncr);
}
} // namespace org::apache::lucene::analysis::ngram