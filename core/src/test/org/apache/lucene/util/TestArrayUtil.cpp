using namespace std;

#include "TestArrayUtil.h"

namespace org::apache::lucene::util
{

void TestArrayUtil::testGrowth()
{
  int currentSize = 0;
  int64_t copyCost = 0;

  // Make sure ArrayUtil hits Integer.MAX_VALUE, if we insist:
  while (currentSize != ArrayUtil::MAX_ARRAY_LENGTH) {
    int nextSize = ArrayUtil::oversize(1 + currentSize,
                                       RamUsageEstimator::NUM_BYTES_OBJECT_REF);
    assertTrue(nextSize > currentSize);
    if (currentSize > 0) {
      copyCost += currentSize;
      double copyCostPerElement = (static_cast<double>(copyCost)) / currentSize;
      assertTrue(L"cost " + to_wstring(copyCostPerElement),
                 copyCostPerElement < 10.0);
    }
    currentSize = nextSize;
  }
}

void TestArrayUtil::testMaxSize()
{
  // intentionally pass invalid elemSizes:
  for (int elemSize = 0; elemSize < 10; elemSize++) {
    assertEquals(ArrayUtil::MAX_ARRAY_LENGTH,
                 ArrayUtil::oversize(ArrayUtil::MAX_ARRAY_LENGTH, elemSize));
    assertEquals(
        ArrayUtil::MAX_ARRAY_LENGTH,
        ArrayUtil::oversize(ArrayUtil::MAX_ARRAY_LENGTH - 1, elemSize));
  }
}

void TestArrayUtil::testTooBig()
{
  expectThrows(invalid_argument::typeid, [&]() {
    ArrayUtil::oversize(ArrayUtil::MAX_ARRAY_LENGTH + 1, 1);
  });
}

void TestArrayUtil::testExactLimit()
{
  assertEquals(ArrayUtil::MAX_ARRAY_LENGTH,
               ArrayUtil::oversize(ArrayUtil::MAX_ARRAY_LENGTH, 1));
}

void TestArrayUtil::testInvalidElementSizes()
{
  shared_ptr<Random> *const rnd = random();
  constexpr int num = atLeast(10000);
  for (int iter = 0; iter < num; iter++) {
    constexpr int minTargetSize = rnd->nextInt(ArrayUtil::MAX_ARRAY_LENGTH);
    constexpr int elemSize = rnd->nextInt(11);
    constexpr int v = ArrayUtil::oversize(minTargetSize, elemSize);
    assertTrue(v >= minTargetSize);
  }
}

int TestArrayUtil::parseInt(const wstring &s)
{
  int start = random()->nextInt(5);
  std::deque<wchar_t> chars(s.length() + start + random()->nextInt(4));
  s.getChars(0, s.length(), chars, start);
  return ArrayUtil::parseInt(chars, start, s.length());
}

void TestArrayUtil::testParseInt() 
{
  expectThrows(NumberFormatException::typeid, [&]() { parseInt(L""); });

  expectThrows(NumberFormatException::typeid, [&]() { parseInt(L"foo"); });

  expectThrows(NumberFormatException::typeid, [&]() {
    parseInt(wstring::valueOf(numeric_limits<int64_t>::max()));
  });

  expectThrows(NumberFormatException::typeid, [&]() { parseInt(L"0.34"); });

  int test = parseInt(L"1");
  assertTrue(to_wstring(test) + L" does not equal: " + to_wstring(1),
             test == 1);
  test = parseInt(L"-10000");
  assertTrue(to_wstring(test) + L" does not equal: " + to_wstring(-10000),
             test == -10000);
  test = parseInt(L"1923");
  assertTrue(to_wstring(test) + L" does not equal: " + to_wstring(1923),
             test == 1923);
  test = parseInt(L"-1");
  assertTrue(to_wstring(test) + L" does not equal: " + to_wstring(-1),
             test == -1);
  test = ArrayUtil::parseInt((wstring(L"foo 1923 bar")).toCharArray(), 4, 4);
  assertTrue(to_wstring(test) + L" does not equal: " + to_wstring(1923),
             test == 1923);
}

std::deque<optional<int>> TestArrayUtil::createRandomArray(int maxSize)
{
  shared_ptr<Random> *const rnd = random();
  const std::deque<optional<int>> a =
      std::deque<optional<int>>(rnd->nextInt(maxSize) + 1);
  for (int i = 0; i < a.size(); i++) {
    a[i] = static_cast<Integer>(rnd->nextInt(a.size()));
  }
  return a;
}

void TestArrayUtil::testIntroSort()
{
  int num = atLeast(50);
  for (int i = 0; i < num; i++) {
    std::deque<optional<int>> a1 = createRandomArray(2000), a2 = a1.clone();
    ArrayUtil::introSort(a1);
    Arrays::sort(a2);
    assertArrayEquals(a2, a1);

    a1 = createRandomArray(2000);
    a2 = a1.clone();
    ArrayUtil::introSort(a1, Collections::reverseOrder());
    Arrays::sort(a2, Collections::reverseOrder());
    assertArrayEquals(a2, a1);
    // reverse back, so we can test that completely backwards sorted array
    // (worst case) is working:
    ArrayUtil::introSort(a1);
    Arrays::sort(a2);
    assertArrayEquals(a2, a1);
  }
}

std::deque<optional<int>> TestArrayUtil::createSparseRandomArray(int maxSize)
{
  shared_ptr<Random> *const rnd = random();
  const std::deque<optional<int>> a =
      std::deque<optional<int>>(rnd->nextInt(maxSize) + 1);
  for (int i = 0; i < a.size(); i++) {
    a[i] = static_cast<Integer>(rnd->nextInt(2));
  }
  return a;
}

void TestArrayUtil::testQuickToHeapSortFallback()
{
  int num = atLeast(50);
  for (int i = 0; i < num; i++) {
    std::deque<optional<int>> a1 = createSparseRandomArray(40000),
                               a2 = a1.clone();
    ArrayUtil::introSort(a1);
    Arrays::sort(a2);
    assertArrayEquals(a2, a1);
  }
}

void TestArrayUtil::testTimSort()
{
  int num = atLeast(50);
  for (int i = 0; i < num; i++) {
    std::deque<optional<int>> a1 = createRandomArray(2000), a2 = a1.clone();
    ArrayUtil::timSort(a1);
    Arrays::sort(a2);
    assertArrayEquals(a2, a1);

    a1 = createRandomArray(2000);
    a2 = a1.clone();
    ArrayUtil::timSort(a1, Collections::reverseOrder());
    Arrays::sort(a2, Collections::reverseOrder());
    assertArrayEquals(a2, a1);
    // reverse back, so we can test that completely backwards sorted array
    // (worst case) is working:
    ArrayUtil::timSort(a1);
    Arrays::sort(a2);
    assertArrayEquals(a2, a1);
  }
}

TestArrayUtil::Item::Item(int val, int order) : val(val), order(order) {}

int TestArrayUtil::Item::compareTo(shared_ptr<Item> other)
{
  return this->order - other->order;
}

wstring TestArrayUtil::Item::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(val);
}

void TestArrayUtil::testMergeSortStability()
{
  shared_ptr<Random> *const rnd = random();
  std::deque<std::shared_ptr<Item>> items(100);
  for (int i = 0; i < items.size(); i++) {
    // half of the items have value but same order. The value of this items is
    // sorted, so they should always be in order after sorting. The other half
    // has defined order, but no (-1) value (they should appear after all above,
    // when sorted).
    constexpr bool equal = rnd->nextBoolean();
    items[i] = make_shared<Item>(equal ? (i + 1) : -1,
                                 equal ? 0 : (rnd->nextInt(1000) + 1));
  }

  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Before: " << Arrays->toString(items) << endl;
  }
  // if you replace this with ArrayUtil.quickSort(), test should fail:
  ArrayUtil::timSort(items);
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Sorted: " << Arrays->toString(items) << endl;
  }

  shared_ptr<Item> last = items[0];
  for (int i = 1; i < items.size(); i++) {
    shared_ptr<Item> *const act = items[i];
    if (act->order == 0) {
      // order of "equal" items should be not mixed up
      assertTrue(act->val > last->val);
    }
    assertTrue(act->order >= last->order);
    last = act;
  }
}

void TestArrayUtil::testTimSortStability()
{
  shared_ptr<Random> *const rnd = random();
  std::deque<std::shared_ptr<Item>> items(100);
  for (int i = 0; i < items.size(); i++) {
    // half of the items have value but same order. The value of this items is
    // sorted, so they should always be in order after sorting. The other half
    // has defined order, but no (-1) value (they should appear after all above,
    // when sorted).
    constexpr bool equal = rnd->nextBoolean();
    items[i] = make_shared<Item>(equal ? (i + 1) : -1,
                                 equal ? 0 : (rnd->nextInt(1000) + 1));
  }

  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Before: " << Arrays->toString(items) << endl;
  }
  // if you replace this with ArrayUtil.quickSort(), test should fail:
  ArrayUtil::timSort(items);
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Sorted: " << Arrays->toString(items) << endl;
  }

  shared_ptr<Item> last = items[0];
  for (int i = 1; i < items.size(); i++) {
    shared_ptr<Item> *const act = items[i];
    if (act->order == 0) {
      // order of "equal" items should be not mixed up
      assertTrue(act->val > last->val);
    }
    assertTrue(act->order >= last->order);
    last = act;
  }
}

void TestArrayUtil::testEmptyArraySort()
{
  std::deque<optional<int>> a(0);
  ArrayUtil::introSort(a);
  ArrayUtil::timSort(a);
  ArrayUtil::introSort(a, Collections::reverseOrder());
  ArrayUtil::timSort(a, Collections::reverseOrder());
}

void TestArrayUtil::testSelect()
{
  for (int iter = 0; iter < 100; ++iter) {
    doTestSelect();
  }
}

void TestArrayUtil::doTestSelect()
{
  constexpr int from = random()->nextInt(5);
  constexpr int to = from + TestUtil::nextInt(random(), 1, 10000);
  constexpr int max = random()->nextBoolean() ? random()->nextInt(100)
                                              : random()->nextInt(100000);
  std::deque<optional<int>> arr(from + to + random()->nextInt(5));
  for (int i = 0; i < arr.size(); ++i) {
    arr[i] = TestUtil::nextInt(random(), 0, max);
  }
  constexpr int k = TestUtil::nextInt(random(), from, to - 1);

  std::deque<optional<int>> expected = arr.clone();
  Arrays::sort(expected, from, to);

  std::deque<optional<int>> actual = arr.clone();
  ArrayUtil::select(actual, from, to, k, Comparator::naturalOrder());

  assertEquals(expected[k], actual[k]);
  for (int i = 0; i < actual.size(); ++i) {
    if (i < from || i >= to) {
      assertSame(arr[i], actual[i]);
    } else if (i <= k) {
      assertTrue(actual[i] <= actual[k]);
    } else {
      assertTrue(actual[i] >= actual[k]);
    }
  }
}
} // namespace org::apache::lucene::util