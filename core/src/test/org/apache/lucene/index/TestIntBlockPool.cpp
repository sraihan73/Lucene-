using namespace std;

#include "TestIntBlockPool.h"

namespace org::apache::lucene::index
{
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIntBlockPool::testSingleWriterReader()
{
  shared_ptr<Counter> bytesUsed = Counter::newCounter();
  shared_ptr<IntBlockPool> pool =
      make_shared<IntBlockPool>(make_shared<ByteTrackingAllocator>(bytesUsed));

  for (int j = 0; j < 2; j++) {
    shared_ptr<IntBlockPool::SliceWriter> writer =
        make_shared<IntBlockPool::SliceWriter>(pool);
    int start = writer->startNewSlice();
    int num = atLeast(100);
    for (int i = 0; i < num; i++) {
      writer->writeInt(i);
    }

    int upto = writer->getCurrentOffset();
    shared_ptr<IntBlockPool::SliceReader> reader =
        make_shared<IntBlockPool::SliceReader>(pool);
    reader->reset(start, upto);
    for (int i = 0; i < num; i++) {
      assertEquals(i, reader->readInt());
    }
    assertTrue(reader->endOfSlice());
    if (random()->nextBoolean()) {
      pool->reset(true, false);
      assertEquals(0, bytesUsed->get());
    } else {
      pool->reset(true, true);
      assertEquals(IntBlockPool::INT_BLOCK_SIZE * Integer::BYTES,
                   bytesUsed->get());
    }
  }
}

void TestIntBlockPool::testMultipleWriterReader()
{
  shared_ptr<Counter> bytesUsed = Counter::newCounter();
  shared_ptr<IntBlockPool> pool =
      make_shared<IntBlockPool>(make_shared<ByteTrackingAllocator>(bytesUsed));
  for (int j = 0; j < 2; j++) {
    deque<std::shared_ptr<StartEndAndValues>> holders =
        deque<std::shared_ptr<StartEndAndValues>>();
    int num = atLeast(4);
    for (int i = 0; i < num; i++) {
      holders.push_back(
          make_shared<StartEndAndValues>(random()->nextInt(1000)));
    }
    shared_ptr<IntBlockPool::SliceWriter> writer =
        make_shared<IntBlockPool::SliceWriter>(pool);
    shared_ptr<IntBlockPool::SliceReader> reader =
        make_shared<IntBlockPool::SliceReader>(pool);

    int numValuesToWrite = atLeast(10000);
    for (int i = 0; i < numValuesToWrite; i++) {
      shared_ptr<StartEndAndValues> values =
          holders[random()->nextInt(holders.size())];
      if (values->valueCount == 0) {
        values->start = writer->startNewSlice();
      } else {
        writer->reset(values->end);
      }
      writer->writeInt(values->nextValue());
      values->end = writer->getCurrentOffset();
      if (random()->nextInt(5) == 0) {
        // pick one and reader the ints
        assertReader(reader, holders[random()->nextInt(holders.size())]);
      }
    }

    while (!holders.empty()) {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      shared_ptr<StartEndAndValues> values =
          holders.remove(random()->nextInt(holders.size()));
      assertReader(reader, values);
    }
    if (random()->nextBoolean()) {
      pool->reset(true, false);
      assertEquals(0, bytesUsed->get());
    } else {
      pool->reset(true, true);
      assertEquals(IntBlockPool::INT_BLOCK_SIZE * Integer::BYTES,
                   bytesUsed->get());
    }
  }
}

TestIntBlockPool::ByteTrackingAllocator::ByteTrackingAllocator(
    shared_ptr<Counter> bytesUsed)
    : ByteTrackingAllocator(IntBlockPool::INT_BLOCK_SIZE, bytesUsed)
{
}

TestIntBlockPool::ByteTrackingAllocator::ByteTrackingAllocator(
    int blockSize, shared_ptr<Counter> bytesUsed)
    : org::apache::lucene::util::IntBlockPool::Allocator(blockSize),
      bytesUsed(bytesUsed)
{
}

std::deque<int> TestIntBlockPool::ByteTrackingAllocator::getIntBlock()
{
  bytesUsed->addAndGet(blockSize * Integer::BYTES);
  return std::deque<int>(blockSize);
}

void TestIntBlockPool::ByteTrackingAllocator::recycleIntBlocks(
    std::deque<std::deque<int>> &blocks, int start, int end)
{
  bytesUsed->addAndGet(-((end - start) * blockSize * Integer::BYTES));
}

void TestIntBlockPool::assertReader(
    shared_ptr<IntBlockPool::SliceReader> reader,
    shared_ptr<StartEndAndValues> values)
{
  reader->reset(values->start, values->end);
  for (int i = 0; i < values->valueCount; i++) {
    assertEquals(values->valueOffset + i, reader->readInt());
  }
  assertTrue(reader->endOfSlice());
}

TestIntBlockPool::StartEndAndValues::StartEndAndValues(int valueOffset)
{
  this->valueOffset = valueOffset;
}

int TestIntBlockPool::StartEndAndValues::nextValue()
{
  return valueOffset + valueCount++;
}
} // namespace org::apache::lucene::index