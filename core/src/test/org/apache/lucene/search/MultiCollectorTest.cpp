using namespace std;

#include "MultiCollectorTest.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

void MultiCollectorTest::DummyCollector::collect(int doc) 
{
  collectCalled = true;
}

void MultiCollectorTest::DummyCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  setNextReaderCalled = true;
}

void MultiCollectorTest::DummyCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  setScorerCalled = true;
}

bool MultiCollectorTest::DummyCollector::needsScores() { return true; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNullCollectors() throws Exception
void MultiCollectorTest::testNullCollectors() 
{
  // Tests that the collector rejects all null collectors.
  expectThrows(invalid_argument::typeid, [&]() {
    MultiCollector::wrap({nullptr, nullptr});
  });

  // Tests that the collector handles some null collectors well. If it
  // doesn't, an NPE would be thrown.
  shared_ptr<Collector> c = MultiCollector::wrap(
      {make_shared<DummyCollector>(), nullptr, make_shared<DummyCollector>()});
  assertTrue(std::dynamic_pointer_cast<MultiCollector>(c) != nullptr);
  shared_ptr<LeafCollector> *const ac = c->getLeafCollector(nullptr);
  ac->collect(1);
  c->getLeafCollector(nullptr);
  c->getLeafCollector(nullptr)->setScorer(make_shared<FakeScorer>());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSingleCollector() throws Exception
void MultiCollectorTest::testSingleCollector() 
{
  // Tests that if a single Collector is input, it is returned (and not
  // MultiCollector).
  shared_ptr<DummyCollector> dc = make_shared<DummyCollector>();
  assertSame(dc, MultiCollector::wrap({dc}));
  assertSame(dc, MultiCollector::wrap({dc, nullptr}));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCollector() throws Exception
void MultiCollectorTest::testCollector() 
{
  // Tests that the collector delegates calls to input collectors properly.

  // Tests that the collector handles some null collectors well. If it
  // doesn't, an NPE would be thrown.
  std::deque<std::shared_ptr<DummyCollector>> dcs = {
      make_shared<DummyCollector>(), make_shared<DummyCollector>()};
  shared_ptr<Collector> c = MultiCollector::wrap(dcs);
  shared_ptr<LeafCollector> ac = c->getLeafCollector(nullptr);
  ac->collect(1);
  ac = c->getLeafCollector(nullptr);
  ac->setScorer(make_shared<FakeScorer>());

  for (auto dc : dcs) {
    assertTrue(dc->collectCalled);
    assertTrue(dc->setNextReaderCalled);
    assertTrue(dc->setScorerCalled);
  }
}

shared_ptr<Collector> MultiCollectorTest::collector(bool needsScores,
                                                    type_info expectedScorer)
{
  return make_shared<CollectorAnonymousInnerClass>(needsScores, expectedScorer);
}

MultiCollectorTest::CollectorAnonymousInnerClass::CollectorAnonymousInnerClass(
    bool needsScores, type_info expectedScorer)
{
  this->needsScores = needsScores;
  this->expectedScorer = expectedScorer;
}

shared_ptr<LeafCollector>
MultiCollectorTest::CollectorAnonymousInnerClass::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this());
}

MultiCollectorTest::CollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass::LeafCollectorAnonymousInnerClass(
        shared_ptr<CollectorAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

void MultiCollectorTest::CollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  assertEquals(outerInstance->expectedScorer, scorer->getClass());
}

void MultiCollectorTest::CollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass::collect(int doc) 
{
}

bool MultiCollectorTest::CollectorAnonymousInnerClass::needsScores()
{
  return needsScores;
}

void MultiCollectorTest::testCacheScoresIfNecessary() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  iw->addDocument(make_shared<Document>());
  iw->commit();
  shared_ptr<DirectoryReader> reader = iw->getReader();
  delete iw;

  shared_ptr<LeafReaderContext> *const ctx = reader->leaves()->get(0);

  expectThrows(AssertionError::typeid, [&]() {
    collector(false, ScoreCachingWrappingScorer::typeid)
        ->getLeafCollector(ctx)
        ->setScorer(make_shared<FakeScorer>());
  });

  // no collector needs scores => no caching
  shared_ptr<Collector> c1 = collector(false, FakeScorer::typeid);
  shared_ptr<Collector> c2 = collector(false, FakeScorer::typeid);
  MultiCollector::wrap({c1, c2})->getLeafCollector(ctx)->setScorer(
      make_shared<FakeScorer>());

  // only one collector needs scores => no caching
  c1 = collector(true, FakeScorer::typeid);
  c2 = collector(false, FakeScorer::typeid);
  MultiCollector::wrap({c1, c2})->getLeafCollector(ctx)->setScorer(
      make_shared<FakeScorer>());

  // several collectors need scores => caching
  c1 = collector(true, ScoreCachingWrappingScorer::typeid);
  c2 = collector(true, ScoreCachingWrappingScorer::typeid);
  MultiCollector::wrap({c1, c2})->getLeafCollector(ctx)->setScorer(
      make_shared<FakeScorer>());

  reader->close();
  delete dir;
}
} // namespace org::apache::lucene::search