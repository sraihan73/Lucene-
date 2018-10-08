using namespace std;

#include "BitDocIdSet.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

BitDocIdSet::BitDocIdSet(shared_ptr<BitSet> set, int64_t cost)
    : set(set), cost(cost)
{
  if (cost < 0) {
    throw invalid_argument(L"cost must be >= 0, got " + to_wstring(cost));
  }
}

BitDocIdSet::BitDocIdSet(shared_ptr<BitSet> set)
    : BitDocIdSet(set, set->approximateCardinality())
{
}

shared_ptr<DocIdSetIterator> BitDocIdSet::iterator()
{
  return make_shared<BitSetIterator>(set, cost);
}

shared_ptr<BitSet> BitDocIdSet::bits() { return set; }

int64_t BitDocIdSet::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + set->ramBytesUsed();
}

wstring BitDocIdSet::toString()
{
  return getClass().getSimpleName() + L"(set=" + set + L",cost=" +
         to_wstring(cost) + L")";
}
} // namespace org::apache::lucene::util