using namespace std;

#include "BitSetIterator.h"

namespace org::apache::lucene::util
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

template <typename T>
T BitSetIterator::getBitSet(shared_ptr<DocIdSetIterator> iterator,
                            type_info clazz)
{
  static_assert(is_base_of<BitSet, T>::value, L"T must inherit from BitSet");

  if (std::dynamic_pointer_cast<BitSetIterator>(iterator) != nullptr) {
    shared_ptr<BitSet> bits =
        (std::static_pointer_cast<BitSetIterator>(iterator))->bits;
    assert(bits != nullptr);
    if (clazz.isInstance(bits)) {
      return clazz.cast(bits);
    }
  }
  return nullptr;
}

shared_ptr<FixedBitSet>
BitSetIterator::getFixedBitSetOrNull(shared_ptr<DocIdSetIterator> iterator)
{
  return getBitSet(iterator, FixedBitSet::typeid);
}

shared_ptr<SparseFixedBitSet> BitSetIterator::getSparseFixedBitSetOrNull(
    shared_ptr<DocIdSetIterator> iterator)
{
  return getBitSet(iterator, SparseFixedBitSet::typeid);
}

BitSetIterator::BitSetIterator(shared_ptr<BitSet> bits, int64_t cost)
    : bits(bits), length(bits->length()), cost(cost)
{
  if (cost < 0) {
    throw invalid_argument(L"cost must be >= 0, got " + to_wstring(cost));
  }
}

shared_ptr<BitSet> BitSetIterator::getBitSet() { return bits; }

int BitSetIterator::docID() { return doc; }

void BitSetIterator::setDocId(int docId) { this->doc = docId; }

int BitSetIterator::nextDoc() { return advance(doc + 1); }

int BitSetIterator::advance(int target)
{
  if (target >= length) {
    return doc = NO_MORE_DOCS;
  }
  return doc = bits->nextSetBit(target);
}

int64_t BitSetIterator::cost() { return cost_; }
} // namespace org::apache::lucene::util