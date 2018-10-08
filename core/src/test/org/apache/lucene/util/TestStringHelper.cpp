using namespace std;

#include "TestStringHelper.h"

namespace org::apache::lucene::util
{

void TestStringHelper::testBytesDifference()
{
  shared_ptr<BytesRef> left = make_shared<BytesRef>(L"foobar");
  shared_ptr<BytesRef> right = make_shared<BytesRef>(L"foozo");
  assertEquals(3, StringHelper::bytesDifference(left, right));
  assertEquals(2, StringHelper::bytesDifference(make_shared<BytesRef>(L"foo"),
                                                make_shared<BytesRef>(L"for")));
  assertEquals(
      2, StringHelper::bytesDifference(make_shared<BytesRef>(L"foo1234"),
                                       make_shared<BytesRef>(L"for1234")));
  assertEquals(1, StringHelper::bytesDifference(make_shared<BytesRef>(L"foo"),
                                                make_shared<BytesRef>(L"fz")));
  assertEquals(0, StringHelper::bytesDifference(make_shared<BytesRef>(L"foo"),
                                                make_shared<BytesRef>(L"g")));
  assertEquals(3,
               StringHelper::bytesDifference(make_shared<BytesRef>(L"foo"),
                                             make_shared<BytesRef>(L"food")));
  // we can detect terms are out of order if we see a duplicate
  expectThrows(invalid_argument::typeid, [&]() {
    StringHelper::bytesDifference(make_shared<BytesRef>(L"ab"),
                                  make_shared<BytesRef>(L"ab"));
  });
}

void TestStringHelper::testStartsWith()
{
  shared_ptr<BytesRef> ref = make_shared<BytesRef>(L"foobar");
  shared_ptr<BytesRef> slice = make_shared<BytesRef>(L"foo");
  assertTrue(StringHelper::startsWith(ref, slice));
}

void TestStringHelper::testEndsWith()
{
  shared_ptr<BytesRef> ref = make_shared<BytesRef>(L"foobar");
  shared_ptr<BytesRef> slice = make_shared<BytesRef>(L"bar");
  assertTrue(StringHelper::endsWith(ref, slice));
}

void TestStringHelper::testStartsWithWhole()
{
  shared_ptr<BytesRef> ref = make_shared<BytesRef>(L"foobar");
  shared_ptr<BytesRef> slice = make_shared<BytesRef>(L"foobar");
  assertTrue(StringHelper::startsWith(ref, slice));
}

void TestStringHelper::testEndsWithWhole()
{
  shared_ptr<BytesRef> ref = make_shared<BytesRef>(L"foobar");
  shared_ptr<BytesRef> slice = make_shared<BytesRef>(L"foobar");
  assertTrue(StringHelper::endsWith(ref, slice));
}

void TestStringHelper::testMurmurHash3() 
{
  // Hashes computed using murmur3_32 from https://code.google.com/p/pyfasthash
  assertEquals(0xf6a5c420, StringHelper::murmurhash3_x86_32(
                               make_shared<BytesRef>(L"foo"), 0));
  assertEquals(0xcd018ef6, StringHelper::murmurhash3_x86_32(
                               make_shared<BytesRef>(L"foo"), 16));
  assertEquals(0x111e7435, StringHelper::murmurhash3_x86_32(
                               make_shared<BytesRef>(
                                   L"You want weapons? We're in a library! "
                                   L"Books! The best weapons in the world!"),
                               0));
  assertEquals(0x2c628cd0, StringHelper::murmurhash3_x86_32(
                               make_shared<BytesRef>(
                                   L"You want weapons? We're in a library! "
                                   L"Books! The best weapons in the world!"),
                               3476));
}

void TestStringHelper::testSortKeyLength() 
{
  assertEquals(3, StringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                              make_shared<BytesRef>(L"for")));
  assertEquals(3,
               StringHelper::sortKeyLength(make_shared<BytesRef>(L"foo1234"),
                                           make_shared<BytesRef>(L"for1234")));
  assertEquals(2, StringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                              make_shared<BytesRef>(L"fz")));
  assertEquals(1, StringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                              make_shared<BytesRef>(L"g")));
  assertEquals(4, StringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                              make_shared<BytesRef>(L"food")));
  // we can detect terms are out of order if we see a duplicate
  expectThrows(invalid_argument::typeid, [&]() {
    StringHelper::sortKeyLength(make_shared<BytesRef>(L"ab"),
                                make_shared<BytesRef>(L"ab"));
  });
}
} // namespace org::apache::lucene::util