using namespace std;

#include "PorterStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "PorterStemmer.h"

namespace org::apache::lucene::analysis::en
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

PorterStemFilter::PorterStemFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool PorterStemFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  if ((!keywordAttr->isKeyword()) &&
      stemmer->stem(termAtt->buffer(), 0, termAtt->length())) {
    termAtt->copyBuffer(stemmer->getResultBuffer(), 0,
                        stemmer->getResultLength());
  }
  return true;
}
} // namespace org::apache::lucene::analysis::en