using namespace std;

#include "TestCollectionUtil.h"

namespace org::apache::lucene::util
{

deque<int> TestCollectionUtil::createRandomList(int maxSize)
{
  shared_ptr<Random> *const rnd = random();
  const std::deque<optional<int>> a =
      std::deque<optional<int>>(rnd->nextInt(maxSize) + 1);
  for (int i = 0; i < a.size(); i++) {
    a[i] = static_cast<Integer>(rnd->nextInt(a.size()));
  }
  return Arrays::asList(a);
}

void TestCollectionUtil::testIntroSort()
{
  for (int i = 0, c = atLeast(500); i < c; i++) {
    deque<int> list1 = createRandomList(2000), list2 = deque<int>(list1);
    CollectionUtil::introSort(list1);
    sort(list2.begin(), list2.end());
    assertEquals(list2, list1);

    list1 = createRandomList(2000);
    list2 = deque<>(list1);
    CollectionUtil::introSort(list1, Collections::reverseOrder());
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(list2,
    // java.util.Collections.reverseOrder());
    sort(list2.begin(), list2.end(), Collections::reverseOrder());
    assertEquals(list2, list1);
    // reverse back, so we can test that completely backwards sorted array
    // (worst case) is working:
    CollectionUtil::introSort(list1);
    sort(list2.begin(), list2.end());
    assertEquals(list2, list1);
  }
}

void TestCollectionUtil::testTimSort()
{
  for (int i = 0, c = atLeast(500); i < c; i++) {
    deque<int> list1 = createRandomList(2000), list2 = deque<int>(list1);
    CollectionUtil::timSort(list1);
    sort(list2.begin(), list2.end());
    assertEquals(list2, list1);

    list1 = createRandomList(2000);
    list2 = deque<>(list1);
    CollectionUtil::timSort(list1, Collections::reverseOrder());
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(list2,
    // java.util.Collections.reverseOrder());
    sort(list2.begin(), list2.end(), Collections::reverseOrder());
    assertEquals(list2, list1);
    // reverse back, so we can test that completely backwards sorted array
    // (worst case) is working:
    CollectionUtil::timSort(list1);
    sort(list2.begin(), list2.end());
    assertEquals(list2, list1);
  }
}

void TestCollectionUtil::testEmptyListSort()
{
  // should produce no exceptions
  deque<int> deque =
      Arrays::asList(std::deque<optional<int>>(0)); // LUCENE-2989
  CollectionUtil::introSort(deque);
  CollectionUtil::timSort(deque);
  CollectionUtil::introSort(deque, Collections::reverseOrder());
  CollectionUtil::timSort(deque, Collections::reverseOrder());

  // check that empty non-random access lists pass sorting without ex (as
  // sorting is not needed)
  deque = deque<>();
  CollectionUtil::introSort(deque);
  CollectionUtil::timSort(deque);
  CollectionUtil::introSort(deque, Collections::reverseOrder());
  CollectionUtil::timSort(deque, Collections::reverseOrder());
}

void TestCollectionUtil::testOneElementListSort()
{
  // check that one-element non-random access lists pass sorting without ex (as
  // sorting is not needed)
  deque<int> deque = deque<int>();
  deque.push_back(1);
  CollectionUtil::introSort(deque);
  CollectionUtil::timSort(deque);
  CollectionUtil::introSort(deque, Collections::reverseOrder());
  CollectionUtil::timSort(deque, Collections::reverseOrder());
}
} // namespace org::apache::lucene::util