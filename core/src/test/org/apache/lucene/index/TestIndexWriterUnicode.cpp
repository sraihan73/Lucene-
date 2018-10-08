using namespace std;

#include "TestIndexWriterUnicode.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

int TestIndexWriterUnicode::nextInt(int lim) { return random()->nextInt(lim); }

int TestIndexWriterUnicode::nextInt(int start, int end)
{
  return start + nextInt(end - start);
}

bool TestIndexWriterUnicode::fillUnicode(std::deque<wchar_t> &buffer,
                                         std::deque<wchar_t> &expected,
                                         int offset, int count)
{
  constexpr int len = offset + count;
  bool hasIllegal = false;

  if (offset > 0 && buffer[offset] >= 0xdc00 && buffer[offset] < 0xe000) {
    // Don't start in the middle of a valid surrogate pair
    offset--;
  }

  for (int i = offset; i < len; i++) {
    int t = nextInt(6);
    if (0 == t && i < len - 1) {
      // Make a surrogate pair
      // High surrogate
      expected[i] = buffer[i++] = static_cast<wchar_t>(nextInt(0xd800, 0xdc00));
      // Low surrogate
      expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0xdc00, 0xe000));
    } else if (t <= 1) {
      expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0x80));
    } else if (2 == t) {
      expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0x80, 0x800));
    } else if (3 == t) {
      expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0x800, 0xd800));
    } else if (4 == t) {
      expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0xe000, 0xffff));
    } else if (5 == t && i < len - 1) {
      // Illegal unpaired surrogate
      if (nextInt(10) == 7) {
        if (random()->nextBoolean()) {
          buffer[i] = static_cast<wchar_t>(nextInt(0xd800, 0xdc00));
        } else {
          buffer[i] = static_cast<wchar_t>(nextInt(0xdc00, 0xe000));
        }
        expected[i++] = 0xfffd;
        expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0x800, 0xd800));
        hasIllegal = true;
      } else {
        expected[i] = buffer[i] = static_cast<wchar_t>(nextInt(0x800, 0xd800));
      }
    } else {
      expected[i] = buffer[i] = L' ';
    }
  }

  return hasIllegal;
}

int TestIndexWriterUnicode::getInt(shared_ptr<Random> r, int start, int end)
{
  return start + r->nextInt(1 + end - start);
}

wstring TestIndexWriterUnicode::asUnicodeChar(wchar_t c)
{
  return L"U+" + Integer::toHexString(c);
}

wstring TestIndexWriterUnicode::termDesc(const wstring &s)
{
  const wstring s0;
  assertTrue(s.length() <= 2);
  if (s.length() == 1) {
    s0 = asUnicodeChar(s[0]);
  } else {
    s0 = asUnicodeChar(s[0]) + L"," + asUnicodeChar(s[1]);
  }
  return s0;
}

void TestIndexWriterUnicode::checkTermsOrder(shared_ptr<IndexReader> r,
                                             shared_ptr<Set<wstring>> allTerms,
                                             bool isTop) 
{
  shared_ptr<TermsEnum> terms = MultiFields::getTerms(r, L"f")->begin();

  shared_ptr<BytesRefBuilder> last = make_shared<BytesRefBuilder>();

  shared_ptr<Set<wstring>> seenTerms = unordered_set<wstring>();

  while (true) {
    shared_ptr<BytesRef> *const term = terms->next();
    if (term == nullptr) {
      break;
    }

    assertTrue(last->get()->compareTo(term) < 0);
    last->copyBytes(term);

    const wstring s = term->utf8ToString();
    assertTrue(L"term " + termDesc(s) + L" was not added to index (count=" +
                   allTerms->size() + L")",
               allTerms->contains(s));
    seenTerms->add(s);
  }

  if (isTop) {
    assertTrue(allTerms->equals(seenTerms));
  }

  // Test seeking:
  Set<wstring>::const_iterator it = seenTerms->begin();
  while (it != seenTerms->end()) {
    shared_ptr<BytesRef> tr = make_shared<BytesRef>(*it);
    assertEquals(L"seek failed for term=" + termDesc(tr->utf8ToString()),
                 TermsEnum::SeekStatus::FOUND, terms->seekCeil(tr));
    it++;
  }
}

void TestIndexWriterUnicode::testRandomUnicodeStrings() 
{
  std::deque<wchar_t> buffer(20);
  std::deque<wchar_t> expected(20);

  shared_ptr<CharsRefBuilder> utf16 = make_shared<CharsRefBuilder>();

  int num = atLeast(100000);
  for (int iter = 0; iter < num; iter++) {
    bool hasIllegal = fillUnicode(buffer, expected, 0, 20);

    shared_ptr<BytesRef> utf8 =
        make_shared<BytesRef>(CharBuffer::wrap(buffer, 0, 20));
    if (!hasIllegal) {
      std::deque<char> b =
          (wstring(buffer, 0, 20))->getBytes(StandardCharsets::UTF_8);
      assertEquals(b.size(), utf8->length);
      for (int i = 0; i < b.size(); i++) {
        assertEquals(b[i], utf8->bytes[i]);
      }
    }

    utf16->copyUTF8Bytes(utf8->bytes, 0, utf8->length);
    assertEquals(utf16->length(), 20);
    for (int i = 0; i < 20; i++) {
      assertEquals(expected[i], utf16->charAt(i));
    }
  }
}

void TestIndexWriterUnicode::testAllUnicodeChars() 
{

  shared_ptr<CharsRefBuilder> utf16 = make_shared<CharsRefBuilder>();
  std::deque<wchar_t> chars(2);
  for (int ch = 0; ch < 0x0010FFFF; ch++) {

    if (ch == 0xd800) {
      // Skip invalid code points
      ch = 0xe000;
    }

    int len = 0;
    if (ch <= 0xffff) {
      chars[len++] = static_cast<wchar_t>(ch);
    } else {
      chars[len++] = static_cast<wchar_t>(((ch - 0x0010000) >> 10) +
                                          UnicodeUtil::UNI_SUR_HIGH_START);
      chars[len++] = static_cast<wchar_t>(((ch - 0x0010000) & 0x3FFLL) +
                                          UnicodeUtil::UNI_SUR_LOW_START);
    }

    shared_ptr<BytesRef> utf8 =
        make_shared<BytesRef>(CharBuffer::wrap(chars, 0, len));

    wstring s1 = wstring(chars, 0, len);
    wstring s2 = wstring(utf8->bytes, 0, utf8->length, StandardCharsets::UTF_8);
    assertEquals(L"codepoint " + to_wstring(ch), s1, s2);

    utf16->copyUTF8Bytes(utf8->bytes, 0, utf8->length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"codepoint " + to_wstring(ch), s1, utf16->toString());

    std::deque<char> b = s1.getBytes(StandardCharsets::UTF_8);
    assertEquals(utf8->length, b.size());
    for (int j = 0; j < utf8->length; j++) {
      assertEquals(utf8->bytes[j], b[j]);
    }
  }
}

void TestIndexWriterUnicode::testEmbeddedFFFF() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a a\uffffb", Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  assertEquals(1, r->docFreq(make_shared<Term>(L"field", L"a\uffffb")));
  delete r;
  delete w;
  delete d;
}

void TestIndexWriterUnicode::testInvalidUTF16() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(
               make_shared<TestIndexWriter::StringSplitAnalyzer>()));
  shared_ptr<Document> doc = make_shared<Document>();

  constexpr int count = utf8Data.size() / 2;
  for (int i = 0; i < count; i++) {
    doc->push_back(
        newTextField(L"f" + to_wstring(i), utf8Data[2 * i], Field::Store::YES));
  }
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Document> doc2 = ir->document(0);
  for (int i = 0; i < count; i++) {
    assertEquals(L"field " + to_wstring(i) + L" was not indexed correctly", 1,
                 ir->docFreq(make_shared<Term>(L"f" + to_wstring(i),
                                               utf8Data[2 * i + 1])));
    assertEquals(L"field " + to_wstring(i) + L" is incorrect",
                 utf8Data[2 * i + 1],
                 doc2->getField(L"f" + to_wstring(i))->stringValue());
  }
  delete ir;
  delete dir;
}

void TestIndexWriterUnicode::testTermUTF16SortOrder() 
{
  shared_ptr<Random> rnd = random();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(rnd, dir);
  shared_ptr<Document> d = make_shared<Document>();
  // Single segment
  shared_ptr<Field> f = newStringField(L"f", L"", Field::Store::NO);
  d->push_back(f);
  std::deque<wchar_t> chars(2);
  shared_ptr<Set<wstring>> *const allTerms = unordered_set<wstring>();

  int num = atLeast(200);
  for (int i = 0; i < num; i++) {

    const wstring s;
    if (rnd->nextBoolean()) {
      // Single char
      if (rnd->nextBoolean()) {
        // Above surrogates
        chars[0] = static_cast<wchar_t>(
            getInt(rnd, 1 + UnicodeUtil::UNI_SUR_LOW_END, 0xffff));
      } else {
        // Below surrogates
        chars[0] = static_cast<wchar_t>(
            getInt(rnd, 0, UnicodeUtil::UNI_SUR_HIGH_START - 1));
      }
      s = wstring(chars, 0, 1);
    } else {
      // Surrogate pair
      chars[0] = static_cast<wchar_t>(getInt(
          rnd, UnicodeUtil::UNI_SUR_HIGH_START, UnicodeUtil::UNI_SUR_HIGH_END));
      assertTrue((static_cast<int>(chars[0])) >=
                     UnicodeUtil::UNI_SUR_HIGH_START &&
                 (static_cast<int>(chars[0])) <= UnicodeUtil::UNI_SUR_HIGH_END);
      chars[1] = static_cast<wchar_t>(getInt(
          rnd, UnicodeUtil::UNI_SUR_LOW_START, UnicodeUtil::UNI_SUR_LOW_END));
      s = wstring(chars, 0, 2);
    }
    allTerms->add(s);
    f->setStringValue(s);

    writer->addDocument(d);

    if ((1 + i) % 42 == 0) {
      writer->commit();
    }
  }

  shared_ptr<IndexReader> r = writer->getReader();

  // Test each sub-segment
  for (auto ctx : r->leaves()) {
    checkTermsOrder(ctx->reader(), allTerms, false);
  }
  checkTermsOrder(r, allTerms, true);

  // Test multi segment
  delete r;

  writer->forceMerge(1);

  // Test single segment
  r = writer->getReader();
  checkTermsOrder(r, allTerms, true);
  delete r;

  delete writer;
  delete dir;
}
} // namespace org::apache::lucene::index