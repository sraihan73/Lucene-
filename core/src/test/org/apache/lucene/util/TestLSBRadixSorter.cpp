using namespace std;

#include "TestLSBRadixSorter.h"

namespace org::apache::lucene::util
{
using PackedInts = org::apache::lucene::util::packed::PackedInts;

void TestLSBRadixSorter::test(shared_ptr<LSBRadixSorter> sorter, int maxLen)
{
  for (int iter = 0; iter < 10; ++iter) {
    constexpr int len = TestUtil::nextInt(random(), 0, maxLen);
    std::deque<int> arr(len + random()->nextInt(10));
    constexpr int numBits = random()->nextInt(31);
    constexpr int maxValue = (1 << numBits) - 1;
    for (int i = 0; i < arr.size(); ++i) {
      arr[i] = TestUtil::nextInt(random(), 0, maxValue);
    }
    test(sorter, arr, len);
  }
}

void TestLSBRadixSorter::test(shared_ptr<LSBRadixSorter> sorter,
                              std::deque<int> &arr, int len)
{
  const std::deque<int> expected = Arrays::copyOf(arr, len);
  Arrays::sort(expected);

  int numBits = 0;
  for (int i = 0; i < len; ++i) {
    numBits = max(numBits, PackedInts::bitsRequired(arr[i]));
  }

  if (random()->nextBoolean()) {
    numBits = TestUtil::nextInt(random(), numBits, 32);
  }

  sorter->sort(numBits, arr, len);
  const std::deque<int> actual = Arrays::copyOf(arr, len);
  assertArrayEquals(expected, actual);
}

void TestLSBRadixSorter::testEmpty() { test(make_shared<LSBRadixSorter>(), 0); }

void TestLSBRadixSorter::testOne() { test(make_shared<LSBRadixSorter>(), 1); }

void TestLSBRadixSorter::testTwo() { test(make_shared<LSBRadixSorter>(), 2); }

void TestLSBRadixSorter::testSimple()
{
  test(make_shared<LSBRadixSorter>(), 100);
}

void TestLSBRadixSorter::testRandom()
{
  test(make_shared<LSBRadixSorter>(), 10000);
}

void TestLSBRadixSorter::testSorted()
{
  shared_ptr<LSBRadixSorter> sorter = make_shared<LSBRadixSorter>();
  for (int iter = 0; iter < 10; ++iter) {
    std::deque<int> arr(10000);
    int a = 0;
    for (int i = 0; i < arr.size(); ++i) {
      a += random()->nextInt(10);
      arr[i] = a;
    }
    constexpr int len = TestUtil::nextInt(random(), 0, arr.size());
    test(sorter, arr, len);
  }
}
} // namespace org::apache::lucene::util