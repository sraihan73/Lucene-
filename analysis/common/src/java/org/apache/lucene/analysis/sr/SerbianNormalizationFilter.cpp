using namespace std;

#include "SerbianNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::sr
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

SerbianNormalizationFilter::SerbianNormalizationFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool SerbianNormalizationFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> buffer = termAtt->buffer();
    int length = termAtt->length();
    for (int i = 0; i < length; i++) {
      constexpr wchar_t c = buffer[i];
      switch (c) {
      case L'а':
        buffer[i] = L'a';
        break;
      case L'б':
        buffer[i] = L'b';
        break;
      case L'в':
        buffer[i] = L'v';
        break;
      case L'г':
        buffer[i] = L'g';
        break;
      case L'д':
        buffer[i] = L'd';
        break;
      case L'ђ':
      case L'đ':
        buffer = termAtt->resizeBuffer(1 + length);
        if (i < length) {
          System::arraycopy(buffer, i, buffer, i + 1, (length - i));
        }
        buffer[i] = L'd';
        buffer[++i] = L'j';
        length++;
        break;
      case L'е':
        buffer[i] = L'e';
        break;
      case L'ж':
      case L'з':
      case L'ž':
        buffer[i] = L'z';
        break;
      case L'и':
        buffer[i] = L'i';
        break;
      case L'ј':
        buffer[i] = L'j';
        break;
      case L'к':
        buffer[i] = L'k';
        break;
      case L'л':
        buffer[i] = L'l';
        break;
      case L'љ':
        buffer = termAtt->resizeBuffer(1 + length);
        if (i < length) {
          System::arraycopy(buffer, i, buffer, i + 1, (length - i));
        }
        buffer[i] = L'l';
        buffer[++i] = L'j';
        length++;
        break;
      case L'м':
        buffer[i] = L'm';
        break;
      case L'н':
        buffer[i] = L'n';
        break;
      case L'њ':
        buffer = termAtt->resizeBuffer(1 + length);
        if (i < length) {
          System::arraycopy(buffer, i, buffer, i + 1, (length - i));
        }
        buffer[i] = L'n';
        buffer[++i] = L'j';
        length++;
        break;
      case L'о':
        buffer[i] = L'o';
        break;
      case L'п':
        buffer[i] = L'p';
        break;
      case L'р':
        buffer[i] = L'r';
        break;
      case L'с':
        buffer[i] = L's';
        break;
      case L'т':
        buffer[i] = L't';
        break;
      case L'ћ':
      case L'ц':
      case L'ч':
      case L'č':
      case L'ć':
        buffer[i] = L'c';
        break;
      case L'у':
        buffer[i] = L'u';
        break;
      case L'ф':
        buffer[i] = L'f';
        break;
      case L'х':
        buffer[i] = L'h';
        break;
      case L'џ':
        buffer = termAtt->resizeBuffer(1 + length);
        if (i < length) {
          System::arraycopy(buffer, i, buffer, i + 1, (length - i));
        }
        buffer[i] = L'd';
        buffer[++i] = L'z';
        length++;
        break;
      case L'ш':
      case L'š':
        buffer[i] = L's';
        break;
      default:
        break;
      }
    }
    termAtt->setLength(length);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::sr