using namespace std;

#include "TruncateTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

TruncateTokenFilter::TruncateTokenFilter(shared_ptr<TokenStream> input,
                                         int length)
    : org::apache::lucene::analysis::TokenFilter(input), length(length)
{
  if (length < 1) {
    throw invalid_argument(L"length parameter must be a positive number: " +
                           to_wstring(length));
  }
}

bool TruncateTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAttr->isKeyword() && termAttribute->length() > length) {
      termAttribute->setLength(length);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous