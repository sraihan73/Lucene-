using namespace std;

#include "GermanNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../util/StemmerUtil.h"

namespace org::apache::lucene::analysis::de
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using StemmerUtil = org::apache::lucene::analysis::util::StemmerUtil;

GermanNormalizationFilter::GermanNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool GermanNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    int state = N;
    std::deque<wchar_t> buffer = termAtt->buffer();
    int length = termAtt->length();
    for (int i = 0; i < length; i++) {
      constexpr wchar_t c = buffer[i];
      switch (c) {
      case L'a':
      case L'o':
        state = U;
        break;
      case L'u':
        state = (state == N) ? U : V;
        break;
      case L'e':
        if (state == U) {
          length = StemmerUtil::delete (buffer, i--, length);
        }
        state = V;
        break;
      case L'i':
      case L'q':
      case L'y':
        state = V;
        break;
      case L'ä':
        buffer[i] = L'a';
        state = V;
        break;
      case L'ö':
        buffer[i] = L'o';
        state = V;
        break;
      case L'ü':
        buffer[i] = L'u';
        state = V;
        break;
      case L'ß':
        buffer[i++] = L's';
        buffer = termAtt->resizeBuffer(1 + length);
        if (i < length) {
          System::arraycopy(buffer, i, buffer, i + 1, (length - i));
        }
        buffer[i] = L's';
        length++;
        state = N;
        break;
      default:
        state = N;
      }
    }
    termAtt->setLength(length);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::de