using namespace std;

#include "KStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "KStemmer.h"

namespace org::apache::lucene::analysis::en
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

KStemFilter::KStemFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool KStemFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  std::deque<wchar_t> term = termAttribute->buffer();
  int len = termAttribute->length();
  if ((!keywordAtt->isKeyword()) && stemmer->stem(term, len)) {
    termAttribute->setEmpty()->append(stemmer->asCharSequence());
  }

  return true;
}
} // namespace org::apache::lucene::analysis::en