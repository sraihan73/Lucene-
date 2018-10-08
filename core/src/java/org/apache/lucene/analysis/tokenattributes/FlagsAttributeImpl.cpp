using namespace std;

#include "FlagsAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

FlagsAttributeImpl::FlagsAttributeImpl() {}

int FlagsAttributeImpl::getFlags() { return flags; }

void FlagsAttributeImpl::setFlags(int flags) { this->flags = flags; }

void FlagsAttributeImpl::clear() { flags = 0; }

bool FlagsAttributeImpl::equals(any other)
{
  if (shared_from_this() == other) {
    return true;
  }

  if (std::dynamic_pointer_cast<FlagsAttributeImpl>(other) != nullptr) {
    return (any_cast<std::shared_ptr<FlagsAttributeImpl>>(other))->flags ==
           flags;
  }

  return false;
}

int FlagsAttributeImpl::hashCode() { return flags; }

void FlagsAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<FlagsAttribute> t =
      std::static_pointer_cast<FlagsAttribute>(target);
  t->setFlags(flags);
}

void FlagsAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(FlagsAttribute::typeid, L"flags", flags);
}
} // namespace org::apache::lucene::analysis::tokenattributes