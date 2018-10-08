using namespace std;

#include "StatePair.h"

namespace org::apache::lucene::util::automaton
{

StatePair::StatePair(int s, int s1, int s2)
{
  this->s = s;
  this->s1 = s1;
  this->s2 = s2;
}

StatePair::StatePair(int s1, int s2)
{
  this->s1 = s1;
  this->s2 = s2;
  this->s = -1;
}

bool StatePair::equals(any obj)
{
  if (std::dynamic_pointer_cast<StatePair>(obj) != nullptr) {
    shared_ptr<StatePair> p = any_cast<std::shared_ptr<StatePair>>(obj);
    return p->s1 == s1 && p->s2 == s2;
  } else {
    return false;
  }
}

int StatePair::hashCode()
{
  // Don't use s1 ^ s2 since it's vulnerable to the case where s1 == s2 always
  // --> hashCode = 0, e.g. if you call Operations.sameLanguage, passing the
  // same automaton against itself:
  return s1 * 31 + s2;
}

wstring StatePair::toString()
{
  return L"StatePair(s1=" + to_wstring(s1) + L" s2=" + to_wstring(s2) + L")";
}
} // namespace org::apache::lucene::util::automaton