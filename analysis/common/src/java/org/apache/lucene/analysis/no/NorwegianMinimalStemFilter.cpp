using namespace std;

#include "NorwegianMinimalStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "NorwegianMinimalStemmer.h"

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

NorwegianMinimalStemFilter::NorwegianMinimalStemFilter(
    shared_ptr<TokenStream> input)
    : NorwegianMinimalStemFilter(input, NorwegianLightStemmer::BOKMAAL)
{
}

NorwegianMinimalStemFilter::NorwegianMinimalStemFilter(
    shared_ptr<TokenStream> input, int flags)
    : org::apache::lucene::analysis::TokenFilter(input),
      stemmer(make_shared<NorwegianMinimalStemmer>(flags))
{
}

bool NorwegianMinimalStemFilter::incrementToken() 
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