using namespace std;

#include "DocIdSet.h"

namespace org::apache::lucene::search
{
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;
const shared_ptr<DocIdSet> DocIdSet::EMPTY =
    make_shared<DocIdSetAnonymousInnerClass>();

DocIdSet::DocIdSetAnonymousInnerClass::DocIdSetAnonymousInnerClass() {}

shared_ptr<DocIdSetIterator> DocIdSet::DocIdSetAnonymousInnerClass::iterator()
{
  return DocIdSetIterator::empty();
}

shared_ptr<Bits> DocIdSet::DocIdSetAnonymousInnerClass::bits()
{
  return nullptr;
}

int64_t DocIdSet::DocIdSetAnonymousInnerClass::ramBytesUsed() { return 0LL; }

shared_ptr<Bits> DocIdSet::bits()  { return nullptr; }
} // namespace org::apache::lucene::search