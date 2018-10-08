using namespace std;

#include "DocsWithFieldSet.h"

namespace org::apache::lucene::index
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
int64_t DocsWithFieldSet::BASE_RAM_BYTES_USED =
    org::apache::lucene::util::RamUsageEstimator::shallowSizeOfInstance(
        DocsWithFieldSet::typeid);

void DocsWithFieldSet::add(int docID)
{
  if (docID <= lastDocId) {
    throw invalid_argument(L"Out of order doc ids: last=" +
                           to_wstring(lastDocId) + L", next=" +
                           to_wstring(docID));
  }
  if (set != nullptr) {
    set = FixedBitSet::ensureCapacity(set, docID);
    set->set(docID);
  } else if (docID != cost) {
    // migrate to a sparse encoding using a bit set
    set = make_shared<FixedBitSet>(docID + 1);
    set->set(0, cost);
    set->set(docID);
  }
  lastDocId = docID;
  cost++;
}

int64_t DocsWithFieldSet::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + (set == nullptr ? 0 : set->ramBytesUsed());
}

shared_ptr<DocIdSetIterator> DocsWithFieldSet::iterator()
{
  return set != nullptr ? make_shared<BitSetIterator>(set, cost)
                        : DocIdSetIterator::all(cost);
}
} // namespace org::apache::lucene::index