using namespace std;

#include "BlockPackedReader.h"

namespace org::apache::lucene::util::packed
{
//    import static org.apache.lucene.util.BitUtil.zigZagDecode;
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.BPV_SHIFT; import
//    static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MAX_BLOCK_SIZE;
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MIN_BLOCK_SIZE;
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MIN_VALUE_EQUALS_0;
//    import static
//    org.apache.lucene.util.packed.BlockPackedReaderIterator.readVLong; import
//    static org.apache.lucene.util.packed.PackedInts.checkBlockSize; import
//    static org.apache.lucene.util.packed.PackedInts.numBlocks;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;

BlockPackedReader::BlockPackedReader(shared_ptr<IndexInput> in_,
                                     int packedIntsVersion, int blockSize,
                                     int64_t valueCount,
                                     bool direct) 
    : blockShift(checkBlockSize(blockSize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE)),
      blockMask(blockSize - 1), valueCount(valueCount),
      subReaders(std::deque<std::shared_ptr<PackedInts::Reader>>(numBlocks)),
      sumBPV(sumBPV)
{
  constexpr int numBlocks = numBlocks(valueCount, blockSize);
  std::deque<int64_t> minValues;
  int64_t sumBPV = 0;
  for (int i = 0; i < numBlocks; ++i) {
    constexpr int token = in_->readByte() & 0xFF;
    constexpr int bitsPerValue =
        static_cast<int>(static_cast<unsigned int>(token) >> BPV_SHIFT);
    sumBPV += bitsPerValue;
    if (bitsPerValue > 64) {
      throw make_shared<CorruptIndexException>(
          L"Corrupted Block#" + to_wstring(i), in_);
    }
    if ((token & MIN_VALUE_EQUALS_0) == 0) {
      if (minValues.empty()) {
        minValues = std::deque<int64_t>(numBlocks);
      }
      minValues[i] = zigZagDecode(1LL + readVLong(in_));
    }
    if (bitsPerValue == 0) {
      subReaders[i] = make_shared<PackedInts::NullReader>(blockSize);
    } else {
      constexpr int size = static_cast<int>(
          min(blockSize, valueCount - static_cast<int64_t>(i) * blockSize));
      if (direct) {
        constexpr int64_t pointer = in_->getFilePointer();
        subReaders[i] = PackedInts::getDirectReaderNoHeader(
            in_, PackedInts::Format::PACKED, packedIntsVersion, size,
            bitsPerValue);
        in_->seek(pointer + PackedInts::Format::PACKED.byteCount(
                                packedIntsVersion, size, bitsPerValue));
      } else {
        subReaders[i] = PackedInts::getReaderNoHeader(
            in_, PackedInts::Format::PACKED, packedIntsVersion, size,
            bitsPerValue);
      }
    }
  }
  this->minValues = minValues;
}

int64_t BlockPackedReader::get(int64_t index)
{
  assert(index >= 0 && index < valueCount);
  constexpr int block = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(index) >> blockShift));
  constexpr int idx = static_cast<int>(index & blockMask);
  return (minValues.empty() ? 0 : minValues[block]) +
         subReaders[block]->get(idx);
}

int64_t BlockPackedReader::ramBytesUsed()
{
  int64_t size = 0;
  for (auto reader : subReaders) {
    size += reader->ramBytesUsed();
  }
  return size;
}

wstring BlockPackedReader::toString()
{
  int64_t avgBPV = subReaders.empty() ? 0 : sumBPV / subReaders.size();
  return getClass().getSimpleName() + L"(blocksize=" +
         to_wstring(1 << blockShift) + L",size=" + to_wstring(valueCount) +
         L",avgBPV=" + to_wstring(avgBPV) + L")";
}
} // namespace org::apache::lucene::util::packed