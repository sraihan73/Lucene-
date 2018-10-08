using namespace std;

#include "SoraniNormalizer.h"

namespace org::apache::lucene::analysis::ckb
{
//    import static org.apache.lucene.analysis.util.StemmerUtil.delete;

int SoraniNormalizer::normalize(std::deque<wchar_t> &s, int len)
{
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case YEH:
    case DOTLESS_YEH:
      s[i] = FARSI_YEH;
      break;
    case KAF:
      s[i] = KEHEH;
      break;
    case ZWNJ:
      if (i > 0 && s[i - 1] == HEH) {
        s[i - 1] = AE;
      }
      len = delete (s, i, len);
      i--;
      break;
    case HEH:
      if (i == len - 1) {
        s[i] = AE;
      }
      break;
    case TEH_MARBUTA:
      s[i] = AE;
      break;
    case HEH_DOACHASHMEE:
      s[i] = HEH;
      break;
    case REH:
      if (i == 0) {
        s[i] = RREH;
      }
      break;
    case RREH_ABOVE:
      s[i] = RREH;
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
      len = delete (s, i, len);
      i--;
      break;
    default:
      if (Character::getType(s[i]) == Character::FORMAT) {
        len = delete (s, i, len);
        i--;
      }
    }
  }
  return len;
}
} // namespace org::apache::lucene::analysis::ckb