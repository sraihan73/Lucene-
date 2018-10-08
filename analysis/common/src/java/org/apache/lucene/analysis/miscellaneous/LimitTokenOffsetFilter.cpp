using namespace std;

#include "LimitTokenOffsetFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

LimitTokenOffsetFilter::LimitTokenOffsetFilter(shared_ptr<TokenStream> input,
                                               int maxStartOffset)
    : LimitTokenOffsetFilter(input, maxStartOffset, false)
{
}

LimitTokenOffsetFilter::LimitTokenOffsetFilter(shared_ptr<TokenStream> input,
                                               int maxStartOffset,
                                               bool consumeAllTokens)
    : org::apache::lucene::analysis::TokenFilter(input),
      consumeAllTokens(consumeAllTokens)
{
  if (maxStartOffset < 0) {
    throw invalid_argument(L"maxStartOffset must be >= zero");
  }
  this->maxStartOffset = maxStartOffset;
}

bool LimitTokenOffsetFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }
  if (offsetAttrib->startOffset() <= maxStartOffset) {
    return true;
  }
  if (consumeAllTokens) {
    while (input->incrementToken()) {
      // no-op
    }
  }
  return false;
}
} // namespace org::apache::lucene::analysis::miscellaneous