using namespace std;

#include "SoraniStemmer.h"

namespace org::apache::lucene::analysis::ckb
{
//    import static org.apache.lucene.analysis.util.StemmerUtil.endsWith;

int SoraniStemmer::stem(std::deque<wchar_t> &s, int len)
{
  // postposition
  if (len > 5 && endsWith(s, len, L"دا")) {
    len -= 2;
  } else if (len > 4 && endsWith(s, len, L"نا")) {
    len--;
  } else if (len > 6 && endsWith(s, len, L"ەوە")) {
    len -= 3;
  }

  // possessive pronoun
  if (len > 6 && (endsWith(s, len, L"مان") || endsWith(s, len, L"یان") ||
                  endsWith(s, len, L"تان"))) {
    len -= 3;
  }

  // indefinite singular ezafe
  if (len > 6 && endsWith(s, len, L"ێکی")) {
    return len - 3;
  } else if (len > 7 && endsWith(s, len, L"یەکی")) {
    return len - 4;
  }
  // indefinite singular
  if (len > 5 && endsWith(s, len, L"ێک")) {
    return len - 2;
  } else if (len > 6 && endsWith(s, len, L"یەک")) {
    return len - 3;
  }
  // definite singular
  else if (len > 6 && endsWith(s, len, L"ەکە")) {
    return len - 3;
  } else if (len > 5 && endsWith(s, len, L"کە")) {
    return len - 2;
  }
  // definite plural
  else if (len > 7 && endsWith(s, len, L"ەکان")) {
    return len - 4;
  } else if (len > 6 && endsWith(s, len, L"کان")) {
    return len - 3;
  }
  // indefinite plural ezafe
  else if (len > 7 && endsWith(s, len, L"یانی")) {
    return len - 4;
  } else if (len > 6 && endsWith(s, len, L"انی")) {
    return len - 3;
  }
  // indefinite plural
  else if (len > 6 && endsWith(s, len, L"یان")) {
    return len - 3;
  } else if (len > 5 && endsWith(s, len, L"ان")) {
    return len - 2;
  }
  // demonstrative plural
  else if (len > 7 && endsWith(s, len, L"یانە")) {
    return len - 4;
  } else if (len > 6 && endsWith(s, len, L"انە")) {
    return len - 3;
  }
  // demonstrative singular
  else if (len > 5 && (endsWith(s, len, L"ایە") || endsWith(s, len, L"ەیە"))) {
    return len - 2;
  } else if (len > 4 && endsWith(s, len, L"ە")) {
    return len - 1;
  }
  // absolute singular ezafe
  else if (len > 4 && endsWith(s, len, L"ی")) {
    return len - 1;
  }
  return len;
}
} // namespace org::apache::lucene::analysis::ckb