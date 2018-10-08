using namespace std;

#include "ProtectedTermFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

ProtectedTermFilter::ProtectedTermFilter(
    shared_ptr<CharArraySet> protectedTerms, shared_ptr<TokenStream> input,
    function<TokenStream *(TokenStream *)> &inputFactory)
    : ConditionalTokenFilter(input, inputFactory),
      protectedTerms(protectedTerms)
{
}

bool ProtectedTermFilter::shouldFilter()
{
  bool b = protectedTerms->contains(termAtt->buffer(), 0, termAtt->length());
  return b == false;
}
} // namespace org::apache::lucene::analysis::miscellaneous