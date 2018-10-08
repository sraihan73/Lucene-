using namespace std;

#include "FilteringTokenFilter.h"
#include "TokenStream.h"
#include "tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis
{
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

FilteringTokenFilter::FilteringTokenFilter(shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

bool FilteringTokenFilter::incrementToken() 
{
  skippedPositions = 0;
  while (input->incrementToken()) {
    if (accept()) {
      if (skippedPositions != 0) {
        posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                         skippedPositions);
      }
      return true;
    }
    skippedPositions += posIncrAtt->getPositionIncrement();
  }

  // reached EOS -- return false
  return false;
}

void FilteringTokenFilter::reset() 
{
  TokenFilter::reset();
  skippedPositions = 0;
}

void FilteringTokenFilter::end() 
{
  TokenFilter::end();
  posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                   skippedPositions);
}
} // namespace org::apache::lucene::analysis