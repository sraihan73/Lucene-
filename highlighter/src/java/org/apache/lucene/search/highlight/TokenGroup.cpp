using namespace std;

#include "TokenGroup.h"

namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

TokenGroup::TokenGroup(shared_ptr<TokenStream> tokenStream)
{
  offsetAtt = tokenStream->addAttribute(OffsetAttribute::typeid);
  termAtt = tokenStream->addAttribute(CharTermAttribute::typeid);
}

void TokenGroup::addToken(float score)
{
  if (numTokens < MAX_NUM_TOKENS_PER_GROUP) {
    constexpr int termStartOffset = offsetAtt->startOffset();
    constexpr int termEndOffset = offsetAtt->endOffset();
    if (numTokens == 0) {
      startOffset = matchStartOffset = termStartOffset;
      endOffset = matchEndOffset = termEndOffset;
      tot += score;
    } else {
      startOffset = min(startOffset, termStartOffset);
      endOffset = max(endOffset, termEndOffset);
      if (score > 0) {
        if (tot == 0) {
          matchStartOffset = termStartOffset;
          matchEndOffset = termEndOffset;
        } else {
          matchStartOffset = min(matchStartOffset, termStartOffset);
          matchEndOffset = max(matchEndOffset, termEndOffset);
        }
        tot += score;
      }
    }

    scores[numTokens] = score;
    numTokens++;
  }
}

bool TokenGroup::isDistinct() { return offsetAtt->startOffset() >= endOffset; }

void TokenGroup::clear()
{
  numTokens = 0;
  tot = 0;
}

float TokenGroup::getScore(int index) { return scores[index]; }

int TokenGroup::getStartOffset() { return matchStartOffset; }

int TokenGroup::getEndOffset() { return matchEndOffset; }

int TokenGroup::getNumTokens() { return numTokens; }

float TokenGroup::getTotalScore() { return tot; }
} // namespace org::apache::lucene::search::highlight