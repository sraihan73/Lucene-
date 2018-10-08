using namespace std;

#include "ByteBlockPool.h"

namespace org::apache::lucene::util
{
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF;

ByteBlockPool::Allocator::Allocator(int blockSize) : blockSize(blockSize) {}

void ByteBlockPool::Allocator::recycleByteBlocks(
    deque<std::deque<char>> &blocks)
{
  const std::deque<std::deque<char>> b =
      blocks.toArray(std::deque<std::deque<char>>(blocks.size()));
  recycleByteBlocks(b, 0, b.size());
}

std::deque<char> ByteBlockPool::Allocator::getByteBlock()
{
  return std::deque<char>(blockSize);
}

ByteBlockPool::DirectAllocator::DirectAllocator()
    : DirectAllocator(BYTE_BLOCK_SIZE)
{
}

ByteBlockPool::DirectAllocator::DirectAllocator(int blockSize)
    : Allocator(blockSize)
{
}

void ByteBlockPool::DirectAllocator::recycleByteBlocks(
    std::deque<std::deque<char>> &blocks, int start, int end)
{
}

ByteBlockPool::DirectTrackingAllocator::DirectTrackingAllocator(
    shared_ptr<Counter> bytesUsed)
    : DirectTrackingAllocator(BYTE_BLOCK_SIZE, bytesUsed)
{
}

ByteBlockPool::DirectTrackingAllocator::DirectTrackingAllocator(
    int blockSize, shared_ptr<Counter> bytesUsed)
    : Allocator(blockSize), bytesUsed(bytesUsed)
{
}

std::deque<char> ByteBlockPool::DirectTrackingAllocator::getByteBlock()
{
  bytesUsed->addAndGet(blockSize);
  return std::deque<char>(blockSize);
}

void ByteBlockPool::DirectTrackingAllocator::recycleByteBlocks(
    std::deque<std::deque<char>> &blocks, int start, int end)
{
  bytesUsed->addAndGet(-((end - start) * blockSize));
  for (int i = start; i < end; i++) {
    blocks[i].clear();
  }
}

ByteBlockPool::ByteBlockPool(shared_ptr<Allocator> allocator)
    : allocator(allocator)
{
}

void ByteBlockPool::reset() { reset(true, true); }

void ByteBlockPool::reset(bool zeroFillBuffers, bool reuseFirst)
{
  if (bufferUpto != -1) {
    // We allocated at least one buffer

    if (zeroFillBuffers) {
      for (int i = 0; i < bufferUpto; i++) {
        // Fully zero fill buffers that we fully used
        Arrays::fill(buffers[i], static_cast<char>(0));
      }
      // Partial zero fill the final buffer
      Arrays::fill(buffers[bufferUpto], 0, byteUpto, static_cast<char>(0));
    }

    if (bufferUpto > 0 || !reuseFirst) {
      constexpr int offset = reuseFirst ? 1 : 0;
      // Recycle all but the first buffer
      allocator->recycleByteBlocks(buffers, offset, 1 + bufferUpto);
      Arrays::fill(buffers, offset, 1 + bufferUpto, nullptr);
    }
    if (reuseFirst) {
      // Re-use the first buffer
      bufferUpto = 0;
      byteUpto = 0;
      byteOffset = 0;
      buffer = buffers[0];
    } else {
      bufferUpto = -1;
      byteUpto = BYTE_BLOCK_SIZE;
      byteOffset = -BYTE_BLOCK_SIZE;
      buffer.clear();
    }
  }
}

void ByteBlockPool::nextBuffer()
{
  if (1 + bufferUpto == buffers.size()) {
    std::deque<std::deque<char>> newBuffers(
        ArrayUtil::oversize(buffers.size() + 1, NUM_BYTES_OBJECT_REF));
    System::arraycopy(buffers, 0, newBuffers, 0, buffers.size());
    buffers = newBuffers;
  }
  buffer = buffers[1 + bufferUpto] = allocator->getByteBlock();
  bufferUpto++;

  byteUpto = 0;
  byteOffset += BYTE_BLOCK_SIZE;
}

int ByteBlockPool::newSlice(int const size)
{
  if (byteUpto > BYTE_BLOCK_SIZE - size) {
    nextBuffer();
  }
  constexpr int upto = byteUpto;
  byteUpto += size;
  buffer[byteUpto - 1] = 16;
  return upto;
}

std::deque<int> const ByteBlockPool::NEXT_LEVEL_ARRAY = {1, 2, 3, 4, 5,
                                                          6, 7, 8, 9, 9};
std::deque<int> const ByteBlockPool::LEVEL_SIZE_ARRAY = {5,  14, 20, 30,  40,
                                                          40, 80, 80, 120, 200};

int ByteBlockPool::allocSlice(std::deque<char> &slice, int const upto)
{

  constexpr int level = slice[upto] & 15;
  constexpr int newLevel = NEXT_LEVEL_ARRAY[level];
  constexpr int newSize = LEVEL_SIZE_ARRAY[newLevel];

  // Maybe allocate another block
  if (byteUpto > BYTE_BLOCK_SIZE - newSize) {
    nextBuffer();
  }

  constexpr int newUpto = byteUpto;
  constexpr int offset = newUpto + byteOffset;
  byteUpto += newSize;

  // Copy forward the past 3 bytes (which we are about
  // to overwrite with the forwarding address):
  buffer[newUpto] = slice[upto - 3];
  buffer[newUpto + 1] = slice[upto - 2];
  buffer[newUpto + 2] = slice[upto - 1];

  // Write forwarding address at end of last slice:
  slice[upto - 3] = static_cast<char>(
      static_cast<int>(static_cast<unsigned int>(offset) >> 24));
  slice[upto - 2] = static_cast<char>(
      static_cast<int>(static_cast<unsigned int>(offset) >> 16));
  slice[upto - 1] = static_cast<char>(
      static_cast<int>(static_cast<unsigned int>(offset) >> 8));
  slice[upto] = static_cast<char>(offset);

  // Write new level:
  buffer[byteUpto - 1] = static_cast<char>(16 | newLevel);

  return newUpto + 3;
}

void ByteBlockPool::setBytesRef(shared_ptr<BytesRefBuilder> builder,
                                shared_ptr<BytesRef> result, int64_t offset,
                                int length)
{
  result->length = length;

  int bufferIndex = static_cast<int>(offset >> BYTE_BLOCK_SHIFT);
  std::deque<char> buffer = buffers[bufferIndex];
  int pos = static_cast<int>(offset & BYTE_BLOCK_MASK);
  if (pos + length <= BYTE_BLOCK_SIZE) {
    // common case where the slice lives in a single block: just reference the
    // buffer directly without copying
    result->bytes = buffer;
    result->offset = pos;
  } else {
    // uncommon case: the slice spans at least 2 blocks, so we must copy the
    // bytes:
    builder->grow(length);
    result->bytes = builder->get().bytes;
    result->offset = 0;
    readBytes(offset, result->bytes, 0, length);
  }
}

void ByteBlockPool::setBytesRef(shared_ptr<BytesRef> term, int textStart)
{
  const std::deque<char> bytes = term->bytes =
      buffers[textStart >> BYTE_BLOCK_SHIFT];
  int pos = textStart & BYTE_BLOCK_MASK;
  if ((bytes[pos] & 0x80) == 0) {
    // length is 1 byte
    term->length = bytes[pos];
    term->offset = pos + 1;
  } else {
    // length is 2 bytes
    term->length = (bytes[pos] & 0x7f) + ((bytes[pos + 1] & 0xff) << 7);
    term->offset = pos + 2;
  }
  assert(term->length >= 0);
}

void ByteBlockPool::append(shared_ptr<BytesRef> bytes)
{
  int bytesLeft = bytes->length;
  int offset = bytes->offset;
  while (bytesLeft > 0) {
    int bufferLeft = BYTE_BLOCK_SIZE - byteUpto;
    if (bytesLeft < bufferLeft) {
      // fits within current buffer
      System::arraycopy(bytes->bytes, offset, buffer, byteUpto, bytesLeft);
      byteUpto += bytesLeft;
      break;
    } else {
      // fill up this buffer and move to next one
      if (bufferLeft > 0) {
        System::arraycopy(bytes->bytes, offset, buffer, byteUpto, bufferLeft);
      }
      nextBuffer();
      bytesLeft -= bufferLeft;
      offset += bufferLeft;
    }
  }
}

void ByteBlockPool::readBytes(int64_t const offset, std::deque<char> &bytes,
                              int bytesOffset, int bytesLength)
{
  int bytesLeft = bytesLength;
  int bufferIndex = static_cast<int>(offset >> BYTE_BLOCK_SHIFT);
  int pos = static_cast<int>(offset & BYTE_BLOCK_MASK);
  while (bytesLeft > 0) {
    std::deque<char> buffer = buffers[bufferIndex++];
    int chunk = min(bytesLeft, BYTE_BLOCK_SIZE - pos);
    System::arraycopy(buffer, pos, bytes, bytesOffset, chunk);
    bytesOffset += chunk;
    bytesLeft -= chunk;
    pos = 0;
  }
}

void ByteBlockPool::setRawBytesRef(shared_ptr<BytesRef> ref,
                                   int64_t const offset)
{
  int bufferIndex = static_cast<int>(offset >> BYTE_BLOCK_SHIFT);
  int pos = static_cast<int>(offset & BYTE_BLOCK_MASK);
  if (pos + ref->length <= BYTE_BLOCK_SIZE) {
    ref->bytes = buffers[bufferIndex];
    ref->offset = pos;
  } else {
    ref->bytes = std::deque<char>(ref->length);
    ref->offset = 0;
    readBytes(offset, ref->bytes, 0, ref->length);
  }
}

char ByteBlockPool::readByte(int64_t offset)
{
  int bufferIndex = static_cast<int>(offset >> BYTE_BLOCK_SHIFT);
  int pos = static_cast<int>(offset & BYTE_BLOCK_MASK);
  std::deque<char> buffer = buffers[bufferIndex];
  return buffer[pos];
}
} // namespace org::apache::lucene::util