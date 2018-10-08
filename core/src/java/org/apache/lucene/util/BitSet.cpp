using namespace std;

#include "BitSet.h"

namespace org::apache::lucene::util
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

shared_ptr<BitSet> BitSet::of(shared_ptr<DocIdSetIterator> it,
                              int maxDoc) 
{
  constexpr int64_t cost = it->cost();
  constexpr int threshold =
      static_cast<int>(static_cast<unsigned int>(maxDoc) >> 7);
  shared_ptr<BitSet> set;
  if (cost < threshold) {
    set = make_shared<SparseFixedBitSet>(maxDoc);
  } else {
    set = make_shared<FixedBitSet>(maxDoc);
  }
  set->or (it);
  return set;
}

int BitSet::approximateCardinality() { return cardinality(); }

void BitSet::checkUnpositioned(shared_ptr<DocIdSetIterator> iter)
{
  if (iter->docID() != -1) {
    throw make_shared<IllegalStateException>(
        L"This operation only works with an unpositioned iterator, got current "
        L"position = " +
        to_wstring(iter->docID()));
  }
}

void BitSet:: or (shared_ptr<DocIdSetIterator> iter) 
{
  checkUnpositioned(iter);
  for (int doc = iter->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = iter->nextDoc()) {
    set(doc);
  }
}
} // namespace org::apache::lucene::util