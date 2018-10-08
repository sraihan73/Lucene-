using namespace std;

#include "ApostropheFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::tr
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

ApostropheFilter::ApostropheFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool ApostropheFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  const std::deque<wchar_t> buffer = termAtt->buffer();
  constexpr int length = termAtt->length();

  for (int i = 0; i < length; i++) {
    if (buffer[i] == L'\'' || buffer[i] == L'\u2019') {
      termAtt->setLength(i);
      return true;
    }
  }
  return true;
}
} // namespace org::apache::lucene::analysis::tr