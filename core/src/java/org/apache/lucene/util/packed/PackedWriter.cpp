using namespace std;

#include "PackedWriter.h"

namespace org::apache::lucene::util::packed
{
using DataOutput = org::apache::lucene::store::DataOutput;

PackedWriter::PackedWriter(PackedInts::Format format,
                           shared_ptr<DataOutput> out, int valueCount,
                           int bitsPerValue, int mem)
    : PackedInts::Writer(out, valueCount, bitsPerValue), format(format),
      encoder(BulkOperation::of(format, bitsPerValue)),
      nextBlocks(std::deque<char>(iterations * encoder->byteBlockCount())),
      nextValues(
          std::deque<int64_t>(iterations * encoder->byteValueCount())),
      iterations(encoder->computeIterations(valueCount, mem))
{
  off = 0;
  written = 0;
  finished = false;
}

PackedInts::Format PackedWriter::getFormat() { return format; }

void PackedWriter::add(int64_t v) 
{
  assert(PackedInts::unsignedBitsRequired(v) <= bitsPerValue_);
  assert(!finished);
  if (valueCount != -1 && written >= valueCount) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"Writing past end of stream");
  }
  nextValues[off++] = v;
  if (off == nextValues.size()) {
    flush();
  }
  ++written;
}

void PackedWriter::finish() 
{
  assert(!finished);
  if (valueCount != -1) {
    while (written < valueCount) {
      add(0LL);
    }
  }
  flush();
  finished = true;
}

void PackedWriter::flush() 
{
  encoder->encode(nextValues, 0, nextBlocks, 0, iterations);
  constexpr int blockCount = static_cast<int>(
      format.byteCount(PackedInts::VERSION_CURRENT, off, bitsPerValue_));
  out->writeBytes(nextBlocks, blockCount);
  Arrays::fill(nextValues, 0LL);
  off = 0;
}

int PackedWriter::ord() { return written - 1; }
} // namespace org::apache::lucene::util::packed