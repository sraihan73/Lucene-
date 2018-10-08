using namespace std;

#include "PorterStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"

namespace org::apache::lucene::analysis::en
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

PorterStemmer::PorterStemmer()
{
  b = std::deque<wchar_t>(INITIAL_SIZE);
  i = 0;
}

void PorterStemmer::reset()
{
  i = 0;
  dirty = false;
}

void PorterStemmer::add(wchar_t ch)
{
  if (b.size() <= i) {
    b = ArrayUtil::grow(b, i + 1);
  }
  b[i++] = ch;
}

wstring PorterStemmer::toString() { return wstring(b, 0, i); }

int PorterStemmer::getResultLength() { return i; }

std::deque<wchar_t> PorterStemmer::getResultBuffer() { return b; }

bool PorterStemmer::cons(int i)
{
  switch (b[i]) {
  case L'a':
  case L'e':
  case L'i':
  case L'o':
  case L'u':
    return false;
  case L'y':
    return (i == k0) ? true : !cons(i - 1);
  default:
    return true;
  }
}

int PorterStemmer::m()
{
  int n = 0;
  int i = k0;
  while (true) {
    if (i > j) {
      return n;
    }
    if (!cons(i)) {
      break;
    }
    i++;
  }
  i++;
  while (true) {
    while (true) {
      if (i > j) {
        return n;
      }
      if (cons(i)) {
        break;
      }
      i++;
    }
    i++;
    n++;
    while (true) {
      if (i > j) {
        return n;
      }
      if (!cons(i)) {
        break;
      }
      i++;
    }
    i++;
  }
}

bool PorterStemmer::vowelinstem()
{
  int i;
  for (i = k0; i <= j; i++) {
    if (!cons(i)) {
      return true;
    }
  }
  return false;
}

bool PorterStemmer::doublec(int j)
{
  if (j < k0 + 1) {
    return false;
  }
  if (b[j] != b[j - 1]) {
    return false;
  }
  return cons(j);
}

bool PorterStemmer::cvc(int i)
{
  if (i < k0 + 2 || !cons(i) || cons(i - 1) || !cons(i - 2)) {
    return false;
  } else {
    int ch = b[i];
    if (ch == L'w' || ch == L'x' || ch == L'y') {
      return false;
    }
  }
  return true;
}

bool PorterStemmer::ends(const wstring &s)
{
  int l = s.length();
  int o = k - l + 1;
  if (o < k0) {
    return false;
  }
  for (int i = 0; i < l; i++) {
    if (b[o + i] != s[i]) {
      return false;
    }
  }
  j = k - l;
  return true;
}

void PorterStemmer::setto(const wstring &s)
{
  int l = s.length();
  int o = j + 1;
  for (int i = 0; i < l; i++) {
    b[o + i] = s[i];
  }
  k = j + l;
  dirty = true;
}

void PorterStemmer::r(const wstring &s)
{
  if (m() > 0) {
    setto(s);
  }
}

void PorterStemmer::step1()
{
  if (b[k] == L's') {
    if (ends(L"sses")) {
      k -= 2;
    } else if (ends(L"ies")) {
      setto(L"i");
    } else if (b[k - 1] != L's') {
      k--;
    }
  }
  if (ends(L"eed")) {
    if (m() > 0) {
      k--;
    }
  } else if ((ends(L"ed") || ends(L"ing")) && vowelinstem()) {
    k = j;
    if (ends(L"at")) {
      setto(L"ate");
    } else if (ends(L"bl")) {
      setto(L"ble");
    } else if (ends(L"iz")) {
      setto(L"ize");
    } else if (doublec(k)) {
      int ch = b[k--];
      if (ch == L'l' || ch == L's' || ch == L'z') {
        k++;
      }
    } else if (m() == 1 && cvc(k)) {
      setto(L"e");
    }
  }
}

void PorterStemmer::step2()
{
  if (ends(L"y") && vowelinstem()) {
    b[k] = L'i';
    dirty = true;
  }
}

void PorterStemmer::step3()
{
  if (k == k0) {
    return; // For Bug 1
  }
  switch (b[k - 1]) {
  case L'a':
    if (ends(L"ational")) {
      r(L"ate");
      break;
    }
    if (ends(L"tional")) {
      r(L"tion");
      break;
    }
    break;
  case L'c':
    if (ends(L"enci")) {
      r(L"ence");
      break;
    }
    if (ends(L"anci")) {
      r(L"ance");
      break;
    }
    break;
  case L'e':
    if (ends(L"izer")) {
      r(L"ize");
      break;
    }
    break;
  case L'l':
    if (ends(L"bli")) {
      r(L"ble");
      break;
    }
    if (ends(L"alli")) {
      r(L"al");
      break;
    }
    if (ends(L"entli")) {
      r(L"ent");
      break;
    }
    if (ends(L"eli")) {
      r(L"e");
      break;
    }
    if (ends(L"ousli")) {
      r(L"ous");
      break;
    }
    break;
  case L'o':
    if (ends(L"ization")) {
      r(L"ize");
      break;
    }
    if (ends(L"ation")) {
      r(L"ate");
      break;
    }
    if (ends(L"ator")) {
      r(L"ate");
      break;
    }
    break;
  case L's':
    if (ends(L"alism")) {
      r(L"al");
      break;
    }
    if (ends(L"iveness")) {
      r(L"ive");
      break;
    }
    if (ends(L"fulness")) {
      r(L"ful");
      break;
    }
    if (ends(L"ousness")) {
      r(L"ous");
      break;
    }
    break;
  case L't':
    if (ends(L"aliti")) {
      r(L"al");
      break;
    }
    if (ends(L"iviti")) {
      r(L"ive");
      break;
    }
    if (ends(L"biliti")) {
      r(L"ble");
      break;
    }
    break;
  case L'g':
    if (ends(L"logi")) {
      r(L"log");
      break;
    }
  }
}

void PorterStemmer::step4()
{
  switch (b[k]) {
  case L'e':
    if (ends(L"icate")) {
      r(L"ic");
      break;
    }
    if (ends(L"ative")) {
      r(L"");
      break;
    }
    if (ends(L"alize")) {
      r(L"al");
      break;
    }
    break;
  case L'i':
    if (ends(L"iciti")) {
      r(L"ic");
      break;
    }
    break;
  case L'l':
    if (ends(L"ical")) {
      r(L"ic");
      break;
    }
    if (ends(L"ful")) {
      r(L"");
      break;
    }
    break;
  case L's':
    if (ends(L"ness")) {
      r(L"");
      break;
    }
    break;
  }
}

void PorterStemmer::step5()
{
  if (k == k0) {
    return; // for Bug 1
  }
  switch (b[k - 1]) {
  case L'a':
    if (ends(L"al")) {
      break;
    }
    return;
  case L'c':
    if (ends(L"ance")) {
      break;
    }
    if (ends(L"ence")) {
      break;
    }
    return;
  case L'e':
    if (ends(L"er")) {
      break;
    }
    return;
  case L'i':
    if (ends(L"ic")) {
      break;
    }
    return;
  case L'l':
    if (ends(L"able")) {
      break;
    }
    if (ends(L"ible")) {
      break;
    }
    return;
  case L'n':
    if (ends(L"ant")) {
      break;
    }
    if (ends(L"ement")) {
      break;
    }
    if (ends(L"ment")) {
      break;
    }
    /* element etc. not stripped before the m */
    if (ends(L"ent")) {
      break;
    }
    return;
  case L'o':
    if (ends(L"ion") && j >= 0 && (b[j] == L's' || b[j] == L't')) {
      break;
    }
    /* j >= 0 fixes Bug 2 */
    if (ends(L"ou")) {
      break;
    }
    return;
    /* takes care of -ous */
  case L's':
    if (ends(L"ism")) {
      break;
    }
    return;
  case L't':
    if (ends(L"ate")) {
      break;
    }
    if (ends(L"iti")) {
      break;
    }
    return;
  case L'u':
    if (ends(L"ous")) {
      break;
    }
    return;
  case L'v':
    if (ends(L"ive")) {
      break;
    }
    return;
  case L'z':
    if (ends(L"ize")) {
      break;
    }
    return;
  default:
    return;
  }
  if (m() > 1) {
    k = j;
  }
}

void PorterStemmer::step6()
{
  j = k;
  if (b[k] == L'e') {
    int a = m();
    if (a > 1 || a == 1 && !cvc(k - 1)) {
      k--;
    }
  }
  if (b[k] == L'l' && doublec(k) && m() > 1) {
    k--;
  }
}

wstring PorterStemmer::stem(const wstring &s)
{
  if (stem(s.toCharArray(), s.length())) {
    return toString();
  } else {
    return s;
  }
}

bool PorterStemmer::stem(std::deque<wchar_t> &word)
{
  return stem(word, word.size());
}

bool PorterStemmer::stem(std::deque<wchar_t> &wordBuffer, int offset,
                         int wordLen)
{
  reset();
  if (b.size() < wordLen) {
    b = std::deque<wchar_t>(ArrayUtil::oversize(wordLen, Character::BYTES));
  }
  System::arraycopy(wordBuffer, offset, b, 0, wordLen);
  i = wordLen;
  return stem(0);
}

bool PorterStemmer::stem(std::deque<wchar_t> &word, int wordLen)
{
  return stem(word, 0, wordLen);
}

bool PorterStemmer::stem() { return stem(0); }

bool PorterStemmer::stem(int i0)
{
  k = i - 1;
  k0 = i0;
  if (k > k0 + 1) {
    step1();
    step2();
    step3();
    step4();
    step5();
    step6();
  }
  // Also, a word is considered dirty if we lopped off letters
  // Thanks to Ifigenia Vairelles for pointing this out.
  if (i != k + 1) {
    dirty = true;
  }
  i = k + 1;
  return dirty;
}
} // namespace org::apache::lucene::analysis::en