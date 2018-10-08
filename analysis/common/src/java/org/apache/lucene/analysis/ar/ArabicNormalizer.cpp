using namespace std;

#include "ArabicNormalizer.h"

namespace org::apache::lucene::analysis::ar
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int ArabicNormalizer::normalize(std::deque<wchar_t> &s, int len)
{

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case ALEF_MADDA:
    case ALEF_HAMZA_ABOVE:
    case ALEF_HAMZA_BELOW:
      s[i] = ALEF;
      break;
    case DOTLESS_YEH:
      s[i] = YEH;
      break;
    case TEH_MARBUTA:
      s[i] = HEH;
      break;
    case TATWEEL:
    case KASRATAN:
    case DAMMATAN:
    case FATHATAN:
    case FATHA:
    case DAMMA:
    case KASRA:
    case SHADDA:
    case SUKUN:
      len = StemmerUtil::delete (s, i, len);
      i--;
      break;
    default:
      break;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::ar