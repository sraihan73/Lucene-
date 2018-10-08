using namespace std;

#include "IntBlockPool.h"

namespace org::apache::lucene::util
{

IntBlockPool::Allocator::Allocator(int blockSize) : blockSize(blockSize) {}

std::deque<int> IntBlockPool::Allocator::getIntBlock()
{
  return std::deque<int>(blockSize);
}

IntBlockPool::DirectAllocator::DirectAllocator() : Allocator(INT_BLOCK_SIZE) {}

void IntBlockPool::DirectAllocator::recycleIntBlocks(
    std::deque<std::deque<int>> &blocks, int start, int end)
{
}

IntBlockPool::IntBlockPool() : IntBlockPool(new DirectAllocator()) {}

IntBlockPool::IntBlockPool(shared_ptr<Allocator> allocator)
    : allocator(allocator)
{
}

void IntBlockPool::reset() { this->reset(true, true); }

void IntBlockPool::reset(bool zeroFillBuffers, bool reuseFirst)
{
  if (bufferUpto != -1) {
    // We allocated at least one buffer

    if (zeroFillBuffers) {
      for (int i = 0; i < bufferUpto; i++) {
        // Fully zero fill buffers that we fully used
        Arrays::fill(buffers[i], 0);
      }
      // Partial zero fill the final buffer
      Arrays::fill(buffers[bufferUpto], 0, intUpto, 0);
    }

    if (bufferUpto > 0 || !reuseFirst) {
      constexpr int offset = reuseFirst ? 1 : 0;
      // Recycle all but the first buffer
      allocator->recycleIntBlocks(buffers, offset, 1 + bufferUpto);
      Arrays::fill(buffers, offset, bufferUpto + 1, nullptr);
    }
    if (reuseFirst) {
      // Re-use the first buffer
      bufferUpto = 0;
      intUpto = 0;
      intOffset = 0;
      buffer = buffers[0];
    } else {
      bufferUpto = -1;
      intUpto = INT_BLOCK_SIZE;
      intOffset = -INT_BLOCK_SIZE;
      buffer.clear();
    }
  }
}

void IntBlockPool::nextBuffer()
{
  if (1 + bufferUpto == buffers.size()) {
    std::deque<std::deque<int>> newBuffers(
        static_cast<int>(buffers.size() * 1.5));
    System::arraycopy(buffers, 0, newBuffers, 0, buffers.size());
    buffers = newBuffers;
  }
  buffer = buffers[1 + bufferUpto] = allocator->getIntBlock();
  bufferUpto++;

  intUpto = 0;
  intOffset += INT_BLOCK_SIZE;
}

int IntBlockPool::newSlice(int const size)
{
  if (intUpto > INT_BLOCK_SIZE - size) {
    nextBuffer();
    assert(assertSliceBuffer(buffer));
  }

  constexpr int upto = intUpto;
  intUpto += size;
  buffer[intUpto - 1] = 1;
  return upto;
}

bool IntBlockPool::assertSliceBuffer(std::deque<int> &buffer)
{
  int count = 0;
  for (int i = 0; i < buffer.size(); i++) {
    count += buffer[i]; // for slices the buffer must only have 0 values
  }
  return count == 0;
}

std::deque<int> const IntBlockPool::NEXT_LEVEL_ARRAY = {1, 2, 3, 4, 5,
                                                         6, 7, 8, 9, 9};
std::deque<int> const IntBlockPool::LEVEL_SIZE_ARRAY = {
    2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

int IntBlockPool::allocSlice(std::deque<int> &slice, int const sliceOffset)
{
  constexpr int level = slice[sliceOffset];
  constexpr int newLevel = NEXT_LEVEL_ARRAY[level - 1];
  constexpr int newSize = LEVEL_SIZE_ARRAY[newLevel];
  // Maybe allocate another block
  if (intUpto > INT_BLOCK_SIZE - newSize) {
    nextBuffer();
    assert(assertSliceBuffer(buffer));
  }

  constexpr int newUpto = intUpto;
  constexpr int offset = newUpto + intOffset;
  intUpto += newSize;
  // Write forwarding address at end of last slice:
  slice[sliceOffset] = offset;

  // Write new level:
  buffer[intUpto - 1] = newLevel;

  return newUpto;
}

IntBlockPool::SliceWriter::SliceWriter(shared_ptr<IntBlockPool> pool)
    : pool(pool)
{
}

void IntBlockPool::SliceWriter::reset(int sliceOffset)
{
  this->offset = sliceOffset;
}

void IntBlockPool::SliceWriter::writeInt(int value)
{
  std::deque<int> ints = pool->buffers[offset >> INT_BLOCK_SHIFT];
  assert(ints.size() > 0);
  int relativeOffset = offset & INT_BLOCK_MASK;
  if (ints[relativeOffset] != 0) {
    // End of slice; allocate a new one
    relativeOffset = pool->allocSlice(ints, relativeOffset);
    ints = pool->buffer;
    offset = relativeOffset + pool->intOffset;
  }
  ints[relativeOffset] = value;
  offset++;
}

int IntBlockPool::SliceWriter::startNewSlice()
{
  return offset = pool->newSlice(FIRST_LEVEL_SIZE) + pool->intOffset;
}

int IntBlockPool::SliceWriter::getCurrentOffset() { return offset; }

IntBlockPool::SliceReader::SliceReader(shared_ptr<IntBlockPool> pool)
    : pool(pool)
{
}

void IntBlockPool::SliceReader::reset(int startOffset, int endOffset)
{
  bufferUpto = startOffset / INT_BLOCK_SIZE;
  bufferOffset = bufferUpto * INT_BLOCK_SIZE;
  this->end = endOffset;
  upto = startOffset;
  level = 1;

  buffer = pool->buffers[bufferUpto];
  upto = startOffset & INT_BLOCK_MASK;

  constexpr int firstSize = IntBlockPool::LEVEL_SIZE_ARRAY[0];
  if (startOffset + firstSize >= endOffset) {
    // There is only this one slice to read
    limit = endOffset & INT_BLOCK_MASK;
  } else {
    limit = upto + firstSize - 1;
  }
}

bool IntBlockPool::SliceReader::endOfSlice()
{
  assert(upto + bufferOffset <= end);
  return upto + bufferOffset == end;
}

int IntBlockPool::SliceReader::readInt()
{
  assert(!endOfSlice());
  assert(upto <= limit);
  if (upto == limit) {
    nextSlice();
  }
  return buffer[upto++];
}

void IntBlockPool::SliceReader::nextSlice()
{
  // Skip to our next slice
  constexpr int nextIndex = buffer[limit];
  level = NEXT_LEVEL_ARRAY[level - 1];
  constexpr int newSize = LEVEL_SIZE_ARRAY[level];

  bufferUpto = nextIndex / INT_BLOCK_SIZE;
  bufferOffset = bufferUpto * INT_BLOCK_SIZE;

  buffer = pool->buffers[bufferUpto];
  upto = nextIndex & INT_BLOCK_MASK;

  if (nextIndex + newSize >= end) {
    // We are advancing to the final slice
    assert(end - nextIndex > 0);
    limit = end - bufferOffset;
  } else {
    // This is not the final slice (subtract 4 for the
    // forwarding address at the end of this new slice)
    limit = upto + newSize - 1;
  }
}
} // namespace org::apache::lucene::util