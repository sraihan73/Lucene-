using namespace std;

#include "BengaliNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "BengaliNormalizer.h"

namespace org::apache::lucene::analysis::bn
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

BengaliNormalizationFilter::BengaliNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool BengaliNormalizationFilter::incrementToken() 
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
} // namespace org::apache::lucene::analysis::bn