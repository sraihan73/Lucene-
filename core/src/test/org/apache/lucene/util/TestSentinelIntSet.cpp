using namespace std;

#include "TestSentinelIntSet.h"

namespace org::apache::lucene::util
{
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void test() throws Exception
void TestSentinelIntSet::test() 
{
  shared_ptr<SentinelIntSet> set = make_shared<SentinelIntSet>(10, -1);
  assertFalse(set->exists(50));
  set->put(50);
  assertTrue(set->exists(50));
  assertEquals(1, set->size());
  assertEquals(-11, set->find(10));
  assertEquals(1, set->size());
  set->clear();
  assertEquals(0, set->size());
  assertEquals(50, set->hash(50));
  // force a rehash
  for (int i = 0; i < 20; i++) {
    set->put(i);
  }
  assertEquals(20, set->size());
  assertEquals(24, set->rehashCount);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRandom() throws Exception
void TestSentinelIntSet::testRandom() 
{
  for (int i = 0; i < 10000; i++) {
    int initSz = random()->nextInt(20);
    int num = random()->nextInt(30);
    int maxVal = (random()->nextBoolean()
                      ? random()->nextInt(50)
                      : random()->nextInt(numeric_limits<int>::max())) +
                 1;

    unordered_set<int> a = unordered_set<int>(initSz);
    shared_ptr<SentinelIntSet> b = make_shared<SentinelIntSet>(initSz, -1);

    for (int j = 0; j < num; j++) {
      int val = random()->nextInt(maxVal);
      bool exists = !a.insert(val);
      bool existsB = b->exists(val);
      assertEquals(exists, existsB);
      int slot = b->find(val);
      assertEquals(exists, slot >= 0);
      b->put(val);

      assertEquals(a.size(), b->size());
    }
  }
}
} // namespace org::apache::lucene::util