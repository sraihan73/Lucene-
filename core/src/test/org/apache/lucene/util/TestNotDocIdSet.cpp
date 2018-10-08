using namespace std;

#include "TestNotDocIdSet.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;

shared_ptr<NotDocIdSet> TestNotDocIdSet::copyOf(shared_ptr<BitSet> bs,
                                                int length) 
{
  shared_ptr<FixedBitSet> *const set = make_shared<FixedBitSet>(length);
  for (int doc = bs->nextClearBit(0); doc < length;
       doc = bs->nextClearBit(doc + 1)) {
    set->set(doc);
  }
  return make_shared<NotDocIdSet>(length, make_shared<BitDocIdSet>(set));
}

void TestNotDocIdSet::assertEquals(
    int numBits, shared_ptr<BitSet> ds1,
    shared_ptr<NotDocIdSet> ds2) 
{
  BaseDocIdSetTestCase<NotDocIdSet>::assertEquals(numBits, ds1, ds2);
  shared_ptr<Bits> *const bits2 = ds2->bits();
  assertNotNull(bits2); // since we wrapped a FixedBitSet
  assertEquals(numBits, bits2->length());
  for (int i = 0; i < numBits; ++i) {
    assertEquals(ds1->get(i), bits2->get(i));
  }
}

void TestNotDocIdSet::testBits() 
{
  assertNull((make_shared<NotDocIdSet>(3, DocIdSet::EMPTY))->bits());
  assertNotNull((make_shared<NotDocIdSet>(
                     3, make_shared<BitDocIdSet>(make_shared<FixedBitSet>(3))))
                    ->bits());
}
} // namespace org::apache::lucene::util