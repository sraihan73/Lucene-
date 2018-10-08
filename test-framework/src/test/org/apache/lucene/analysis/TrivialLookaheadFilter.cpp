using namespace std;

#include "TrivialLookaheadFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

TrivialLookaheadFilter::TrivialLookaheadFilter(shared_ptr<TokenStream> input)
    : LookaheadTokenFilter<TestPosition>(input)
{
}

shared_ptr<TestPosition> TrivialLookaheadFilter::newPosition()
{
  return make_shared<TestPosition>();
}

bool TrivialLookaheadFilter::incrementToken() 
{
  // At the outset, getMaxPos is -1. So we'll peek. When we reach the end of the
  // sentence and go to the first token of the next sentence, maxPos will be the
  // prev sentence's end token, and we'll go again.
  if (positions::getMaxPos() < outputPos) {
    peekSentence();
  }

  return nextToken();
}

void TrivialLookaheadFilter::reset() 
{
  LookaheadTokenFilter<TestPosition>::reset();
  insertUpto = -1;
}

void TrivialLookaheadFilter::afterPosition() 
{
  if (insertUpto < outputPos) {
    insertToken();
    // replace term with 'improved' term.
    clearAttributes();
    termAtt->setEmpty();
    posIncAtt->setPositionIncrement(0);
    termAtt->append(positions->get(outputPos).getFact());
    offsetAtt->setOffset(positions->get(outputPos).startOffset,
                         positions->get(outputPos + 1).endOffset);
    insertUpto = outputPos;
  }
}

void TrivialLookaheadFilter::peekSentence() 
{
  deque<wstring> facts = deque<wstring>();
  bool haveSentence = false;
  do {
    if (peekToken()) {

      wstring term = wstring(termAtt->buffer(), 0, termAtt->length());
      facts.push_back(term + L"-huh?");
      if (L"." == term) {
        haveSentence = true;
      }

    } else {
      haveSentence = true;
    }

  } while (!haveSentence);

  // attach the (now disambiguated) analyzed tokens to the positions.
  for (int x = 0; x < facts.size(); x++) {
    // sentenceTokens is just relative to sentence, positions is absolute.
    positions->get(outputPos + x).setFact(facts[x]);
  }
}
} // namespace org::apache::lucene::analysis