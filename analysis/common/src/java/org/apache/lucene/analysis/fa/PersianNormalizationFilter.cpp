using namespace std;

#include "PersianNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "PersianNormalizer.h"

namespace org::apache::lucene::analysis::fa
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

PersianNormalizationFilter::PersianNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool PersianNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    constexpr int newlen =
        normalizer->normalize(termAtt->buffer(), termAtt->length());
    termAtt->setLength(newlen);
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::fa