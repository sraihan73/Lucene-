using namespace std;

#include "LimitTokenPositionFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

LimitTokenPositionFilter::LimitTokenPositionFilter(shared_ptr<TokenStream> in_,
                                                   int maxTokenPosition)
    : LimitTokenPositionFilter(in_, maxTokenPosition, false)
{
}

LimitTokenPositionFilter::LimitTokenPositionFilter(shared_ptr<TokenStream> in_,
                                                   int maxTokenPosition,
                                                   bool consumeAllTokens)
    : org::apache::lucene::analysis::TokenFilter(in_),
      maxTokenPosition(maxTokenPosition), consumeAllTokens(consumeAllTokens)
{
  if (maxTokenPosition < 1) {
    throw invalid_argument(L"maxTokenPosition must be greater than zero");
  }
}

bool LimitTokenPositionFilter::incrementToken() 
{
  if (exhausted) {
    return false;
  }
  if (input->incrementToken()) {
    tokenPosition += posIncAtt->getPositionIncrement();
    if (tokenPosition <= maxTokenPosition) {
      return true;
    } else {
      while (consumeAllTokens && input->incrementToken()) {
      }
      exhausted = true;
      return false;
    }
  } else {
    exhausted = true;
    return false;
  }
}

void LimitTokenPositionFilter::reset() 
{
  TokenFilter::reset();
  tokenPosition = 0;
  exhausted = false;
}
} // namespace org::apache::lucene::analysis::miscellaneous