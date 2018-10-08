using namespace std;

#include "HindiNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "HindiNormalizer.h"

namespace org::apache::lucene::analysis::hi
{
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

HindiNormalizationFilter::HindiNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool HindiNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAtt->isKeyword()) {
      termAtt->setLength(
          normalizer->normalize(termAtt->buffer(), termAtt->length()));
    }
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::hi