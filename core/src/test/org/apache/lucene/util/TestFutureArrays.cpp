using namespace std;

#include "TestFutureArrays.h"

namespace org::apache::lucene::util
{

void TestFutureArrays::testByteMismatch()
{
  assertEquals(1,
               FutureArrays::mismatch(bytes(L"ab"), 0, 2, bytes(L"ac"), 0, 2));
  assertEquals(0,
               FutureArrays::mismatch(bytes(L"ab"), 0, 2, bytes(L"b"), 0, 1));
  assertEquals(-1,
               FutureArrays::mismatch(bytes(L"ab"), 0, 2, bytes(L"ab"), 0, 2));
  assertEquals(1,
               FutureArrays::mismatch(bytes(L"ab"), 0, 2, bytes(L"a"), 0, 1));
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::mismatch(bytes(L"ab"), 2, 1, bytes(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::mismatch(bytes(L"ab"), 2, 1, bytes(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::mismatch(nullptr, 0, 2, bytes(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::mismatch(bytes(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::mismatch(bytes(L"ab"), 0, 3, bytes(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::mismatch(bytes(L"ab"), 0, 2, bytes(L"a"), 0, 2);
  });
}

void TestFutureArrays::testCharMismatch()
{
  assertEquals(1,
               FutureArrays::mismatch(chars(L"ab"), 0, 2, chars(L"ac"), 0, 2));
  assertEquals(0,
               FutureArrays::mismatch(chars(L"ab"), 0, 2, chars(L"b"), 0, 1));
  assertEquals(-1,
               FutureArrays::mismatch(chars(L"ab"), 0, 2, chars(L"ab"), 0, 2));
  assertEquals(1,
               FutureArrays::mismatch(chars(L"ab"), 0, 2, chars(L"a"), 0, 1));
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::mismatch(chars(L"ab"), 2, 1, chars(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::mismatch(chars(L"ab"), 2, 1, chars(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::mismatch(nullptr, 0, 2, chars(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::mismatch(chars(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::mismatch(chars(L"ab"), 0, 3, chars(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::mismatch(chars(L"ab"), 0, 2, chars(L"a"), 0, 2);
  });
}

void TestFutureArrays::testByteCompareUnsigned()
{
  assertEquals(1, Integer::signum(FutureArrays::compareUnsigned(
                      bytes(L"ab"), 0, 2, bytes(L"a"), 0, 1)));
  assertEquals(1, Integer::signum(FutureArrays::compareUnsigned(
                      bytes(L"ab"), 0, 2, bytes(L"aa"), 0, 2)));
  assertEquals(0, Integer::signum(FutureArrays::compareUnsigned(
                      bytes(L"ab"), 0, 2, bytes(L"ab"), 0, 2)));
  assertEquals(-1, Integer::signum(FutureArrays::compareUnsigned(
                       bytes(L"a"), 0, 1, bytes(L"ab"), 0, 2)));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compareUnsigned(bytes(L"ab"), 2, 1, bytes(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compareUnsigned(bytes(L"ab"), 2, 1, bytes(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compareUnsigned(nullptr, 0, 2, bytes(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compareUnsigned(bytes(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compareUnsigned(bytes(L"ab"), 0, 3, bytes(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compareUnsigned(bytes(L"ab"), 0, 2, bytes(L"a"), 0, 2);
  });
}

void TestFutureArrays::testCharCompare()
{
  assertEquals(1, Integer::signum(FutureArrays::compare(chars(L"ab"), 0, 2,
                                                        chars(L"a"), 0, 1)));
  assertEquals(1, Integer::signum(FutureArrays::compare(chars(L"ab"), 0, 2,
                                                        chars(L"aa"), 0, 2)));
  assertEquals(0, Integer::signum(FutureArrays::compare(chars(L"ab"), 0, 2,
                                                        chars(L"ab"), 0, 2)));
  assertEquals(-1, Integer::signum(FutureArrays::compare(chars(L"a"), 0, 1,
                                                         chars(L"ab"), 0, 2)));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compare(chars(L"ab"), 2, 1, chars(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compare(chars(L"ab"), 2, 1, chars(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compare(nullptr, 0, 2, chars(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compare(chars(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compare(chars(L"ab"), 0, 3, chars(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compare(chars(L"ab"), 0, 2, chars(L"a"), 0, 2);
  });
}

void TestFutureArrays::testIntCompare()
{
  assertEquals(1, Integer::signum(FutureArrays::compare(ints(L"ab"), 0, 2,
                                                        ints(L"a"), 0, 1)));
  assertEquals(1, Integer::signum(FutureArrays::compare(ints(L"ab"), 0, 2,
                                                        ints(L"aa"), 0, 2)));
  assertEquals(0, Integer::signum(FutureArrays::compare(ints(L"ab"), 0, 2,
                                                        ints(L"ab"), 0, 2)));
  assertEquals(-1, Integer::signum(FutureArrays::compare(ints(L"a"), 0, 1,
                                                         ints(L"ab"), 0, 2)));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compare(ints(L"ab"), 2, 1, ints(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compare(ints(L"ab"), 2, 1, ints(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compare(nullptr, 0, 2, ints(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compare(ints(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compare(ints(L"ab"), 0, 3, ints(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compare(ints(L"ab"), 0, 2, ints(L"a"), 0, 2);
  });
}

void TestFutureArrays::testLongCompare()
{
  assertEquals(1, Integer::signum(FutureArrays::compare(longs(L"ab"), 0, 2,
                                                        longs(L"a"), 0, 1)));
  assertEquals(1, Integer::signum(FutureArrays::compare(longs(L"ab"), 0, 2,
                                                        longs(L"aa"), 0, 2)));
  assertEquals(0, Integer::signum(FutureArrays::compare(longs(L"ab"), 0, 2,
                                                        longs(L"ab"), 0, 2)));
  assertEquals(-1, Integer::signum(FutureArrays::compare(longs(L"a"), 0, 1,
                                                         longs(L"ab"), 0, 2)));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compare(longs(L"ab"), 2, 1, longs(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::compare(longs(L"ab"), 2, 1, longs(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compare(nullptr, 0, 2, longs(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::compare(longs(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compare(longs(L"ab"), 0, 3, longs(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::compare(longs(L"ab"), 0, 2, longs(L"a"), 0, 2);
  });
}

void TestFutureArrays::testByteEquals()
{
  assertFalse(FutureArrays::equals(bytes(L"ab"), 0, 2, bytes(L"a"), 0, 1));
  assertFalse(FutureArrays::equals(bytes(L"ab"), 0, 2, bytes(L"aa"), 0, 2));
  assertTrue(FutureArrays::equals(bytes(L"ab"), 0, 2, bytes(L"ab"), 0, 2));
  assertFalse(FutureArrays::equals(bytes(L"a"), 0, 1, bytes(L"ab"), 0, 2));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(bytes(L"ab"), 2, 1, bytes(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(bytes(L"ab"), 2, 1, bytes(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(nullptr, 0, 2, bytes(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(bytes(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(bytes(L"ab"), 0, 3, bytes(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(bytes(L"ab"), 0, 2, bytes(L"a"), 0, 2);
  });
}

void TestFutureArrays::testCharEquals()
{
  assertFalse(FutureArrays::equals(chars(L"ab"), 0, 2, chars(L"a"), 0, 1));
  assertFalse(FutureArrays::equals(chars(L"ab"), 0, 2, chars(L"aa"), 0, 2));
  assertTrue(FutureArrays::equals(chars(L"ab"), 0, 2, chars(L"ab"), 0, 2));
  assertFalse(FutureArrays::equals(chars(L"a"), 0, 1, chars(L"ab"), 0, 2));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(chars(L"ab"), 2, 1, chars(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(chars(L"ab"), 2, 1, chars(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(nullptr, 0, 2, chars(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(chars(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(chars(L"ab"), 0, 3, chars(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(chars(L"ab"), 0, 2, chars(L"a"), 0, 2);
  });
}

void TestFutureArrays::testIntEquals()
{
  assertFalse(FutureArrays::equals(ints(L"ab"), 0, 2, ints(L"a"), 0, 1));
  assertFalse(FutureArrays::equals(ints(L"ab"), 0, 2, ints(L"aa"), 0, 2));
  assertTrue(FutureArrays::equals(ints(L"ab"), 0, 2, ints(L"ab"), 0, 2));
  assertFalse(FutureArrays::equals(ints(L"a"), 0, 1, ints(L"ab"), 0, 2));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(ints(L"ab"), 2, 1, ints(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(ints(L"ab"), 2, 1, ints(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(nullptr, 0, 2, ints(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(ints(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(ints(L"ab"), 0, 3, ints(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(ints(L"ab"), 0, 2, ints(L"a"), 0, 2);
  });
}

void TestFutureArrays::testLongEquals()
{
  assertFalse(FutureArrays::equals(longs(L"ab"), 0, 2, longs(L"a"), 0, 1));
  assertFalse(FutureArrays::equals(longs(L"ab"), 0, 2, longs(L"aa"), 0, 2));
  assertTrue(FutureArrays::equals(longs(L"ab"), 0, 2, longs(L"ab"), 0, 2));
  assertFalse(FutureArrays::equals(longs(L"a"), 0, 1, longs(L"ab"), 0, 2));

  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(longs(L"ab"), 2, 1, longs(L"a"), 0, 1);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    FutureArrays::equals(longs(L"ab"), 2, 1, longs(L"a"), 1, 0);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(nullptr, 0, 2, longs(L"a"), 0, 1);
  });
  expectThrows(NullPointerException::typeid, [&]() {
    FutureArrays::equals(longs(L"ab"), 0, 2, nullptr, 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(longs(L"ab"), 0, 3, longs(L"a"), 0, 1);
  });
  expectThrows(out_of_range::typeid, [&]() {
    FutureArrays::equals(longs(L"ab"), 0, 2, longs(L"a"), 0, 2);
  });
}

std::deque<char> TestFutureArrays::bytes(const wstring &s)
{
  return s.getBytes(StandardCharsets::UTF_8);
}

std::deque<wchar_t> TestFutureArrays::chars(const wstring &s)
{
  return s.toCharArray();
}

std::deque<int> TestFutureArrays::ints(const wstring &s)
{
  std::deque<int> ints(s.length());
  for (int i = 0; i < s.length(); i++) {
    ints[i] = s[i];
  }
  return ints;
}

std::deque<int64_t> TestFutureArrays::longs(const wstring &s)
{
  std::deque<int64_t> longs(s.length());
  for (int i = 0; i < s.length(); i++) {
    longs[i] = s[i];
  }
  return longs;
}
} // namespace org::apache::lucene::util