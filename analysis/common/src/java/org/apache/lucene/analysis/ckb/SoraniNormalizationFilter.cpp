using namespace std;

#include "SoraniNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "SoraniNormalizer.h"

namespace org::apache::lucene::analysis::ckb
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

SoraniNormalizationFilter::SoraniNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool SoraniNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    constexpr int newlen =
        normalizer->normalize(termAtt->buffer(), termAtt->length());
    termAtt->setLength(newlen);
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::ckb