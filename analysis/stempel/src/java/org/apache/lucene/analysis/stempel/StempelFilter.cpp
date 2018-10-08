using namespace std;

#include "StempelFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "StempelStemmer.h"

namespace org::apache::lucene::analysis::stempel
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

StempelFilter::StempelFilter(shared_ptr<TokenStream> in_,
                             shared_ptr<StempelStemmer> stemmer)
    : StempelFilter(in_, stemmer, DEFAULT_MIN_LENGTH)
{
}

StempelFilter::StempelFilter(shared_ptr<TokenStream> in_,
                             shared_ptr<StempelStemmer> stemmer, int minLength)
    : org::apache::lucene::analysis::TokenFilter(in_), stemmer(stemmer),
      minLength(minLength)
{
}

bool StempelFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAtt->isKeyword() && termAtt->length() > minLength) {
      shared_ptr<StringBuilder> sb = stemmer->stem(termAtt);
      if (sb != nullptr) // if we can't stem it, return unchanged
      {
        termAtt->setEmpty()->append(sb);
      }
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::stempel