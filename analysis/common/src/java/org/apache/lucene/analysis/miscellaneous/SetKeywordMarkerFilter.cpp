using namespace std;

#include "SetKeywordMarkerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

SetKeywordMarkerFilter::SetKeywordMarkerFilter(
    shared_ptr<TokenStream> in_, shared_ptr<CharArraySet> keywordSet)
    : KeywordMarkerFilter(in_), keywordSet(keywordSet)
{
}

bool SetKeywordMarkerFilter::isKeyword()
{
  return keywordSet->contains(termAtt->buffer(), 0, termAtt->length());
}
} // namespace org::apache::lucene::analysis::miscellaneous