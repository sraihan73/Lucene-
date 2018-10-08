using namespace std;

#include "BoostAttributeImpl.h"

namespace org::apache::lucene::search
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

void BoostAttributeImpl::setBoost(float boost) { this->boost = boost; }

float BoostAttributeImpl::getBoost() { return boost; }

void BoostAttributeImpl::clear() { boost = 1.0f; }

void BoostAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  (std::static_pointer_cast<BoostAttribute>(target))->setBoost(boost);
}

void BoostAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(BoostAttribute::typeid, L"boost", boost);
}
} // namespace org::apache::lucene::search