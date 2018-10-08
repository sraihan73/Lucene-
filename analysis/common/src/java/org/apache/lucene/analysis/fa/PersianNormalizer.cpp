using namespace std;

#include "PersianNormalizer.h"

namespace org::apache::lucene::analysis::fa
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int PersianNormalizer::normalize(std::deque<wchar_t> &s, int len)
{

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case FARSI_YEH:
    case YEH_BARREE:
      s[i] = YEH;
      break;
    case KEHEH:
      s[i] = KAF;
      break;
    case HEH_YEH:
    case HEH_GOAL:
      s[i] = HEH;
      break;
    case HAMZA_ABOVE: // necessary for HEH + HAMZA
      len = StemmerUtil::delete (s, i, len);
      i--;
      break;
    default:
      break;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::fa