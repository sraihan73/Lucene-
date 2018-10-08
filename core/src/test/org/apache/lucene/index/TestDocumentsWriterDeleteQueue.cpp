using namespace std;

#include "TestDocumentsWriterDeleteQueue.h"

namespace org::apache::lucene::index
{
using DeleteSlice =
    org::apache::lucene::index::DocumentsWriterDeleteQueue::DeleteSlice;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using TermQuery = org::apache::lucene::search::TermQuery;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

void TestDocumentsWriterDeleteQueue::testUpdateDelteSlices() throw(
    runtime_error)
{
  shared_ptr<DocumentsWriterDeleteQueue> queue =
      make_shared<DocumentsWriterDeleteQueue>(nullptr);
  constexpr int size = 200 + random()->nextInt(500) * RANDOM_MULTIPLIER;
  std::deque<optional<int>> ids(size);
  for (int i = 0; i < ids.size(); i++) {
    ids[i] = random()->nextInt();
  }
  shared_ptr<DeleteSlice> slice1 = queue->newSlice();
  shared_ptr<DeleteSlice> slice2 = queue->newSlice();
  shared_ptr<BufferedUpdates> bd1 = make_shared<BufferedUpdates>(L"bd1");
  shared_ptr<BufferedUpdates> bd2 = make_shared<BufferedUpdates>(L"bd2");
  int last1 = 0;
  int last2 = 0;
  shared_ptr<Set<std::shared_ptr<Term>>> uniqueValues =
      unordered_set<std::shared_ptr<Term>>();
  for (int j = 0; j < ids.size(); j++) {
    optional<int> i = ids[j];
    // create an array here since we compare identity below against tailItem
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::deque<std::shared_ptr<Term>> term = {
        make_shared<Term>(L"id", i.toString())};
    uniqueValues->add(term[0]);
    queue->addDelete(term);
    if (random()->nextInt(20) == 0 || j == ids.size() - 1) {
      queue->updateSlice(slice1);
      assertTrue(slice1->isTailItem(term));
      slice1->apply(bd1, j);
      assertAllBetween(last1, j, bd1, ids);
      last1 = j + 1;
    }
    if (random()->nextInt(10) == 5 || j == ids.size() - 1) {
      queue->updateSlice(slice2);
      assertTrue(slice2->isTailItem(term));
      slice2->apply(bd2, j);
      assertAllBetween(last2, j, bd2, ids);
      last2 = j + 1;
    }
    assertEquals(j + 1, queue->numGlobalTermDeletes());
  }
  assertEquals(uniqueValues, bd1->deleteTerms.keySet());
  assertEquals(uniqueValues, bd2->deleteTerms.keySet());
  unordered_set<std::shared_ptr<Term>> frozenSet =
      unordered_set<std::shared_ptr<Term>>();
  shared_ptr<BytesRefBuilder> bytesRef = make_shared<BytesRefBuilder>();
  shared_ptr<TermIterator> iter =
      queue->freezeGlobalBuffer(nullptr)->deleteTerms->begin();
  while (iter->next() != nullptr) {
    bytesRef->copyBytes(iter->bytes);
    frozenSet.insert(make_shared<Term>(iter->field(), bytesRef->toBytesRef()));
  }
  assertEquals(uniqueValues, frozenSet);
  assertEquals(L"num deletes must be 0 after freeze", 0,
               queue->numGlobalTermDeletes());
}

void TestDocumentsWriterDeleteQueue::assertAllBetween(
    int start, int end, shared_ptr<BufferedUpdates> deletes,
    std::deque<Integer> &ids)
{
  for (int i = start; i <= end; i++) {
    assertEquals(
        static_cast<Integer>(end),
        deletes->deleteTerms[make_shared<Term>(L"id", to_wstring(ids[i]))]);
  }
}

void TestDocumentsWriterDeleteQueue::testClear()
{
  shared_ptr<DocumentsWriterDeleteQueue> queue =
      make_shared<DocumentsWriterDeleteQueue>(nullptr);
  assertFalse(queue->anyChanges());
  queue->clear();
  assertFalse(queue->anyChanges());
  constexpr int size = 200 + random()->nextInt(500) * RANDOM_MULTIPLIER;
  for (int i = 0; i < size; i++) {
    shared_ptr<Term> term = make_shared<Term>(L"id", L"" + to_wstring(i));
    if (random()->nextInt(10) == 0) {
      queue->addDelete({make_shared<TermQuery>(term)});
    } else {
      queue->addDelete({term});
    }
    assertTrue(queue->anyChanges());
    if (random()->nextInt(10) == 0) {
      queue->clear();
      queue->tryApplyGlobalSlice();
      assertFalse(queue->anyChanges());
    }
  }
}

void TestDocumentsWriterDeleteQueue::testAnyChanges() 
{
  shared_ptr<DocumentsWriterDeleteQueue> queue =
      make_shared<DocumentsWriterDeleteQueue>(nullptr);
  constexpr int size = 200 + random()->nextInt(500) * RANDOM_MULTIPLIER;
  int termsSinceFreeze = 0;
  int queriesSinceFreeze = 0;
  for (int i = 0; i < size; i++) {
    shared_ptr<Term> term = make_shared<Term>(L"id", L"" + to_wstring(i));
    if (random()->nextInt(10) == 0) {
      queue->addDelete({make_shared<TermQuery>(term)});
      queriesSinceFreeze++;
    } else {
      queue->addDelete({term});
      termsSinceFreeze++;
    }
    assertTrue(queue->anyChanges());
    if (random()->nextInt(5) == 0) {
      shared_ptr<FrozenBufferedUpdates> freezeGlobalBuffer =
          queue->freezeGlobalBuffer(nullptr);
      assertEquals(termsSinceFreeze, freezeGlobalBuffer->deleteTerms->size());
      assertEquals(queriesSinceFreeze,
                   freezeGlobalBuffer->deleteQueries.size());
      queriesSinceFreeze = 0;
      termsSinceFreeze = 0;
      assertFalse(queue->anyChanges());
    }
  }
}

void TestDocumentsWriterDeleteQueue::testPartiallyAppliedGlobalSlice() throw(
    runtime_error)
{
  shared_ptr<DocumentsWriterDeleteQueue> *const queue =
      make_shared<DocumentsWriterDeleteQueue>(nullptr);
  shared_ptr<ReentrantLock> lock = queue->globalBufferLock;
  lock->lock();
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this(), queue);
  t->start();
  t->join();
  lock->unlock();
  assertTrue(L"changes in del queue but not in slice yet", queue->anyChanges());
  queue->tryApplyGlobalSlice();
  assertTrue(L"changes in global buffer", queue->anyChanges());
  shared_ptr<FrozenBufferedUpdates> freezeGlobalBuffer =
      queue->freezeGlobalBuffer(nullptr);
  assertTrue(freezeGlobalBuffer->any());
  assertEquals(1, freezeGlobalBuffer->deleteTerms->size());
  assertFalse(L"all changes applied", queue->anyChanges());
}

TestDocumentsWriterDeleteQueue::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestDocumentsWriterDeleteQueue> outerInstance,
        shared_ptr<org::apache::lucene::index::DocumentsWriterDeleteQueue>
            queue)
{
  this->outerInstance = outerInstance;
  this->queue = queue;
}

void TestDocumentsWriterDeleteQueue::ThreadAnonymousInnerClass::run()
{
  queue->addDelete({make_shared<Term>(L"foo", L"bar")});
}

void TestDocumentsWriterDeleteQueue::testStressDeleteQueue() throw(
    runtime_error)
{
  shared_ptr<DocumentsWriterDeleteQueue> queue =
      make_shared<DocumentsWriterDeleteQueue>(nullptr);
  shared_ptr<Set<std::shared_ptr<Term>>> uniqueValues =
      unordered_set<std::shared_ptr<Term>>();
  constexpr int size = 10000 + random()->nextInt(500) * RANDOM_MULTIPLIER;
  std::deque<optional<int>> ids(size);
  for (int i = 0; i < ids.size(); i++) {
    ids[i] = random()->nextInt();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    uniqueValues->add(make_shared<Term>(L"id", ids[i].toString()));
  }
  shared_ptr<CountDownLatch> latch = make_shared<CountDownLatch>(1);
  shared_ptr<AtomicInteger> index = make_shared<AtomicInteger>(0);
  constexpr int numThreads = 2 + random()->nextInt(5);
  std::deque<std::shared_ptr<UpdateThread>> threads(numThreads);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<UpdateThread>(queue, index, ids, latch);
    threads[i]->start();
  }
  latch->countDown();
  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }

  for (auto updateThread : threads) {
    shared_ptr<DeleteSlice> slice = updateThread->slice;
    queue->updateSlice(slice);
    shared_ptr<BufferedUpdates> deletes = updateThread->deletes;
    slice->apply(deletes, BufferedUpdates::MAX_INT);
    assertEquals(uniqueValues, deletes->deleteTerms.keySet());
  }
  queue->tryApplyGlobalSlice();
  shared_ptr<Set<std::shared_ptr<Term>>> frozenSet =
      unordered_set<std::shared_ptr<Term>>();
  shared_ptr<BytesRefBuilder> builder = make_shared<BytesRefBuilder>();

  shared_ptr<TermIterator> iter =
      queue->freezeGlobalBuffer(nullptr)->deleteTerms->begin();
  while (iter->next() != nullptr) {
    builder->copyBytes(iter->bytes);
    frozenSet->add(make_shared<Term>(iter->field(), builder->toBytesRef()));
  }

  assertEquals(L"num deletes must be 0 after freeze", 0,
               queue->numGlobalTermDeletes());
  assertEquals(uniqueValues->size(), frozenSet->size());
  assertEquals(uniqueValues, frozenSet);
}

TestDocumentsWriterDeleteQueue::UpdateThread::UpdateThread(
    shared_ptr<DocumentsWriterDeleteQueue> queue,
    shared_ptr<AtomicInteger> index, std::deque<Integer> &ids,
    shared_ptr<CountDownLatch> latch)
    : queue(queue), index(index), ids(ids), slice(queue->newSlice()),
      deletes(make_shared<BufferedUpdates>(L"deletes")), latch(latch)
{
}

void TestDocumentsWriterDeleteQueue::UpdateThread::run()
{
  try {
    latch->await();
  } catch (const InterruptedException &e) {
    throw make_shared<ThreadInterruptedException>(e);
  }
  int i = 0;
  while ((i = index->getAndIncrement()) < ids.size()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Term> term = make_shared<Term>(L"id", ids[i].toString());
    shared_ptr<DocumentsWriterDeleteQueue::Node<std::shared_ptr<Term>>>
        termNode = DocumentsWriterDeleteQueue::newNode(term);
    queue->add(termNode, slice);
    assertTrue(slice->isTail(termNode));
    slice->apply(deletes, BufferedUpdates::MAX_INT);
  }
}
} // namespace org::apache::lucene::index