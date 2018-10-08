using namespace std;

#include "ByteRunAutomaton.h"

namespace org::apache::lucene::util::automaton
{

ByteRunAutomaton::ByteRunAutomaton(shared_ptr<Automaton> a)
    : ByteRunAutomaton(a, false, Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

ByteRunAutomaton::ByteRunAutomaton(shared_ptr<Automaton> a, bool isBinary,
                                   int maxDeterminizedStates)
    : RunAutomaton(isBinary ? a : new UTF32ToUTF8().convert(a), 256,
                   maxDeterminizedStates)
{
}

bool ByteRunAutomaton::run(std::deque<char> &s, int offset, int length)
{
  int p = 0;
  int l = offset + length;
  for (int i = offset; i < l; i++) {
    p = step(p, s[i] & 0xFF);
    if (p == -1) {
      return false;
    }
  }
  return accept[p];
}
} // namespace org::apache::lucene::util::automaton