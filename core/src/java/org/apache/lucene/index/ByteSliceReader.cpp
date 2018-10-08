using namespace std;

#include "ByteSliceReader.h"

namespace org::apache::lucene::index
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;

void ByteSliceReader::init(shared_ptr<ByteBlockPool> pool, int startIndex,
                           int endIndex)
{

  assert(endIndex - startIndex >= 0);
  assert(startIndex >= 0);
  assert(endIndex >= 0);

  this->pool = pool;
  this->endIndex = endIndex;

  level = 0;
  bufferUpto = startIndex / ByteBlockPool::BYTE_BLOCK_SIZE;
  bufferOffset = bufferUpto * ByteBlockPool::BYTE_BLOCK_SIZE;
  buffer = pool->buffers[bufferUpto];
  upto = startIndex & ByteBlockPool::BYTE_BLOCK_MASK;

  constexpr int firstSize = ByteBlockPool::LEVEL_SIZE_ARRAY[0];

  if (startIndex + firstSize >= endIndex) {
    // There is only this one slice to read
    limit = endIndex & ByteBlockPool::BYTE_BLOCK_MASK;
  } else {
    limit = upto + firstSize - 4;
  }
}

bool ByteSliceReader::eof()
{
  assert(upto + bufferOffset <= endIndex);
  return upto + bufferOffset == endIndex;
}

char ByteSliceReader::readByte()
{
  assert(!eof());
  assert(upto <= limit);
  if (upto == limit) {
    nextSlice();
  }
  return buffer[upto++];
}

int64_t
ByteSliceReader::writeTo(shared_ptr<DataOutput> out) 
{
  int64_t size = 0;
  while (true) {
    if (limit + bufferOffset == endIndex) {
      assert(endIndex - bufferOffset >= upto);
      out->writeBytes(buffer, upto, limit - upto);
      size += limit - upto;
      break;
    } else {
      out->writeBytes(buffer, upto, limit - upto);
      size += limit - upto;
      nextSlice();
    }
  }

  return size;
}

void ByteSliceReader::nextSlice()
{

  // Skip to our next slice
  constexpr int nextIndex =
      ((buffer[limit] & 0xff) << 24) + ((buffer[1 + limit] & 0xff) << 16) +
      ((buffer[2 + limit] & 0xff) << 8) + (buffer[3 + limit] & 0xff);

  level = ByteBlockPool::NEXT_LEVEL_ARRAY[level];
  constexpr int newSize = ByteBlockPool::LEVEL_SIZE_ARRAY[level];

  bufferUpto = nextIndex / ByteBlockPool::BYTE_BLOCK_SIZE;
  bufferOffset = bufferUpto * ByteBlockPool::BYTE_BLOCK_SIZE;

  buffer = pool->buffers[bufferUpto];
  upto = nextIndex & ByteBlockPool::BYTE_BLOCK_MASK;

  if (nextIndex + newSize >= endIndex) {
    // We are advancing to the final slice
    assert(endIndex - nextIndex > 0);
    limit = endIndex - bufferOffset;
  } else {
    // This is not the final slice (subtract 4 for the
    // forwarding address at the end of this new slice)
    limit = upto + newSize - 4;
  }
}

void ByteSliceReader::readBytes(std::deque<char> &b, int offset, int len)
{
  while (len > 0) {
    constexpr int numLeft = limit - upto;
    if (numLeft < len) {
      // Read entire slice
      System::arraycopy(buffer, upto, b, offset, numLeft);
      offset += numLeft;
      len -= numLeft;
      nextSlice();
    } else {
      // This slice is the last one
      System::arraycopy(buffer, upto, b, offset, len);
      upto += len;
      break;
    }
  }
}
} // namespace org::apache::lucene::index