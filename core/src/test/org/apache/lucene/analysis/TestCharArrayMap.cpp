using namespace std;

#include "TestCharArrayMap.h"

namespace org::apache::lucene::analysis
{
using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCharArrayMap::doRandom(int iter, bool ignoreCase)
{
  shared_ptr<CharArrayMap<int>> map_obj =
      make_shared<CharArrayMap<int>>(1, ignoreCase);
  unordered_map<wstring, int> hmap = unordered_map<wstring, int>();

  std::deque<wchar_t> key;
  for (int i = 0; i < iter; i++) {
    int len = random()->nextInt(5);
    key = std::deque<wchar_t>(len);
    for (int j = 0; j < key.size(); j++) {
      key[j] = static_cast<wchar_t>(random()->nextInt(127));
    }
    wstring keyStr = wstring(key);
    wstring hmapKey = ignoreCase ? keyStr.toLowerCase(Locale::ROOT) : keyStr;

    int val = random()->nextInt();

    any o1 = map_obj->put(key, val);
    any o2 = hmap.emplace(hmapKey, val);
    assertEquals(o1, o2);

    // add it again with the string method
    assertEquals(val, map_obj->put(keyStr, val).value());

    assertEquals(val, map_obj->get(key, 0, key.size()).intValue());
    assertEquals(val, map_obj->get(key).intValue());
    assertEquals(val, map_obj->get(keyStr).intValue());

    assertEquals(hmap.size(), map_obj->size());
  }
}

void TestCharArrayMap::testCharArrayMap()
{
  int num = 5 * RANDOM_MULTIPLIER;
  for (int i = 0; i < num; i++) { // pump this up for more random testing
    doRandom(1000, false);
    doRandom(1000, true);
  }
}

void TestCharArrayMap::testMethods()
{
  shared_ptr<CharArrayMap<int>> cm = make_shared<CharArrayMap<int>>(2, false);
  unordered_map<wstring, int> hm = unordered_map<wstring, int>();
  hm.emplace(L"foo", 1);
  hm.emplace(L"bar", 2);
  cm->putAll(hm);
  assertEquals(hm.size(), cm->size());
  hm.emplace(L"baz", 3);
  cm->putAll(hm);
  assertEquals(hm.size(), cm->size());

  shared_ptr<CharArraySet> cs = cm->keySet();
  int n = 0;
  for (auto o : cs) {
    assertTrue(cm->containsKey(o));
    std::deque<wchar_t> co = static_cast<std::deque<wchar_t>>(o);
    assertTrue(cm->containsKey(co, 0, co.size()));
    n++;
  }
  assertEquals(hm.size(), n);
  assertEquals(hm.size(), cs->size());
  assertEquals(cm->size(), cs->size());
  cs->clear();
  assertEquals(0, cs->size());
  assertEquals(0, cm->size());
  // keySet() should not allow adding new keys
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { cs->add(L"test"); });

  cm->putAll(hm);
  assertEquals(hm.size(), cs->size());
  assertEquals(cm->size(), cs->size());

  CharArrayMap<int>::const_iterator iter1 = cm->entrySet()->begin();
  n = 0;
  while (iter1 != cm->end()) {
    shared_ptr<unordered_map::Entry<any, int>> entry = *iter1;
    any key = entry->getKey();
    optional<int> val = entry->getValue();
    assertEquals(cm->get(key), val);
    entry->setValue(val * 100);
    assertEquals(val * 100, static_cast<int>(cm->get(key)));
    n++;
    iter1++;
  }
  assertEquals(hm.size(), n);
  cm->clear();
  cm->putAll(hm);
  assertEquals(cm->size(), n);

  shared_ptr<CharArrayMap<int>::EntryIterator> iter2 = cm->entrySet()->begin();
  n = 0;
  while (iter2->hasNext()) {
    std::deque<wchar_t> keyc = iter2->nextKey();
    optional<int> val = iter2->currentValue();
    assertEquals(hm[wstring(keyc)], val);
    iter2->setValue(val * 100);
    assertEquals(val * 100, static_cast<int>(cm->get(keyc)));
    n++;
    iter2++;
  }
  assertEquals(hm.size(), n);

  cm->entrySet()->clear();
  assertEquals(0, cm->size());
  assertEquals(0, cm->entrySet()->size());
  assertTrue(cm->isEmpty());
}

void TestCharArrayMap::testModifyOnUnmodifiable()
{
  shared_ptr<CharArrayMap<int>> map_obj = make_shared<CharArrayMap<int>>(2, false);
  map_obj->put(L"foo", 1);
  map_obj->put(L"bar", 2);
  constexpr int size = map_obj->size();
  assertEquals(2, size);
  assertTrue(map_obj->containsKey(L"foo"));
  assertEquals(1, map_obj->get(L"foo").intValue());
  assertTrue(map_obj->containsKey(L"bar"));
  assertEquals(2, map_obj->get(L"bar").intValue());

  map_obj = CharArrayMap::unmodifiableMap(map_obj);
  assertEquals(L"Map size changed due to unmodifiableMap call", size,
               map_obj->size());
  wstring NOT_IN_MAP = L"SirGallahad";
  assertFalse(L"Test std::wstring already exists in map_obj",
              map_obj->containsKey(NOT_IN_MAP));
  assertNull(L"Test std::wstring already exists in map_obj", map_obj->get(NOT_IN_MAP));

  try {
    map_obj->put(NOT_IN_MAP.toCharArray(), 3);
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable map_obj",
                map_obj->containsKey(NOT_IN_MAP));
    assertNull(L"Test std::wstring has been added to unmodifiable map_obj",
               map_obj->get(NOT_IN_MAP));
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->put(NOT_IN_MAP, 3);
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable map_obj",
                map_obj->containsKey(NOT_IN_MAP));
    assertNull(L"Test std::wstring has been added to unmodifiable map_obj",
               map_obj->get(NOT_IN_MAP));
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->put(make_shared<StringBuilder>(NOT_IN_MAP), 3);
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable map_obj",
                map_obj->containsKey(NOT_IN_MAP));
    assertNull(L"Test std::wstring has been added to unmodifiable map_obj",
               map_obj->get(NOT_IN_MAP));
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->clear();
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->entrySet()->clear();
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->keySet()->clear();
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->put(static_cast<any>(NOT_IN_MAP), 3);
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable map_obj",
                map_obj->containsKey(NOT_IN_MAP));
    assertNull(L"Test std::wstring has been added to unmodifiable map_obj",
               map_obj->get(NOT_IN_MAP));
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  try {
    map_obj->putAll(Collections::singletonMap(NOT_IN_MAP, 3));
    fail(L"Modified unmodifiable map_obj");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable map_obj",
                map_obj->containsKey(NOT_IN_MAP));
    assertNull(L"Test std::wstring has been added to unmodifiable map_obj",
               map_obj->get(NOT_IN_MAP));
    assertEquals(L"Size of unmodifiable map_obj has changed", size, map_obj->size());
  }

  assertTrue(map_obj->containsKey(L"foo"));
  assertEquals(1, map_obj->get(L"foo").intValue());
  assertTrue(map_obj->containsKey(L"bar"));
  assertEquals(2, map_obj->get(L"bar").intValue());
}

void TestCharArrayMap::testToString()
{
  shared_ptr<CharArrayMap<int>> cm = make_shared<CharArrayMap<int>>(
      Collections::singletonMap(L"test", 1), false);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[test]", cm->keySet()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[1]", cm->values()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[test=1]", cm->entrySet()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"{test=1}", cm->toString());
  cm->put(L"test2", 2);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(cm->keySet()->toString()->find(L", ") != wstring::npos);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(cm->values()->toString()->contains(L", "));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(cm->entrySet()->toString()->contains(L", "));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(cm->toString()->find(L", ") != wstring::npos);
}
} // namespace org::apache::lucene::analysis