using namespace std;

#include "BengaliStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "BengaliStemmer.h"

namespace org::apache::lucene::analysis::bn
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

BengaliStemFilter::BengaliStemFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool BengaliStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAttribute->isKeyword()) {
      termAttribute->setLength(bengaliStemmer->stem(termAttribute->buffer(),
                                                    termAttribute->length()));
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::bn