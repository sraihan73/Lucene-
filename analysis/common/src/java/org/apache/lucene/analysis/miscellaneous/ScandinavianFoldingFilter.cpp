using namespace std;

#include "ScandinavianFoldingFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../util/StemmerUtil.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using StemmerUtil = org::apache::lucene::analysis::util::StemmerUtil;

ScandinavianFoldingFilter::ScandinavianFoldingFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool ScandinavianFoldingFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  std::deque<wchar_t> buffer = charTermAttribute->buffer();
  int length = charTermAttribute->length();

  int i;
  for (i = 0; i < length; i++) {

    if (buffer[i] == aa || buffer[i] == ae_se || buffer[i] == ae) {

      buffer[i] = L'a';

    } else if (buffer[i] == AA || buffer[i] == AE_se || buffer[i] == AE) {

      buffer[i] = L'A';

    } else if (buffer[i] == oe || buffer[i] == oe_se) {

      buffer[i] = L'o';

    } else if (buffer[i] == OE || buffer[i] == OE_se) {

      buffer[i] = L'O';

    } else if (length - 1 > i) {

      if ((buffer[i] == L'a' || buffer[i] == L'A') &&
          (buffer[i + 1] == L'a' || buffer[i + 1] == L'A' ||
           buffer[i + 1] == L'e' || buffer[i + 1] == L'E' ||
           buffer[i + 1] == L'o' || buffer[i + 1] == L'O')) {

        length = StemmerUtil::delete (buffer, i + 1, length);

      } else if ((buffer[i] == L'o' || buffer[i] == L'O') &&
                 (buffer[i + 1] == L'e' || buffer[i + 1] == L'E' ||
                  buffer[i + 1] == L'o' || buffer[i + 1] == L'O')) {

        length = StemmerUtil::delete (buffer, i + 1, length);
      }
    }
  }

  charTermAttribute->setLength(length);

  return true;
}
} // namespace org::apache::lucene::analysis::miscellaneous