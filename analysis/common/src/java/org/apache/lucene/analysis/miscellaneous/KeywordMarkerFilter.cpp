using namespace std;

#include "KeywordMarkerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

KeywordMarkerFilter::KeywordMarkerFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool KeywordMarkerFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (isKeyword()) {
      keywordAttr->setKeyword(true);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous