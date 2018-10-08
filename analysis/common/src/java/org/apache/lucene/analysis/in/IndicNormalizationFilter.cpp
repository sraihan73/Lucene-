using namespace std;

#include "IndicNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "IndicNormalizer.h"

namespace org::apache::lucene::analysis::in_
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

IndicNormalizationFilter::IndicNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool IndicNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    termAtt->setLength(
        normalizer->normalize(termAtt->buffer(), termAtt->length()));
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::in_