using namespace std;

#include "BlockPackedReaderIterator.h"

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
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using LongsRef = org::apache::lucene::util::LongsRef;

int64_t BlockPackedReaderIterator::readVLong(shared_ptr<DataInput> in_) throw(
    IOException)
{
  char b = in_->readByte();
  if (b >= 0) {
    return b;
  }
  int64_t i = b & 0x7FLL;
  b = in_->readByte();
  i |= (b & 0x7FLL) << 7;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0x7FLL) << 14;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0x7FLL) << 21;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0x7FLL) << 28;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0x7FLL) << 35;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0x7FLL) << 42;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0x7FLL) << 49;
  if (b >= 0) {
    return i;
  }
  b = in_->readByte();
  i |= (b & 0xFFLL) << 56;
  return i;
}

BlockPackedReaderIterator::BlockPackedReaderIterator(shared_ptr<DataInput> in_,
                                                     int packedIntsVersion,
                                                     int blockSize,
                                                     int64_t valueCount)
    : packedIntsVersion(packedIntsVersion), blockSize(blockSize),
      values(std::deque<int64_t>(blockSize)),
      valuesRef(make_shared<LongsRef>(this->values, 0, 0))
{
  checkBlockSize(blockSize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE);
  reset(in_, valueCount);
}

void BlockPackedReaderIterator::reset(shared_ptr<DataInput> in_,
                                      int64_t valueCount)
{
  this->in_ = in_;
  assert(valueCount >= 0);
  this->valueCount = valueCount;
  off = blockSize;
  ord_ = 0;
}

void BlockPackedReaderIterator::skip(int64_t count) 
{
  assert(count >= 0);
  if (ord_ + count > valueCount || ord_ + count < 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException();
  }

  // 1. skip buffered values
  constexpr int skipBuffer = static_cast<int>(min(count, blockSize - off));
  off += skipBuffer;
  ord_ += skipBuffer;
  count -= skipBuffer;
  if (count == 0LL) {
    return;
  }

  // 2. skip as many blocks as necessary
  assert(off == blockSize);
  while (count >= blockSize) {
    constexpr int token = in_->readByte() & 0xFF;
    constexpr int bitsPerValue =
        static_cast<int>(static_cast<unsigned int>(token) >> BPV_SHIFT);
    if (bitsPerValue > 64) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"Corrupted");
    }
    if ((token & MIN_VALUE_EQUALS_0) == 0) {
      readVLong(in_);
    }
    constexpr int64_t blockBytes = PackedInts::Format::PACKED.byteCount(
        packedIntsVersion, blockSize, bitsPerValue);
    skipBytes(blockBytes);
    ord_ += blockSize;
    count -= blockSize;
  }
  if (count == 0LL) {
    return;
  }

  // 3. skip last values
  assert(count < blockSize);
  refill();
  ord_ += count;
  off += count;
}

void BlockPackedReaderIterator::skipBytes(int64_t count) 
{
  if (std::dynamic_pointer_cast<IndexInput>(in_) != nullptr) {
    shared_ptr<IndexInput> *const iin =
        std::static_pointer_cast<IndexInput>(in_);
    iin->seek(iin->getFilePointer() + count);
  } else {
    if (blocks.empty()) {
      blocks = std::deque<char>(blockSize);
    }
    int64_t skipped = 0;
    while (skipped < count) {
      constexpr int toSkip =
          static_cast<int>(min(blocks.size(), count - skipped));
      in_->readBytes(blocks, 0, toSkip);
      skipped += toSkip;
    }
  }
}

int64_t BlockPackedReaderIterator::next() 
{
  if (ord_ == valueCount) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException();
  }
  if (off == blockSize) {
    refill();
  }
  constexpr int64_t value = values[off++];
  ++ord_;
  return value;
}

shared_ptr<LongsRef>
BlockPackedReaderIterator::next(int count) 
{
  assert(count > 0);
  if (ord_ == valueCount) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException();
  }
  if (off == blockSize) {
    refill();
  }

  count = min(count, blockSize - off);
  count = static_cast<int>(min(count, valueCount - ord_));

  valuesRef->offset = off;
  valuesRef->length = count;
  off += count;
  ord_ += count;
  return valuesRef;
}

void BlockPackedReaderIterator::refill() 
{
  constexpr int token = in_->readByte() & 0xFF;
  constexpr bool minEquals0 = (token & MIN_VALUE_EQUALS_0) != 0;
  constexpr int bitsPerValue =
      static_cast<int>(static_cast<unsigned int>(token) >> BPV_SHIFT);
  if (bitsPerValue > 64) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Corrupted");
  }
  constexpr int64_t minValue =
      minEquals0 ? 0LL : zigZagDecode(1LL + readVLong(in_));
  assert(minEquals0 || minValue != 0);

  if (bitsPerValue == 0) {
    Arrays::fill(values, minValue);
  } else {
    shared_ptr<PackedInts::Decoder> *const decoder = PackedInts::getDecoder(
        PackedInts::Format::PACKED, packedIntsVersion, bitsPerValue);
    constexpr int iterations = blockSize / decoder->byteValueCount();
    constexpr int blocksSize = iterations * decoder->byteBlockCount();
    if (blocks.empty() || blocks.size() < blocksSize) {
      blocks = std::deque<char>(blocksSize);
    }

    constexpr int valueCount =
        static_cast<int>(min(this->valueCount - ord_, blockSize));
    constexpr int blocksCount =
        static_cast<int>(PackedInts::Format::PACKED.byteCount(
            packedIntsVersion, valueCount, bitsPerValue));
    in_->readBytes(blocks, 0, blocksCount);

    decoder->decode(blocks, 0, values, 0, iterations);

    if (minValue != 0) {
      for (int i = 0; i < valueCount; ++i) {
        values[i] += minValue;
      }
    }
  }
  off = 0;
}

int64_t BlockPackedReaderIterator::ord() { return ord_; }
} // namespace org::apache::lucene::util::packed