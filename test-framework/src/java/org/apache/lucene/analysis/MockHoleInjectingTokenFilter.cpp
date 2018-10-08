using namespace std;

#include "MockHoleInjectingTokenFilter.h"

namespace org::apache::lucene::analysis
{
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;

MockHoleInjectingTokenFilter::MockHoleInjectingTokenFilter(
    shared_ptr<Random> random, shared_ptr<TokenStream> in_)
    : TokenFilter(in_), randomSeed(random->nextLong())
{
}

void MockHoleInjectingTokenFilter::reset() 
{
  TokenFilter::reset();
  random = make_shared<Random>(randomSeed);
  maxPos = -1;
  pos = -1;
}

bool MockHoleInjectingTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    constexpr int posInc = posIncAtt->getPositionIncrement();

    int nextPos = pos + posInc;

    // Carefully inject a hole only where it won't mess up
    // the graph:
    if (posInc > 0 && maxPos <= nextPos && random->nextInt(5) == 3) {
      constexpr int holeSize = TestUtil::nextInt(random, 1, 5);
      posIncAtt->setPositionIncrement(posInc + holeSize);
      nextPos += holeSize;
    }

    pos = nextPos;
    maxPos = max(maxPos, pos + posLenAtt->getPositionLength());

    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis