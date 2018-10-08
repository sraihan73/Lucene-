using namespace std;

#include "RecyclingIntBlockAllocator.h"

namespace org::apache::lucene::util
{
using Allocator = org::apache::lucene::util::IntBlockPool::Allocator;

RecyclingIntBlockAllocator::RecyclingIntBlockAllocator(
    int blockSize, int maxBufferedBlocks, shared_ptr<Counter> bytesUsed)
    : org::apache::lucene::util::IntBlockPool::Allocator(blockSize),
      maxBufferedBlocks(maxBufferedBlocks), bytesUsed(bytesUsed)
{
  freeByteBlocks = std::deque<std::deque<int>>(maxBufferedBlocks);
}

RecyclingIntBlockAllocator::RecyclingIntBlockAllocator(int blockSize,
                                                       int maxBufferedBlocks)
    : RecyclingIntBlockAllocator(blockSize, maxBufferedBlocks,
                                 Counter::newCounter(false))
{
}

RecyclingIntBlockAllocator::RecyclingIntBlockAllocator()
    : RecyclingIntBlockAllocator(IntBlockPool::INT_BLOCK_SIZE, 64,
                                 Counter::newCounter(false))
{
}

std::deque<int> RecyclingIntBlockAllocator::getIntBlock()
{
  if (freeBlocks_ == 0) {
    bytesUsed_->addAndGet(blockSize * Integer::BYTES);
    return std::deque<int>(blockSize);
  }
  const std::deque<int> b = freeByteBlocks[--freeBlocks_];
  freeByteBlocks[freeBlocks_].smartpointerreset();
  return b;
}

void RecyclingIntBlockAllocator::recycleIntBlocks(
    std::deque<std::deque<int>> &blocks, int start, int end)
{
  constexpr int numBlocks = min(maxBufferedBlocks_ - freeBlocks_, end - start);
  constexpr int size = freeBlocks_ + numBlocks;
  if (size >= freeByteBlocks.size()) {
    const std::deque<std::deque<int>> newBlocks =
        std::deque<std::deque<int>>(
            ArrayUtil::oversize(size, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(freeByteBlocks, 0, newBlocks, 0, freeBlocks_);
    freeByteBlocks = newBlocks;
  }
  constexpr int stop = start + numBlocks;
  for (int i = start; i < stop; i++) {
    freeByteBlocks[freeBlocks_++] = blocks[i];
    blocks[i].clear();
  }
  for (int i = stop; i < end; i++) {
    blocks[i].clear();
  }
  bytesUsed_->addAndGet(-(end - stop) * (blockSize * Integer::BYTES));
  assert(bytesUsed_->get() >= 0);
}

int RecyclingIntBlockAllocator::numBufferedBlocks() { return freeBlocks_; }

int64_t RecyclingIntBlockAllocator::bytesUsed() { return bytesUsed_->get(); }

int RecyclingIntBlockAllocator::maxBufferedBlocks()
{
  return maxBufferedBlocks_;
}

int RecyclingIntBlockAllocator::freeBlocks(int num)
{
  assert((num >= 0, L"free blocks must be >= 0 but was: " + to_wstring(num)));
  constexpr int stop;
  constexpr int count;
  if (num > freeBlocks_) {
    stop = 0;
    count = freeBlocks_;
  } else {
    stop = freeBlocks_ - num;
    count = num;
  }
  while (freeBlocks_ > stop) {
    freeByteBlocks[--freeBlocks_].smartpointerreset();
  }
  bytesUsed_->addAndGet(-count * blockSize * Integer::BYTES);
  assert(bytesUsed_->get() >= 0);
  return count;
}
} // namespace org::apache::lucene::util