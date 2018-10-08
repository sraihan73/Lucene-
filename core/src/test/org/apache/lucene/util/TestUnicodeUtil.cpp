using namespace std;

#include "TestUnicodeUtil.h"

namespace org::apache::lucene::util
{

void TestUnicodeUtil::testCodePointCount()
{
  // Check invalid codepoints.
  assertcodePointCountThrowsAssertionOn(
      asByteArray({L'z', 0x80, L'z', L'z', L'z'}));
  assertcodePointCountThrowsAssertionOn(
      asByteArray({L'z', 0xc0 - 1, L'z', L'z', L'z'}));
  // Check 5-byte and longer sequences.
  assertcodePointCountThrowsAssertionOn(
      asByteArray({L'z', 0xf8, L'z', L'z', L'z'}));
  assertcodePointCountThrowsAssertionOn(
      asByteArray({L'z', 0xfc, L'z', L'z', L'z'}));
  // Check improperly terminated codepoints.
  assertcodePointCountThrowsAssertionOn(asByteArray({L'z', 0xc2}));
  assertcodePointCountThrowsAssertionOn(asByteArray({L'z', 0xe2}));
  assertcodePointCountThrowsAssertionOn(asByteArray({L'z', 0xe2, 0x82}));
  assertcodePointCountThrowsAssertionOn(asByteArray({L'z', 0xf0}));
  assertcodePointCountThrowsAssertionOn(asByteArray({L'z', 0xf0, 0xa4}));
  assertcodePointCountThrowsAssertionOn(asByteArray({L'z', 0xf0, 0xa4, 0xad}));

  // Check some typical examples (multibyte).
  assertEquals(
      0, UnicodeUtil::codePointCount(make_shared<BytesRef>(asByteArray())));
  assertEquals(3, UnicodeUtil::codePointCount(
                      make_shared<BytesRef>(asByteArray({L'z', L'z', L'z'}))));
  assertEquals(2, UnicodeUtil::codePointCount(
                      make_shared<BytesRef>(asByteArray({L'z', 0xc2, 0xa2}))));
  assertEquals(2, UnicodeUtil::codePointCount(make_shared<BytesRef>(
                      asByteArray({L'z', 0xe2, 0x82, 0xac}))));
  assertEquals(2, UnicodeUtil::codePointCount(make_shared<BytesRef>(
                      asByteArray({L'z', 0xf0, 0xa4, 0xad, 0xa2}))));

  // And do some random stuff.
  int num = atLeast(50000);
  for (int i = 0; i < num; i++) {
    const wstring s = TestUtil::randomUnicodeString(random());
    const std::deque<char> utf8 =
        std::deque<char>(UnicodeUtil::maxUTF8Length(s.length()));
    constexpr int utf8Len = UnicodeUtil::UTF16toUTF8(s, 0, s.length(), utf8);
    assertEquals(
        s.codePointCount(0, s.length()),
        UnicodeUtil::codePointCount(make_shared<BytesRef>(utf8, 0, utf8Len)));
  }
}

std::deque<char> TestUnicodeUtil::asByteArray(deque<int> &ints)
{
  std::deque<char> asByteArray(ints->length);
  for (int i = 0; i < ints->length; i++) {
    asByteArray[i] = static_cast<char>(ints[i]);
  }
  return asByteArray;
}

void TestUnicodeUtil::assertcodePointCountThrowsAssertionOn(deque<char> &bytes)
{
  expectThrows(invalid_argument::typeid, [&]() {
    UnicodeUtil::codePointCount(make_shared<BytesRef>(bytes));
  });
}

void TestUnicodeUtil::testUTF8toUTF32()
{
  std::deque<int> utf32(0);
  int num = atLeast(50000);
  for (int i = 0; i < num; i++) {
    const wstring s = TestUtil::randomUnicodeString(random());
    const std::deque<char> utf8 =
        std::deque<char>(UnicodeUtil::maxUTF8Length(s.length()));
    constexpr int utf8Len = UnicodeUtil::UTF16toUTF8(s, 0, s.length(), utf8);
    utf32 = ArrayUtil::grow(utf32, utf8Len);
    constexpr int utf32Len = UnicodeUtil::UTF8toUTF32(
        make_shared<BytesRef>(utf8, 0, utf8Len), utf32);

    std::deque<int> codePoints = s.codePoints().toArray();
    if (!FutureArrays::equals(codePoints, 0, codePoints.size(), utf32, 0,
                              codePoints.size())) {
      wcout << L"FAILED" << endl;
      for (int j = 0; j < s.length(); j++) {
        wcout << L"  char[" << j << L"]=" << Integer::toHexString(s[j]) << endl;
      }
      wcout << endl;
      assertEquals(codePoints.size(), utf32Len);
      for (int j = 0; j < codePoints.size(); j++) {
        wcout << L"  " << Integer::toHexString(utf32[j]) << L" vs "
              << Integer::toHexString(codePoints[j]) << endl;
      }
      fail(L"mismatch");
    }
  }
}

void TestUnicodeUtil::testNewString()
{
  const std::deque<int> codePoints = {
      Character::toCodePoint(Character::MIN_HIGH_SURROGATE,
                             Character::MAX_LOW_SURROGATE),
      Character::toCodePoint(Character::MAX_HIGH_SURROGATE,
                             Character::MIN_LOW_SURROGATE),
      Character::MAX_HIGH_SURROGATE, L'A', -1};

  const wstring cpString =
      L"" + Character::MIN_HIGH_SURROGATE + Character::MAX_LOW_SURROGATE +
      Character::MAX_HIGH_SURROGATE + Character::MIN_LOW_SURROGATE +
      Character::MAX_HIGH_SURROGATE + L'A';

  const std::deque<std::deque<int>> tests = {
      std::deque<int>{0, 1, 0, 2},  std::deque<int>{0, 2, 0, 4},
      std::deque<int>{1, 1, 2, 2},  std::deque<int>{1, 2, 2, 3},
      std::deque<int>{1, 3, 2, 4},  std::deque<int>{2, 2, 4, 2},
      std::deque<int>{2, 3, 0, -1}, std::deque<int>{4, 5, 0, -1},
      std::deque<int>{3, -1, 0, -1}};

  for (int i = 0; i < tests.size(); ++i) {
    std::deque<int> t = tests[i];
    int s = t[0];
    int c = t[1];
    int rs = t[2];
    int rc = t[3];

    try {
      wstring str = UnicodeUtil::newString(codePoints, s, c);
      assertFalse(rc == -1);
      assertEquals(cpString.substr(rs, rc), str);
      continue;
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (IndexOutOfBoundsException | invalid_argument e1) {
      // Ignored.
    }
    assertTrue(rc == -1);
  }
}

void TestUnicodeUtil::testUTF8UTF16CharsRef()
{
  int num = atLeast(3989);
  for (int i = 0; i < num; i++) {
    wstring unicode = TestUtil::randomRealisticUnicodeString(random());
    shared_ptr<BytesRef> ref = make_shared<BytesRef>(unicode);
    shared_ptr<CharsRefBuilder> cRef = make_shared<CharsRefBuilder>();
    cRef->copyUTF8Bytes(ref);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(cRef->toString(), unicode);
  }
}

void TestUnicodeUtil::testCalcUTF16toUTF8Length()
{
  int num = atLeast(5000);
  for (int i = 0; i < num; i++) {
    wstring unicode = TestUtil::randomUnicodeString(random());
    std::deque<char> utf8(UnicodeUtil::maxUTF8Length(unicode.length()));
    int len = UnicodeUtil::UTF16toUTF8(unicode, 0, unicode.length(), utf8);
    assertEquals(
        len, UnicodeUtil::calcUTF16toUTF8Length(unicode, 0, unicode.length()));
  }
}
} // namespace org::apache::lucene::util