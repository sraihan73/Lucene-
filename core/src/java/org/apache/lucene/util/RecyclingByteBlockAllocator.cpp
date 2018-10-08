using namespace std;

#include "RecyclingByteBlockAllocator.h"

namespace org::apache::lucene::util
{
using Allocator = org::apache::lucene::util::ByteBlockPool::Allocator;

RecyclingByteBlockAllocator::RecyclingByteBlockAllocator(
    int blockSize, int maxBufferedBlocks, shared_ptr<Counter> bytesUsed)
    : ByteBlockPool::Allocator(blockSize), maxBufferedBlocks(maxBufferedBlocks),
      bytesUsed(bytesUsed)
{
  freeByteBlocks = std::deque<std::deque<char>>(maxBufferedBlocks);
}

RecyclingByteBlockAllocator::RecyclingByteBlockAllocator(int blockSize,
                                                         int maxBufferedBlocks)
    : RecyclingByteBlockAllocator(blockSize, maxBufferedBlocks,
                                  Counter::newCounter(false))
{
}

RecyclingByteBlockAllocator::RecyclingByteBlockAllocator()
    : RecyclingByteBlockAllocator(ByteBlockPool::BYTE_BLOCK_SIZE, 64,
                                  Counter::newCounter(false))
{
}

std::deque<char> RecyclingByteBlockAllocator::getByteBlock()
{
  if (freeBlocks_ == 0) {
    bytesUsed_->addAndGet(blockSize);
    return std::deque<char>(blockSize);
  }
  const std::deque<char> b = freeByteBlocks[--freeBlocks_];
  freeByteBlocks[freeBlocks_].smartpointerreset();
  return b;
}

void RecyclingByteBlockAllocator::recycleByteBlocks(
    std::deque<std::deque<char>> &blocks, int start, int end)
{
  constexpr int numBlocks = min(maxBufferedBlocks_ - freeBlocks_, end - start);
  constexpr int size = freeBlocks_ + numBlocks;
  if (size >= freeByteBlocks.size()) {
    const std::deque<std::deque<char>> newBlocks =
        std::deque<std::deque<char>>(
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
  bytesUsed_->addAndGet(-(end - stop) * blockSize);
  assert(bytesUsed_->get() >= 0);
}

int RecyclingByteBlockAllocator::numBufferedBlocks() { return freeBlocks_; }

int64_t RecyclingByteBlockAllocator::bytesUsed() { return bytesUsed_->get(); }

int RecyclingByteBlockAllocator::maxBufferedBlocks()
{
  return maxBufferedBlocks_;
}

int RecyclingByteBlockAllocator::freeBlocks(int num)
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
  bytesUsed_->addAndGet(-count * blockSize);
  assert(bytesUsed_->get() >= 0);
  return count;
}
} // namespace org::apache::lucene::util