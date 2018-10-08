using namespace std;

#include "TestWeakIdentityMap.h"

namespace org::apache::lucene::util
{

void TestWeakIdentityMap::testSimpleHashMap()
{
  shared_ptr<WeakIdentityMap<wstring, wstring>> *const map_obj =
      WeakIdentityMap::newHashMap(random()->nextBoolean());
  // we keep strong references to the keys,
  // so WeakIdentityMap will not forget about them:
  wstring key1 = wstring(L"foo");
  wstring key2 = wstring(L"foo");
  wstring key3 = wstring(L"foo");

  assertNotSame(key1, key2);
  assertEquals(key1, key2);
  assertNotSame(key1, key3);
  assertEquals(key1, key3);
  assertNotSame(key2, key3);
  assertEquals(key2, key3);

  // try null key & check its iterator also return null:
  map_obj->put(L"", L"null");
  {
    Iterator<wstring> it = map_obj->keyIterator();
    assertTrue(it->hasNext());
    assertNull(it->next());
    assertFalse(it->hasNext());
    assertFalse(it->hasNext());
  }
  // 2 more keys:
  map_obj->put(key1, L"bar1");
  map_obj->put(key2, L"bar2");

  assertEquals(3, map_obj->size());

  assertEquals(L"bar1", map_obj->get(key1));
  assertEquals(L"bar2", map_obj->get(key2));
  assertEquals(nullptr, map_obj->get(key3));
  assertEquals(L"null", map_obj->get(nullptr));

  assertTrue(map_obj->containsKey(key1));
  assertTrue(map_obj->containsKey(key2));
  assertFalse(map_obj->containsKey(key3));
  assertTrue(map_obj->containsKey(nullptr));

  // repeat and check that we have no double entries
  map_obj->put(key1, L"bar1");
  map_obj->put(key2, L"bar2");
  map_obj->put(L"", L"null");

  assertEquals(3, map_obj->size());

  assertEquals(L"bar1", map_obj->get(key1));
  assertEquals(L"bar2", map_obj->get(key2));
  assertEquals(nullptr, map_obj->get(key3));
  assertEquals(L"null", map_obj->get(nullptr));

  assertTrue(map_obj->containsKey(key1));
  assertTrue(map_obj->containsKey(key2));
  assertFalse(map_obj->containsKey(key3));
  assertTrue(map_obj->containsKey(nullptr));

  map_obj->remove(nullptr);
  assertEquals(2, map_obj->size());
  map_obj->remove(key1);
  assertEquals(1, map_obj->size());
  map_obj->put(key1, L"bar1");
  map_obj->put(key2, L"bar2");
  map_obj->put(key3, L"bar3");
  assertEquals(3, map_obj->size());

  int c = 0, keysAssigned = 0;
  for (shared_ptr<Iterator<wstring>> it = map_obj->keyIterator(); it->hasNext();) {
    assertTrue(it->hasNext()); // try again, should return same result!
    const wstring k = it->next();
    assertTrue(k == key1 || k == key2 | k == key3);
    keysAssigned += (k == key1) ? 1 : ((k == key2) ? 2 : 4);
    c++;
  }
  assertEquals(3, c);
  assertEquals(L"all keys must have been seen", 1 + 2 + 4, keysAssigned);

  c = 0;
  for (shared_ptr<Iterator<wstring>> it = map_obj->valueIterator();
       it->hasNext();) {
    const wstring v = it->next();
    assertTrue(StringHelper::startsWith(v, L"bar"));
    c++;
  }
  assertEquals(3, c);

  // clear strong refs
  key1 = key2 = key3 = L"";

  // check that GC does not cause problems in reap() method, wait 1 second and
  // let GC work:
  int size = map_obj->size();
  for (int i = 0; size > 0 && i < 10; i++) {
    try {
      System::runFinalization();
      System::gc();
      int newSize = map_obj->size();
      assertTrue(L"previousSize(" + to_wstring(size) + L")>=newSize(" +
                     to_wstring(newSize) + L")",
                 size >= newSize);
      size = newSize;
      delay(100LL);
      c = 0;
      for (shared_ptr<Iterator<wstring>> it = map_obj->keyIterator();
           it->hasNext();) {
        assertNotNull(it->next());
        c++;
      }
      newSize = map_obj->size();
      assertTrue(L"previousSize(" + to_wstring(size) + L")>=iteratorSize(" +
                     to_wstring(c) + L")",
                 size >= c);
      assertTrue(L"iteratorSize(" + to_wstring(c) + L")>=newSize(" +
                     to_wstring(newSize) + L")",
                 c >= newSize);
      size = newSize;
    } catch (const InterruptedException &ie) {
    }
  }

  map_obj->clear();
  assertEquals(0, map_obj->size());
  assertTrue(map_obj->isEmpty());

  Iterator<wstring> it = map_obj->keyIterator();
  assertFalse(it->hasNext());
  expectThrows(NoSuchElementException::typeid, [&]() { it->next(); });

  key1 = wstring(L"foo");
  key2 = wstring(L"foo");
  map_obj->put(key1, L"bar1");
  map_obj->put(key2, L"bar2");
  assertEquals(2, map_obj->size());

  map_obj->clear();
  assertEquals(0, map_obj->size());
  assertTrue(map_obj->isEmpty());
}

void TestWeakIdentityMap::testConcurrentHashMap() 
{
  // don't make threadCount and keyCount random, otherwise easily OOMs or fails
  // otherwise:
  constexpr int threadCount = 8, keyCount = 1024;
  shared_ptr<ExecutorService> *const exec = Executors::newFixedThreadPool(
      threadCount, make_shared<NamedThreadFactory>(L"testConcurrentHashMap"));
  shared_ptr<WeakIdentityMap<any, int>> *const map_obj =
      WeakIdentityMap::newConcurrentHashMap(random()->nextBoolean());
  // we keep strong references to the keys,
  // so WeakIdentityMap will not forget about them:
  shared_ptr<AtomicReferenceArray<any>> *const keys =
      make_shared<AtomicReferenceArray<any>>(keyCount);
  for (int j = 0; j < keyCount; j++) {
    keys->set(j, any());
  }

  try {
    for (int t = 0; t < threadCount; t++) {
      shared_ptr<Random> *const rnd = make_shared<Random>(random()->nextLong());
      exec->execute([&]() {
        constexpr int count = atLeast(rnd, 10000);
        for (int i = 0; i < count; i++) {
          constexpr int j = rnd->nextInt(keyCount);
          switch (rnd->nextInt(5)) {
          case 0:
            map_obj->put(keys->get(j), static_cast<Integer>(j));
            break;
          case 1: {
            const optional<int> v = map_obj->get(keys->get(j));
            if (v) {
              assertEquals(j, v.value());
            }
            break;
          }
          case 2:
            map_obj->remove(keys->get(j));
            break;
          case 3:
            // renew key, the old one will be GCed at some time:
            keys->set(j, any());
            break;
          case 4:
            // check iterator still working
            for (shared_ptr<Iterator<any>> it = map_obj->keyIterator();
                 it->hasNext();) {
              assertNotNull(it->next());
            }
            break;
          default:
            fail(L"Should not get here.");
          }
        }
      });
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    exec->shutdown();
    while (!exec->awaitTermination(1000LL, TimeUnit::MILLISECONDS)) {
      ;
    }
  }

  // clear strong refs
  for (int j = 0; j < keyCount; j++) {
    keys->set(j, nullptr);
  }

  // check that GC does not cause problems in reap() method:
  int size = map_obj->size();
  for (int i = 0; size > 0 && i < 10; i++) {
    try {
      System::runFinalization();
      System::gc();
      int newSize = map_obj->size();
      assertTrue(L"previousSize(" + to_wstring(size) + L")>=newSize(" +
                     to_wstring(newSize) + L")",
                 size >= newSize);
      size = newSize;
      delay(100LL);
      int c = 0;
      for (shared_ptr<Iterator<any>> it = map_obj->keyIterator(); it->hasNext();) {
        assertNotNull(it->next());
        c++;
      }
      newSize = map_obj->size();
      assertTrue(L"previousSize(" + to_wstring(size) + L")>=iteratorSize(" +
                     to_wstring(c) + L")",
                 size >= c);
      assertTrue(L"iteratorSize(" + to_wstring(c) + L")>=newSize(" +
                     to_wstring(newSize) + L")",
                 c >= newSize);
      size = newSize;
    } catch (const InterruptedException &ie) {
    }
  }
}
} // namespace org::apache::lucene::util