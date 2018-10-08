using namespace std;

#include "TestCharsRef.h"

namespace org::apache::lucene::util
{

void TestCharsRef::testUTF16InUTF8Order()
{
  constexpr int numStrings = atLeast(1000);
  std::deque<std::shared_ptr<BytesRef>> utf8(numStrings);
  std::deque<std::shared_ptr<CharsRef>> utf16(numStrings);

  for (int i = 0; i < numStrings; i++) {
    wstring s = TestUtil::randomUnicodeString(random());
    utf8[i] = make_shared<BytesRef>(s);
    utf16[i] = make_shared<CharsRef>(s);
  }

  Arrays::sort(utf8);
  Arrays::sort(utf16, CharsRef::getUTF16SortedAsUTF8Comparator());

  for (int i = 0; i < numStrings; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(utf8[i]->utf8ToString(), utf16[i]->toString());
  }
}

void TestCharsRef::testAppend()
{
  shared_ptr<CharsRefBuilder> ref = make_shared<CharsRefBuilder>();
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int numStrings = atLeast(10);
  for (int i = 0; i < numStrings; i++) {
    std::deque<wchar_t> charArray =
        TestUtil::randomRealisticUnicodeString(random(), 1, 100).toCharArray();
    int offset = random()->nextInt(charArray.size());
    int length = charArray.size() - offset;
    builder->append(charArray, offset, length);
    ref->append(charArray, offset, length);
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(builder->toString(), ref->get()->toString());
}

void TestCharsRef::testCopy()
{
  int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<CharsRefBuilder> ref = make_shared<CharsRefBuilder>();
    std::deque<wchar_t> charArray =
        TestUtil::randomRealisticUnicodeString(random(), 1, 100).toCharArray();
    int offset = random()->nextInt(charArray.size());
    int length = charArray.size() - offset;
    wstring str = wstring(charArray, offset, length);
    ref->copyChars(charArray, offset, length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(str, ref->toString());
  }
}

void TestCharsRef::testCharSequenceCharAt()
{
  shared_ptr<CharsRef> c = make_shared<CharsRef>(L"abc");

  assertEquals(L'b', c->charAt(1));

  expectThrows(out_of_range::typeid, [&]() { c->charAt(-1); });

  expectThrows(out_of_range::typeid, [&]() { c->charAt(3); });
}

void TestCharsRef::testCharSequenceSubSequence()
{
  std::deque<std::shared_ptr<std::wstring>> sequences = {
      make_shared<CharsRef>(L"abc"),
      make_shared<CharsRef>((wstring(L"0abc")).toCharArray(), 1, 3),
      make_shared<CharsRef>((wstring(L"abc0")).toCharArray(), 0, 3),
      make_shared<CharsRef>((wstring(L"0abc0")).toCharArray(), 1, 3)};

  for (auto c : sequences) {
    doTestSequence(c);
  }
}

void TestCharsRef::doTestSequence(shared_ptr<std::wstring> c)
{

  // slice
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"a", c->substr(0, 1)->toString());
  // mid subsequence
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b", c->substr(1, 1)->toString());
  // end subsequence
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"bc", c->substr(1, 2)->toString());
  // empty subsequence
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"", c->substr(0, 0)->toString());

  expectThrows(out_of_range::typeid, [&]() { c->substr(-1, 2); });

  expectThrows(out_of_range::typeid, [&]() { c->substr(0, -1); });

  expectThrows(out_of_range::typeid, [&]() { c->substr(0, 4); });

  expectThrows(out_of_range::typeid, [&]() { c->substr(2, -1); });
}
} // namespace org::apache::lucene::util