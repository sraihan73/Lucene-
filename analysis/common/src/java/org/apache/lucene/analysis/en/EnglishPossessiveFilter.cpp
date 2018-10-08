using namespace std;

#include "EnglishPossessiveFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::en
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

EnglishPossessiveFilter::EnglishPossessiveFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool EnglishPossessiveFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  const std::deque<wchar_t> buffer = termAtt->buffer();
  constexpr int bufferLength = termAtt->length();

  if (bufferLength >= 2 &&
      (buffer[bufferLength - 2] == L'\'' ||
       buffer[bufferLength - 2] == L'\u2019' ||
       buffer[bufferLength - 2] == L'\uFF07') &&
      (buffer[bufferLength - 1] == L's' || buffer[bufferLength - 1] == L'S')) {
    termAtt->setLength(bufferLength - 2); // Strip last 2 characters off
  }

  return true;
}
} // namespace org::apache::lucene::analysis::en