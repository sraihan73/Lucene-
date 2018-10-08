using namespace std;

#include "HindiStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "HindiStemmer.h"

namespace org::apache::lucene::analysis::hi
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

HindiStemFilter::HindiStemFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool HindiStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAtt->isKeyword()) {
      termAtt->setLength(stemmer->stem(termAtt->buffer(), termAtt->length()));
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::hi