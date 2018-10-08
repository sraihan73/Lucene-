using namespace std;

#include "BlockPackedWriter.h"

namespace org::apache::lucene::util::packed
{
//    import static org.apache.lucene.util.BitUtil.zigZagEncode;
using DataOutput = org::apache::lucene::store::DataOutput;

BlockPackedWriter::BlockPackedWriter(shared_ptr<DataOutput> out, int blockSize)
    : AbstractBlockPackedWriter(out, blockSize)
{
}

void BlockPackedWriter::flush() 
{
  assert(off > 0);
  int64_t min = numeric_limits<int64_t>::max(),
            max = numeric_limits<int64_t>::min();
  for (int i = 0; i < off; ++i) {
    min = min(values[i], min);
    max = max(values[i], max);
  }

  constexpr int64_t delta = max - min;
  int bitsRequired = delta == 0 ? 0 : PackedInts::unsignedBitsRequired(delta);
  if (bitsRequired == 64) {
    // no need to delta-encode
    min = 0LL;
  } else if (min > 0LL) {
    // make min as small as possible so that writeVLong requires fewer bytes
    min = max(0LL, max - PackedInts::maxValue(bitsRequired));
  }

  constexpr int token =
      (bitsRequired << BPV_SHIFT) | (min == 0 ? MIN_VALUE_EQUALS_0 : 0);
  out->writeByte(static_cast<char>(token));

  if (min != 0) {
    writeVLong(out, zigZagEncode(min) - 1);
  }

  if (bitsRequired > 0) {
    if (min != 0) {
      for (int i = 0; i < off; ++i) {
        values[i] -= min;
      }
    }
    writeValues(bitsRequired);
  }

  off = 0;
}
} // namespace org::apache::lucene::util::packed