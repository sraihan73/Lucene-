using namespace std;

#include "MonotonicBlockPackedReader.h"

namespace org::apache::lucene::util::packed
{
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MAX_BLOCK_SIZE;
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MIN_BLOCK_SIZE;
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
//    import static org.apache.lucene.util.packed.PackedInts.numBlocks;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

int64_t MonotonicBlockPackedReader::expected(int64_t origin, float average,
                                               int index)
{
  return origin +
         static_cast<int64_t>(average * static_cast<int64_t>(index));
}

shared_ptr<MonotonicBlockPackedReader> MonotonicBlockPackedReader::of(
    shared_ptr<IndexInput> in_, int packedIntsVersion, int blockSize,
    int64_t valueCount, bool direct) 
{
  return make_shared<MonotonicBlockPackedReader>(in_, packedIntsVersion,
                                                 blockSize, valueCount, direct);
}

MonotonicBlockPackedReader::MonotonicBlockPackedReader(
    shared_ptr<IndexInput> in_, int packedIntsVersion, int blockSize,
    int64_t valueCount, bool direct) 
    : blockShift(checkBlockSize(blockSize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE)),
      blockMask(blockSize - 1), valueCount(valueCount),
      minValues(std::deque<int64_t>(numBlocks)),
      averages(std::deque<float>(numBlocks)),
      subReaders(std::deque<std::shared_ptr<PackedInts::Reader>>(numBlocks)),
      sumBPV(sumBPV)
{
  constexpr int numBlocks = numBlocks(valueCount, blockSize);
  int64_t sumBPV = 0;
  for (int i = 0; i < numBlocks; ++i) {
    minValues[i] = in_->readZLong();
    averages[i] = Float::intBitsToFloat(in_->readInt());
    constexpr int bitsPerValue = in_->readVInt();
    sumBPV += bitsPerValue;
    if (bitsPerValue > 64) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"Corrupted");
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
}

int64_t MonotonicBlockPackedReader::get(int64_t index)
{
  assert(index >= 0 && index < valueCount);
  constexpr int block = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(index) >> blockShift));
  constexpr int idx = static_cast<int>(index & blockMask);
  return expected(minValues[block], averages[block], idx) +
         subReaders[block]->get(idx);
}

int64_t MonotonicBlockPackedReader::size() { return valueCount; }

int64_t MonotonicBlockPackedReader::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  sizeInBytes += RamUsageEstimator::sizeOf(minValues);
  sizeInBytes += RamUsageEstimator::sizeOf(averages);
  for (auto reader : subReaders) {
    sizeInBytes += reader->ramBytesUsed();
  }
  return sizeInBytes;
}

wstring MonotonicBlockPackedReader::toString()
{
  int64_t avgBPV = subReaders.empty() ? 0 : sumBPV / subReaders.size();
  return getClass().getSimpleName() + L"(blocksize=" +
         to_wstring(1 << blockShift) + L",size=" + to_wstring(valueCount) +
         L",avgBPV=" + to_wstring(avgBPV) + L")";
}
} // namespace org::apache::lucene::util::packed