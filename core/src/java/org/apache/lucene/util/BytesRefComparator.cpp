using namespace std;

#include "BytesRefComparator.h"

namespace org::apache::lucene::util
{

BytesRefComparator::BytesRefComparator(int comparedBytesCount)
    : comparedBytesCount(comparedBytesCount)
{
}

int BytesRefComparator::compare(shared_ptr<BytesRef> o1,
                                shared_ptr<BytesRef> o2)
{
  for (int i = 0; i < comparedBytesCount; ++i) {
    constexpr int b1 = byteAt(o1, i);
    constexpr int b2 = byteAt(o2, i);
    if (b1 != b2) {
      return b1 - b2;
    } else if (b1 == -1) {
      break;
    }
  }
  return 0;
}
} // namespace org::apache::lucene::util