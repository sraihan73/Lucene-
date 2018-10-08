using namespace std;

#include "FixedBits.h"

namespace org::apache::lucene::util
{

FixedBits::FixedBits(std::deque<int64_t> &bits, int length)
    : bits(bits), length(length)
{
}

bool FixedBits::get(int index)
{
  assert((index >= 0 && index < length_,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(length_)));
  int i = index >> 6; // div 64
  // signed shift will keep a negative index and force an
  // array-index-out-of-bounds-exception, removing the need for an explicit
  // check.
  int64_t bitmask = 1LL << index;
  return (bits[i] & bitmask) != 0;
}

int FixedBits::length() { return length_; }
} // namespace org::apache::lucene::util