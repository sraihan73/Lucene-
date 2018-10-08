using namespace std;

#include "SuggestStopFilter.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

SuggestStopFilter::SuggestStopFilter(shared_ptr<TokenStream> input,
                                     shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::TokenFilter(input), stopWords(stopWords)
{
}

void SuggestStopFilter::reset() 
{
  TokenFilter::reset();
  endState.reset();
}

void SuggestStopFilter::end() 
{
  if (endState == nullptr) {
    TokenFilter::end();
  } else {
    // NOTE: we already called .end() from our .next() when
    // the stream was complete, so we do not call
    // super.end() here
    restoreState(endState);
  }
}

bool SuggestStopFilter::incrementToken() 
{
  if (endState != nullptr) {
    return false;
  }

  if (!input->incrementToken()) {
    return false;
  }

  int skippedPositions = 0;
  while (true) {
    if (stopWords->contains(termAtt->buffer(), 0, termAtt->length())) {
      int posInc = posIncAtt->getPositionIncrement();
      int endOffset = offsetAtt->endOffset();
      // This token may be a stopword, if it's not end:
      shared_ptr<State> sav = captureState();
      if (input->incrementToken()) {
        // It was a stopword; skip it
        skippedPositions += posInc;
      } else {
        clearAttributes();
        input->end();
        endState = captureState();
        int finalEndOffset = offsetAtt->endOffset();
        assert(finalEndOffset >= endOffset);
        if (finalEndOffset > endOffset) {
          // OK there was a token separator after the
          // stopword, so it was a stopword
          return false;
        } else {
          // No token separator after final token that
          // looked like a stop-word; don't filter it:
          restoreState(sav);
          posIncAtt->setPositionIncrement(skippedPositions +
                                          posIncAtt->getPositionIncrement());
          keywordAtt->setKeyword(true);
          return true;
        }
      }
    } else {
      // Not a stopword; return the current token:
      posIncAtt->setPositionIncrement(skippedPositions +
                                      posIncAtt->getPositionIncrement());
      return true;
    }
  }
}
} // namespace org::apache::lucene::search::suggest::analyzing