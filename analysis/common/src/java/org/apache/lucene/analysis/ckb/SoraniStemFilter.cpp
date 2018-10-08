using namespace std;

#include "SoraniStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "SoraniStemmer.h"

namespace org::apache::lucene::analysis::ckb
{
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

SoraniStemFilter::SoraniStemFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool SoraniStemFilter::incrementToken() 
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
} // namespace org::apache::lucene::analysis::ckb