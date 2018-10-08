using namespace std;

#include "ClassicFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

namespace org::apache::lucene::analysis::standard
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

ClassicFilter::ClassicFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

const wstring ClassicFilter::APOSTROPHE_TYPE =
    ClassicTokenizer::TOKEN_TYPES[ClassicTokenizer::APOSTROPHE];
const wstring ClassicFilter::ACRONYM_TYPE =
    ClassicTokenizer::TOKEN_TYPES[ClassicTokenizer::ACRONYM];

bool ClassicFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  const std::deque<wchar_t> buffer = termAtt->buffer();
  constexpr int bufferLength = termAtt->length();
  const wstring type = typeAtt->type();

  if (type == APOSTROPHE_TYPE && bufferLength >= 2 &&
      buffer[bufferLength - 2] == L'\'' &&
      (buffer[bufferLength - 1] == L's' || buffer[bufferLength - 1] == L'S')) {
    // Strip last 2 characters off
    termAtt->setLength(bufferLength - 2);
  } else if (type == ACRONYM_TYPE) { // remove dots
    int upto = 0;
    for (int i = 0; i < bufferLength; i++) {
      wchar_t c = buffer[i];
      if (c != L'.') {
        buffer[upto++] = c;
      }
    }
    termAtt->setLength(upto);
  }

  return true;
}
} // namespace org::apache::lucene::analysis::standard