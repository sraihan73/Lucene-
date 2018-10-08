using namespace std;

#include "LimitTokenCountFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;

LimitTokenCountFilter::LimitTokenCountFilter(shared_ptr<TokenStream> in_,
                                             int maxTokenCount)
    : LimitTokenCountFilter(in_, maxTokenCount, false)
{
}

LimitTokenCountFilter::LimitTokenCountFilter(shared_ptr<TokenStream> in_,
                                             int maxTokenCount,
                                             bool consumeAllTokens)
    : org::apache::lucene::analysis::TokenFilter(in_),
      maxTokenCount(maxTokenCount), consumeAllTokens(consumeAllTokens)
{
  if (maxTokenCount < 1) {
    throw invalid_argument(L"maxTokenCount must be greater than zero");
  }
}

bool LimitTokenCountFilter::incrementToken() 
{
  if (exhausted) {
    return false;
  } else if (tokenCount < maxTokenCount) {
    if (input->incrementToken()) {
      tokenCount++;
      return true;
    } else {
      exhausted = true;
      return false;
    }
  } else {
    while (consumeAllTokens && input->incrementToken()) {
    }
    return false;
  }
}

void LimitTokenCountFilter::reset() 
{
  TokenFilter::reset();
  tokenCount = 0;
  exhausted = false;
}
} // namespace org::apache::lucene::analysis::miscellaneous