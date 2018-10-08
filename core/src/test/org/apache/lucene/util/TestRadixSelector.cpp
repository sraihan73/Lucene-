using namespace std;

#include "TestRadixSelector.h"

namespace org::apache::lucene::util
{

void TestRadixSelector::testSelect()
{
  for (int iter = 0; iter < 100; ++iter) {
    doTestSelect();
  }
}

void TestRadixSelector::doTestSelect()
{
  constexpr int from = random()->nextInt(5);
  constexpr int to = from + TestUtil::nextInt(random(), 1, 10000);
  constexpr int maxLen = TestUtil::nextInt(random(), 1, 12);
  std::deque<std::shared_ptr<BytesRef>> arr(from + to + random()->nextInt(5));
  for (int i = 0; i < arr.size(); ++i) {
    std::deque<char> bytes(TestUtil::nextInt(random(), 0, maxLen));
    random()->nextBytes(bytes);
    arr[i] = make_shared<BytesRef>(bytes);
  }
  doTest(arr, from, to, maxLen);
}

void TestRadixSelector::testSharedPrefixes()
{
  for (int iter = 0; iter < 100; ++iter) {
    doTestSharedPrefixes();
  }
}

void TestRadixSelector::doTestSharedPrefixes()
{
  constexpr int from = random()->nextInt(5);
  constexpr int to = from + TestUtil::nextInt(random(), 1, 10000);
  constexpr int maxLen = TestUtil::nextInt(random(), 1, 12);
  std::deque<std::shared_ptr<BytesRef>> arr(from + to + random()->nextInt(5));
  for (int i = 0; i < arr.size(); ++i) {
    std::deque<char> bytes(TestUtil::nextInt(random(), 0, maxLen));
    random()->nextBytes(bytes);
    arr[i] = make_shared<BytesRef>(bytes);
  }
  constexpr int sharedPrefixLength =
      min(arr[0]->length, TestUtil::nextInt(random(), 1, maxLen));
  for (int i = 1; i < arr.size(); ++i) {
    System::arraycopy(arr[0]->bytes, arr[0]->offset, arr[i]->bytes,
                      arr[i]->offset, min(sharedPrefixLength, arr[i]->length));
  }
  doTest(arr, from, to, maxLen);
}

void TestRadixSelector::doTest(std::deque<std::shared_ptr<BytesRef>> &arr,
                               int from, int to, int maxLen)
{
  constexpr int k = TestUtil::nextInt(random(), from, to - 1);

  std::deque<std::shared_ptr<BytesRef>> expected = arr.clone();
  Arrays::sort(expected, from, to);

  std::deque<std::shared_ptr<BytesRef>> actual = arr.clone();
  constexpr int enforcedMaxLen =
      random()->nextBoolean() ? maxLen : numeric_limits<int>::max();
  shared_ptr<RadixSelector> selector =
      make_shared<RadixSelectorAnonymousInnerClass>(shared_from_this(), k,
                                                    actual, enforcedMaxLen);
  selector->select(from, to, k);

  assertEquals(expected[k], actual[k]);
  for (int i = 0; i < actual.size(); ++i) {
    if (i < from || i >= to) {
      assertSame(arr[i], actual[i]);
    } else if (i <= k) {
      assertTrue(actual[i]->compareTo(actual[k]) <= 0);
    } else {
      assertTrue(actual[i]->compareTo(actual[k]) >= 0);
    }
  }
}

TestRadixSelector::RadixSelectorAnonymousInnerClass::
    RadixSelectorAnonymousInnerClass(
        shared_ptr<TestRadixSelector> outerInstance, int k,
        deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> &actual,
        int enforcedMaxLen)
    : RadixSelector(enforcedMaxLen)
{
  this->outerInstance = outerInstance;
  this->k = k;
  this->actual = actual;
  this->enforcedMaxLen = enforcedMaxLen;
}

void TestRadixSelector::RadixSelectorAnonymousInnerClass::swap(int i, int j)
{
  ArrayUtil::swap(actual, i, j);
}

int TestRadixSelector::RadixSelectorAnonymousInnerClass::byteAt(int i, int k)
{
  assertTrue(k < enforcedMaxLen);
  shared_ptr<BytesRef> b = actual[i];
  if (k >= b->length) {
    return -1;
  } else {
    return Byte::toUnsignedInt(b->bytes[b->offset + k]);
  }
}
} // namespace org::apache::lucene::util