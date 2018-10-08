using namespace std;

#include "DirectMonotonicWriter.h"

namespace org::apache::lucene::util::packed
{
using IndexOutput = org::apache::lucene::store::IndexOutput;

DirectMonotonicWriter::DirectMonotonicWriter(shared_ptr<IndexOutput> metaOut,
                                             shared_ptr<IndexOutput> dataOut,
                                             int64_t numValues,
                                             int blockShift)
    : meta(metaOut), data(dataOut), numValues(numValues),
      baseDataPointer(dataOut->getFilePointer()),
      buffer(std::deque<int64_t>(blockSize))
{
  if (blockShift < 2 || blockShift > 30) {
    throw invalid_argument(L"blockShift must be in [3-30], got " +
                           to_wstring(blockShift));
  }
  constexpr int blockSize = 1 << blockShift;
  this->bufferSize = 0;
}

void DirectMonotonicWriter::flush() 
{
  assert(bufferSize != 0);

  constexpr float avgInc = static_cast<float>(
      static_cast<double>(buffer[bufferSize - 1] - buffer[0]) /
      max(1, bufferSize - 1));
  for (int i = 0; i < bufferSize; ++i) {
    constexpr int64_t expected =
        static_cast<int64_t>(avgInc * static_cast<int64_t>(i));
    buffer[i] -= expected;
  }

  int64_t min = buffer[0];
  for (int i = 1; i < bufferSize; ++i) {
    min = min(buffer[i], min);
  }

  int64_t maxDelta = 0;
  for (int i = 0; i < bufferSize; ++i) {
    buffer[i] -= min;
    // use | will change nothing when it comes to computing required bits
    // but has the benefit of working fine with negative values too
    // (in case of overflow)
    maxDelta |= buffer[i];
  }

  meta->writeLong(min);
  meta->writeInt(Float::floatToIntBits(avgInc));
  meta->writeLong(data->getFilePointer() - baseDataPointer);
  if (maxDelta == 0) {
    meta->writeByte(static_cast<char>(0));
  } else {
    constexpr int bitsRequired = DirectWriter::unsignedBitsRequired(maxDelta);
    shared_ptr<DirectWriter> writer =
        DirectWriter::getInstance(data, bufferSize, bitsRequired);
    for (int i = 0; i < bufferSize; ++i) {
      writer->add(buffer[i]);
    }
    writer->finish();
    meta->writeByte(static_cast<char>(bitsRequired));
  }
  bufferSize = 0;
}

void DirectMonotonicWriter::add(int64_t v) 
{
  if (v < previous) {
    throw invalid_argument(L"Values do not come in order: " +
                           to_wstring(previous) + L", " + to_wstring(v));
  }
  if (bufferSize == buffer.size()) {
    flush();
  }
  buffer[bufferSize++] = v;
  previous = v;
  count++;
}

void DirectMonotonicWriter::finish() 
{
  if (count != numValues) {
    throw make_shared<IllegalStateException>(
        L"Wrong number of values added, expected: " + to_wstring(numValues) +
        L", got: " + to_wstring(count));
  }
  if (finished) {
    throw make_shared<IllegalStateException>(
        L"#finish has been called already");
  }
  if (bufferSize > 0) {
    flush();
  }
  finished = true;
}

shared_ptr<DirectMonotonicWriter>
DirectMonotonicWriter::getInstance(shared_ptr<IndexOutput> metaOut,
                                   shared_ptr<IndexOutput> dataOut,
                                   int64_t numValues, int blockShift)
{
  return make_shared<DirectMonotonicWriter>(metaOut, dataOut, numValues,
                                            blockShift);
}
} // namespace org::apache::lucene::util::packed