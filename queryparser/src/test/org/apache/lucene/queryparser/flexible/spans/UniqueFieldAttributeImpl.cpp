using namespace std;

#include "UniqueFieldAttributeImpl.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

UniqueFieldAttributeImpl::UniqueFieldAttributeImpl() { clear(); }

void UniqueFieldAttributeImpl::clear() { this->uniqueField = L""; }

void UniqueFieldAttributeImpl::setUniqueField(
    shared_ptr<std::wstring> uniqueField)
{
  this->uniqueField = uniqueField;
}

shared_ptr<std::wstring> UniqueFieldAttributeImpl::getUniqueField()
{
  return this->uniqueField;
}

void UniqueFieldAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{

  if (!(std::dynamic_pointer_cast<UniqueFieldAttributeImpl>(target) !=
        nullptr)) {
    throw invalid_argument(L"cannot copy the values from attribute "
                           L"UniqueFieldAttribute to an instance of " +
                           target->getClassName());
  }

  shared_ptr<UniqueFieldAttributeImpl> uniqueFieldAttr =
      std::static_pointer_cast<UniqueFieldAttributeImpl>(target);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  uniqueFieldAttr->uniqueField = uniqueField->toString();
}

bool UniqueFieldAttributeImpl::equals(any other)
{

  if (std::dynamic_pointer_cast<UniqueFieldAttributeImpl>(other) != nullptr) {

    return (any_cast<std::shared_ptr<UniqueFieldAttributeImpl>>(other))
        .uniqueField::equals(this->uniqueField);
  }

  return false;
}

int UniqueFieldAttributeImpl::hashCode()
{
  return this->uniqueField->hashCode();
}

void UniqueFieldAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(UniqueFieldAttribute::typeid, L"uniqueField", uniqueField);
}
} // namespace org::apache::lucene::queryparser::flexible::spans