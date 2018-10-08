using namespace std;

#include "TestSparseFixedBitSet.h"

namespace org::apache::lucene::util
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

shared_ptr<SparseFixedBitSet>
TestSparseFixedBitSet::copyOf(shared_ptr<BitSet> bs,
                              int length) 
{
  shared_ptr<SparseFixedBitSet> *const set =
      make_shared<SparseFixedBitSet>(length);
  for (int doc = bs->nextSetBit(0); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = doc + 1 >= length ? DocIdSetIterator::NO_MORE_DOCS
                               : bs->nextSetBit(doc + 1)) {
    set->set(doc);
  }
  return set;
}

void TestSparseFixedBitSet::assertEquals(shared_ptr<BitSet> set1,
                                         shared_ptr<SparseFixedBitSet> set2,
                                         int maxDoc)
{
  BaseBitSetTestCase<SparseFixedBitSet>::assertEquals(set1, set2, maxDoc);
  // check invariants of the sparse set
  int nonZeroLongCount = 0;
  for (int i = 0; i < set2->indices.size(); ++i) {
    constexpr int n = Long::bitCount(set2->indices[i]);
    if (n != 0) {
      nonZeroLongCount += n;
      for (int j = n; j < set2->bits[i].length; ++j) {
        assertEquals(0, set2->bits[i][j]);
      }
    }
  }
  assertEquals(nonZeroLongCount, set2->nonZeroLongCount);
}

void TestSparseFixedBitSet::testApproximateCardinality()
{
  shared_ptr<SparseFixedBitSet> *const set =
      make_shared<SparseFixedBitSet>(10000);
  constexpr int first = random().nextInt(1000);
  constexpr int interval = 200 + random().nextInt(1000);
  for (int i = first; i < set->length(); i += interval) {
    set->set(i);
  }
  assertEquals(set->cardinality(), set->approximateCardinality(), 20);
}

void TestSparseFixedBitSet::testApproximateCardinalityOnDenseSet()
{
  // this tests that things work as expected in approximateCardinality when
  // all longs are different than 0, in which case we divide by zero
  constexpr int numDocs = TestUtil::nextInt(random(), 1, 10000);
  shared_ptr<SparseFixedBitSet> *const set =
      make_shared<SparseFixedBitSet>(numDocs);
  for (int i = 0; i < set->length(); ++i) {
    set->set(i);
  }
  assertEquals(numDocs, set->approximateCardinality());
}
} // namespace org::apache::lucene::util