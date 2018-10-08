using namespace std;

#include "TestPriorityQueue.h"

namespace org::apache::lucene::util
{

TestPriorityQueue::IntegerQueue::IntegerQueue(int count)
    : PriorityQueue<int>(count)
{
}

bool TestPriorityQueue::IntegerQueue::lessThan(optional<int> &a,
                                               optional<int> &b)
{
  if (a == b) {
    assert(a != b);
    int hashA = System::identityHashCode(a);
    int hashB = System::identityHashCode(b);
    assert(hashA != hashB);
    return hashA < hashB;
  }
  return (a < b);
}

void TestPriorityQueue::IntegerQueue::checkValidity()
{
  std::deque<any> heapArray = getHeapArray();
  for (int i = 1; i <= size(); i++) {
    int parent = static_cast<int>(static_cast<unsigned int>(i) >> 1);
    if (parent > 1) {
      assertTrue(lessThan(any_cast<optional<int>>(heapArray[parent]),
                          any_cast<optional<int>>(heapArray[i])));
    }
  }
}

void TestPriorityQueue::testPQ() 
{
  testPQ(atLeast(10000), random());
}

void TestPriorityQueue::testPQ(int count, shared_ptr<Random> gen)
{
  shared_ptr<PriorityQueue<int>> pq = make_shared<IntegerQueue>(count);
  int sum = 0, sum2 = 0;

  for (int i = 0; i < count; i++) {
    int next = gen->nextInt();
    sum += next;
    pq->push_back(next);
  }

  //      Date end = new Date();

  //      System.out.print(((float)(end.getTime()-start.getTime()) / count) *
  //      1000); System.out.println(" microseconds/put");

  //      start = new Date();

  int last = numeric_limits<int>::min();
  for (int i = 0; i < count; i++) {
    optional<int> next = pq->pop();
    assertTrue(next.value() >= last);
    last = next.value();
    sum2 += last;
  }

  assertEquals(sum, sum2);
  //      end = new Date();

  //      System.out.print(((float)(end.getTime()-start.getTime()) / count) *
  //      1000); System.out.println(" microseconds/pop");
}

void TestPriorityQueue::testClear()
{
  shared_ptr<PriorityQueue<int>> pq = make_shared<IntegerQueue>(3);
  pq->push_back(2);
  pq->push_back(3);
  pq->push_back(1);
  assertEquals(3, pq->size());
  pq->clear();
  assertEquals(0, pq->size());
}

void TestPriorityQueue::testFixedSize()
{
  shared_ptr<PriorityQueue<int>> pq = make_shared<IntegerQueue>(3);
  pq->insertWithOverflow(2);
  pq->insertWithOverflow(3);
  pq->insertWithOverflow(1);
  pq->insertWithOverflow(5);
  pq->insertWithOverflow(7);
  pq->insertWithOverflow(1);
  assertEquals(3, pq->size());
  assertEquals(static_cast<optional<int>>(3), pq->top());
}

void TestPriorityQueue::testInsertWithOverflow()
{
  int size = 4;
  shared_ptr<PriorityQueue<int>> pq = make_shared<IntegerQueue>(size);
  optional<int> i1 = 2;
  optional<int> i2 = 3;
  optional<int> i3 = 1;
  optional<int> i4 = 5;
  optional<int> i5 = 7;
  optional<int> i6 = 1;

  assertNull(pq->insertWithOverflow(i1));
  assertNull(pq->insertWithOverflow(i2));
  assertNull(pq->insertWithOverflow(i3));
  assertNull(pq->insertWithOverflow(i4));
  assertTrue(pq->insertWithOverflow(i5) == i3); // i3 should have been dropped
  assertTrue(pq->insertWithOverflow(i6) ==
             i6); // i6 should not have been inserted
  assertEquals(size, pq->size());
  assertEquals(static_cast<optional<int>>(2), pq->top());
}

void TestPriorityQueue::testRemovalsAndInsertions()
{
  shared_ptr<Random> random = TestPriorityQueue::random();
  int numDocsInPQ = TestUtil::nextInt(random, 1, 100);
  shared_ptr<IntegerQueue> pq = make_shared<IntegerQueue>(numDocsInPQ);
  optional<int> lastLeast = nullopt;

  // Basic insertion of new content
  deque<int> sds = deque<int>(numDocsInPQ);
  for (int i = 0; i < numDocsInPQ * 10; i++) {
    optional<int> newEntry = optional<int>(abs(random->nextInt()));
    sds.push_back(newEntry);
    optional<int> evicted = pq->insertWithOverflow(newEntry);
    pq->checkValidity();
    if (evicted) {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      assertTrue(sds.remove(evicted));
      if (evicted != newEntry) {
        assertTrue(evicted == lastLeast);
      }
    }
    optional<int> newLeast = pq->top();
    if ((lastLeast) && (newLeast != newEntry) && (newLeast != lastLeast)) {
      // If there has been a change of least entry and it wasn't our new
      // addition we expect the scores to increase
      assertTrue(newLeast <= newEntry);
      assertTrue(newLeast >= lastLeast);
    }
    lastLeast = newLeast;
  }

  // Try many random additions to existing entries - we should always see
  // increasing scores in the lowest entry in the PQ
  for (int p = 0; p < 500000; p++) {
    int element = static_cast<int>(random->nextFloat() * (sds.size() - 1));
    optional<int> objectToRemove = sds[element];
    assertTrue(sds.erase(sds.begin() + element) == objectToRemove);
    // C++ TODO: The Java deque 'remove(Object)' method is not converted:
    assertTrue(pq->remove(objectToRemove));
    pq->checkValidity();
    optional<int> newEntry = optional<int>(abs(random->nextInt()));
    sds.push_back(newEntry);
    assertNull(pq->insertWithOverflow(newEntry));
    pq->checkValidity();
    optional<int> newLeast = pq->top();
    if ((objectToRemove != lastLeast) && (lastLeast) &&
        (newLeast != newEntry)) {
      // If there has been a change of least entry and it wasn't our new
      // addition or the loss of our randomly removed entry we expect the
      // scores to increase
      assertTrue(newLeast <= newEntry);
      assertTrue(newLeast >= lastLeast);
    }
    lastLeast = newLeast;
  }
}

void TestPriorityQueue::testIteratorEmpty()
{
  shared_ptr<IntegerQueue> queue = make_shared<IntegerQueue>(3);

  org::apache::lucene::util::TestPriorityQueue::IntegerQueue::const_iterator
      it = queue->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(it->hasNext());
  expectThrows(NoSuchElementException::typeid, [&]() { it->next(); });
}

void TestPriorityQueue::testIteratorOne()
{
  shared_ptr<IntegerQueue> queue = make_shared<IntegerQueue>(3);

  queue->push_back(1);
  org::apache::lucene::util::TestPriorityQueue::IntegerQueue::const_iterator
      it = queue->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(static_cast<Integer>(1), it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(it->hasNext());
  expectThrows(NoSuchElementException::typeid, [&]() { it->next(); });
}

void TestPriorityQueue::testIteratorTwo()
{
  shared_ptr<IntegerQueue> queue = make_shared<IntegerQueue>(3);

  queue->push_back(1);
  queue->push_back(2);
  org::apache::lucene::util::TestPriorityQueue::IntegerQueue::const_iterator
      it = queue->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(static_cast<Integer>(1), it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(static_cast<Integer>(2), it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(it->hasNext());
  expectThrows(NoSuchElementException::typeid, [&]() { it->next(); });
}

void TestPriorityQueue::testIteratorRandom()
{
  constexpr int maxSize = TestUtil::nextInt(random(), 1, 20);
  shared_ptr<IntegerQueue> queue = make_shared<IntegerQueue>(maxSize);
  constexpr int iters = atLeast(100);
  const deque<int> expected = deque<int>();
  for (int iter = 0; iter < iters; ++iter) {
    if (queue->empty() ||
        (queue->size() < maxSize && random()->nextBoolean())) {
      const optional<int> value = optional<int>(random()->nextInt(10));
      queue->push_back(value);
      expected.push_back(value);
    } else {
      expected.erase(expected.begin() + queue->pop());
    }
    deque<int> actual = deque<int>();
    for (shared_ptr<> : : optional<int> value : queue) {
      actual.push_back(value);
    }
    CollectionUtil::introSort(expected);
    CollectionUtil::introSort(actual);
    assertEquals(expected, actual);
  }
}
} // namespace org::apache::lucene::util