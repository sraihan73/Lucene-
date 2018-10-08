using namespace std;

#include "CharacterRunAutomaton.h"

namespace org::apache::lucene::util::automaton
{

CharacterRunAutomaton::CharacterRunAutomaton(shared_ptr<Automaton> a)
    : CharacterRunAutomaton(a, Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

CharacterRunAutomaton::CharacterRunAutomaton(shared_ptr<Automaton> a,
                                             int maxDeterminizedStates)
    : RunAutomaton(a, Character::MAX_CODE_POINT + 1, maxDeterminizedStates)
{
}

bool CharacterRunAutomaton::run(const wstring &s)
{
  int p = 0;
  int l = s.length();
  for (int i = 0, cp = 0; i < l; i += Character::charCount(cp)) {
    p = step(p, cp = s.codePointAt(i));
    if (p == -1) {
      return false;
    }
  }
  return accept[p];
}

bool CharacterRunAutomaton::run(std::deque<wchar_t> &s, int offset, int length)
{
  int p = 0;
  int l = offset + length;
  for (int i = offset, cp = 0; i < l; i += Character::charCount(cp)) {
    p = step(p, cp = Character::codePointAt(s, i, l));
    if (p == -1) {
      return false;
    }
  }
  return accept[p];
}
} // namespace org::apache::lucene::util::automaton