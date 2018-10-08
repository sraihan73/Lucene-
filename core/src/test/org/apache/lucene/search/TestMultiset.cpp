using namespace std;

#include "TestMultiset.h"

namespace org::apache::lucene::search
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiset::testDuplicatesMatter()
{
  shared_ptr<Multiset<int>> s1 = make_shared<Multiset<int>>();
  shared_ptr<Multiset<int>> s2 = make_shared<Multiset<int>>();
  assertEquals(s1->size(), s2->size());
  assertEquals(s1, s2);

  assertTrue(s1->add(42));
  assertTrue(s2->add(42));
  assertEquals(s1, s2);

  s2->add(42);
  assertFalse(s1->equals(s2));

  s1->add(43);
  s1->add(43);
  s2->add(43);
  assertEquals(s1->size(), s2->size());
  assertFalse(s1->equals(s2));
}

template <typename T>
unordered_map<T, int> TestMultiset::toCountMap(shared_ptr<Multiset<T>> set)
{
  unordered_map<T, int> map_obj = unordered_map<T, int>();
  int recomputedSize = 0;
  for (auto element : set) {
    add(map_obj, element);
    recomputedSize += 1;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(set->toString(), recomputedSize, set->size());
  return map_obj;
}

template <typename T>
void TestMultiset::add(unordered_map<T, int> &map_obj, T element)
{
  map_obj.emplace(element, map_obj.getOrDefault(element, 0) + 1);
}

template <typename T>
void TestMultiset::remove(unordered_map<T, int> &map_obj, T element)
{
  optional<int> count = map_obj[element];
  if (!count) {
    return;
  } else if (count.value() == 1) {
    map_obj.erase(element);
  } else {
    map_obj.emplace(element, count - 1);
  }
}

void TestMultiset::testRandom()
{
  unordered_map<int, int> reference = unordered_map<int, int>();
  shared_ptr<Multiset<int>> multiset = make_shared<Multiset<int>>();
  constexpr int iters = atLeast(100);
  for (int i = 0; i < iters; ++i) {
    constexpr int value = random()->nextInt(10);
    switch (random()->nextInt(10)) {
    case 0:
    case 1:
    case 2:
      remove(reference, value);
      multiset->remove(value);
      break;
    case 3:
      reference.clear();
      multiset->clear();
      break;
    default:
      add(reference, value);
      multiset->add(value);
      break;
    }
    assertEquals(reference, toCountMap(multiset));
  }
}
} // namespace org::apache::lucene::search