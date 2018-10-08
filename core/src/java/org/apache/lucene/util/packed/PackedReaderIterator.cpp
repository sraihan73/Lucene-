using namespace std;

#include "PackedReaderIterator.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using LongsRef = org::apache::lucene::util::LongsRef;

PackedReaderIterator::PackedReaderIterator(PackedInts::Format format,
                                           int packedIntsVersion,
                                           int valueCount, int bitsPerValue,
                                           shared_ptr<DataInput> in_, int mem)
    : PackedInts::ReaderIteratorImpl(valueCount, bitsPerValue, in_),
      packedIntsVersion(packedIntsVersion), format(format),
      bulkOperation(BulkOperation::of(format, bitsPerValue)),
      nextBlocks(
          std::deque<char>(iterations * bulkOperation->byteBlockCount())),
      nextValues(make_shared<LongsRef>(
          std::deque<int64_t>(iterations * bulkOperation->byteValueCount()),
          0, 0)),
      iterations(bulkOperation->computeIterations(valueCount, mem))
{
  assert(valueCount == 0 || iterations > 0);
  nextValues->offset = nextValues->longs.size();
  position = -1;
}

shared_ptr<LongsRef> PackedReaderIterator::next(int count) 
{
  assert(nextValues->length >= 0);
  assert(count > 0);
  assert(nextValues->offset + nextValues->length <= nextValues->longs.size());

  nextValues->offset += nextValues->length;

  constexpr int remaining = valueCount - position - 1;
  if (remaining <= 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException();
  }
  count = min(remaining, count);

  if (nextValues->offset == nextValues->longs.size()) {
    constexpr int64_t remainingBlocks =
        format.byteCount(packedIntsVersion, remaining, bitsPerValue);
    constexpr int blocksToRead =
        static_cast<int>(min(remainingBlocks, nextBlocks.size()));
    in_->readBytes(nextBlocks, 0, blocksToRead);
    if (blocksToRead < nextBlocks.size()) {
      Arrays::fill(nextBlocks, blocksToRead, nextBlocks.size(),
                   static_cast<char>(0));
    }

    bulkOperation->decode(nextBlocks, 0, nextValues->longs, 0, iterations);
    nextValues->offset = 0;
  }

  nextValues->length =
      min(nextValues->longs.size() - nextValues->offset, count);
  position += nextValues->length;
  return nextValues;
}

int PackedReaderIterator::ord() { return position; }
} // namespace org::apache::lucene::util::packed