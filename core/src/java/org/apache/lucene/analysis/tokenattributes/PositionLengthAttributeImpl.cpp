using namespace std;

#include "PositionLengthAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

PositionLengthAttributeImpl::PositionLengthAttributeImpl() {}

void PositionLengthAttributeImpl::setPositionLength(int positionLength)
{
  if (positionLength < 1) {
    throw invalid_argument(L"Position length must be 1 or greater; got " +
                           to_wstring(positionLength));
  }
  this->positionLength = positionLength;
}

int PositionLengthAttributeImpl::getPositionLength() { return positionLength; }

void PositionLengthAttributeImpl::clear() { this->positionLength = 1; }

bool PositionLengthAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<PositionLengthAttributeImpl>(other) !=
      nullptr) {
    shared_ptr<PositionLengthAttributeImpl> _other =
        any_cast<std::shared_ptr<PositionLengthAttributeImpl>>(other);
    return positionLength == _other->positionLength;
  }

  return false;
}

int PositionLengthAttributeImpl::hashCode() { return positionLength; }

void PositionLengthAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<PositionLengthAttribute> t =
      std::static_pointer_cast<PositionLengthAttribute>(target);
  t->setPositionLength(positionLength);
}

void PositionLengthAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(PositionLengthAttribute::typeid, L"positionLength", positionLength);
}
} // namespace org::apache::lucene::analysis::tokenattributes