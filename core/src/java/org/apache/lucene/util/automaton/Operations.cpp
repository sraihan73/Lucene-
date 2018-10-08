using namespace std;

#include "Operations.h"

namespace org::apache::lucene::util::automaton
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Operations::Operations() {}

shared_ptr<Automaton> Operations::concatenate(shared_ptr<Automaton> a1,
                                              shared_ptr<Automaton> a2)
{
  return concatenate(Arrays::asList(a1, a2));
}

shared_ptr<Automaton>
Operations::concatenate(deque<std::shared_ptr<Automaton>> &l)
{
  shared_ptr<Automaton> result = make_shared<Automaton>();

  // First pass: create all states
  for (auto a : l) {
    if (a->getNumStates() == 0) {
      result->finishState();
      return result;
    }
    int numStates = a->getNumStates();
    for (int s = 0; s < numStates; s++) {
      result->createState();
    }
  }

  // Second pass: add transitions, carefully linking accept
  // states of A to init state of next A:
  int stateOffset = 0;
  shared_ptr<Transition> t = make_shared<Transition>();
  for (int i = 0; i < l.size(); i++) {
    shared_ptr<Automaton> a = l[i];
    int numStates = a->getNumStates();

    shared_ptr<Automaton> nextA = (i == l.size() - 1) ? nullptr : l[i + 1];

    for (int s = 0; s < numStates; s++) {
      int numTransitions = a->initTransition(s, t);
      for (int j = 0; j < numTransitions; j++) {
        a->getNextTransition(t);
        result->addTransition(stateOffset + s, stateOffset + t->dest, t->min,
                              t->max);
      }

      if (a->isAccept(s)) {
        shared_ptr<Automaton> followA = nextA;
        int followOffset = stateOffset;
        int upto = i + 1;
        while (true) {
          if (followA != nullptr) {
            // Adds a "virtual" epsilon transition:
            numTransitions = followA->initTransition(0, t);
            for (int j = 0; j < numTransitions; j++) {
              followA->getNextTransition(t);
              result->addTransition(stateOffset + s,
                                    followOffset + numStates + t->dest, t->min,
                                    t->max);
            }
            if (followA->isAccept(0)) {
              // Keep chaining if followA accepts empty string
              followOffset += followA->getNumStates();
              followA = (upto == l.size() - 1) ? nullptr : l[upto + 1];
              upto++;
            } else {
              break;
            }
          } else {
            result->setAccept(stateOffset + s, true);
            break;
          }
        }
      }
    }

    stateOffset += numStates;
  }

  if (result->getNumStates() == 0) {
    result->createState();
  }

  result->finishState();

  return result;
}

shared_ptr<Automaton> Operations::optional(shared_ptr<Automaton> a)
{
  shared_ptr<Automaton> result = make_shared<Automaton>();
  result->createState();
  result->setAccept(0, true);
  if (a->getNumStates() > 0) {
    result->copy(a);
    result->addEpsilon(0, 1);
  }
  result->finishState();
  return result;
}

shared_ptr<Automaton> Operations::repeat(shared_ptr<Automaton> a)
{
  if (a->getNumStates() == 0) {
    // Repeating the empty automata will still only accept the empty automata.
    return a;
  }
  shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();
  builder->createState();
  builder->setAccept(0, true);
  builder->copy(a);

  shared_ptr<Transition> t = make_shared<Transition>();
  int count = a->initTransition(0, t);
  for (int i = 0; i < count; i++) {
    a->getNextTransition(t);
    builder->addTransition(0, t->dest + 1, t->min, t->max);
  }

  int numStates = a->getNumStates();
  for (int s = 0; s < numStates; s++) {
    if (a->isAccept(s)) {
      count = a->initTransition(0, t);
      for (int i = 0; i < count; i++) {
        a->getNextTransition(t);
        builder->addTransition(s + 1, t->dest + 1, t->min, t->max);
      }
    }
  }

  return builder->finish();
}

shared_ptr<Automaton> Operations::repeat(shared_ptr<Automaton> a, int count)
{
  if (count == 0) {
    return repeat(a);
  }
  deque<std::shared_ptr<Automaton>> as = deque<std::shared_ptr<Automaton>>();
  while (count-- > 0) {
    as.push_back(a);
  }
  as.push_back(repeat(a));
  return concatenate(as);
}

shared_ptr<Automaton> Operations::repeat(shared_ptr<Automaton> a, int min,
                                         int max)
{
  if (min > max) {
    return Automata::makeEmpty();
  }

  shared_ptr<Automaton> b;
  if (min == 0) {
    b = Automata::makeEmptyString();
  } else if (min == 1) {
    b = make_shared<Automaton>();
    b->copy(a);
  } else {
    deque<std::shared_ptr<Automaton>> as =
        deque<std::shared_ptr<Automaton>>();
    for (int i = 0; i < min; i++) {
      as.push_back(a);
    }
    b = concatenate(as);
  }

  shared_ptr<Set<int>> prevAcceptStates = toSet(b, 0);
  shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();
  builder->copy(b);
  for (int i = min; i < max; i++) {
    int numStates = builder->getNumStates();
    builder->copy(a);
    for (auto s : prevAcceptStates) {
      builder->addEpsilon(s, numStates);
    }
    prevAcceptStates = toSet(a, numStates);
  }

  return builder->finish();
}

shared_ptr<Set<int>> Operations::toSet(shared_ptr<Automaton> a, int offset)
{
  int numStates = a->getNumStates();
  shared_ptr<BitSet> isAccept = a->getAcceptStates();
  shared_ptr<Set<int>> result = unordered_set<int>();
  int upto = 0;
  while (upto < numStates && (upto = isAccept->nextSetBit(upto)) != -1) {
    result->add(offset + upto);
    upto++;
  }

  return result;
}

shared_ptr<Automaton> Operations::complement(shared_ptr<Automaton> a,
                                             int maxDeterminizedStates)
{
  a = totalize(determinize(a, maxDeterminizedStates));
  int numStates = a->getNumStates();
  for (int p = 0; p < numStates; p++) {
    a->setAccept(p, !a->isAccept(p));
  }
  return removeDeadStates(a);
}

shared_ptr<Automaton> Operations::minus(shared_ptr<Automaton> a1,
                                        shared_ptr<Automaton> a2,
                                        int maxDeterminizedStates)
{
  if (Operations::isEmpty(a1) || a1 == a2) {
    return Automata::makeEmpty();
  }
  if (Operations::isEmpty(a2)) {
    return a1;
  }
  return intersection(a1, complement(a2, maxDeterminizedStates));
}

shared_ptr<Automaton> Operations::intersection(shared_ptr<Automaton> a1,
                                               shared_ptr<Automaton> a2)
{
  if (a1 == a2) {
    return a1;
  }
  if (a1->getNumStates() == 0) {
    return a1;
  }
  if (a2->getNumStates() == 0) {
    return a2;
  }
  std::deque<std::deque<std::shared_ptr<Transition>>> transitions1 =
      a1->getSortedTransitions();
  std::deque<std::deque<std::shared_ptr<Transition>>> transitions2 =
      a2->getSortedTransitions();
  shared_ptr<Automaton> c = make_shared<Automaton>();
  c->createState();
  shared_ptr<ArrayDeque<std::shared_ptr<StatePair>>> worklist =
      make_shared<ArrayDeque<std::shared_ptr<StatePair>>>();
  unordered_map<std::shared_ptr<StatePair>, std::shared_ptr<StatePair>>
      newstates = unordered_map<std::shared_ptr<StatePair>,
                                std::shared_ptr<StatePair>>();
  shared_ptr<StatePair> p = make_shared<StatePair>(0, 0, 0);
  worklist->add(p);
  newstates.emplace(p, p);
  while (worklist->size() > 0) {
    p = worklist->removeFirst();
    c->setAccept(p->s, a1->isAccept(p->s1) && a2->isAccept(p->s2));
    std::deque<std::shared_ptr<Transition>> t1 = transitions1[p->s1];
    std::deque<std::shared_ptr<Transition>> t2 = transitions2[p->s2];
    for (int n1 = 0, b2 = 0; n1 < t1.size(); n1++) {
      while (b2 < t2.size() && t2[b2]->max < t1[n1]->min) {
        b2++;
      }
      for (int n2 = b2; n2 < t2.size() && t1[n1]->max >= t2[n2]->min; n2++) {
        if (t2[n2]->max >= t1[n1]->min) {
          shared_ptr<StatePair> q =
              make_shared<StatePair>(t1[n1]->dest, t2[n2]->dest);
          shared_ptr<StatePair> r = newstates[q];
          if (r == nullptr) {
            q->s = c->createState();
            worklist->add(q);
            newstates.emplace(q, q);
            r = q;
          }
          int min = t1[n1]->min > t2[n2]->min ? t1[n1]->min : t2[n2]->min;
          int max = t1[n1]->max < t2[n2]->max ? t1[n1]->max : t2[n2]->max;
          c->addTransition(p->s, r->s, min, max);
        }
      }
    }
  }
  c->finishState();

  return removeDeadStates(c);
}

bool Operations::sameLanguage(shared_ptr<Automaton> a1,
                              shared_ptr<Automaton> a2)
{
  if (a1 == a2) {
    return true;
  }
  return subsetOf(a2, a1) && subsetOf(a1, a2);
}

bool Operations::hasDeadStates(shared_ptr<Automaton> a)
{
  shared_ptr<BitSet> liveStates = getLiveStates(a);
  int numLive = liveStates->cardinality();
  int numStates = a->getNumStates();
  assert((numLive <= numStates, L"numLive=" + to_wstring(numLive) +
                                    L" numStates=" + to_wstring(numStates) +
                                    L" " + liveStates));
  return numLive < numStates;
}

bool Operations::hasDeadStatesFromInitial(shared_ptr<Automaton> a)
{
  shared_ptr<BitSet> reachableFromInitial = getLiveStatesFromInitial(a);
  shared_ptr<BitSet> reachableFromAccept = getLiveStatesToAccept(a);
  reachableFromInitial->andNot(reachableFromAccept);
  return reachableFromInitial->isEmpty() == false;
}

bool Operations::hasDeadStatesToAccept(shared_ptr<Automaton> a)
{
  shared_ptr<BitSet> reachableFromInitial = getLiveStatesFromInitial(a);
  shared_ptr<BitSet> reachableFromAccept = getLiveStatesToAccept(a);
  reachableFromAccept->andNot(reachableFromInitial);
  return reachableFromAccept->isEmpty() == false;
}

bool Operations::subsetOf(shared_ptr<Automaton> a1, shared_ptr<Automaton> a2)
{
  if (a1->isDeterministic() == false) {
    throw invalid_argument(L"a1 must be deterministic");
  }
  if (a2->isDeterministic() == false) {
    throw invalid_argument(L"a2 must be deterministic");
  }
  assert(hasDeadStatesFromInitial(a1) == false);
  assert(hasDeadStatesFromInitial(a2) == false);
  if (a1->getNumStates() == 0) {
    // Empty language is alwyas a subset of any other language
    return true;
  } else if (a2->getNumStates() == 0) {
    return isEmpty(a1);
  }

  // TODO: cutover to iterators instead
  std::deque<std::deque<std::shared_ptr<Transition>>> transitions1 =
      a1->getSortedTransitions();
  std::deque<std::deque<std::shared_ptr<Transition>>> transitions2 =
      a2->getSortedTransitions();
  shared_ptr<ArrayDeque<std::shared_ptr<StatePair>>> worklist =
      make_shared<ArrayDeque<std::shared_ptr<StatePair>>>();
  unordered_set<std::shared_ptr<StatePair>> visited =
      unordered_set<std::shared_ptr<StatePair>>();
  shared_ptr<StatePair> p = make_shared<StatePair>(0, 0);
  worklist->add(p);
  visited.insert(p);
  while (worklist->size() > 0) {
    p = worklist->removeFirst();
    if (a1->isAccept(p->s1) && a2->isAccept(p->s2) == false) {
      return false;
    }
    std::deque<std::shared_ptr<Transition>> t1 = transitions1[p->s1];
    std::deque<std::shared_ptr<Transition>> t2 = transitions2[p->s2];
    for (int n1 = 0, b2 = 0; n1 < t1.size(); n1++) {
      while (b2 < t2.size() && t2[b2]->max < t1[n1]->min) {
        b2++;
      }
      int min1 = t1[n1]->min, max1 = t1[n1]->max;

      for (int n2 = b2; n2 < t2.size() && t1[n1]->max >= t2[n2]->min; n2++) {
        if (t2[n2]->min > min1) {
          return false;
        }
        if (t2[n2]->max < Character::MAX_CODE_POINT) {
          min1 = t2[n2]->max + 1;
        } else {
          min1 = Character::MAX_CODE_POINT;
          max1 = Character::MIN_CODE_POINT;
        }
        shared_ptr<StatePair> q =
            make_shared<StatePair>(t1[n1]->dest, t2[n2]->dest);
        if (!find(visited.begin(), visited.end(), q) != visited.end()) {
          worklist->add(q);
          visited.insert(q);
        }
      }
      if (min1 <= max1) {
        return false;
      }
    }
  }
  return true;
}

shared_ptr<Automaton> Operations::union_(shared_ptr<Automaton> a1,
                                         shared_ptr<Automaton> a2)
{
  return union_(Arrays::asList(a1, a2));
}

shared_ptr<Automaton>
Operations::union_(shared_ptr<deque<std::shared_ptr<Automaton>>> l)
{
  shared_ptr<Automaton> result = make_shared<Automaton>();

  // Create initial state:
  result->createState();

  // Copy over all automata
  for (auto a : l) {
    result->copy(a);
  }

  // Add epsilon transition from new initial state
  int stateOffset = 1;
  for (auto a : l) {
    if (a->getNumStates() == 0) {
      continue;
    }
    result->addEpsilon(0, stateOffset);
    stateOffset += a->getNumStates();
  }

  result->finishState();

  return removeDeadStates(result);
}

void Operations::TransitionList::add(shared_ptr<Transition> t)
{
  if (transitions.size() < next + 3) {
    transitions = ArrayUtil::grow(transitions, next + 3);
  }
  transitions[next] = t->dest;
  transitions[next + 1] = t->min;
  transitions[next + 2] = t->max;
  next += 3;
}

int Operations::PointTransitions::compareTo(shared_ptr<PointTransitions> other)
{
  return point - other->point;
}

void Operations::PointTransitions::reset(int point)
{
  this->point = point;
  ends->next = 0;
  starts->next = 0;
}

bool Operations::PointTransitions::equals(any other)
{
  return (any_cast<std::shared_ptr<PointTransitions>>(other))->point == point;
}

int Operations::PointTransitions::hashCode() { return point; }

shared_ptr<PointTransitions> Operations::PointTransitionSet::next(int point)
{
  // 1st time we are seeing this point
  if (count == points.size()) {
    std::deque<std::shared_ptr<PointTransitions>> newArray(ArrayUtil::oversize(
        1 + count, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(points, 0, newArray, 0, count);
    points = newArray;
  }
  shared_ptr<PointTransitions> points0 = points[count];
  if (points0 == nullptr) {
    points0 = points[count] = make_shared<PointTransitions>();
  }
  points0->reset(point);
  count++;
  return points0;
}

shared_ptr<PointTransitions> Operations::PointTransitionSet::find(int point)
{
  if (useHash) {
    const optional<int> pi = point;
    shared_ptr<PointTransitions> p = map_obj[pi];
    if (p == nullptr) {
      p = next(point);
      map_obj.emplace(pi, p);
    }
    return p;
  } else {
    for (int i = 0; i < count; i++) {
      if (points[i]->point == point) {
        return points[i];
      }
    }

    shared_ptr<PointTransitions> *const p = next(point);
    if (count == HASHMAP_CUTOVER) {
      // switch to HashMap on the fly
      assert(map_obj.empty());
      for (int i = 0; i < count; i++) {
        map_obj.emplace(points[i]->point, points[i]);
      }
      useHash = true;
    }
    return p;
  }
}

void Operations::PointTransitionSet::reset()
{
  if (useHash) {
    map_obj.clear();
    useHash = false;
  }
  count = 0;
}

void Operations::PointTransitionSet::sort()
{
  // Tim sort performs well on already sorted arrays:
  if (count > 1) {
    ArrayUtil::timSort(points, 0, count);
  }
}

void Operations::PointTransitionSet::add(shared_ptr<Transition> t)
{
  find(t->min)->starts->add(t);
  find(1 + t->max)->ends->add(t);
}

wstring Operations::PointTransitionSet::toString()
{
  shared_ptr<StringBuilder> s = make_shared<StringBuilder>();
  for (int i = 0; i < count; i++) {
    if (i > 0) {
      s->append(L' ');
    }
    s->append(points[i]->point)
        ->append(L':')
        ->append(points[i]->starts->next / 3)
        ->append(L',')
        ->append(points[i]->ends->next / 3);
  }
  return s->toString();
}

shared_ptr<Automaton> Operations::determinize(shared_ptr<Automaton> a,
                                              int maxDeterminizedStates)
{
  if (a->isDeterministic()) {
    // Already determinized
    return a;
  }
  if (a->getNumStates() <= 1) {
    // Already determinized
    return a;
  }

  // subset construction
  shared_ptr<Automaton::Builder> b = make_shared<Automaton::Builder>();

  // System.out.println("DET:");
  // a.writeDot("/l/la/lucene/core/detin.dot");

  shared_ptr<SortedIntSet::FrozenIntSet> initialset =
      make_shared<SortedIntSet::FrozenIntSet>(0, 0);

  // Create state 0:
  b->createState();

  shared_ptr<ArrayDeque<std::shared_ptr<SortedIntSet::FrozenIntSet>>> worklist =
      make_shared<ArrayDeque<std::shared_ptr<SortedIntSet::FrozenIntSet>>>();
  unordered_map<std::shared_ptr<SortedIntSet::FrozenIntSet>, int> newstate =
      unordered_map<std::shared_ptr<SortedIntSet::FrozenIntSet>, int>();

  worklist->add(initialset);

  b->setAccept(0, a->isAccept(0));
  newstate.emplace(initialset, 0);

  // like Set<Integer,PointTransitions>
  shared_ptr<PointTransitionSet> *const points =
      make_shared<PointTransitionSet>();

  // like SortedMap<Integer,Integer>
  shared_ptr<SortedIntSet> *const statesSet = make_shared<SortedIntSet>(5);

  shared_ptr<Transition> t = make_shared<Transition>();

  while (worklist->size() > 0) {
    shared_ptr<SortedIntSet::FrozenIntSet> s = worklist->removeFirst();
    // System.out.println("det: pop set=" + s);

    // Collate all outgoing transitions by min/1+max:
    for (int i = 0; i < s->values.size(); i++) {
      constexpr int s0 = s->values[i];
      int numTransitions = a->getNumTransitions(s0);
      a->initTransition(s0, t);
      for (int j = 0; j < numTransitions; j++) {
        a->getNextTransition(t);
        points->add(t);
      }
    }

    if (points->count == 0) {
      // No outgoing transitions -- skip it
      continue;
    }

    points->sort();

    int lastPoint = -1;
    int accCount = 0;

    constexpr int r = s->state;

    for (int i = 0; i < points->count; i++) {

      constexpr int point = points->points[i]->point;

      if (statesSet->upto > 0) {
        assert(lastPoint != -1);

        statesSet->computeHash();

        optional<int> q = newstate[statesSet];
        if (!q) {
          q = b->createState();
          if (q >= maxDeterminizedStates) {
            throw make_shared<TooComplexToDeterminizeException>(
                a, maxDeterminizedStates);
          }
          shared_ptr<SortedIntSet::FrozenIntSet> *const p =
              statesSet->freeze(q);
          // System.out.println("  make new state=" + q + " -> " + p + "
          // accCount=" + accCount);
          worklist->add(p);
          b->setAccept(q, accCount > 0);
          newstate.emplace(p, q);
        } else {
          assert(accCount > 0 ? true : false) == b->isAccept(q)
              : L"accCount=" + to_wstring(accCount) + L" vs existing accept=" +
                                                     StringHelper::toString(
                                                         b->isAccept(q)) +
                                                     L" states=" + statesSet;
        }

        // System.out.println("  add trans src=" + r + " dest=" + q + " min=" +
        // lastPoint + " max=" + (point-1));

        b->addTransition(r, q, lastPoint, point - 1);
      }

      // process transitions that end on this point
      // (closes an overlapping interval)
      std::deque<int> transitions = points->points[i]->ends.transitions;
      int limit = points->points[i]->ends->next;
      for (int j = 0; j < limit; j += 3) {
        int dest = transitions[j];
        statesSet->decr(dest);
        accCount -= a->isAccept(dest) ? 1 : 0;
      }
      points->points[i]->ends->next = 0;

      // process transitions that start on this point
      // (opens a new interval)
      transitions = points->points[i]->starts.transitions;
      limit = points->points[i]->starts->next;
      for (int j = 0; j < limit; j += 3) {
        int dest = transitions[j];
        statesSet->incr(dest);
        accCount += a->isAccept(dest) ? 1 : 0;
      }
      lastPoint = point;
      points->points[i]->starts->next = 0;
    }
    points->reset();
    assert((statesSet->upto == 0, L"upto=" + to_wstring(statesSet->upto)));
  }

  shared_ptr<Automaton> result = b->finish();
  assert(result->isDeterministic());
  return result;
}

bool Operations::isEmpty(shared_ptr<Automaton> a)
{
  if (a->getNumStates() == 0) {
    // Common case: no states
    return true;
  }
  if (a->isAccept(0) == false && a->getNumTransitions(0) == 0) {
    // Common case: just one initial state
    return true;
  }
  if (a->isAccept(0) == true) {
    // Apparently common case: it accepts the damned empty string
    return false;
  }

  shared_ptr<ArrayDeque<int>> workList = make_shared<ArrayDeque<int>>();
  shared_ptr<BitSet> seen = make_shared<BitSet>(a->getNumStates());
  workList->add(0);
  seen->set(0);

  shared_ptr<Transition> t = make_shared<Transition>();
  while (workList->isEmpty() == false) {
    int state = workList->removeFirst();
    if (a->isAccept(state)) {
      return false;
    }
    int count = a->initTransition(state, t);
    for (int i = 0; i < count; i++) {
      a->getNextTransition(t);
      if (seen->get(t->dest) == false) {
        workList->add(t->dest);
        seen->set(t->dest);
      }
    }
  }

  return true;
}

bool Operations::isTotal(shared_ptr<Automaton> a)
{
  return isTotal(a, Character::MIN_CODE_POINT, Character::MAX_CODE_POINT);
}

bool Operations::isTotal(shared_ptr<Automaton> a, int minAlphabet,
                         int maxAlphabet)
{
  if (a->isAccept(0) && a->getNumTransitions(0) == 1) {
    shared_ptr<Transition> t = make_shared<Transition>();
    a->getTransition(0, 0, t);
    return t->dest == 0 && t->min == minAlphabet && t->max == maxAlphabet;
  }
  return false;
}

bool Operations::run(shared_ptr<Automaton> a, const wstring &s)
{
  assert(a->isDeterministic());
  int state = 0;
  for (int i = 0, cp = 0; i < s.length(); i += Character::charCount(cp)) {
    int nextState = a->step(state, cp = s.codePointAt(i));
    if (nextState == -1) {
      return false;
    }
    state = nextState;
  }
  return a->isAccept(state);
}

bool Operations::run(shared_ptr<Automaton> a, shared_ptr<IntsRef> s)
{
  assert(a->isDeterministic());
  int state = 0;
  for (int i = 0; i < s->length; i++) {
    int nextState = a->step(state, s->ints[s->offset + i]);
    if (nextState == -1) {
      return false;
    }
    state = nextState;
  }
  return a->isAccept(state);
}

shared_ptr<BitSet> Operations::getLiveStates(shared_ptr<Automaton> a)
{
  shared_ptr<BitSet> live = getLiveStatesFromInitial(a);
  live->and (getLiveStatesToAccept(a));
  return live;
}

shared_ptr<BitSet> Operations::getLiveStatesFromInitial(shared_ptr<Automaton> a)
{
  int numStates = a->getNumStates();
  shared_ptr<BitSet> live = make_shared<BitSet>(numStates);
  if (numStates == 0) {
    return live;
  }
  shared_ptr<ArrayDeque<int>> workList = make_shared<ArrayDeque<int>>();
  live->set(0);
  workList->add(0);

  shared_ptr<Transition> t = make_shared<Transition>();
  while (workList->isEmpty() == false) {
    int s = workList->removeFirst();
    int count = a->initTransition(s, t);
    for (int i = 0; i < count; i++) {
      a->getNextTransition(t);
      if (live->get(t->dest) == false) {
        live->set(t->dest);
        workList->add(t->dest);
      }
    }
  }

  return live;
}

shared_ptr<BitSet> Operations::getLiveStatesToAccept(shared_ptr<Automaton> a)
{
  shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();

  // NOTE: not quite the same thing as what SpecialOperations.reverse does:
  shared_ptr<Transition> t = make_shared<Transition>();
  int numStates = a->getNumStates();
  for (int s = 0; s < numStates; s++) {
    builder->createState();
  }
  for (int s = 0; s < numStates; s++) {
    int count = a->initTransition(s, t);
    for (int i = 0; i < count; i++) {
      a->getNextTransition(t);
      builder->addTransition(t->dest, s, t->min, t->max);
    }
  }
  shared_ptr<Automaton> a2 = builder->finish();

  shared_ptr<ArrayDeque<int>> workList = make_shared<ArrayDeque<int>>();
  shared_ptr<BitSet> live = make_shared<BitSet>(numStates);
  shared_ptr<BitSet> acceptBits = a->getAcceptStates();
  int s = 0;
  while (s < numStates && (s = acceptBits->nextSetBit(s)) != -1) {
    live->set(s);
    workList->add(s);
    s++;
  }

  while (workList->isEmpty() == false) {
    s = workList->removeFirst();
    int count = a2->initTransition(s, t);
    for (int i = 0; i < count; i++) {
      a2->getNextTransition(t);
      if (live->get(t->dest) == false) {
        live->set(t->dest);
        workList->add(t->dest);
      }
    }
  }

  return live;
}

shared_ptr<Automaton> Operations::removeDeadStates(shared_ptr<Automaton> a)
{
  int numStates = a->getNumStates();
  shared_ptr<BitSet> liveSet = getLiveStates(a);

  std::deque<int> map_obj(numStates);

  shared_ptr<Automaton> result = make_shared<Automaton>();
  // System.out.println("liveSet: " + liveSet + " numStates=" + numStates);
  for (int i = 0; i < numStates; i++) {
    if (liveSet->get(i)) {
      map_obj[i] = result->createState();
      result->setAccept(map_obj[i], a->isAccept(i));
    }
  }

  shared_ptr<Transition> t = make_shared<Transition>();

  for (int i = 0; i < numStates; i++) {
    if (liveSet->get(i)) {
      int numTransitions = a->initTransition(i, t);
      // filter out transitions to dead states:
      for (int j = 0; j < numTransitions; j++) {
        a->getNextTransition(t);
        if (liveSet->get(t->dest)) {
          result->addTransition(map_obj[i], map_obj[t->dest], t->min, t->max);
        }
      }
    }
  }

  result->finishState();
  assert(hasDeadStates(result) == false);
  return result;
}

bool Operations::isFinite(shared_ptr<Automaton> a)
{
  if (a->getNumStates() == 0) {
    return true;
  }
  return isFinite(make_shared<Transition>(), a, 0,
                  make_shared<BitSet>(a->getNumStates()),
                  make_shared<BitSet>(a->getNumStates()), 0);
}

bool Operations::isFinite(shared_ptr<Transition> scratch,
                          shared_ptr<Automaton> a, int state,
                          shared_ptr<BitSet> path, shared_ptr<BitSet> visited,
                          int level)
{
  if (level > MAX_RECURSION_LEVEL) {
    throw invalid_argument(L"input automaton is too large: " +
                           to_wstring(level));
  }
  path->set(state);
  int numTransitions = a->initTransition(state, scratch);
  for (int t = 0; t < numTransitions; t++) {
    a->getTransition(state, t, scratch);
    if (path->get(scratch->dest) ||
        (!visited->get(scratch->dest) &&
         !isFinite(scratch, a, scratch->dest, path, visited, level + 1))) {
      return false;
    }
  }
  path->clear(state);
  visited->set(state);
  return true;
}

wstring Operations::getCommonPrefix(shared_ptr<Automaton> a)
{
  if (a->isDeterministic() == false) {
    throw invalid_argument(L"input automaton must be deterministic");
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  unordered_set<int> visited = unordered_set<int>();
  int s = 0;
  bool done;
  shared_ptr<Transition> t = make_shared<Transition>();
  do {
    done = true;
    visited.insert(s);
    if (a->isAccept(s) == false && a->getNumTransitions(s) == 1) {
      a->getTransition(s, 0, t);
      if (t->min == t->max &&
          !find(visited.begin(), visited.end(), t->dest) != visited.end()) {
        b->appendCodePoint(t->min);
        s = t->dest;
        done = false;
      }
    }
  } while (!done);

  return b->toString();
}

shared_ptr<BytesRef>
Operations::getCommonPrefixBytesRef(shared_ptr<Automaton> a)
{
  shared_ptr<BytesRefBuilder> builder = make_shared<BytesRefBuilder>();
  unordered_set<int> visited = unordered_set<int>();
  int s = 0;
  bool done;
  shared_ptr<Transition> t = make_shared<Transition>();
  do {
    done = true;
    visited.insert(s);
    if (a->isAccept(s) == false && a->getNumTransitions(s) == 1) {
      a->getTransition(s, 0, t);
      if (t->min == t->max &&
          !find(visited.begin(), visited.end(), t->dest) != visited.end()) {
        builder->append(static_cast<char>(t->min));
        s = t->dest;
        done = false;
      }
    }
  } while (!done);

  return builder->get();
}

shared_ptr<IntsRef> Operations::getSingleton(shared_ptr<Automaton> a)
{
  if (a->isDeterministic() == false) {
    throw invalid_argument(L"input automaton must be deterministic");
  }
  shared_ptr<IntsRefBuilder> builder = make_shared<IntsRefBuilder>();
  unordered_set<int> visited = unordered_set<int>();
  int s = 0;
  shared_ptr<Transition> t = make_shared<Transition>();
  while (true) {
    visited.insert(s);
    if (a->isAccept(s) == false) {
      if (a->getNumTransitions(s) == 1) {
        a->getTransition(s, 0, t);
        if (t->min == t->max &&
            !find(visited.begin(), visited.end(), t->dest) != visited.end()) {
          builder->append(t->min);
          s = t->dest;
          continue;
        }
      }
    } else if (a->getNumTransitions(s) == 0) {
      return builder->get();
    }

    // Automaton accepts more than one string:
    return nullptr;
  }
}

shared_ptr<BytesRef>
Operations::getCommonSuffixBytesRef(shared_ptr<Automaton> a,
                                    int maxDeterminizedStates)
{
  // reverse the language of the automaton, then reverse its common prefix.
  shared_ptr<Automaton> r =
      Operations::determinize(reverse(a), maxDeterminizedStates);
  shared_ptr<BytesRef> ref = getCommonPrefixBytesRef(r);
  reverseBytes(ref);
  return ref;
}

void Operations::reverseBytes(shared_ptr<BytesRef> ref)
{
  if (ref->length <= 1) {
    return;
  }
  int num = ref->length >> 1;
  for (int i = ref->offset; i < (ref->offset + num); i++) {
    char b = ref->bytes[i];
    ref->bytes[i] = ref->bytes[ref->offset * 2 + ref->length - i - 1];
    ref->bytes[ref->offset * 2 + ref->length - i - 1] = b;
  }
}

shared_ptr<Automaton> Operations::reverse(shared_ptr<Automaton> a)
{
  return reverse(a, nullptr);
}

shared_ptr<Automaton> Operations::reverse(shared_ptr<Automaton> a,
                                          shared_ptr<Set<int>> initialStates)
{

  if (Operations::isEmpty(a)) {
    return make_shared<Automaton>();
  }

  int numStates = a->getNumStates();

  // Build a new automaton with all edges reversed
  shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();

  // Initial node; we'll add epsilon transitions in the end:
  builder->createState();

  for (int s = 0; s < numStates; s++) {
    builder->createState();
  }

  // Old initial state becomes new accept state:
  builder->setAccept(1, true);

  shared_ptr<Transition> t = make_shared<Transition>();
  for (int s = 0; s < numStates; s++) {
    int numTransitions = a->getNumTransitions(s);
    a->initTransition(s, t);
    for (int i = 0; i < numTransitions; i++) {
      a->getNextTransition(t);
      builder->addTransition(t->dest + 1, s + 1, t->min, t->max);
    }
  }

  shared_ptr<Automaton> result = builder->finish();

  int s = 0;
  shared_ptr<BitSet> acceptStates = a->getAcceptStates();
  while (s < numStates && (s = acceptStates->nextSetBit(s)) != -1) {
    result->addEpsilon(0, s + 1);
    if (initialStates != nullptr) {
      initialStates->add(s + 1);
    }
    s++;
  }

  result->finishState();

  return result;
}

shared_ptr<Automaton> Operations::totalize(shared_ptr<Automaton> a)
{
  shared_ptr<Automaton> result = make_shared<Automaton>();
  int numStates = a->getNumStates();
  for (int i = 0; i < numStates; i++) {
    result->createState();
    result->setAccept(i, a->isAccept(i));
  }

  int deadState = result->createState();
  result->addTransition(deadState, deadState, Character::MIN_CODE_POINT,
                        Character::MAX_CODE_POINT);

  shared_ptr<Transition> t = make_shared<Transition>();
  for (int i = 0; i < numStates; i++) {
    int maxi = Character::MIN_CODE_POINT;
    int count = a->initTransition(i, t);
    for (int j = 0; j < count; j++) {
      a->getNextTransition(t);
      result->addTransition(i, t->dest, t->min, t->max);
      if (t->min > maxi) {
        result->addTransition(i, deadState, maxi, t->min - 1);
      }
      if (t->max + 1 > maxi) {
        maxi = t->max + 1;
      }
    }

    if (maxi <= Character::MAX_CODE_POINT) {
      result->addTransition(i, deadState, maxi, Character::MAX_CODE_POINT);
    }
  }

  result->finishState();
  return result;
}

std::deque<int> Operations::topoSortStates(shared_ptr<Automaton> a)
{
  if (a->getNumStates() == 0) {
    return std::deque<int>(0);
  }
  int numStates = a->getNumStates();
  std::deque<int> states(numStates);
  shared_ptr<BitSet> *const visited = make_shared<BitSet>(numStates);
  int upto = topoSortStatesRecurse(a, visited, states, 0, 0, 0);

  if (upto < states.size()) {
    // There were dead states
    std::deque<int> newStates(upto);
    System::arraycopy(states, 0, newStates, 0, upto);
    states = newStates;
  }

  // Reverse the order:
  for (int i = 0; i < states.size() / 2; i++) {
    int s = states[i];
    states[i] = states[states.size() - 1 - i];
    states[states.size() - 1 - i] = s;
  }

  return states;
}

int Operations::topoSortStatesRecurse(shared_ptr<Automaton> a,
                                      shared_ptr<BitSet> visited,
                                      std::deque<int> &states, int upto,
                                      int state, int level)
{
  if (level > MAX_RECURSION_LEVEL) {
    throw invalid_argument(L"input automaton is too large: " +
                           to_wstring(level));
  }
  shared_ptr<Transition> t = make_shared<Transition>();
  int count = a->initTransition(state, t);
  for (int i = 0; i < count; i++) {
    a->getNextTransition(t);
    if (!visited->get(t->dest)) {
      visited->set(t->dest);
      upto =
          topoSortStatesRecurse(a, visited, states, upto, t->dest, level + 1);
    }
  }
  states[upto] = state;
  upto++;
  return upto;
}
} // namespace org::apache::lucene::util::automaton