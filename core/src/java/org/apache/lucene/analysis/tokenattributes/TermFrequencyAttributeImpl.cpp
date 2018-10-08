using namespace std;

#include "TermFrequencyAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

TermFrequencyAttributeImpl::TermFrequencyAttributeImpl() {}

void TermFrequencyAttributeImpl::setTermFrequency(int termFrequency)
{
  if (termFrequency < 1) {
    throw invalid_argument(L"Term frequency must be 1 or greater; got " +
                           to_wstring(termFrequency));
  }
  this->termFrequency = termFrequency;
}

int TermFrequencyAttributeImpl::getTermFrequency() { return termFrequency; }

void TermFrequencyAttributeImpl::clear() { this->termFrequency = 1; }

void TermFrequencyAttributeImpl::end() { this->termFrequency = 1; }

bool TermFrequencyAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<TermFrequencyAttributeImpl>(other) != nullptr) {
    shared_ptr<TermFrequencyAttributeImpl> _other =
        any_cast<std::shared_ptr<TermFrequencyAttributeImpl>>(other);
    return termFrequency == _other->termFrequency;
  }

  return false;
}

int TermFrequencyAttributeImpl::hashCode()
{
  return Integer::hashCode(termFrequency);
}

void TermFrequencyAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<TermFrequencyAttribute> t =
      std::static_pointer_cast<TermFrequencyAttribute>(target);
  t->setTermFrequency(termFrequency);
}

void TermFrequencyAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(TermFrequencyAttribute::typeid, L"termFrequency", termFrequency);
}
} // namespace org::apache::lucene::analysis::tokenattributes