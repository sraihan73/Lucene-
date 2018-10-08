using namespace std;

#include "PayloadAttributeImpl.h"
#include "../../util/AttributeReflector.h"
#include "../../util/BytesRef.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

PayloadAttributeImpl::PayloadAttributeImpl() {}

PayloadAttributeImpl::PayloadAttributeImpl(shared_ptr<BytesRef> payload)
{
  this->payload = payload;
}

shared_ptr<BytesRef> PayloadAttributeImpl::getPayload()
{
  return this->payload;
}

void PayloadAttributeImpl::setPayload(shared_ptr<BytesRef> payload)
{
  this->payload = payload;
}

void PayloadAttributeImpl::clear() { payload.reset(); }

shared_ptr<PayloadAttributeImpl> PayloadAttributeImpl::clone()
{
  shared_ptr<PayloadAttributeImpl> clone =
      std::static_pointer_cast<PayloadAttributeImpl>(AttributeImpl::clone());
  if (payload != nullptr) {
    clone->payload = BytesRef::deepCopyOf(payload);
  }
  return clone;
}

bool PayloadAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<PayloadAttribute>(other) != nullptr) {
    shared_ptr<PayloadAttributeImpl> o =
        any_cast<std::shared_ptr<PayloadAttributeImpl>>(other);
    if (o->payload == nullptr || payload == nullptr) {
      return o->payload == nullptr && payload == nullptr;
    }

    return o->payload->equals(payload);
  }

  return false;
}

int PayloadAttributeImpl::hashCode()
{
  return (payload == nullptr) ? 0 : payload->hashCode();
}

void PayloadAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<PayloadAttribute> t =
      std::static_pointer_cast<PayloadAttribute>(target);
  t->setPayload((payload == nullptr) ? nullptr : BytesRef::deepCopyOf(payload));
}

void PayloadAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(PayloadAttribute::typeid, L"payload", payload);
}
} // namespace org::apache::lucene::analysis::tokenattributes