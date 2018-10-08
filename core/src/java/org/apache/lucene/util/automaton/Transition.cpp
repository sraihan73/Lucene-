using namespace std;

#include "Transition.h"

namespace org::apache::lucene::util::automaton
{

Transition::Transition() {}

wstring Transition::toString()
{
  return to_wstring(source) + L" --> " + to_wstring(dest) + L" " +
         StringHelper::toString(static_cast<wchar_t>(min)) + L"-" +
         StringHelper::toString(static_cast<wchar_t>(max));
}
} // namespace org::apache::lucene::util::automaton