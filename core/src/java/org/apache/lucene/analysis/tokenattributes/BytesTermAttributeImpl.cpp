using namespace std;

#include "BytesTermAttributeImpl.h"
#include "../../util/AttributeReflector.h"
#include "../../util/BytesRef.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

BytesTermAttributeImpl::BytesTermAttributeImpl() {}

shared_ptr<BytesRef> BytesTermAttributeImpl::getBytesRef() { return bytes; }

void BytesTermAttributeImpl::setBytesRef(shared_ptr<BytesRef> bytes)
{
  this->bytes = bytes;
}

void BytesTermAttributeImpl::clear() { this->bytes.reset(); }

void BytesTermAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<BytesTermAttributeImpl> other =
      std::static_pointer_cast<BytesTermAttributeImpl>(target);
  other->bytes = bytes == nullptr ? nullptr : BytesRef::deepCopyOf(bytes);
}

shared_ptr<AttributeImpl> BytesTermAttributeImpl::clone()
{
  shared_ptr<BytesTermAttributeImpl> c =
      std::static_pointer_cast<BytesTermAttributeImpl>(AttributeImpl::clone());
  copyTo(c);
  return c;
}

void BytesTermAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(TermToBytesRefAttribute::typeid, L"bytes", bytes);
}

bool BytesTermAttributeImpl::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<BytesTermAttributeImpl>(o) != nullptr)) {
    return false;
  }
  shared_ptr<BytesTermAttributeImpl> that =
      any_cast<std::shared_ptr<BytesTermAttributeImpl>>(o);
  return Objects::equals(bytes, that->bytes);
}

int BytesTermAttributeImpl::hashCode() { return Objects::hash(bytes); }
} // namespace org::apache::lucene::analysis::tokenattributes