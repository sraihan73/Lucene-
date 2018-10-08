using namespace std;

#include "IndonesianStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "IndonesianStemmer.h"

namespace org::apache::lucene::analysis::id
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

IndonesianStemFilter::IndonesianStemFilter(shared_ptr<TokenStream> input)
    : IndonesianStemFilter(input, true)
{
}

IndonesianStemFilter::IndonesianStemFilter(shared_ptr<TokenStream> input,
                                           bool stemDerivational)
    : org::apache::lucene::analysis::TokenFilter(input),
      stemDerivational(stemDerivational)
{
}

bool IndonesianStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAtt->isKeyword()) {
      constexpr int newlen =
          stemmer->stem(termAtt->buffer(), termAtt->length(), stemDerivational);
      termAtt->setLength(newlen);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::id