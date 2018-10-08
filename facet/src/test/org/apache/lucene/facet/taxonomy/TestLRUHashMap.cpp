using namespace std;

#include "TestLRUHashMap.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using LRUHashMap = org::apache::lucene::facet::taxonomy::LRUHashMap;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLRU() throws Exception
void TestLRUHashMap::testLRU() 
{
  shared_ptr<LRUHashMap<wstring, wstring>> lru =
      make_shared<LRUHashMap<wstring, wstring>>(3);
  assertEquals(0, lru->size());
  lru->put(L"one", L"Hello world");
  assertEquals(1, lru->size());
  lru->put(L"two", L"Hi man");
  assertEquals(2, lru->size());
  lru->put(L"three", L"Bonjour");
  assertEquals(3, lru->size());
  lru->put(L"four", L"Shalom");
  assertEquals(3, lru->size());
  assertNotNull(lru->get(L"three"));
  assertNotNull(lru->get(L"two"));
  assertNotNull(lru->get(L"four"));
  assertNull(lru->get(L"one"));
  lru->put(L"five", L"Yo!");
  assertEquals(3, lru->size());
  assertNull(lru->get(L"three")); // three was last used, so it got removed
  assertNotNull(lru->get(L"five"));
  lru->get(L"four");
  lru->put(L"six", L"hi");
  lru->put(L"seven", L"hey dude");
  assertEquals(3, lru->size());
  assertNull(lru->get(L"one"));
  assertNull(lru->get(L"two"));
  assertNull(lru->get(L"three"));
  assertNotNull(lru->get(L"four"));
  assertNull(lru->get(L"five"));
  assertNotNull(lru->get(L"six"));
  assertNotNull(lru->get(L"seven"));
}
} // namespace org::apache::lucene::facet::taxonomy