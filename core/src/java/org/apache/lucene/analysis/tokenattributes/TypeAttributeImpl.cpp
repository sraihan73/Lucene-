using namespace std;

#include "TypeAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

TypeAttributeImpl::TypeAttributeImpl() : TypeAttributeImpl(DEFAULT_TYPE) {}

TypeAttributeImpl::TypeAttributeImpl(const wstring &type)
{
  this->type_ = type;
}

wstring TypeAttributeImpl::type() { return type_; }

void TypeAttributeImpl::setType(const wstring &type) { this->type_ = type; }

void TypeAttributeImpl::clear() { type_ = DEFAULT_TYPE; }

bool TypeAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<TypeAttributeImpl>(other) != nullptr) {
    shared_ptr<TypeAttributeImpl> *const o =
        any_cast<std::shared_ptr<TypeAttributeImpl>>(other);
    return (this->type_ == L"" ? o->type_ == L"" : this->type_ == o->type_);
  }

  return false;
}

int TypeAttributeImpl::hashCode()
{
  return (type_ == L"") ? 0 : type_.hashCode();
}

void TypeAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<TypeAttribute> t = std::static_pointer_cast<TypeAttribute>(target);
  t->setType(type_);
}

void TypeAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(TypeAttribute::typeid, L"type", type_);
}
} // namespace org::apache::lucene::analysis::tokenattributes