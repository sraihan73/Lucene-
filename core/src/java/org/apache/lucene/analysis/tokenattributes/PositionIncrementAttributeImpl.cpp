using namespace std;

#include "PositionIncrementAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

PositionIncrementAttributeImpl::PositionIncrementAttributeImpl() {}

void PositionIncrementAttributeImpl::setPositionIncrement(int positionIncrement)
{
  if (positionIncrement < 0) {
    throw invalid_argument(L"Position increment must be zero or greater; got " +
                           to_wstring(positionIncrement));
  }
  this->positionIncrement = positionIncrement;
}

int PositionIncrementAttributeImpl::getPositionIncrement()
{
  return positionIncrement;
}

void PositionIncrementAttributeImpl::clear() { this->positionIncrement = 1; }

void PositionIncrementAttributeImpl::end() { this->positionIncrement = 0; }

bool PositionIncrementAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<PositionIncrementAttributeImpl>(other) !=
      nullptr) {
    shared_ptr<PositionIncrementAttributeImpl> _other =
        any_cast<std::shared_ptr<PositionIncrementAttributeImpl>>(other);
    return positionIncrement == _other->positionIncrement;
  }

  return false;
}

int PositionIncrementAttributeImpl::hashCode() { return positionIncrement; }

void PositionIncrementAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<PositionIncrementAttribute> t =
      std::static_pointer_cast<PositionIncrementAttribute>(target);
  t->setPositionIncrement(positionIncrement);
}

void PositionIncrementAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(PositionIncrementAttribute::typeid, L"positionIncrement",
            positionIncrement);
}
} // namespace org::apache::lucene::analysis::tokenattributes