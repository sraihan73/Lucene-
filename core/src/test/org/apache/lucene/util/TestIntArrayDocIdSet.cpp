using namespace std;

#include "TestIntArrayDocIdSet.h"

namespace org::apache::lucene::util
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

shared_ptr<IntArrayDocIdSet>
TestIntArrayDocIdSet::copyOf(shared_ptr<BitSet> bs,
                             int length) 
{
  std::deque<int> docs(0);
  int l = 0;
  for (int i = bs->nextSetBit(0); i != -1; i = bs->nextSetBit(i + 1)) {
    docs = ArrayUtil::grow(docs, length + 1);
    docs[l++] = i;
  }
  docs = ArrayUtil::grow(docs, length + 1);
  docs[l] = DocIdSetIterator::NO_MORE_DOCS;
  return make_shared<IntArrayDocIdSet>(docs, l);
}
} // namespace org::apache::lucene::util