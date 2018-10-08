using namespace std;

#include "ByteSliceWriter.h"

namespace org::apache::lucene::index
{
using DataOutput = org::apache::lucene::store::DataOutput;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;

ByteSliceWriter::ByteSliceWriter(shared_ptr<ByteBlockPool> pool) : pool(pool) {}

void ByteSliceWriter::init(int address)
{
  slice = pool->buffers[address >> ByteBlockPool::BYTE_BLOCK_SHIFT];
  assert(slice.size() > 0);
  upto = address & ByteBlockPool::BYTE_BLOCK_MASK;
  offset0 = address;
  assert(upto < slice.size());
}

void ByteSliceWriter::writeByte(char b)
{
  assert(slice.size() > 0);
  if (slice[upto] != 0) {
    upto = pool->allocSlice(slice, upto);
    slice = pool->buffer;
    offset0 = pool->byteOffset;
    assert(slice.size() > 0);
  }
  slice[upto++] = b;
  assert(upto != slice.size());
}

void ByteSliceWriter::writeBytes(std::deque<char> &b, int offset,
                                 int const len)
{
  constexpr int offsetEnd = offset + len;
  while (offset < offsetEnd) {
    if (slice[upto] != 0) {
      // End marker
      upto = pool->allocSlice(slice, upto);
      slice = pool->buffer;
      offset0 = pool->byteOffset;
    }

    slice[upto++] = b[offset++];
    assert(upto != slice.size());
  }
}

int ByteSliceWriter::getAddress()
{
  return upto + (offset0 & DocumentsWriterPerThread::BYTE_BLOCK_NOT_MASK);
}
} // namespace org::apache::lucene::index