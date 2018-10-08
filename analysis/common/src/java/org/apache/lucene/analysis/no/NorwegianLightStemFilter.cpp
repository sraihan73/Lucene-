using namespace std;

#include "NorwegianLightStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "NorwegianLightStemmer.h"

namespace org::apache::lucene::analysis::no
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

NorwegianLightStemFilter::NorwegianLightStemFilter(
    shared_ptr<TokenStream> input)
    : NorwegianLightStemFilter(input, NorwegianLightStemmer::BOKMAAL)
{
}

NorwegianLightStemFilter::NorwegianLightStemFilter(
    shared_ptr<TokenStream> input, int flags)
    : org::apache::lucene::analysis::TokenFilter(input),
      stemmer(make_shared<NorwegianLightStemmer>(flags))
{
}

bool NorwegianLightStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAttr->isKeyword()) {
      constexpr int newlen =
          stemmer->stem(termAtt->buffer(), termAtt->length());
      termAtt->setLength(newlen);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::no