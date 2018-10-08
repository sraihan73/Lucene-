using namespace std;

#include "TestFixedBitDocIdSet.h"

namespace org::apache::lucene::util
{

shared_ptr<BitDocIdSet>
TestFixedBitDocIdSet::copyOf(shared_ptr<BitSet> bs,
                             int length) 
{
  shared_ptr<FixedBitSet> *const set = make_shared<FixedBitSet>(length);
  for (int doc = bs->nextSetBit(0); doc != -1; doc = bs->nextSetBit(doc + 1)) {
    set->set(doc);
  }
  return make_shared<BitDocIdSet>(set);
}
} // namespace org::apache::lucene::util