using namespace std;

#include "PatternKeywordMarkerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

PatternKeywordMarkerFilter::PatternKeywordMarkerFilter(
    shared_ptr<TokenStream> in_, shared_ptr<Pattern> pattern)
    : KeywordMarkerFilter(in_), matcher(pattern->matcher(L""))
{
}

bool PatternKeywordMarkerFilter::isKeyword()
{
  matcher->reset(termAtt);
  return matcher->matches();
}
} // namespace org::apache::lucene::analysis::miscellaneous