using namespace std;

#include "BengaliStemmer.h"

namespace org::apache::lucene::analysis::bn
{
//    import static org.apache.lucene.analysis.util.StemmerUtil.endsWith;

int BengaliStemmer::stem(std::deque<wchar_t> &buffer, int len)
{

  // 8
  if (len > 9 && (endsWith(buffer, len, L"িয়াছিলাম") ||
                  endsWith(buffer, len, L"িতেছিলাম") ||
                  endsWith(buffer, len, L"িতেছিলেন") ||
                  endsWith(buffer, len, L"ইতেছিলেন") ||
                  endsWith(buffer, len, L"িয়াছিলেন") ||
                  endsWith(buffer, len, L"ইয়াছিলেন"))) {
    return len - 8;
  }

  // 7
  if ((len > 8) &&
      (endsWith(buffer, len, L"িতেছিলি") || endsWith(buffer, len, L"িতেছিলে") ||
       endsWith(buffer, len, L"িয়াছিলা") || endsWith(buffer, len, L"িয়াছিলে") ||
       endsWith(buffer, len, L"িতেছিলা") || endsWith(buffer, len, L"িয়াছিলি") ||
       endsWith(buffer, len, L"য়েদেরকে"))) {
    return len - 7;
  }

  // 6
  if ((len > 7) &&
      (endsWith(buffer, len, L"িতেছিস") || endsWith(buffer, len, L"িতেছেন") ||
       endsWith(buffer, len, L"িয়াছিস") || endsWith(buffer, len, L"িয়াছেন") ||
       endsWith(buffer, len, L"েছিলাম") || endsWith(buffer, len, L"েছিলেন") ||
       endsWith(buffer, len, L"েদেরকে"))) {
    return len - 6;
  }

  // 5
  if ((len > 6) &&
      (endsWith(buffer, len, L"িতেছি") || endsWith(buffer, len, L"িতেছা") ||
       endsWith(buffer, len, L"িতেছে") || endsWith(buffer, len, L"ছিলাম") ||
       endsWith(buffer, len, L"ছিলেন") || endsWith(buffer, len, L"িয়াছি") ||
       endsWith(buffer, len, L"িয়াছা") || endsWith(buffer, len, L"িয়াছে") ||
       endsWith(buffer, len, L"েছিলে") || endsWith(buffer, len, L"েছিলা") ||
       endsWith(buffer, len, L"য়েদের") || endsWith(buffer, len, L"দেরকে"))) {
    return len - 5;
  }

  // 4
  if ((len > 5) &&
      (endsWith(buffer, len, L"িলাম") || endsWith(buffer, len, L"িলেন") ||
       endsWith(buffer, len, L"িতাম") || endsWith(buffer, len, L"িতেন") ||
       endsWith(buffer, len, L"িবেন") || endsWith(buffer, len, L"ছিলি") ||
       endsWith(buffer, len, L"ছিলে") || endsWith(buffer, len, L"ছিলা") ||
       endsWith(buffer, len, L"তেছে") || endsWith(buffer, len, L"িতেছ") ||
       endsWith(buffer, len, L"খানা") || endsWith(buffer, len, L"খানি") ||
       endsWith(buffer, len, L"গুলো") || endsWith(buffer, len, L"গুলি") ||
       endsWith(buffer, len, L"য়েরা") || endsWith(buffer, len, L"েদের"))) {
    return len - 4;
  }

  // 3
  if ((len > 4) &&
      (endsWith(buffer, len, L"লাম") || endsWith(buffer, len, L"িলি") ||
       endsWith(buffer, len, L"ইলি") || endsWith(buffer, len, L"িলে") ||
       endsWith(buffer, len, L"ইলে") || endsWith(buffer, len, L"লেন") ||
       endsWith(buffer, len, L"িলা") || endsWith(buffer, len, L"ইলা") ||
       endsWith(buffer, len, L"তাম") || endsWith(buffer, len, L"িতি") ||
       endsWith(buffer, len, L"ইতি") || endsWith(buffer, len, L"িতে") ||
       endsWith(buffer, len, L"ইতে") || endsWith(buffer, len, L"তেন") ||
       endsWith(buffer, len, L"িতা") || endsWith(buffer, len, L"িবা") ||
       endsWith(buffer, len, L"ইবা") || endsWith(buffer, len, L"িবি") ||
       endsWith(buffer, len, L"ইবি") || endsWith(buffer, len, L"বেন") ||
       endsWith(buffer, len, L"িবে") || endsWith(buffer, len, L"ইবে") ||
       endsWith(buffer, len, L"ছেন") || endsWith(buffer, len, L"য়োন") ||
       endsWith(buffer, len, L"য়ের") || endsWith(buffer, len, L"েরা") ||
       endsWith(buffer, len, L"দের"))) {
    return len - 3;
  }

  // 2
  if ((len > 3) &&
      (endsWith(buffer, len, L"িস") || endsWith(buffer, len, L"েন") ||
       endsWith(buffer, len, L"লি") || endsWith(buffer, len, L"লে") ||
       endsWith(buffer, len, L"লা") || endsWith(buffer, len, L"তি") ||
       endsWith(buffer, len, L"তে") || endsWith(buffer, len, L"তা") ||
       endsWith(buffer, len, L"বি") || endsWith(buffer, len, L"বে") ||
       endsWith(buffer, len, L"বা") || endsWith(buffer, len, L"ছি") ||
       endsWith(buffer, len, L"ছা") || endsWith(buffer, len, L"ছে") ||
       endsWith(buffer, len, L"ুন") || endsWith(buffer, len, L"ুক") ||
       endsWith(buffer, len, L"টা") || endsWith(buffer, len, L"টি") ||
       endsWith(buffer, len, L"নি") || endsWith(buffer, len, L"ের") ||
       endsWith(buffer, len, L"তে") || endsWith(buffer, len, L"রা") ||
       endsWith(buffer, len, L"কে"))) {
    return len - 2;
  }

  // 1
  if ((len > 2) &&
      (endsWith(buffer, len, L"ি") || endsWith(buffer, len, L"ী") ||
       endsWith(buffer, len, L"া") || endsWith(buffer, len, L"ো") ||
       endsWith(buffer, len, L"ে") || endsWith(buffer, len, L"ব") ||
       endsWith(buffer, len, L"ত"))) {
    return len - 1;
  }

  return len;
}
} // namespace org::apache::lucene::analysis::bn