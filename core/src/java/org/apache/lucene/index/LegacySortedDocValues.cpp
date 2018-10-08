using namespace std;

#include "LegacySortedDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

LegacySortedDocValues::LegacySortedDocValues() {}

shared_ptr<BytesRef> LegacySortedDocValues::get(int docID)
{
  int ord = getOrd(docID);
  if (ord == -1) {
    return empty;
  } else {
    return lookupOrd(ord);
  }
}

int LegacySortedDocValues::lookupTerm(shared_ptr<BytesRef> key)
{
  int low = 0;
  int high = getValueCount() - 1;

  while (low <= high) {
    int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
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

shared_ptr<TermsEnum> LegacySortedDocValues::termsEnum()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::index