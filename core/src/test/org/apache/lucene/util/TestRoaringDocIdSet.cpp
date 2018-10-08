using namespace std;

#include "TestRoaringDocIdSet.h"

namespace org::apache::lucene::util
{

shared_ptr<RoaringDocIdSet>
TestRoaringDocIdSet::copyOf(shared_ptr<BitSet> bs,
                            int length) 
{
  shared_ptr<RoaringDocIdSet::Builder> *const builder =
      make_shared<RoaringDocIdSet::Builder>(length);
  for (int i = bs->nextSetBit(0); i != -1; i = bs->nextSetBit(i + 1)) {
    builder->add(i);
  }
  return builder->build();
}

void TestRoaringDocIdSet::assertEquals(
    int numBits, shared_ptr<BitSet> ds1,
    shared_ptr<RoaringDocIdSet> ds2) 
{
  BaseDocIdSetTestCase<RoaringDocIdSet>::assertEquals(numBits, ds1, ds2);
  assertEquals(ds1->cardinality(), ds2->cardinality());
}
} // namespace org::apache::lucene::util