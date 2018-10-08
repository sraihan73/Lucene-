using namespace std;

#include "TestCachingCollector.h"

namespace org::apache::lucene::search
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestCachingCollector::MockScorer::MockScorer() : Scorer((Weight) nullptr) {}

float TestCachingCollector::MockScorer::score()  { return 0; }

int TestCachingCollector::MockScorer::docID() { return 0; }

shared_ptr<DocIdSetIterator> TestCachingCollector::MockScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>();
}

void TestCachingCollector::NoOpCollector::collect(int doc)  {}

bool TestCachingCollector::NoOpCollector::needsScores() { return false; }

void TestCachingCollector::testBasic() 
{
  for (auto cacheScores : std::deque<bool>{false, true}) {
    shared_ptr<CachingCollector> cc = CachingCollector::create(
        make_shared<NoOpCollector>(), cacheScores, 1.0);
    shared_ptr<LeafCollector> acc = cc->getLeafCollector(nullptr);
    acc->setScorer(make_shared<MockScorer>());

    // collect 1000 docs
    for (int i = 0; i < 1000; i++) {
      acc->collect(i);
    }

    // now replay them
    cc->replay(
        make_shared<SimpleCollectorAnonymousInnerClass>(shared_from_this()));
  }
}

TestCachingCollector::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestCachingCollector> outerInstance)
{
  this->outerInstance = outerInstance;
  prevDocID = -1;
}

void TestCachingCollector::SimpleCollectorAnonymousInnerClass::collect(int doc)
{
  assertEquals(prevDocID + 1, doc);
  prevDocID = doc;
}

bool TestCachingCollector::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void TestCachingCollector::testIllegalStateOnReplay() 
{
  shared_ptr<CachingCollector> cc = CachingCollector::create(
      make_shared<NoOpCollector>(), true, 50 * ONE_BYTE);
  shared_ptr<LeafCollector> acc = cc->getLeafCollector(nullptr);
  acc->setScorer(make_shared<MockScorer>());

  // collect 130 docs, this should be enough for triggering cache abort.
  for (int i = 0; i < 130; i++) {
    acc->collect(i);
  }

  assertFalse(L"CachingCollector should not be cached due to low memory limit",
              cc->isCached());

  expectThrows(IllegalStateException::typeid,
               [&]() { cc->replay(make_shared<NoOpCollector>()); });
}

void TestCachingCollector::testCachedArraysAllocation() 
{
  // tests the cached arrays allocation -- if the 'nextLength' was too high,
  // caching would terminate even if a smaller length would suffice.

  // set RAM limit enough for 150 docs + random(10000)
  int numDocs = random()->nextInt(10000) + 150;
  for (auto cacheScores : std::deque<bool>{false, true}) {
    int bytesPerDoc = cacheScores ? 8 : 4;
    shared_ptr<CachingCollector> cc =
        CachingCollector::create(make_shared<NoOpCollector>(), cacheScores,
                                 bytesPerDoc * ONE_BYTE * numDocs);
    shared_ptr<LeafCollector> acc = cc->getLeafCollector(nullptr);
    acc->setScorer(make_shared<MockScorer>());
    for (int i = 0; i < numDocs; i++) {
      acc->collect(i);
    }
    assertTrue(cc->isCached());

    // The 151's document should terminate caching
    acc->collect(numDocs);
    assertFalse(cc->isCached());
  }
}

void TestCachingCollector::testNoWrappedCollector() 
{
  for (auto cacheScores : std::deque<bool>{false, true}) {
    // create w/ null wrapped collector, and test that the methods work
    shared_ptr<CachingCollector> cc =
        CachingCollector::create(cacheScores, 50 * ONE_BYTE);
    shared_ptr<LeafCollector> acc = cc->getLeafCollector(nullptr);
    acc->setScorer(make_shared<MockScorer>());
    acc->collect(0);

    assertTrue(cc->isCached());
    cc->replay(make_shared<NoOpCollector>());
  }
}
} // namespace org::apache::lucene::search