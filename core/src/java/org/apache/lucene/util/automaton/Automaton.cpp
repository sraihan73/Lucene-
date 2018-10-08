using namespace std;

#include "Automaton.h"

namespace org::apache::lucene::util::automaton
{
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using FutureObjects = org::apache::lucene::util::FutureObjects;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Sorter = org::apache::lucene::util::Sorter;

Automaton::Automaton() : Automaton(2, 2) {}

Automaton::Automaton(int numStates, int numTransitions)
    : isAccept(make_shared<BitSet>(numStates))
{
  states = std::deque<int>(numStates * 2);
  transitions = std::deque<int>(numTransitions * 3);
}

int Automaton::createState()
{
  growStates();
  int state = nextState / 2;
  states[nextState] = -1;
  nextState += 2;
  return state;
}

void Automaton::setAccept(int state, bool accept)
{
  FutureObjects::checkIndex(state, getNumStates());
  isAccept_->set(state, accept);
}

std::deque<std::deque<std::shared_ptr<Transition>>>
Automaton::getSortedTransitions()
{
  int numStates = getNumStates();
  std::deque<std::deque<std::shared_ptr<Transition>>> transitions(numStates);
  for (int s = 0; s < numStates; s++) {
    int numTransitions = getNumTransitions(s);
    transitions[s] = std::deque<std::shared_ptr<Transition>>(numTransitions);
    for (int t = 0; t < numTransitions; t++) {
      shared_ptr<Transition> transition = make_shared<Transition>();
      getTransition(s, t, transition);
      transitions[s][t] = transition;
    }
  }

  return transitions;
}

shared_ptr<BitSet> Automaton::getAcceptStates() { return isAccept_; }

bool Automaton::isAccept(int state) { return isAccept_->get(state); }

void Automaton::addTransition(int source, int dest, int label)
{
  addTransition(source, dest, label, label);
}

void Automaton::addTransition(int source, int dest, int min, int max)
{
  assert(nextTransition % 3 == 0);

  int bounds = nextState / 2;
  FutureObjects::checkIndex(source, bounds);
  FutureObjects::checkIndex(dest, bounds);

  growTransitions();
  if (curState != source) {
    if (curState != -1) {
      finishCurrentState();
    }

    // Move to next source:
    curState = source;
    if (states[2 * curState] != -1) {
      throw make_shared<IllegalStateException>(
          L"from state (" + to_wstring(source) +
          L") already had transitions added");
    }
    assert(states[2 * curState + 1] == 0);
    states[2 * curState] = nextTransition;
  }

  transitions[nextTransition++] = dest;
  transitions[nextTransition++] = min;
  transitions[nextTransition++] = max;

  // Increment transition count for this state
  states[2 * curState + 1]++;
}

void Automaton::addEpsilon(int source, int dest)
{
  shared_ptr<Transition> t = make_shared<Transition>();
  int count = initTransition(dest, t);
  for (int i = 0; i < count; i++) {
    getNextTransition(t);
    addTransition(source, t->dest, t->min, t->max);
  }
  if (isAccept(dest)) {
    setAccept(source, true);
  }
}

void Automaton::copy(shared_ptr<Automaton> other)
{

  // Bulk copy and then fixup the state pointers:
  int stateOffset = getNumStates();
  states = ArrayUtil::grow(states, nextState + other->nextState);
  System::arraycopy(other->states, 0, states, nextState, other->nextState);
  for (int i = 0; i < other->nextState; i += 2) {
    if (states[nextState + i] != -1) {
      states[nextState + i] += nextTransition;
    }
  }
  nextState += other->nextState;
  int otherNumStates = other->getNumStates();
  shared_ptr<BitSet> otherAcceptStates = other->getAcceptStates();
  int state = 0;
  while (state < otherNumStates &&
         (state = otherAcceptStates->nextSetBit(state)) != -1) {
    setAccept(stateOffset + state, true);
    state++;
  }

  // Bulk copy and then fixup dest for each transition:
  transitions =
      ArrayUtil::grow(transitions, nextTransition + other->nextTransition);
  System::arraycopy(other->transitions, 0, transitions, nextTransition,
                    other->nextTransition);
  for (int i = 0; i < other->nextTransition; i += 3) {
    transitions[nextTransition + i] += stateOffset;
  }
  nextTransition += other->nextTransition;

  if (other->deterministic == false) {
    deterministic = false;
  }
}

void Automaton::finishCurrentState()
{
  int numTransitions = states[2 * curState + 1];
  assert(numTransitions > 0);

  int offset = states[2 * curState];
  int start = offset / 3;
  destMinMaxSorter::sort(start, start + numTransitions);

  // Reduce any "adjacent" transitions:
  int upto = 0;
  int min = -1;
  int max = -1;
  int dest = -1;

  for (int i = 0; i < numTransitions; i++) {
    int tDest = transitions[offset + 3 * i];
    int tMin = transitions[offset + 3 * i + 1];
    int tMax = transitions[offset + 3 * i + 2];

    if (dest == tDest) {
      if (tMin <= max + 1) {
        if (tMax > max) {
          max = tMax;
        }
      } else {
        if (dest != -1) {
          transitions[offset + 3 * upto] = dest;
          transitions[offset + 3 * upto + 1] = min;
          transitions[offset + 3 * upto + 2] = max;
          upto++;
        }
        min = tMin;
        max = tMax;
      }
    } else {
      if (dest != -1) {
        transitions[offset + 3 * upto] = dest;
        transitions[offset + 3 * upto + 1] = min;
        transitions[offset + 3 * upto + 2] = max;
        upto++;
      }
      dest = tDest;
      min = tMin;
      max = tMax;
    }
  }

  if (dest != -1) {
    // Last transition
    transitions[offset + 3 * upto] = dest;
    transitions[offset + 3 * upto + 1] = min;
    transitions[offset + 3 * upto + 2] = max;
    upto++;
  }

  nextTransition -= (numTransitions - upto) * 3;
  states[2 * curState + 1] = upto;

  // Sort transitions by min/max/dest:
  minMaxDestSorter::sort(start, start + upto);

  if (deterministic && upto > 1) {
    int lastMax = transitions[offset + 2];
    for (int i = 1; i < upto; i++) {
      min = transitions[offset + 3 * i + 1];
      if (min <= lastMax) {
        deterministic = false;
        break;
      }
      lastMax = transitions[offset + 3 * i + 2];
    }
  }
}

bool Automaton::isDeterministic() { return deterministic; }

void Automaton::finishState()
{
  if (curState != -1) {
    finishCurrentState();
    curState = -1;
  }
}

int Automaton::getNumStates() { return nextState / 2; }

int Automaton::getNumTransitions() { return nextTransition / 3; }

int Automaton::getNumTransitions(int state)
{
  assert(state >= 0);
  int count = states[2 * state + 1];
  if (count == -1) {
    return 0;
  } else {
    return count;
  }
}

void Automaton::growStates()
{
  if (nextState + 2 > states.size()) {
    states = ArrayUtil::grow(states, nextState + 2);
  }
}

void Automaton::growTransitions()
{
  if (nextTransition + 3 > transitions.size()) {
    transitions = ArrayUtil::grow(transitions, nextTransition + 3);
  }
}

Automaton::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass()
{
}

void Automaton::InPlaceMergeSorterAnonymousInnerClass::swapOne(int i, int j)
{
  int x = outerInstance->transitions[i];
  outerInstance->transitions[i] = outerInstance->transitions[j];
  outerInstance->transitions[j] = x;
}

void Automaton::InPlaceMergeSorterAnonymousInnerClass::swap(int i, int j)
{
  int iStart = 3 * i;
  int jStart = 3 * j;
  swapOne(iStart, jStart);
  swapOne(iStart + 1, jStart + 1);
  swapOne(iStart + 2, jStart + 2);
};

int Automaton::InPlaceMergeSorterAnonymousInnerClass::compare(int i, int j)
{
  int iStart = 3 * i;
  int jStart = 3 * j;

  // First dest:
  int iDest = outerInstance->transitions[iStart];
  int jDest = outerInstance->transitions[jStart];
  if (iDest < jDest) {
    return -1;
  } else if (iDest > jDest) {
    return 1;
  }

  // Then min:
  int iMin = outerInstance->transitions[iStart + 1];
  int jMin = outerInstance->transitions[jStart + 1];
  if (iMin < jMin) {
    return -1;
  } else if (iMin > jMin) {
    return 1;
  }

  // Then max:
  int iMax = outerInstance->transitions[iStart + 2];
  int jMax = outerInstance->transitions[jStart + 2];
  if (iMax < jMax) {
    return -1;
  } else if (iMax > jMax) {
    return 1;
  }

  return 0;
}

Automaton::InPlaceMergeSorterAnonymousInnerClass2::
    InPlaceMergeSorterAnonymousInnerClass2()
{
}

void Automaton::InPlaceMergeSorterAnonymousInnerClass2::swapOne(int i, int j)
{
  int x = outerInstance->transitions[i];
  outerInstance->transitions[i] = outerInstance->transitions[j];
  outerInstance->transitions[j] = x;
}

void Automaton::InPlaceMergeSorterAnonymousInnerClass2::swap(int i, int j)
{
  int iStart = 3 * i;
  int jStart = 3 * j;
  swapOne(iStart, jStart);
  swapOne(iStart + 1, jStart + 1);
  swapOne(iStart + 2, jStart + 2);
};

int Automaton::InPlaceMergeSorterAnonymousInnerClass2::compare(int i, int j)
{
  int iStart = 3 * i;
  int jStart = 3 * j;

  // First min:
  int iMin = outerInstance->transitions[iStart + 1];
  int jMin = outerInstance->transitions[jStart + 1];
  if (iMin < jMin) {
    return -1;
  } else if (iMin > jMin) {
    return 1;
  }

  // Then max:
  int iMax = outerInstance->transitions[iStart + 2];
  int jMax = outerInstance->transitions[jStart + 2];
  if (iMax < jMax) {
    return -1;
  } else if (iMax > jMax) {
    return 1;
  }

  // Then dest:
  int iDest = outerInstance->transitions[iStart];
  int jDest = outerInstance->transitions[jStart];
  if (iDest < jDest) {
    return -1;
  } else if (iDest > jDest) {
    return 1;
  }

  return 0;
}

int Automaton::initTransition(int state, shared_ptr<Transition> t)
{
  assert((state < nextState / 2, L"state=" + to_wstring(state) +
                                     L" nextState=" + to_wstring(nextState)));
  t->source = state;
  t->transitionUpto = states[2 * state];
  return getNumTransitions(state);
}

void Automaton::getNextTransition(shared_ptr<Transition> t)
{
  // Make sure there is still a transition left:
  assert(t->transitionUpto + 3 - states[2 * t->source]) <=
      3 * states[2 * t->source + 1];

  // Make sure transitions are in fact sorted:
  assert(transitionSorted(t));

  t->dest = transitions[t->transitionUpto++];
  t->min = transitions[t->transitionUpto++];
  t->max = transitions[t->transitionUpto++];
}

bool Automaton::transitionSorted(shared_ptr<Transition> t)
{

  int upto = t->transitionUpto;
  if (upto == states[2 * t->source]) {
    // Transition isn't initialzed yet (this is the first transition); don't
    // check:
    return true;
  }

  int nextDest = transitions[upto];
  int nextMin = transitions[upto + 1];
  int nextMax = transitions[upto + 2];
  if (nextMin > t->min) {
    return true;
  } else if (nextMin < t->min) {
    return false;
  }

  // Min is equal, now test max:
  if (nextMax > t->max) {
    return true;
  } else if (nextMax < t->max) {
    return false;
  }

  // Max is also equal, now test dest:
  if (nextDest > t->dest) {
    return true;
  } else if (nextDest < t->dest) {
    return false;
  }

  // We should never see fully equal transitions here:
  return false;
}

void Automaton::getTransition(int state, int index, shared_ptr<Transition> t)
{
  int i = states[2 * state] + 3 * index;
  t->source = state;
  t->dest = transitions[i++];
  t->min = transitions[i++];
  t->max = transitions[i++];
}

void Automaton::appendCharString(int c, shared_ptr<StringBuilder> b)
{
  if (c >= 0x21 && c <= 0x7e && c != L'\\' && c != L'"') {
    b->appendCodePoint(c);
  } else {
    b->append(L"\\\\U");
    wstring s = Integer::toHexString(c);
    if (c < 0x10) {
      b->append(L"0000000")->append(s);
    } else if (c < 0x100) {
      b->append(L"000000")->append(s);
    } else if (c < 0x1000) {
      b->append(L"00000")->append(s);
    } else if (c < 0x10000) {
      b->append(L"0000")->append(s);
    } else if (c < 0x100000) {
      b->append(L"000")->append(s);
    } else if (c < 0x1000000) {
      b->append(L"00")->append(s);
    } else if (c < 0x10000000) {
      b->append(L"0")->append(s);
    } else {
      b->append(s);
    }
  }
}

wstring Automaton::toDot()
{
  // TODO: breadth first search so we can get layered output...

  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"digraph Automaton {\n");
  b->append(L"  rankdir = LR\n");
  b->append(L"  node [width=0.2, height=0.2, fontsize=8]\n");
  constexpr int numStates = getNumStates();
  if (numStates > 0) {
    b->append(L"  initial [shape=plaintext,label=\"\"]\n");
    b->append(L"  initial -> 0\n");
  }

  shared_ptr<Transition> t = make_shared<Transition>();

  for (int state = 0; state < numStates; state++) {
    b->append(L"  ");
    b->append(state);
    if (isAccept(state)) {
      b->append(L" [shape=doublecircle,label=\"" + to_wstring(state) +
                L"\"]\n");
    } else {
      b->append(L" [shape=circle,label=\"" + to_wstring(state) + L"\"]\n");
    }
    int numTransitions = initTransition(state, t);
    // System.out.println("toDot: state " + state + " has " + numTransitions + "
    // transitions; t.nextTrans=" + t.transitionUpto);
    for (int i = 0; i < numTransitions; i++) {
      getNextTransition(t);
      // System.out.println("  t.nextTrans=" + t.transitionUpto + " t=" + t);
      assert(t->max >= t->min);
      b->append(L"  ");
      b->append(state);
      b->append(L" -> ");
      b->append(t->dest);
      b->append(L" [label=\"");
      appendCharString(t->min, b);
      if (t->max != t->min) {
        b->append(L'-');
        appendCharString(t->max, b);
      }
      b->append(L"\"]\n");
      // System.out.println("  t=" + t);
    }
  }
  b->append(L'}');
  return b->toString();
}

std::deque<int> Automaton::getStartPoints()
{
  shared_ptr<Set<int>> pointset = unordered_set<int>();
  pointset->add(Character::MIN_CODE_POINT);
  // System.out.println("getStartPoints");
  for (int s = 0; s < nextState; s += 2) {
    int trans = states[s];
    int limit = trans + 3 * states[s + 1];
    // System.out.println("  state=" + (s/2) + " trans=" + trans + " limit=" +
    // limit);
    while (trans < limit) {
      int min = transitions[trans + 1];
      int max = transitions[trans + 2];
      // System.out.println("    min=" + min);
      pointset->add(min);
      if (max < Character::MAX_CODE_POINT) {
        pointset->add(max + 1);
      }
      trans += 3;
    }
  }
  std::deque<int> points(pointset->size());
  int n = 0;
  for (shared_ptr<> : : optional<int> m : pointset) {
    points[n++] = m;
  }
  Arrays::sort(points);
  return points;
}

int Automaton::step(int state, int label)
{
  assert(state >= 0);
  assert(label >= 0);
  int trans = states[2 * state];
  int limit = trans + 3 * states[2 * state + 1];
  // TODO: we could do bin search; transitions are sorted
  while (trans < limit) {
    int dest = transitions[trans];
    int min = transitions[trans + 1];
    int max = transitions[trans + 2];
    if (min <= label && label <= max) {
      return dest;
    }
    trans += 3;
  }

  return -1;
}

Automaton::Builder::Builder() : Builder(16, 16) {}

Automaton::Builder::Builder(int numStates, int numTransitions)
    : isAccept(make_shared<BitSet>(numStates))
{
  transitions = std::deque<int>(numTransitions * 4);
}

void Automaton::Builder::addTransition(int source, int dest, int label)
{
  addTransition(source, dest, label, label);
}

void Automaton::Builder::addTransition(int source, int dest, int min, int max)
{
  if (transitions.size() < nextTransition + 4) {
    transitions = ArrayUtil::grow(transitions, nextTransition + 4);
  }
  transitions[nextTransition++] = source;
  transitions[nextTransition++] = dest;
  transitions[nextTransition++] = min;
  transitions[nextTransition++] = max;
}

void Automaton::Builder::addEpsilon(int source, int dest)
{
  for (int upto = 0; upto < nextTransition; upto += 4) {
    if (transitions[upto] == dest) {
      addTransition(source, transitions[upto + 1], transitions[upto + 2],
                    transitions[upto + 3]);
    }
  }
  if (isAccept(dest)) {
    setAccept(source, true);
  }
}

Automaton::Builder::InPlaceMergeSorterAnonymousInnerClass3::
    InPlaceMergeSorterAnonymousInnerClass3()
{
}

void Automaton::Builder::InPlaceMergeSorterAnonymousInnerClass3::swapOne(int i,
                                                                         int j)
{
  int x = outerInstance->transitions[i];
  outerInstance->transitions[i] = outerInstance->transitions[j];
  outerInstance->transitions[j] = x;
}

void Automaton::Builder::InPlaceMergeSorterAnonymousInnerClass3::swap(int i,
                                                                      int j)
{
  int iStart = 4 * i;
  int jStart = 4 * j;
  swapOne(iStart, jStart);
  swapOne(iStart + 1, jStart + 1);
  swapOne(iStart + 2, jStart + 2);
  swapOne(iStart + 3, jStart + 3);
};

int Automaton::Builder::InPlaceMergeSorterAnonymousInnerClass3::compare(int i,
                                                                        int j)
{
  int iStart = 4 * i;
  int jStart = 4 * j;

  // First src:
  int iSrc = outerInstance->transitions[iStart];
  int jSrc = outerInstance->transitions[jStart];
  if (iSrc < jSrc) {
    return -1;
  } else if (iSrc > jSrc) {
    return 1;
  }

  // Then min:
  int iMin = outerInstance->transitions[iStart + 2];
  int jMin = outerInstance->transitions[jStart + 2];
  if (iMin < jMin) {
    return -1;
  } else if (iMin > jMin) {
    return 1;
  }

  // Then max:
  int iMax = outerInstance->transitions[iStart + 3];
  int jMax = outerInstance->transitions[jStart + 3];
  if (iMax < jMax) {
    return -1;
  } else if (iMax > jMax) {
    return 1;
  }

  // First dest:
  int iDest = outerInstance->transitions[iStart + 1];
  int jDest = outerInstance->transitions[jStart + 1];
  if (iDest < jDest) {
    return -1;
  } else if (iDest > jDest) {
    return 1;
  }

  return 0;
}

shared_ptr<Automaton> Automaton::Builder::finish()
{
  // Create automaton with the correct size.
  int numStates = nextState;
  int numTransitions = nextTransition / 4;
  shared_ptr<Automaton> a = make_shared<Automaton>(numStates, numTransitions);

  // Create all states.
  for (int state = 0; state < numStates; state++) {
    a->createState();
    a->setAccept(state, isAccept(state));
  }

  // Create all transitions
  sorter::sort(0, numTransitions);
  for (int upto = 0; upto < nextTransition; upto += 4) {
    a->addTransition(transitions[upto], transitions[upto + 1],
                     transitions[upto + 2], transitions[upto + 3]);
  }

  a->finishState();

  return a;
}

int Automaton::Builder::createState() { return nextState++; }

void Automaton::Builder::setAccept(int state, bool accept)
{
  FutureObjects::checkIndex(state, getNumStates());
  this->isAccept_->set(state, accept);
}

bool Automaton::Builder::isAccept(int state)
{
  return this->isAccept_->get(state);
}

int Automaton::Builder::getNumStates() { return nextState; }

void Automaton::Builder::copy(shared_ptr<Automaton> other)
{
  int offset = getNumStates();
  int otherNumStates = other->getNumStates();

  // Copy all states
  copyStates(other);

  // Copy all transitions
  shared_ptr<Transition> t = make_shared<Transition>();
  for (int s = 0; s < otherNumStates; s++) {
    int count = other->initTransition(s, t);
    for (int i = 0; i < count; i++) {
      other->getNextTransition(t);
      addTransition(offset + s, offset + t->dest, t->min, t->max);
    }
  }
}

void Automaton::Builder::copyStates(shared_ptr<Automaton> other)
{
  int otherNumStates = other->getNumStates();
  for (int s = 0; s < otherNumStates; s++) {
    int newState = createState();
    setAccept(newState, other->isAccept(s));
  }
}

int64_t Automaton::ramBytesUsed()
{
  // TODO: BitSet RAM usage (isAccept.size()/8) isn't fully accurate...
  return RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
         RamUsageEstimator::sizeOf(states) +
         RamUsageEstimator::sizeOf(transitions) +
         RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + (isAccept_->size() / 8) +
         RamUsageEstimator::NUM_BYTES_OBJECT_REF +
         2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF + 3 * Integer::BYTES + 1;
}
} // namespace org::apache::lucene::util::automaton