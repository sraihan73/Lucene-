using namespace std;

#include "BengaliNormalizer.h"

namespace org::apache::lucene::analysis::bn
{
//    import static org.apache.lucene.analysis.util.StemmerUtil.delete;

int BengaliNormalizer::normalize(std::deque<wchar_t> &s, int len)
{

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    // delete Chandrabindu
    case L'\u0981':
      len = delete (s, i, len);
      i--;
      break;

    // DirghoI kar -> RosshoI kar
    case L'\u09C0':
      s[i] = L'\u09BF';
      break;

    // DirghoU kar -> RosshoU kar
    case L'\u09C2':
      s[i] = L'\u09C1';
      break;

    // Khio (Ka + Hoshonto + Murdorno Sh)
    case L'\u0995':
      if (i + 2 < len && s[i + 1] == L'\u09CD' && s[i + 2] == L'\u09BF') {
        if (i == 0) {
          s[i] = L'\u0996';
          len = delete (s, i + 2, len);
          len = delete (s, i + 1, len);
        } else {
          s[i + 1] = L'\u0996';
          len = delete (s, i + 2, len);
        }
      }
      break;

    // Nga to Anusvara
    case L'\u0999':
      s[i] = L'\u0982';
      break;

    // Ja Phala
    case L'\u09AF':
      if (i - 2 == 0 && s[i - 1] == L'\u09CD') {
        s[i - 1] = L'\u09C7';

        if (i + 1 < len && s[i + 1] == L'\u09BE') {
          len = delete (s, i + 1, len);
        }
        len = delete (s, i, len);
        i--;
      } else if (i - 1 >= 0 && s[i - 1] == L'\u09CD') {
        len = delete (s, i, len);
        len = delete (s, i - 1, len);
        i -= 2;
      }
      break;

    // Ba Phalaa
    case L'\u09AC':
      if ((i >= 1 && s[i - 1] != L'\u09CD') || i == 0) {
        break;
      }
      if (i - 2 == 0) {
        len = delete (s, i, len);
        len = delete (s, i - 1, len);
        i -= 2;
      } else if (i - 5 >= 0 && s[i - 3] == L'\u09CD') {
        len = delete (s, i, len);
        len = delete (s, i - 1, len);
        i -= 2;
      } else if (i - 2 >= 0) {
        s[i - 1] = s[i - 2];
        len = delete (s, i, len);
        i--;
      }
      break;

    // Visarga
    case L'\u0983':
      if (i == len - 1) {
        if (len <= 3) {
          s[i] = L'\u09B9';
        } else {
          len = delete (s, i, len);
        }
      } else {
        s[i] = s[i + 1];
      }
      break;

    // All sh
    case L'\u09B6':
    case L'\u09B7':
      s[i] = L'\u09B8';
      break;

    // check na
    case L'\u09A3':
      s[i] = L'\u09A8';
      break;

    // check ra
    case L'\u09DC':
    case L'\u09DD':
      s[i] = L'\u09B0';
      break;

    case L'\u09CE':
      s[i] = L'\u09A4';
      break;

    default:
      break;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::bn