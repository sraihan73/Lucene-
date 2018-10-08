using namespace std;

#include "IrishLowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::ga
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

IrishLowerCaseFilter::IrishLowerCaseFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool IrishLowerCaseFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> chArray = termAtt->buffer();
    int chLen = termAtt->length();
    int idx = 0;

    if (chLen > 1 && (chArray[0] == L'n' || chArray[0] == L't') &&
        isUpperVowel(chArray[1])) {
      chArray = termAtt->resizeBuffer(chLen + 1);
      for (int i = chLen; i > 1; i--) {
        chArray[i] = chArray[i - 1];
      }
      chArray[1] = L'-';
      termAtt->setLength(chLen + 1);
      idx = 2;
      chLen = chLen + 1;
    }

    for (int i = idx; i < chLen;) {
      i += Character::toChars(towlower(chArray[i]), chArray, i);
    }
    return true;
  } else {
    return false;
  }
}

bool IrishLowerCaseFilter::isUpperVowel(int v)
{
  switch (v) {
  case L'A':
  case L'E':
  case L'I':
  case L'O':
  case L'U':
  // vowels with acute accent (fada)
  case L'\u00c1':
  case L'\u00c9':
  case L'\u00cd':
  case L'\u00d3':
  case L'\u00da':
    return true;
  default:
    return false;
  }
}
} // namespace org::apache::lucene::analysis::ga