using namespace std;

#include "TestRecyclingIntBlockAllocator.h"

namespace org::apache::lucene::util
{
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void TestRecyclingIntBlockAllocator::setUp() 
{
  LuceneTestCase::setUp();
}

shared_ptr<RecyclingIntBlockAllocator>
TestRecyclingIntBlockAllocator::newAllocator()
{
  return make_shared<RecyclingIntBlockAllocator>(
      1 << (2 + random()->nextInt(15)), random()->nextInt(97),
      Counter::newCounter());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllocate()
void TestRecyclingIntBlockAllocator::testAllocate()
{
  shared_ptr<RecyclingIntBlockAllocator> allocator = newAllocator();
  unordered_set<std::deque<int>> set = unordered_set<std::deque<int>>();
  std::deque<int> block = allocator->getIntBlock();
  set.insert(block);
  assertNotNull(block);
  constexpr int size = block.size();

  int num = atLeast(97);
  for (int i = 0; i < num; i++) {
    block = allocator->getIntBlock();
    assertNotNull(block);
    assertEquals(size, block.size());
    assertTrue(L"block is returned twice", set.insert(block));
    assertEquals(4 * size * (i + 2), allocator->bytesUsed()); // zero based + 1
    assertEquals(0, allocator->numBufferedBlocks());
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllocateAndRecycle()
void TestRecyclingIntBlockAllocator::testAllocateAndRecycle()
{
  shared_ptr<RecyclingIntBlockAllocator> allocator = newAllocator();
  unordered_set<std::deque<int>> allocated = unordered_set<std::deque<int>>();

  std::deque<int> block = allocator->getIntBlock();
  allocated.insert(block);
  assertNotNull(block);
  constexpr int size = block.size();

  int numIters = atLeast(97);
  for (int i = 0; i < numIters; i++) {
    int num = 1 + random()->nextInt(39);
    for (int j = 0; j < num; j++) {
      block = allocator->getIntBlock();
      assertNotNull(block);
      assertEquals(size, block.size());
      assertTrue(L"block is returned twice", allocated.insert(block));
      assertEquals(4 * size *
                       (allocated.size() + allocator->numBufferedBlocks()),
                   allocator->bytesUsed());
    }
    std::deque<std::deque<int>> array_ =
        allocated.toArray(std::deque<std::deque<int>>(0));
    int begin = random()->nextInt(array_.size());
    int end = begin + random()->nextInt(array_.size() - begin);
    deque<std::deque<int>> selected = deque<std::deque<int>>();
    for (int j = begin; j < end; j++) {
      selected.push_back(array_[j]);
    }
    allocator->recycleIntBlocks(array_, begin, end);
    for (int j = begin; j < end; j++) {
      assertNull(array_[j]);
      std::deque<int> b = selected.erase(selected.begin());
      assertTrue(allocated.remove(b));
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllocateAndFree()
void TestRecyclingIntBlockAllocator::testAllocateAndFree()
{
  shared_ptr<RecyclingIntBlockAllocator> allocator = newAllocator();
  unordered_set<std::deque<int>> allocated = unordered_set<std::deque<int>>();
  int freeButAllocated = 0;
  std::deque<int> block = allocator->getIntBlock();
  allocated.insert(block);
  assertNotNull(block);
  constexpr int size = block.size();

  int numIters = atLeast(97);
  for (int i = 0; i < numIters; i++) {
    int num = 1 + random()->nextInt(39);
    for (int j = 0; j < num; j++) {
      block = allocator->getIntBlock();
      freeButAllocated = max(0, freeButAllocated - 1);
      assertNotNull(block);
      assertEquals(size, block.size());
      assertTrue(L"block is returned twice", allocated.insert(block));
      assertEquals(
          L"" +
              (4 * size * (allocated.size() + allocator->numBufferedBlocks()) -
               allocator->bytesUsed()),
          4 * size * (allocated.size() + allocator->numBufferedBlocks()),
          allocator->bytesUsed());
    }

    std::deque<std::deque<int>> array_ =
        allocated.toArray(std::deque<std::deque<int>>(0));
    int begin = random()->nextInt(array_.size());
    int end = begin + random()->nextInt(array_.size() - begin);
    for (int j = begin; j < end; j++) {
      std::deque<int> b = array_[j];
      assertTrue(allocated.remove(b));
    }
    allocator->recycleIntBlocks(array_, begin, end);
    for (int j = begin; j < end; j++) {
      assertNull(array_[j]);
    }
    // randomly free blocks
    int numFreeBlocks = allocator->numBufferedBlocks();
    int freeBlocks = allocator->freeBlocks(
        random()->nextInt(7 + allocator->maxBufferedBlocks()));
    assertEquals(allocator->numBufferedBlocks(), numFreeBlocks - freeBlocks);
  }
}
} // namespace org::apache::lucene::util