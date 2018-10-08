using namespace std;

#include "BytesRefIterator.h"

namespace org::apache::lucene::util
{

const shared_ptr<BytesRefIterator> BytesRefIterator::EMPTY =
    make_shared<BytesRefIteratorAnonymousInnerClass>();

BytesRefIteratorAnonymousInnerClass::BytesRefIteratorAnonymousInnerClass() {}

shared_ptr<BytesRef> BytesRefIteratorAnonymousInnerClass::next()
{
  return nullptr;
}
} // namespace org::apache::lucene::util