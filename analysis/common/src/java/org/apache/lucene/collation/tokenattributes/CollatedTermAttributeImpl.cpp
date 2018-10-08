using namespace std;

#include "CollatedTermAttributeImpl.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::collation::tokenattributes
{
using CharTermAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::CharTermAttributeImpl;
using BytesRef = org::apache::lucene::util::BytesRef;

CollatedTermAttributeImpl::CollatedTermAttributeImpl(
    shared_ptr<Collator> collator)
    : collator(std::static_pointer_cast<Collator>(collator->clone()))
{
  // clone in case JRE doesn't properly sync,
  // or to reduce contention in case they do
}

shared_ptr<BytesRef> CollatedTermAttributeImpl::getBytesRef()
{
  shared_ptr<BytesRef> *const ref = this->builder->get();
  ref->bytes = collator->getCollationKey(toString()).toByteArray();
  ref->offset = 0;
  ref->length = ref->bytes.size();
  return ref;
}
} // namespace org::apache::lucene::collation::tokenattributes