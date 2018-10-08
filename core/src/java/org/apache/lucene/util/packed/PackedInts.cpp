using namespace std;

#include "PackedInts.h"

namespace org::apache::lucene::util::packed
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using LegacyNumericDocValues =
    org::apache::lucene::index::LegacyNumericDocValues;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongsRef = org::apache::lucene::util::LongsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const wstring PackedInts::CODEC_NAME = L"PackedInts";

void PackedInts::checkVersion(int version)
{
  if (version < VERSION_START) {
    throw invalid_argument(L"Version is too old, should be at least " +
                           to_wstring(VERSION_START) + L" (got " +
                           to_wstring(version) + L")");
  } else if (version > VERSION_CURRENT) {
    throw invalid_argument(L"Version is too new, should be at most " +
                           to_wstring(VERSION_CURRENT) + L" (got " +
                           to_wstring(version) + L")");
  }
}

PackedInts::FormatAndBits::FormatAndBits(Format format, int bitsPerValue)
    : format(format), bitsPerValue(bitsPerValue)
{
}

wstring PackedInts::FormatAndBits::toString()
{
  return L"FormatAndBits(format=" + format + L" bitsPerValue=" +
         to_wstring(bitsPerValue) + L")";
}

shared_ptr<FormatAndBits>
PackedInts::fastestFormatAndBits(int valueCount, int bitsPerValue,
                                 float acceptableOverheadRatio)
{
  if (valueCount == -1) {
    valueCount = numeric_limits<int>::max();
  }

  acceptableOverheadRatio = max(COMPACT, acceptableOverheadRatio);
  acceptableOverheadRatio = min(FASTEST, acceptableOverheadRatio);
  float acceptableOverheadPerValue =
      acceptableOverheadRatio * bitsPerValue; // in bits

  int maxBitsPerValue =
      bitsPerValue + static_cast<int>(acceptableOverheadPerValue);

  int actualBitsPerValue = -1;
  Format format = Format::PACKED;

  if (bitsPerValue <= 8 && maxBitsPerValue >= 8) {
    actualBitsPerValue = 8;
  } else if (bitsPerValue <= 16 && maxBitsPerValue >= 16) {
    actualBitsPerValue = 16;
  } else if (bitsPerValue <= 32 && maxBitsPerValue >= 32) {
    actualBitsPerValue = 32;
  } else if (bitsPerValue <= 64 && maxBitsPerValue >= 64) {
    actualBitsPerValue = 64;
  } else if (valueCount <= Packed8ThreeBlocks::MAX_SIZE && bitsPerValue <= 24 &&
             maxBitsPerValue >= 24) {
    actualBitsPerValue = 24;
  } else if (valueCount <= Packed16ThreeBlocks::MAX_SIZE &&
             bitsPerValue <= 48 && maxBitsPerValue >= 48) {
    actualBitsPerValue = 48;
  } else {
    for (int bpv = bitsPerValue; bpv <= maxBitsPerValue; ++bpv) {
      if (Format::PACKED_SINGLE_BLOCK.isSupported(bpv)) {
        float overhead = Format::PACKED_SINGLE_BLOCK.overheadPerValue(bpv);
        float acceptableOverhead =
            acceptableOverheadPerValue + bitsPerValue - bpv;
        if (overhead <= acceptableOverhead) {
          actualBitsPerValue = bpv;
          format = Format::PACKED_SINGLE_BLOCK;
          break;
        }
      }
    }
    if (actualBitsPerValue < 0) {
      actualBitsPerValue = bitsPerValue;
    }
  }

  return make_shared<FormatAndBits>(format, actualBitsPerValue);
}

int PackedInts::Reader::get(int index, std::deque<int64_t> &arr, int off,
                            int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < size());
  assert(off + len <= arr.size());

  constexpr int gets = min(size() - index, len);
  for (int i = index, o = off, end = index + gets; i < end; ++i, ++o) {
    arr[o] = get(i);
  }
  return gets;
}

PackedInts::ReaderIteratorImpl::ReaderIteratorImpl(int valueCount,
                                                   int bitsPerValue,
                                                   shared_ptr<DataInput> in_)
    : in_(in_), bitsPerValue(bitsPerValue), valueCount(valueCount)
{
}

int64_t PackedInts::ReaderIteratorImpl::next() 
{
  shared_ptr<LongsRef> nextValues = next(1);
  assert(nextValues->length > 0);
  constexpr int64_t result = nextValues->longs[nextValues->offset];
  ++nextValues->offset;
  --nextValues->length;
  return result;
}

int PackedInts::ReaderIteratorImpl::getBitsPerValue() { return bitsPerValue; }

int PackedInts::ReaderIteratorImpl::size() { return valueCount; }

int PackedInts::Mutable::set(int index, std::deque<int64_t> &arr, int off,
                             int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < size());
  len = min(len, size() - index);
  assert(off + len <= arr.size());

  for (int i = index, o = off, end = index + len; i < end; ++i, ++o) {
    set(i, arr[o]);
  }
  return len;
}

void PackedInts::Mutable::fill(int fromIndex, int toIndex, int64_t val)
{
  assert(val <= maxValue(getBitsPerValue()));
  assert(fromIndex <= toIndex);
  for (int i = fromIndex; i < toIndex; ++i) {
    set(i, val);
  }
}

void PackedInts::Mutable::clear() { fill(0, size(), 0); }

void PackedInts::Mutable::save(shared_ptr<DataOutput> out) 
{
  shared_ptr<Writer> writer = getWriterNoHeader(
      out, getFormat(), size(), getBitsPerValue(), DEFAULT_BUFFER_SIZE);
  writer->writeHeader();
  for (int i = 0; i < size(); ++i) {
    writer->add(get(i));
  }
  writer->finish();
}

Format PackedInts::Mutable::getFormat() { return Format::PACKED; }

PackedInts::ReaderImpl::ReaderImpl(int valueCount) : valueCount(valueCount) {}

int PackedInts::ReaderImpl::size() { return valueCount; }

PackedInts::MutableImpl::MutableImpl(int valueCount, int bitsPerValue)
    : valueCount(valueCount), bitsPerValue(bitsPerValue)
{
  assert((bitsPerValue > 0 && bitsPerValue <= 64,
          L"bitsPerValue=" + to_wstring(bitsPerValue)));
}

int PackedInts::MutableImpl::getBitsPerValue() { return bitsPerValue; }

int PackedInts::MutableImpl::size() { return valueCount; }

wstring PackedInts::MutableImpl::toString()
{
  return getClass().getSimpleName() + L"(valueCount=" + to_wstring(valueCount) +
         L",bitsPerValue=" + to_wstring(bitsPerValue) + L")";
}

PackedInts::NullReader::NullReader(int valueCount) : valueCount(valueCount) {}

int64_t PackedInts::NullReader::get(int index) { return 0; }

int PackedInts::NullReader::get(int index, std::deque<int64_t> &arr, int off,
                                int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  len = min(len, valueCount - index);
  Arrays::fill(arr, off, off + len, 0);
  return len;
}

int PackedInts::NullReader::size() { return valueCount; }

int64_t PackedInts::NullReader::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
      RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + Integer::BYTES);
}

PackedInts::Writer::Writer(shared_ptr<DataOutput> out, int valueCount,
                           int bitsPerValue)
    : out(out), valueCount(valueCount), bitsPerValue(bitsPerValue)
{
  assert(bitsPerValue <= 64);
  assert(valueCount >= 0 || valueCount == -1);
}

void PackedInts::Writer::writeHeader() 
{
  assert(valueCount != -1);
  CodecUtil::writeHeader(out, CODEC_NAME, VERSION_CURRENT);
  out->writeVInt(bitsPerValue_);
  out->writeVInt(valueCount);
  out->writeVInt(getFormat().getId());
}

int PackedInts::Writer::bitsPerValue() { return bitsPerValue_; }

shared_ptr<Decoder> PackedInts::getDecoder(Format format, int version,
                                           int bitsPerValue)
{
  checkVersion(version);
  return BulkOperation::of(format, bitsPerValue);
}

shared_ptr<Encoder> PackedInts::getEncoder(Format format, int version,
                                           int bitsPerValue)
{
  checkVersion(version);
  return BulkOperation::of(format, bitsPerValue);
}

shared_ptr<Reader>
PackedInts::getReaderNoHeader(shared_ptr<DataInput> in_, Format format,
                              int version, int valueCount,
                              int bitsPerValue) 
{
  checkVersion(version);
  switch (format) {
  case org::apache::lucene::util::packed::PackedInts::Format::
      PACKED_SINGLE_BLOCK:
    return Packed64SingleBlock::create(in_, valueCount, bitsPerValue);
  case org::apache::lucene::util::packed::PackedInts::Format::PACKED:
    switch (bitsPerValue) {
    case 8:
      return make_shared<Direct8>(version, in_, valueCount);
    case 16:
      return make_shared<Direct16>(version, in_, valueCount);
    case 32:
      return make_shared<Direct32>(version, in_, valueCount);
    case 64:
      return make_shared<Direct64>(version, in_, valueCount);
    case 24:
      if (valueCount <= Packed8ThreeBlocks::MAX_SIZE) {
        return make_shared<Packed8ThreeBlocks>(version, in_, valueCount);
      }
      break;
    case 48:
      if (valueCount <= Packed16ThreeBlocks::MAX_SIZE) {
        return make_shared<Packed16ThreeBlocks>(version, in_, valueCount);
      }
      break;
    }
    return make_shared<Packed64>(version, in_, valueCount, bitsPerValue);
  default:
    throw make_shared<AssertionError>(L"Unknown Writer format: " + format);
  }
}

shared_ptr<Reader>
PackedInts::getReader(shared_ptr<DataInput> in_) 
{
  constexpr int version =
      CodecUtil::checkHeader(in_, CODEC_NAME, VERSION_START, VERSION_CURRENT);
  constexpr int bitsPerValue = in_->readVInt();
  assert((bitsPerValue > 0 && bitsPerValue <= 64,
          L"bitsPerValue=" + to_wstring(bitsPerValue)));
  constexpr int valueCount = in_->readVInt();
  constexpr Format format = Format::byId(in_->readVInt());

  return getReaderNoHeader(in_, format, version, valueCount, bitsPerValue);
}

shared_ptr<ReaderIterator>
PackedInts::getReaderIteratorNoHeader(shared_ptr<DataInput> in_, Format format,
                                      int version, int valueCount,
                                      int bitsPerValue, int mem)
{
  checkVersion(version);
  return make_shared<PackedReaderIterator>(format, version, valueCount,
                                           bitsPerValue, in_, mem);
}

shared_ptr<ReaderIterator>
PackedInts::getReaderIterator(shared_ptr<DataInput> in_,
                              int mem) 
{
  constexpr int version =
      CodecUtil::checkHeader(in_, CODEC_NAME, VERSION_START, VERSION_CURRENT);
  constexpr int bitsPerValue = in_->readVInt();
  assert((bitsPerValue > 0 && bitsPerValue <= 64,
          L"bitsPerValue=" + to_wstring(bitsPerValue)));
  constexpr int valueCount = in_->readVInt();
  constexpr Format format = Format::byId(in_->readVInt());
  return getReaderIteratorNoHeader(in_, format, version, valueCount,
                                   bitsPerValue, mem);
}

shared_ptr<Reader>
PackedInts::getDirectReaderNoHeader(shared_ptr<IndexInput> in_, Format format,
                                    int version, int valueCount,
                                    int bitsPerValue)
{
  checkVersion(version);
  switch (format) {
  case org::apache::lucene::util::packed::PackedInts::Format::PACKED:
    return make_shared<DirectPackedReader>(bitsPerValue, valueCount, in_);
  case org::apache::lucene::util::packed::PackedInts::Format::
      PACKED_SINGLE_BLOCK:
    return make_shared<DirectPacked64SingleBlockReader>(bitsPerValue,
                                                        valueCount, in_);
  default:
    throw make_shared<AssertionError>(L"Unknwown format: " + format);
  }
}

shared_ptr<Reader>
PackedInts::getDirectReader(shared_ptr<IndexInput> in_) 
{
  constexpr int version =
      CodecUtil::checkHeader(in_, CODEC_NAME, VERSION_START, VERSION_CURRENT);
  constexpr int bitsPerValue = in_->readVInt();
  assert((bitsPerValue > 0 && bitsPerValue <= 64,
          L"bitsPerValue=" + to_wstring(bitsPerValue)));
  constexpr int valueCount = in_->readVInt();
  constexpr Format format = Format::byId(in_->readVInt());
  return getDirectReaderNoHeader(in_, format, version, valueCount,
                                 bitsPerValue);
}

shared_ptr<Mutable> PackedInts::getMutable(int valueCount, int bitsPerValue,
                                           float acceptableOverheadRatio)
{
  shared_ptr<FormatAndBits> *const formatAndBits =
      fastestFormatAndBits(valueCount, bitsPerValue, acceptableOverheadRatio);
  return getMutable(valueCount, formatAndBits->bitsPerValue,
                    formatAndBits->format);
}

shared_ptr<Mutable> PackedInts::getMutable(int valueCount, int bitsPerValue,
                                           PackedInts::Format format)
{
  assert(valueCount >= 0);
  switch (format) {
  case org::apache::lucene::util::packed::PackedInts::Format::
      PACKED_SINGLE_BLOCK:
    return Packed64SingleBlock::create(valueCount, bitsPerValue);
  case org::apache::lucene::util::packed::PackedInts::Format::PACKED:
    switch (bitsPerValue) {
    case 8:
      return make_shared<Direct8>(valueCount);
    case 16:
      return make_shared<Direct16>(valueCount);
    case 32:
      return make_shared<Direct32>(valueCount);
    case 64:
      return make_shared<Direct64>(valueCount);
    case 24:
      if (valueCount <= Packed8ThreeBlocks::MAX_SIZE) {
        return make_shared<Packed8ThreeBlocks>(valueCount);
      }
      break;
    case 48:
      if (valueCount <= Packed16ThreeBlocks::MAX_SIZE) {
        return make_shared<Packed16ThreeBlocks>(valueCount);
      }
      break;
    }
    return make_shared<Packed64>(valueCount, bitsPerValue);
  default:
    throw make_shared<AssertionError>();
  }
}

shared_ptr<Writer> PackedInts::getWriterNoHeader(shared_ptr<DataOutput> out,
                                                 Format format, int valueCount,
                                                 int bitsPerValue, int mem)
{
  return make_shared<PackedWriter>(format, out, valueCount, bitsPerValue, mem);
}

shared_ptr<Writer>
PackedInts::getWriter(shared_ptr<DataOutput> out, int valueCount,
                      int bitsPerValue,
                      float acceptableOverheadRatio) 
{
  assert(valueCount >= 0);

  shared_ptr<FormatAndBits> *const formatAndBits =
      fastestFormatAndBits(valueCount, bitsPerValue, acceptableOverheadRatio);
  shared_ptr<Writer> *const writer =
      getWriterNoHeader(out, formatAndBits->format, valueCount,
                        formatAndBits->bitsPerValue, DEFAULT_BUFFER_SIZE);
  writer->writeHeader();
  return writer;
}

int PackedInts::bitsRequired(int64_t maxValue)
{
  if (maxValue < 0) {
    throw invalid_argument(L"maxValue must be non-negative (got: " +
                           to_wstring(maxValue) + L")");
  }
  return unsignedBitsRequired(maxValue);
}

int PackedInts::unsignedBitsRequired(int64_t bits)
{
  return max(1, 64 - Long::numberOfLeadingZeros(bits));
}

int64_t PackedInts::maxValue(int bitsPerValue)
{
  return bitsPerValue == 64 ? numeric_limits<int64_t>::max()
                            : ~(~0LL << bitsPerValue);
}

void PackedInts::copy(shared_ptr<Reader> src, int srcPos,
                      shared_ptr<Mutable> dest, int destPos, int len, int mem)
{
  assert(srcPos + len <= src->size());
  assert(destPos + len <= dest->size());
  constexpr int capacity =
      static_cast<int>(static_cast<unsigned int>(mem) >> 3);
  if (capacity == 0) {
    for (int i = 0; i < len; ++i) {
      dest->set(destPos++, src->get(srcPos++));
    }
  } else if (len > 0) {
    // use bulk operations
    const std::deque<int64_t> buf =
        std::deque<int64_t>(min(capacity, len));
    copy(src, srcPos, dest, destPos, len, buf);
  }
}

void PackedInts::copy(shared_ptr<Reader> src, int srcPos,
                      shared_ptr<Mutable> dest, int destPos, int len,
                      std::deque<int64_t> &buf)
{
  assert(buf.size() > 0);
  int remaining = 0;
  while (len > 0) {
    constexpr int read =
        src->get(srcPos, buf, remaining, min(len, buf.size() - remaining));
    assert(read > 0);
    srcPos += read;
    len -= read;
    remaining += read;
    constexpr int written = dest->set(destPos, buf, 0, remaining);
    assert(written > 0);
    destPos += written;
    if (written < remaining) {
      System::arraycopy(buf, written, buf, 0, remaining - written);
    }
    remaining -= written;
  }
  while (remaining > 0) {
    constexpr int written = dest->set(destPos, buf, 0, remaining);
    destPos += written;
    remaining -= written;
    System::arraycopy(buf, written, buf, 0, remaining);
  }
}

int PackedInts::checkBlockSize(int blockSize, int minBlockSize,
                               int maxBlockSize)
{
  if (blockSize < minBlockSize || blockSize > maxBlockSize) {
    throw invalid_argument(L"blockSize must be >= " + to_wstring(minBlockSize) +
                           L" and <= " + to_wstring(maxBlockSize) + L", got " +
                           to_wstring(blockSize));
  }
  if ((blockSize & (blockSize - 1)) != 0) {
    throw invalid_argument(L"blockSize must be a power of two, got " +
                           to_wstring(blockSize));
  }
  return Integer::numberOfTrailingZeros(blockSize);
}

int PackedInts::numBlocks(int64_t size, int blockSize)
{
  constexpr int numBlocks =
      static_cast<int>(size / blockSize) + (size % blockSize == 0 ? 0 : 1);
  if (static_cast<int64_t>(numBlocks) * blockSize < size) {
    throw invalid_argument(L"size is too large for this block size");
  }
  return numBlocks;
}
} // namespace org::apache::lucene::util::packed