using namespace std;

#include "DecimalDigitFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../util/StemmerUtil.h"

namespace org::apache::lucene::analysis::core
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using StemmerUtil = org::apache::lucene::analysis::util::StemmerUtil;

DecimalDigitFilter::DecimalDigitFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool DecimalDigitFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> buffer = termAtt->buffer();
    int length = termAtt->length();

    for (int i = 0; i < length; i++) {
      int ch = Character::codePointAt(buffer, i, length);
      // look for digits outside of basic latin
      if (ch > 0x7F && isdigit(ch)) {
        // replace with equivalent basic latin digit
        buffer[i] = static_cast<wchar_t>(L'0' + Character::getNumericValue(ch));
        // if the original was supplementary, shrink the string
        if (ch > 0xFFFF) {
          length = StemmerUtil::delete (buffer, i + 1, length);
          termAtt->setLength(length);
        }
      }
    }

    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::core