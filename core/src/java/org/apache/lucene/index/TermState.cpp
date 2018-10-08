using namespace std;

#include "TermState.h"

namespace org::apache::lucene::index
{

TermState::TermState() {}

shared_ptr<TermState> TermState::clone()
{
  try {
    return std::static_pointer_cast<TermState>(__super::clone());
  } catch (const CloneNotSupportedException &cnse) {
    // should not happen
    throw runtime_error(cnse);
  }
}

wstring TermState::toString() { return L"TermState"; }
} // namespace org::apache::lucene::index