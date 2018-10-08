using namespace std;

#include "TestSparseFixedBitDocIdSet.h"

namespace org::apache::lucene::util
{

shared_ptr<BitDocIdSet>
TestSparseFixedBitDocIdSet::copyOf(shared_ptr<BitSet> bs,
                                   int length) 
{
  shared_ptr<SparseFixedBitSet> *const set =
      make_shared<SparseFixedBitSet>(length);
  // SparseFixedBitSet can be sensitive to the order of insertion so
  // randomize insertion a bit
  deque<int> buffer = deque<int>();
  for (int doc = bs->nextSetBit(0); doc != -1; doc = bs->nextSetBit(doc + 1)) {
    buffer.push_back(doc);
    if (buffer.size() >= 100000) {
      Collections::shuffle(buffer, random());
      for (auto i : buffer) {
        set->set(i);
      }
      buffer.clear();
    }
  }
  Collections::shuffle(buffer, random());
  for (auto i : buffer) {
    set->set(i);
  }
  return make_shared<BitDocIdSet>(set, set->approximateCardinality());
}

void TestSparseFixedBitDocIdSet::assertEquals(
    int numBits, shared_ptr<BitSet> ds1,
    shared_ptr<BitDocIdSet> ds2) 
{
  for (int i = 0; i < numBits; ++i) {
    assertEquals(ds1->get(i), ds2->bits()->get(i));
  }
  assertEquals(ds1->cardinality(), ds2->bits()->cardinality());
  BaseDocIdSetTestCase<BitDocIdSet>::assertEquals(numBits, ds1, ds2);
}
} // namespace org::apache::lucene::util