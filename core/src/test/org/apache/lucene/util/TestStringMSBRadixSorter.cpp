using namespace std;

#include "TestStringMSBRadixSorter.h"

namespace org::apache::lucene::util
{

void TestStringMSBRadixSorter::test(
    std::deque<std::shared_ptr<BytesRef>> &refs, int len)
{
  std::deque<std::shared_ptr<BytesRef>> expected = Arrays::copyOf(refs, len);
  Arrays::sort(expected);

  make_shared<StringMSBRadixSorterAnonymousInnerClass>(shared_from_this(), refs)
      .sort(0, len);
  std::deque<std::shared_ptr<BytesRef>> actual = Arrays::copyOf(refs, len);
  assertArrayEquals(expected, actual);
}

TestStringMSBRadixSorter::StringMSBRadixSorterAnonymousInnerClass::
    StringMSBRadixSorterAnonymousInnerClass(
        shared_ptr<TestStringMSBRadixSorter> outerInstance,
        deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> &refs)
{
  this->outerInstance = outerInstance;
  this->refs = refs;
}

shared_ptr<BytesRef>
TestStringMSBRadixSorter::StringMSBRadixSorterAnonymousInnerClass::get(int i)
{
  return refs[i];
}

void TestStringMSBRadixSorter::StringMSBRadixSorterAnonymousInnerClass::swap(
    int i, int j)
{
  shared_ptr<BytesRef> tmp = refs[i];
  refs[i] = refs[j];
  refs[j] = tmp;
}

void TestStringMSBRadixSorter::testEmpty()
{
  test(std::deque<std::shared_ptr<BytesRef>>(random()->nextInt(5)), 0);
}

void TestStringMSBRadixSorter::testOneValue()
{
  shared_ptr<BytesRef> bytes =
      make_shared<BytesRef>(TestUtil::randomSimpleString(random()));
  test(std::deque<std::shared_ptr<BytesRef>>{bytes}, 1);
}

void TestStringMSBRadixSorter::testTwoValues()
{
  shared_ptr<BytesRef> bytes1 =
      make_shared<BytesRef>(TestUtil::randomSimpleString(random()));
  shared_ptr<BytesRef> bytes2 =
      make_shared<BytesRef>(TestUtil::randomSimpleString(random()));
  test(std::deque<std::shared_ptr<BytesRef>>{bytes1, bytes2}, 2);
}

void TestStringMSBRadixSorter::testRandom(int commonPrefixLen, int maxLen)
{
  std::deque<char> commonPrefix(commonPrefixLen);
  random()->nextBytes(commonPrefix);
  constexpr int len = random()->nextInt(100000);
  std::deque<std::shared_ptr<BytesRef>> bytes(len + random()->nextInt(50));
  for (int i = 0; i < len; ++i) {
    std::deque<char> b(commonPrefixLen + random()->nextInt(maxLen));
    random()->nextBytes(b);
    System::arraycopy(commonPrefix, 0, b, 0, commonPrefixLen);
    bytes[i] = make_shared<BytesRef>(b);
  }
  test(bytes, len);
}

void TestStringMSBRadixSorter::testRandom()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(0, 10);
  }
}

void TestStringMSBRadixSorter::testRandomWithLotsOfDuplicates()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(0, 2);
  }
}

void TestStringMSBRadixSorter::testRandomWithSharedPrefix()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(TestUtil::nextInt(random(), 1, 30), 10);
  }
}

void TestStringMSBRadixSorter::testRandomWithSharedPrefixAndLotsOfDuplicates()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(TestUtil::nextInt(random(), 1, 30), 2);
  }
}
} // namespace org::apache::lucene::util