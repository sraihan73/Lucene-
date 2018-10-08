using namespace std;

#include "TrimFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

TrimFilter::TrimFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TrimFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  std::deque<wchar_t> termBuffer = termAtt->buffer();
  int len = termAtt->length();
  // TODO: Is this the right behavior or should we return false?  Currently, "
  // ", returns true, so I think this should also return true
  if (len == 0) {
    return true;
  }
  int start = 0;
  int end = 0;

  // eat the first characters
  for (start = 0; start < len && isspace(termBuffer[start]); start++) {
  }
  // eat the end characters
  for (end = len; end >= start && isspace(termBuffer[end - 1]); end--) {
  }
  if (start > 0 || end < len) {
    if (start < end) {
      termAtt->copyBuffer(termBuffer, start, (end - start));
    } else {
      termAtt->setEmpty();
    }
  }

  return true;
}
} // namespace org::apache::lucene::analysis::miscellaneous