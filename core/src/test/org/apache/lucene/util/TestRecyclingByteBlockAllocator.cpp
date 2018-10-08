using namespace std;

#include "TestRecyclingByteBlockAllocator.h"

namespace org::apache::lucene::util
{
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void TestRecyclingByteBlockAllocator::setUp() 
{
  LuceneTestCase::setUp();
}

shared_ptr<RecyclingByteBlockAllocator>
TestRecyclingByteBlockAllocator::newAllocator()
{
  return make_shared<RecyclingByteBlockAllocator>(
      1 << (2 + random()->nextInt(15)), random()->nextInt(97),
      Counter::newCounter());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllocate()
void TestRecyclingByteBlockAllocator::testAllocate()
{
  shared_ptr<RecyclingByteBlockAllocator> allocator = newAllocator();
  unordered_set<std::deque<char>> set = unordered_set<std::deque<char>>();
  std::deque<char> block = allocator->getByteBlock();
  set.insert(block);
  assertNotNull(block);
  constexpr int size = block.size();

  int num = atLeast(97);
  for (int i = 0; i < num; i++) {
    block = allocator->getByteBlock();
    assertNotNull(block);
    assertEquals(size, block.size());
    assertTrue(L"block is returned twice", set.insert(block));
    assertEquals(size * (i + 2), allocator->bytesUsed()); // zero based + 1
    assertEquals(0, allocator->numBufferedBlocks());
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllocateAndRecycle()
void TestRecyclingByteBlockAllocator::testAllocateAndRecycle()
{
  shared_ptr<RecyclingByteBlockAllocator> allocator = newAllocator();
  unordered_set<std::deque<char>> allocated =
      unordered_set<std::deque<char>>();

  std::deque<char> block = allocator->getByteBlock();
  allocated.insert(block);
  assertNotNull(block);
  constexpr int size = block.size();

  int numIters = atLeast(97);
  for (int i = 0; i < numIters; i++) {
    int num = 1 + random()->nextInt(39);
    for (int j = 0; j < num; j++) {
      block = allocator->getByteBlock();
      assertNotNull(block);
      assertEquals(size, block.size());
      assertTrue(L"block is returned twice", allocated.insert(block));
      assertEquals(size * (allocated.size() + allocator->numBufferedBlocks()),
                   allocator->bytesUsed());
    }
    std::deque<std::deque<char>> array_ =
        allocated.toArray(std::deque<std::deque<char>>(0));
    int begin = random()->nextInt(array_.size());
    int end = begin + random()->nextInt(array_.size() - begin);
    deque<std::deque<char>> selected = deque<std::deque<char>>();
    for (int j = begin; j < end; j++) {
      selected.push_back(array_[j]);
    }
    allocator->recycleByteBlocks(array_, begin, end);
    for (int j = begin; j < end; j++) {
      assertNull(array_[j]);
      std::deque<char> b = selected.erase(selected.begin());
      assertTrue(allocated.remove(b));
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllocateAndFree()
void TestRecyclingByteBlockAllocator::testAllocateAndFree()
{
  shared_ptr<RecyclingByteBlockAllocator> allocator = newAllocator();
  unordered_set<std::deque<char>> allocated =
      unordered_set<std::deque<char>>();
  int freeButAllocated = 0;
  std::deque<char> block = allocator->getByteBlock();
  allocated.insert(block);
  assertNotNull(block);
  constexpr int size = block.size();

  int numIters = atLeast(97);
  for (int i = 0; i < numIters; i++) {
    int num = 1 + random()->nextInt(39);
    for (int j = 0; j < num; j++) {
      block = allocator->getByteBlock();
      freeButAllocated = max(0, freeButAllocated - 1);
      assertNotNull(block);
      assertEquals(size, block.size());
      assertTrue(L"block is returned twice", allocated.insert(block));
      assertEquals(size * (allocated.size() + allocator->numBufferedBlocks()),
                   allocator->bytesUsed());
    }

    std::deque<std::deque<char>> array_ =
        allocated.toArray(std::deque<std::deque<char>>(0));
    int begin = random()->nextInt(array_.size());
    int end = begin + random()->nextInt(array_.size() - begin);
    for (int j = begin; j < end; j++) {
      std::deque<char> b = array_[j];
      assertTrue(allocated.remove(b));
    }
    allocator->recycleByteBlocks(array_, begin, end);
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