using namespace std;

#include "ArabicNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "ArabicNormalizer.h"

namespace org::apache::lucene::analysis::ar
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

ArabicNormalizationFilter::ArabicNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool ArabicNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    int newlen = normalizer->normalize(termAtt->buffer(), termAtt->length());
    termAtt->setLength(newlen);
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::ar