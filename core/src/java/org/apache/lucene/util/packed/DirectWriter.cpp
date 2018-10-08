using namespace std;

#include "DirectWriter.h"

namespace org::apache::lucene::util::packed
{
using DataOutput = org::apache::lucene::store::DataOutput;

DirectWriter::DirectWriter(shared_ptr<DataOutput> output, int64_t numValues,
                           int bitsPerValue)
    : bitsPerValue(bitsPerValue), numValues(numValues), output(output),
      nextBlocks(std::deque<char>(iterations * encoder->byteBlockCount())),
      nextValues(
          std::deque<int64_t>(iterations * encoder->byteValueCount())),
      encoder(BulkOperation::of(PackedInts::Format::PACKED, bitsPerValue)),
      iterations(encoder->computeIterations(
          static_cast<int>(min(numValues, numeric_limits<int>::max())),
          PackedInts::DEFAULT_BUFFER_SIZE))
{
}

void DirectWriter::add(int64_t l) 
{
  assert((bitsPerValue == 64 ||
              (l >= 0 && l <= PackedInts::maxValue(bitsPerValue)),
          bitsPerValue));
  assert(!finished);
  if (count >= numValues) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"Writing past end of stream");
  }
  nextValues[off++] = l;
  if (off == nextValues.size()) {
    flush();
  }
  count++;
}

void DirectWriter::flush() 
{
  encoder->encode(nextValues, 0, nextBlocks, 0, iterations);
  constexpr int blockCount =
      static_cast<int>(PackedInts::Format::PACKED.byteCount(
          PackedInts::VERSION_CURRENT, off, bitsPerValue));
  output->writeBytes(nextBlocks, blockCount);
  Arrays::fill(nextValues, 0LL);
  off = 0;
}

void DirectWriter::finish() 
{
  if (count != numValues) {
    throw make_shared<IllegalStateException>(
        L"Wrong number of values added, expected: " + to_wstring(numValues) +
        L", got: " + to_wstring(count));
  }
  assert(!finished);
  flush();
  // pad for fast io: we actually only need this for certain BPV, but its just 3
  // bytes...
  for (int i = 0; i < 3; i++) {
    output->writeByte(static_cast<char>(0));
  }
  finished = true;
}

shared_ptr<DirectWriter>
DirectWriter::getInstance(shared_ptr<DataOutput> output, int64_t numValues,
                          int bitsPerValue)
{
  if (Arrays::binarySearch(SUPPORTED_BITS_PER_VALUE, bitsPerValue) < 0) {
    throw invalid_argument(L"Unsupported bitsPerValue " +
                           to_wstring(bitsPerValue) +
                           L". Did you use bitsRequired?");
  }
  return make_shared<DirectWriter>(output, numValues, bitsPerValue);
}

int DirectWriter::roundBits(int bitsRequired)
{
  int index = Arrays::binarySearch(SUPPORTED_BITS_PER_VALUE, bitsRequired);
  if (index < 0) {
    return SUPPORTED_BITS_PER_VALUE[-index - 1];
  } else {
    return bitsRequired;
  }
}

int DirectWriter::bitsRequired(int64_t maxValue)
{
  return roundBits(PackedInts::bitsRequired(maxValue));
}

int DirectWriter::unsignedBitsRequired(int64_t maxValue)
{
  return roundBits(PackedInts::unsignedBitsRequired(maxValue));
}

std::deque<int> const DirectWriter::SUPPORTED_BITS_PER_VALUE =
    std::deque<int>{1, 2, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64};
} // namespace org::apache::lucene::util::packed