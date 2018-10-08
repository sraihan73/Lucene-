using namespace std;

#include "SwedishLightStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "SwedishLightStemmer.h"

namespace org::apache::lucene::analysis::sv
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

SwedishLightStemFilter::SwedishLightStemFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool SwedishLightStemFilter::incrementToken() 
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
} // namespace org::apache::lucene::analysis::sv