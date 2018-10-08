using namespace std;

#include "DirectPacked64SingleBlockReader.h"

namespace org::apache::lucene::util::packed
{
using IndexInput = org::apache::lucene::store::IndexInput;

DirectPacked64SingleBlockReader::DirectPacked64SingleBlockReader(
    int bitsPerValue, int valueCount, shared_ptr<IndexInput> in_)
    : PackedInts::ReaderImpl(valueCount), in_(in_), bitsPerValue(bitsPerValue),
      startPointer(in_->getFilePointer()), valuesPerBlock(64 / bitsPerValue),
      mask(~(~0LL << bitsPerValue))
{
}

int64_t DirectPacked64SingleBlockReader::get(int index)
{
  constexpr int blockOffset = index / valuesPerBlock;
  constexpr int64_t skip = (static_cast<int64_t>(blockOffset)) << 3;
  try {
    in_->seek(startPointer + skip);

    int64_t block = in_->readLong();
    constexpr int offsetInBlock = index % valuesPerBlock;
    return (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                   (offsetInBlock * bitsPerValue))) &
           mask;
  } catch (const IOException &e) {
    throw make_shared<IllegalStateException>(L"failed", e);
  }
}

int64_t DirectPacked64SingleBlockReader::ramBytesUsed() { return 0; }
} // namespace org::apache::lucene::util::packed