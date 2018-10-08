using namespace std;

#include "HindiStemmer.h"

namespace org::apache::lucene::analysis::hi
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int HindiStemmer::stem(std::deque<wchar_t> &buffer, int len)
{
  // 5
  if ((len > 6) && (StemmerUtil::endsWith(buffer, len, L"ाएंगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ाएंगे") ||
                    StemmerUtil::endsWith(buffer, len, L"ाऊंगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ाऊंगा") ||
                    StemmerUtil::endsWith(buffer, len, L"ाइयाँ") ||
                    StemmerUtil::endsWith(buffer, len, L"ाइयों") ||
                    StemmerUtil::endsWith(buffer, len, L"ाइयां"))) {
    return len - 5;
  }

  // 4
  if ((len > 5) && (StemmerUtil::endsWith(buffer, len, L"ाएगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ाएगा") ||
                    StemmerUtil::endsWith(buffer, len, L"ाओगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ाओगे") ||
                    StemmerUtil::endsWith(buffer, len, L"एंगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ेंगी") ||
                    StemmerUtil::endsWith(buffer, len, L"एंगे") ||
                    StemmerUtil::endsWith(buffer, len, L"ेंगे") ||
                    StemmerUtil::endsWith(buffer, len, L"ूंगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ूंगा") ||
                    StemmerUtil::endsWith(buffer, len, L"ातीं") ||
                    StemmerUtil::endsWith(buffer, len, L"नाओं") ||
                    StemmerUtil::endsWith(buffer, len, L"नाएं") ||
                    StemmerUtil::endsWith(buffer, len, L"ताओं") ||
                    StemmerUtil::endsWith(buffer, len, L"ताएं") ||
                    StemmerUtil::endsWith(buffer, len, L"ियाँ") ||
                    StemmerUtil::endsWith(buffer, len, L"ियों") ||
                    StemmerUtil::endsWith(buffer, len, L"ियां"))) {
    return len - 4;
  }

  // 3
  if ((len > 4) && (StemmerUtil::endsWith(buffer, len, L"ाकर") ||
                    StemmerUtil::endsWith(buffer, len, L"ाइए") ||
                    StemmerUtil::endsWith(buffer, len, L"ाईं") ||
                    StemmerUtil::endsWith(buffer, len, L"ाया") ||
                    StemmerUtil::endsWith(buffer, len, L"ेगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ेगा") ||
                    StemmerUtil::endsWith(buffer, len, L"ोगी") ||
                    StemmerUtil::endsWith(buffer, len, L"ोगे") ||
                    StemmerUtil::endsWith(buffer, len, L"ाने") ||
                    StemmerUtil::endsWith(buffer, len, L"ाना") ||
                    StemmerUtil::endsWith(buffer, len, L"ाते") ||
                    StemmerUtil::endsWith(buffer, len, L"ाती") ||
                    StemmerUtil::endsWith(buffer, len, L"ाता") ||
                    StemmerUtil::endsWith(buffer, len, L"तीं") ||
                    StemmerUtil::endsWith(buffer, len, L"ाओं") ||
                    StemmerUtil::endsWith(buffer, len, L"ाएं") ||
                    StemmerUtil::endsWith(buffer, len, L"ुओं") ||
                    StemmerUtil::endsWith(buffer, len, L"ुएं") ||
                    StemmerUtil::endsWith(buffer, len, L"ुआं"))) {
    return len - 3;
  }

  // 2
  if ((len > 3) && (StemmerUtil::endsWith(buffer, len, L"कर") ||
                    StemmerUtil::endsWith(buffer, len, L"ाओ") ||
                    StemmerUtil::endsWith(buffer, len, L"िए") ||
                    StemmerUtil::endsWith(buffer, len, L"ाई") ||
                    StemmerUtil::endsWith(buffer, len, L"ाए") ||
                    StemmerUtil::endsWith(buffer, len, L"ने") ||
                    StemmerUtil::endsWith(buffer, len, L"नी") ||
                    StemmerUtil::endsWith(buffer, len, L"ना") ||
                    StemmerUtil::endsWith(buffer, len, L"ते") ||
                    StemmerUtil::endsWith(buffer, len, L"ीं") ||
                    StemmerUtil::endsWith(buffer, len, L"ती") ||
                    StemmerUtil::endsWith(buffer, len, L"ता") ||
                    StemmerUtil::endsWith(buffer, len, L"ाँ") ||
                    StemmerUtil::endsWith(buffer, len, L"ां") ||
                    StemmerUtil::endsWith(buffer, len, L"ों") ||
                    StemmerUtil::endsWith(buffer, len, L"ें"))) {
    return len - 2;
  }

  // 1
  if ((len > 2) && (StemmerUtil::endsWith(buffer, len, L"ो") ||
                    StemmerUtil::endsWith(buffer, len, L"े") ||
                    StemmerUtil::endsWith(buffer, len, L"ू") ||
                    StemmerUtil::endsWith(buffer, len, L"ु") ||
                    StemmerUtil::endsWith(buffer, len, L"ी") ||
                    StemmerUtil::endsWith(buffer, len, L"ि") ||
                    StemmerUtil::endsWith(buffer, len, L"ा"))) {
    return len - 1;
  }
  return len;
}
} // namespace org::apache::lucene::analysis::hi