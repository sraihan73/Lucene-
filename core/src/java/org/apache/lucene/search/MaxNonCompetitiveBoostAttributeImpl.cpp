using namespace std;

#include "MaxNonCompetitiveBoostAttributeImpl.h"

namespace org::apache::lucene::search
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

void MaxNonCompetitiveBoostAttributeImpl::setMaxNonCompetitiveBoost(
    float const maxNonCompetitiveBoost)
{
  this->maxNonCompetitiveBoost = maxNonCompetitiveBoost;
}

float MaxNonCompetitiveBoostAttributeImpl::getMaxNonCompetitiveBoost()
{
  return maxNonCompetitiveBoost;
}

void MaxNonCompetitiveBoostAttributeImpl::setCompetitiveTerm(
    shared_ptr<BytesRef> competitiveTerm)
{
  this->competitiveTerm = competitiveTerm;
}

shared_ptr<BytesRef> MaxNonCompetitiveBoostAttributeImpl::getCompetitiveTerm()
{
  return competitiveTerm;
}

void MaxNonCompetitiveBoostAttributeImpl::clear()
{
  maxNonCompetitiveBoost = -numeric_limits<float>::infinity();
  competitiveTerm.reset();
}

void MaxNonCompetitiveBoostAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> target)
{
  shared_ptr<MaxNonCompetitiveBoostAttributeImpl> *const t =
      std::static_pointer_cast<MaxNonCompetitiveBoostAttributeImpl>(target);
  t->setMaxNonCompetitiveBoost(maxNonCompetitiveBoost);
  t->setCompetitiveTerm(competitiveTerm);
}

void MaxNonCompetitiveBoostAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(MaxNonCompetitiveBoostAttribute::typeid, L"maxNonCompetitiveBoost",
            maxNonCompetitiveBoost);
  reflector(MaxNonCompetitiveBoostAttribute::typeid, L"competitiveTerm",
            competitiveTerm);
}
} // namespace org::apache::lucene::search