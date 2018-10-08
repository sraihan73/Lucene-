using namespace std;

#include "ICUCollatedTermAttributeImpl.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::collation::tokenattributes
{
using CharTermAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::CharTermAttributeImpl;
using BytesRef = org::apache::lucene::util::BytesRef;
using com::ibm::icu::text::Collator;
using com::ibm::icu::text::RawCollationKey;

ICUCollatedTermAttributeImpl::ICUCollatedTermAttributeImpl(
    shared_ptr<Collator> collator)
{
  // clone the collator: see
  // http://userguide.icu-project.org/collation/architecture
  try {
    this->collator = std::static_pointer_cast<Collator>(collator->clone());
  } catch (const CloneNotSupportedException &e) {
    throw runtime_error(e);
  }
}

shared_ptr<BytesRef> ICUCollatedTermAttributeImpl::getBytesRef()
{
  collator->getRawCollationKey(toString(), key);
  shared_ptr<BytesRef> *const ref = this->builder->get();
  ref->bytes = key->bytes;
  ref->offset = 0;
  ref->length = key->size;
  return ref;
}
} // namespace org::apache::lucene::collation::tokenattributes