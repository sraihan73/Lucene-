using namespace std;

#include "HyphenationTree.h"
#include "ByteVector.h"
#include "Hyphenation.h"
#include "PatternParser.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{
using org::xml::sax::InputSource;

HyphenationTree::HyphenationTree()
{
  stoplist = unordered_map<wstring, deque<any>>(23); // usually a small table
  classmap = make_shared<TernaryTree>();
  vspace = make_shared<ByteVector>();
  vspace->alloc(1); // this reserves index 0, which we don't use
}

int HyphenationTree::packValues(const wstring &values)
{
  int i, n = values.length();
  int m = (n & 1) == 1 ? (n >> 1) + 2 : (n >> 1) + 1;
  int offset = vspace->alloc(m);
  std::deque<char> va = vspace->getArray();
  for (i = 0; i < n; i++) {
    int j = i >> 1;
    char v = static_cast<char>((values[i] - L'0' + 1) & 0x0f);
    if ((i & 1) == 1) {
      va[j + offset] = static_cast<char>(va[j + offset] | v);
    } else {
      va[j + offset] = static_cast<char>(v << 4); // big endian
    }
  }
  va[m - 1 + offset] = 0; // terminator
  return offset;
}

wstring HyphenationTree::unpackValues(int k)
{
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>();
  char v = vspace->get(k++);
  while (v != 0) {
    wchar_t c = static_cast<wchar_t>(
        (static_cast<int>(static_cast<unsigned int>((v & 0xf0)) >> 4)) - 1 +
        L'0');
    buf->append(c);
    c = static_cast<wchar_t>(v & 0x0f);
    if (c == 0) {
      break;
    }
    c = static_cast<wchar_t>(c - 1 + L'0');
    buf->append(c);
    v = vspace->get(k++);
  }
  return buf->toString();
}

void HyphenationTree::loadPatterns(shared_ptr<InputSource> source) throw(
    IOException)
{
  shared_ptr<PatternParser> pp = make_shared<PatternParser>(shared_from_this());
  ivalues = make_shared<TernaryTree>();

  pp->parse(source);

  // patterns/values should be now in the tree
  // let's optimize a bit
  trimToSize();
  vspace->trimToSize();
  classmap->trimToSize();

  // get rid of the auxiliary map_obj
  ivalues.reset();
}

wstring HyphenationTree::findPattern(const wstring &pat)
{
  int k = TernaryTree::find(pat);
  if (k >= 0) {
    return unpackValues(k);
  }
  return L"";
}

int HyphenationTree::hstrcmp(std::deque<wchar_t> &s, int si,
                             std::deque<wchar_t> &t, int ti)
{
  for (; s[si] == t[ti]; si++, ti++) {
    if (s[si] == 0) {
      return 0;
    }
  }
  if (t[ti] == 0) {
    return 0;
  }
  return s[si] - t[ti];
}

std::deque<char> HyphenationTree::getValues(int k)
{
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>();
  char v = vspace->get(k++);
  while (v != 0) {
    wchar_t c = static_cast<wchar_t>(
        (static_cast<int>(static_cast<unsigned int>((v & 0xf0)) >> 4)) - 1);
    buf->append(c);
    c = static_cast<wchar_t>(v & 0x0f);
    if (c == 0) {
      break;
    }
    c = static_cast<wchar_t>(c - 1);
    buf->append(c);
    v = vspace->get(k++);
  }
  std::deque<char> res(buf->length());
  for (int i = 0; i < res.size(); i++) {
    res[i] = static_cast<char>(buf->charAt(i));
  }
  return res;
}

void HyphenationTree::searchPatterns(std::deque<wchar_t> &word, int index,
                                     std::deque<char> &il)
{
  std::deque<char> values;
  int i = index;
  wchar_t p, q;
  wchar_t sp = word[i];
  p = root;

  while (p > 0 && p < sc.size()) {
    if (sc[p] == 0xFFFF) {
      if (hstrcmp(word, i, kv->getArray(), lo[p]) == 0) {
        values = getValues(eq[p]); // data pointer is in eq[]
        int j = index;
        for (int k = 0; k < values.size(); k++) {
          if (j < il.size() && values[k] > il[j]) {
            il[j] = values[k];
          }
          j++;
        }
      }
      return;
    }
    int d = sp - sc[p];
    if (d == 0) {
      if (sp == 0) {
        break;
      }
      sp = word[++i];
      p = eq[p];
      q = p;

      // look for a pattern ending at this position by searching for
      // the null char ( splitchar == 0 )
      while (q > 0 && q < sc.size()) {
        if (sc[q] == 0xFFFF) { // stop at compressed branch
          break;
        }
        if (sc[q] == 0) {
          values = getValues(eq[q]);
          int j = index;
          for (int k = 0; k < values.size(); k++) {
            if (j < il.size() && values[k] > il[j]) {
              il[j] = values[k];
            }
            j++;
          }
          break;
        } else {
          q = lo[q];

          /**
           * actually the code should be: q = sc[q] < 0 ? hi[q] : lo[q]; but
           * java chars are unsigned
           */
        }
      }
    } else {
      p = d < 0 ? lo[p] : hi[p];
    }
  }
}

shared_ptr<Hyphenation> HyphenationTree::hyphenate(const wstring &word,
                                                   int remainCharCount,
                                                   int pushCharCount)
{
  std::deque<wchar_t> w = word.toCharArray();
  return hyphenate(w, 0, w.size(), remainCharCount, pushCharCount);
}

shared_ptr<Hyphenation> HyphenationTree::hyphenate(std::deque<wchar_t> &w,
                                                   int offset, int len,
                                                   int remainCharCount,
                                                   int pushCharCount)
{
  int i;
  std::deque<wchar_t> word(len + 3);

  // normalize word
  std::deque<wchar_t> c(2);
  int iIgnoreAtBeginning = 0;
  int iLength = len;
  bool bEndOfLetters = false;
  for (i = 1; i <= len; i++) {
    c[0] = w[offset + i - 1];
    int nc = classmap->find(c, 0);
    if (nc < 0) { // found a non-letter character ...
      if (i == (1 + iIgnoreAtBeginning)) {
        // ... before any letter character
        iIgnoreAtBeginning++;
      } else {
        // ... after a letter character
        bEndOfLetters = true;
      }
      iLength--;
    } else {
      if (!bEndOfLetters) {
        word[i - iIgnoreAtBeginning] = static_cast<wchar_t>(nc);
      } else {
        return nullptr;
      }
    }
  }
  len = iLength;
  if (len < (remainCharCount + pushCharCount)) {
    // word is too short to be hyphenated
    return nullptr;
  }
  std::deque<int> result(len + 1);
  int k = 0;

  // check exception deque first
  wstring sw = wstring(word, 1, len);
  if (stoplist.find(sw) != stoplist.end()) {
    // assume only simple hyphens (Hyphen.pre="-", Hyphen.post = Hyphen.no =
    // null)
    deque<any> hw = stoplist[sw];
    int j = 0;
    for (i = 0; i < hw.size(); i++) {
      any o = hw[i];
      // j = index(sw) = letterindex(word)?
      // result[k] = corresponding index(w)
      if (dynamic_cast<wstring>(o) != nullptr) {
        j += (any_cast<wstring>(o))->length();
        if (j >= remainCharCount && j < (len - pushCharCount)) {
          result[k++] = j + iIgnoreAtBeginning;
        }
      }
    }
  } else {
    // use algorithm to get hyphenation points
    word[0] = L'.';                // word start marker
    word[len + 1] = L'.';          // word end marker
    word[len + 2] = 0;             // null terminated
    std::deque<char> il(len + 3); // initialized to zero
    for (i = 0; i < len + 1; i++) {
      searchPatterns(word, i, il);
    }

    // hyphenation points are located where interletter value is odd
    // i is letterindex(word),
    // i + 1 is index(word),
    // result[k] = corresponding index(w)
    for (i = 0; i < len; i++) {
      if (((il[i + 1] & 1) == 1) && i >= remainCharCount &&
          i <= (len - pushCharCount)) {
        result[k++] = i + iIgnoreAtBeginning;
      }
    }
  }

  if (k > 0) {
    // trim result array
    std::deque<int> res(k + 2);
    System::arraycopy(result, 0, res, 1, k);
    // We add the synthetical hyphenation points
    // at the beginning and end of the word
    res[0] = 0;
    res[k + 1] = len;
    return make_shared<Hyphenation>(res);
  } else {
    return nullptr;
  }
}

void HyphenationTree::addClass(const wstring &chargroup)
{
  if (chargroup.length() > 0) {
    wchar_t equivChar = chargroup[0];
    std::deque<wchar_t> key(2);
    key[1] = 0;
    for (int i = 0; i < chargroup.length(); i++) {
      key[0] = chargroup[i];
      classmap->insert(key, 0, equivChar);
    }
  }
}

void HyphenationTree::addException(const wstring &word,
                                   deque<any> &hyphenatedword)
{
  stoplist.emplace(word, hyphenatedword);
}

void HyphenationTree::addPattern(const wstring &pattern, const wstring &ivalue)
{
  int k = ivalues->find(ivalue);
  if (k <= 0) {
    k = packValues(ivalue);
    ivalues->insert(ivalue, static_cast<wchar_t>(k));
  }
  insert(pattern, static_cast<wchar_t>(k));
}

void HyphenationTree::printStats(shared_ptr<PrintStream> out)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  out->println(L"Value space size = " + Integer::toString(vspace->length()));
  TernaryTree::printStats(out);
}
} // namespace org::apache::lucene::analysis::compound::hyphenation