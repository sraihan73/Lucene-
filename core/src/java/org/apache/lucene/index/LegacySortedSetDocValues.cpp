using namespace std;

#include "LegacySortedSetDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

LegacySortedSetDocValues::LegacySortedSetDocValues() {}

int64_t LegacySortedSetDocValues::lookupTerm(shared_ptr<BytesRef> key)
{
  int64_t low = 0;
  int64_t high = getValueCount() - 1;

  while (low <= high) {
    int64_t mid = static_cast<int64_t>(
        static_cast<uint64_t>((low + high)) >> 1);
    shared_ptr<BytesRef> *const term = lookupOrd(mid);
    int cmp = term->compareTo(key);

    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      return mid; // key found
    }
  }

  return -(low + 1); // key not found.
}

shared_ptr<TermsEnum> LegacySortedSetDocValues::termsEnum() 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::index