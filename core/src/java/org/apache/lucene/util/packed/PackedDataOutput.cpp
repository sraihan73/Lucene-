using namespace std;

#include "PackedDataOutput.h"

namespace org::apache::lucene::util::packed
{
using DataOutput = org::apache::lucene::store::DataOutput;

PackedDataOutput::PackedDataOutput(shared_ptr<DataOutput> out) : out(out)
{
  current = 0;
  remainingBits = 8;
}

void PackedDataOutput::writeLong(int64_t value,
                                 int bitsPerValue) 
{
  assert(bitsPerValue == 64 ||
         (value >= 0 && value <= PackedInts::maxValue(bitsPerValue)));
  while (bitsPerValue > 0) {
    if (remainingBits == 0) {
      out->writeByte(static_cast<char>(current));
      current = 0LL;
      remainingBits = 8;
    }
    constexpr int bits = min(remainingBits, bitsPerValue);
    current =
        current |
        (((static_cast<int64_t>(static_cast<uint64_t>(value) >>
                                  (bitsPerValue - bits))) &
          ((1LL << bits) - 1))
         << (remainingBits - bits));
    bitsPerValue -= bits;
    remainingBits -= bits;
  }
}

void PackedDataOutput::flush() 
{
  if (remainingBits < 8) {
    out->writeByte(static_cast<char>(current));
  }
  remainingBits = 8;
  current = 0LL;
}
} // namespace org::apache::lucene::util::packed