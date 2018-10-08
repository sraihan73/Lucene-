using namespace std;

#include "MonotonicBlockPackedWriter.h"

namespace org::apache::lucene::util::packed
{
//    import static
//    org.apache.lucene.util.packed.MonotonicBlockPackedReader.expected;
using DataOutput = org::apache::lucene::store::DataOutput;
using BitUtil = org::apache::lucene::util::BitUtil;

MonotonicBlockPackedWriter::MonotonicBlockPackedWriter(
    shared_ptr<DataOutput> out, int blockSize)
    : AbstractBlockPackedWriter(out, blockSize)
{
}

void MonotonicBlockPackedWriter::add(int64_t l) 
{
  assert(l >= 0);
  AbstractBlockPackedWriter::add(l);
}

void MonotonicBlockPackedWriter::flush() 
{
  assert(off > 0);

  constexpr float avg =
      off == 1 ? 0.0f
               : static_cast<float>(values[off - 1] - values[0]) / (off - 1);
  int64_t min = values[0];
  // adjust min so that all deltas will be positive
  for (int i = 1; i < off; ++i) {
    constexpr int64_t actual = values[i];
    constexpr int64_t expected = expected(min, avg, i);
    if (expected > actual) {
      min -= (expected - actual);
    }
  }

  int64_t maxDelta = 0;
  for (int i = 0; i < off; ++i) {
    values[i] = values[i] - expected(min, avg, i);
    maxDelta = max(maxDelta, values[i]);
  }

  out->writeZLong(min);
  out->writeInt(Float::floatToIntBits(avg));
  if (maxDelta == 0) {
    out->writeVInt(0);
  } else {
    constexpr int bitsRequired = PackedInts::bitsRequired(maxDelta);
    out->writeVInt(bitsRequired);
    writeValues(bitsRequired);
  }

  off = 0;
}
} // namespace org::apache::lucene::util::packed