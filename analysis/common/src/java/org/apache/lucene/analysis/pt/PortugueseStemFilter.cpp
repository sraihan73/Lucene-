using namespace std;

#include "PortugueseStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "PortugueseStemmer.h"

namespace org::apache::lucene::analysis::pt
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

PortugueseStemFilter::PortugueseStemFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool PortugueseStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAttr->isKeyword()) {
      // this stemmer increases word length by 1: worst case '*ã' -> '*ão'
      constexpr int len = termAtt->length();
      constexpr int newlen = stemmer->stem(termAtt->resizeBuffer(len + 1), len);
      termAtt->setLength(newlen);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::pt