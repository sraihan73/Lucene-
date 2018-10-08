using namespace std;

#include "PackedDataInput.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;

PackedDataInput::PackedDataInput(shared_ptr<DataInput> in_) : in_(in_)
{
  skipToNextByte();
}

int64_t PackedDataInput::readLong(int bitsPerValue) 
{
  assert((bitsPerValue > 0 && bitsPerValue <= 64, bitsPerValue));
  int64_t r = 0;
  while (bitsPerValue > 0) {
    if (remainingBits == 0) {
      current = in_->readByte() & 0xFF;
      remainingBits = 8;
    }
    constexpr int bits = min(bitsPerValue, remainingBits);
    r = (r << bits) |
        ((static_cast<int64_t>(static_cast<uint64_t>(current) >>
                                 (remainingBits - bits))) &
         ((1LL << bits) - 1));
    bitsPerValue -= bits;
    remainingBits -= bits;
  }
  return r;
}

void PackedDataInput::skipToNextByte() { remainingBits = 0; }
} // namespace org::apache::lucene::util::packed