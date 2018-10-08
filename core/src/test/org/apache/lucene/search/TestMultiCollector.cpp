using namespace std;

#include "TestMultiCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

TestMultiCollector::TerminateAfterCollector::TerminateAfterCollector(
    shared_ptr<Collector> in_, int terminateAfter)
    : FilterCollector(in_), terminateAfter(terminateAfter)
{
}

shared_ptr<LeafCollector>
TestMultiCollector::TerminateAfterCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  if (count >= terminateAfter) {
    throw make_shared<CollectionTerminatedException>();
  }
  shared_ptr<LeafCollector> *const in_ =
      FilterCollector::getLeafCollector(context);
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this());
}

TestMultiCollector::TerminateAfterCollector::
    FilterLeafCollectorAnonymousInnerClass::
        FilterLeafCollectorAnonymousInnerClass(
            shared_ptr<TerminateAfterCollector> outerInstance)
    : FilterLeafCollector(in_)
{
  this->outerInstance = outerInstance;
}

void TestMultiCollector::TerminateAfterCollector::
    FilterLeafCollectorAnonymousInnerClass::collect(int doc) 
{
  if (outerInstance->count >= outerInstance->terminateAfter) {
    throw make_shared<CollectionTerminatedException>();
  }
  outerInstance->super->collect(doc);
  outerInstance->count++;
}

TestMultiCollector::SetScorerCollector::SetScorerCollector(
    shared_ptr<Collector> in_, shared_ptr<AtomicBoolean> setScorerCalled)
    : FilterCollector(in_), setScorerCalled(setScorerCalled)
{
}

shared_ptr<LeafCollector>
TestMultiCollector::SetScorerCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this(), FilterCollector::getLeafCollector(context));
}

TestMultiCollector::SetScorerCollector::FilterLeafCollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass(
        shared_ptr<SetScorerCollector> outerInstance,
        shared_ptr<org::apache::lucene::search::LeafCollector> getLeafCollector)
    : FilterLeafCollector(getLeafCollector)
{
  this->outerInstance = outerInstance;
}

void TestMultiCollector::SetScorerCollector::
    FilterLeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  outerInstance->super->setScorer(scorer);
  outerInstance->setScorerCalled->set(true);
}

void TestMultiCollector::testCollectionTerminatedExceptionHandling() throw(
    IOException)
{
  constexpr int iters = atLeast(3);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> w =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = TestUtil::nextInt(random(), 100, 1000);
    shared_ptr<Document> *const doc = make_shared<Document>();
    for (int i = 0; i < numDocs; ++i) {
      w->addDocument(doc);
    }
    shared_ptr<IndexReader> *const reader = w->getReader();
    delete w;
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    unordered_map<std::shared_ptr<TotalHitCountCollector>, int> expectedCounts =
        unordered_map<std::shared_ptr<TotalHitCountCollector>, int>();
    deque<std::shared_ptr<Collector>> collectors =
        deque<std::shared_ptr<Collector>>();
    constexpr int numCollectors = TestUtil::nextInt(random(), 1, 5);
    for (int i = 0; i < numCollectors; ++i) {
      constexpr int terminateAfter = random()->nextInt(numDocs + 10);
      constexpr int expectedCount =
          terminateAfter > numDocs ? numDocs : terminateAfter;
      shared_ptr<TotalHitCountCollector> collector =
          make_shared<TotalHitCountCollector>();
      expectedCounts.emplace(collector, expectedCount);
      collectors.push_back(
          make_shared<TerminateAfterCollector>(collector, terminateAfter));
    }
    searcher->search(make_shared<MatchAllDocsQuery>(),
                     MultiCollector::wrap(collectors));
    for (auto expectedCount : expectedCounts) {
      TestUtil::assertEquals(expectedCount.second::intValue(),
                             expectedCount.first::getTotalHits());
    }
    delete reader;
    delete dir;
  }
}

void TestMultiCollector::testSetScorerAfterCollectionTerminated() throw(
    IOException)
{
  shared_ptr<Collector> collector1 = make_shared<TotalHitCountCollector>();
  shared_ptr<Collector> collector2 = make_shared<TotalHitCountCollector>();

  shared_ptr<AtomicBoolean> setScorerCalled1 = make_shared<AtomicBoolean>();
  collector1 = make_shared<SetScorerCollector>(collector1, setScorerCalled1);

  shared_ptr<AtomicBoolean> setScorerCalled2 = make_shared<AtomicBoolean>();
  collector2 = make_shared<SetScorerCollector>(collector2, setScorerCalled2);

  collector1 = make_shared<TerminateAfterCollector>(collector1, 1);
  collector2 = make_shared<TerminateAfterCollector>(collector2, 2);

  shared_ptr<Scorer> scorer = make_shared<FakeScorer>();

  deque<std::shared_ptr<Collector>> collectors =
      Arrays::asList(collector1, collector2);
  Collections::shuffle(collectors, random());
  shared_ptr<Collector> collector = MultiCollector::wrap(collectors);

  shared_ptr<LeafCollector> leafCollector =
      collector->getLeafCollector(nullptr);
  leafCollector->setScorer(scorer);
  assertTrue(setScorerCalled1->get());
  assertTrue(setScorerCalled2->get());

  leafCollector->collect(0);
  leafCollector->collect(1);

  setScorerCalled1->set(false);
  setScorerCalled2->set(false);
  leafCollector->setScorer(scorer);
  assertFalse(setScorerCalled1->get());
  assertTrue(setScorerCalled2->get());

  expectThrows(CollectionTerminatedException::typeid,
               [&]() { leafCollector->collect(1); });

  setScorerCalled1->set(false);
  setScorerCalled2->set(false);
  leafCollector->setScorer(scorer);
  assertFalse(setScorerCalled1->get());
  assertFalse(setScorerCalled2->get());
}
} // namespace org::apache::lucene::search