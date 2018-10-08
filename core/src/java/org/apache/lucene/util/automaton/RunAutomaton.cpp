using namespace std;

#include "RunAutomaton.h"

namespace org::apache::lucene::util::automaton
{

RunAutomaton::RunAutomaton(shared_ptr<Automaton> a, int alphabetSize)
    : RunAutomaton(a, alphabetSize, Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

RunAutomaton::RunAutomaton(shared_ptr<Automaton> a, int alphabetSize,
                           int maxDeterminizedStates)
    : automaton(a), alphabetSize(alphabetSize), size(max(1, a->getNumStates())),
      accept(std::deque<bool>(size)),
      transitions(std::deque<int>(size * points.size())),
      points(a->getStartPoints()),
      classmap(std::deque<int>(min(256, alphabetSize)))
{
  a = Operations::determinize(a, maxDeterminizedStates);
  Arrays::fill(transitions, -1);
  for (int n = 0; n < size; n++) {
    accept[n] = a->isAccept(n);
    for (int c = 0; c < points.size(); c++) {
      int dest = a->step(n, points[c]);
      assert(dest == -1 || dest < size);
      transitions[n * points.size() + c] = dest;
    }
  }

  /*
   * Set alphabet table for optimal run performance.
   */
  int i = 0;
  for (int j = 0; j < classmap.size(); j++) {
    if (i + 1 < points.size() && j == points[i + 1]) {
      i++;
    }
    classmap[j] = i;
  }
}

wstring RunAutomaton::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"initial state: 0\n");
  for (int i = 0; i < size; i++) {
    b->append(L"state " + to_wstring(i));
    if (accept[i]) {
      b->append(L" [accept]:\n");
    } else {
      b->append(L" [reject]:\n");
    }
    for (int j = 0; j < points.size(); j++) {
      int k = transitions[i * points.size() + j];
      if (k != -1) {
        int min = points[j];
        int max;
        if (j + 1 < points.size()) {
          max = (points[j + 1] - 1);
        } else {
          max = alphabetSize;
        }
        b->append(L" ");
        Automaton::appendCharString(min, b);
        if (min != max) {
          b->append(L"-");
          Automaton::appendCharString(max, b);
        }
        b->append(L" -> ")->append(k)->append(L"\n");
      }
    }
  }
  return b->toString();
}

int RunAutomaton::getSize() { return size; }

bool RunAutomaton::isAccept(int state) { return accept[state]; }

std::deque<int> RunAutomaton::getCharIntervals() { return points.clone(); }

int RunAutomaton::getCharClass(int c)
{

  // binary search
  int a = 0;
  int b = points.size();
  while (b - a > 1) {
    int d = static_cast<int>(static_cast<unsigned int>((a + b)) >> 1);
    if (points[d] > c) {
      b = d;
    } else if (points[d] < c) {
      a = d;
    } else {
      return d;
    }
  }
  return a;
}

int RunAutomaton::step(int state, int c)
{
  assert(c < alphabetSize);
  if (c >= classmap.size()) {
    return transitions[state * points.size() + getCharClass(c)];
  } else {
    return transitions[state * points.size() + classmap[c]];
  }
}

int RunAutomaton::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + alphabetSize;
  result = prime * result + points.size();
  result = prime * result + size;
  return result;
}

bool RunAutomaton::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<RunAutomaton> other = any_cast<std::shared_ptr<RunAutomaton>>(obj);
  if (alphabetSize != other->alphabetSize) {
    return false;
  }
  if (size != other->size) {
    return false;
  }
  if (!Arrays::equals(points, other->points)) {
    return false;
  }
  if (!Arrays::equals(accept, other->accept)) {
    return false;
  }
  if (!Arrays::equals(transitions, other->transitions)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::util::automaton