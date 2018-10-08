using namespace std;

#include "OrdTermState.h"

namespace org::apache::lucene::index
{

OrdTermState::OrdTermState() {}

void OrdTermState::copyFrom(shared_ptr<TermState> other)
{
  assert((std::dynamic_pointer_cast<OrdTermState>(other) != nullptr,
          L"can not copy from " + other->getClassName()));
  this->ord = (std::static_pointer_cast<OrdTermState>(other))->ord;
}

wstring OrdTermState::toString()
{
  return L"OrdTermState ord=" + to_wstring(ord);
}
} // namespace org::apache::lucene::index