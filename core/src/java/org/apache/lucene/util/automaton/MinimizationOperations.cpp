using namespace std;

#include "MinimizationOperations.h"

namespace org::apache::lucene::util::automaton
{

MinimizationOperations::MinimizationOperations() {}

shared_ptr<Automaton>
MinimizationOperations::minimize(shared_ptr<Automaton> a,
                                 int maxDeterminizedStates)
{
  if (a->getNumStates() == 0 ||
      (a->isAccept(0) == false && a->getNumTransitions(0) == 0)) {
    // Fastmatch for common case
    return make_shared<Automaton>();
  }
  a = Operations::determinize(a, maxDeterminizedStates);
  // a.writeDot("adet");
  if (a->getNumTransitions(0) == 1) {
    shared_ptr<Transition> t = make_shared<Transition>();
    a->getTransition(0, 0, t);
    if (t->dest == 0 && t->min == Character::MIN_CODE_POINT &&
        t->max == Character::MAX_CODE_POINT) {
      // Accepts all strings
      return a;
    }
  }
  a = Operations::totalize(a);
  // a.writeDot("atot");

  // initialize data structures
  const std::deque<int> sigma = a->getStartPoints();
  constexpr int sigmaLen = sigma.size(), statesLen = a->getNumStates();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
  // java.util.ArrayList<int>[][] reverse = (java.util.ArrayList<int>[][]) new
  // java.util.ArrayList[statesLen][sigmaLen]; C++ NOTE: The following call to
  // the 'RectangularVectors' helper class reproduces the rectangular array
  // initialization that is automatic in Java:
  const std::deque<std::deque<deque<int>>> reverse =
      static_cast<std::deque<std::deque<deque<int>>>>(
          RectangularVectors::ReturnRectangularStdvectorVector(statesLen,
                                                               sigmaLen));
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
  // java.util.HashSet<int>[] partition = (java.util.HashSet<int>[]) new
  // java.util.HashSet[statesLen];
  const std::deque<unordered_set<int>> partition =
      static_cast<std::deque<unordered_set<int>>>(
          std::deque<unordered_set>(statesLen));
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
  // java.util.ArrayList<int>[] splitblock = (java.util.ArrayList<int>[]) new
  // java.util.ArrayList[statesLen];
  const std::deque<deque<int>> splitblock =
      static_cast<std::deque<deque<int>>>(std::deque<deque>(statesLen));
  const std::deque<int> block = std::deque<int>(statesLen);
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: final StateList[][] active = new
  // StateList[statesLen][sigmaLen];
  std::deque<std::deque<std::shared_ptr<StateList>>> active =
      RectangularVectors::ReturnRectangularStateListVector(statesLen, sigmaLen);
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: final StateListNode[][] active2 = new
  // StateListNode[statesLen][sigmaLen];
  std::deque<std::deque<std::shared_ptr<StateListNode>>> active2 =
      RectangularVectors::ReturnRectangularStateListNodeVector(statesLen,
                                                               sigmaLen);
  const deque<std::shared_ptr<IntPair>> pending =
      deque<std::shared_ptr<IntPair>>();
  shared_ptr<BitSet> *const pending2 =
      make_shared<BitSet>(sigmaLen * statesLen);
  shared_ptr<BitSet> *const split = make_shared<BitSet>(statesLen),
                            *const refine = make_shared<BitSet>(statesLen),
                            *const refine2 = make_shared<BitSet>(statesLen);
  for (int q = 0; q < statesLen; q++) {
    splitblock[q] = deque<int>();
    partition[q] = unordered_set<int>();
    for (int x = 0; x < sigmaLen; x++) {
      active[q][x] = make_shared<StateList>();
    }
  }
  // find initial partition and reverse edges
  for (int q = 0; q < statesLen; q++) {
    constexpr int j = a->isAccept(q) ? 0 : 1;
    partition[j].insert(q);
    block[q] = j;
    for (int x = 0; x < sigmaLen; x++) {
      const std::deque<deque<int>> r = reverse[a->step(q, sigma[x])];
      if (r[x].empty()) {
        r[x] = deque<int>();
      }
      r[x].push_back(q);
    }
  }
  // initialize active sets
  for (int j = 0; j <= 1; j++) {
    for (int x = 0; x < sigmaLen; x++) {
      for (auto q : partition[j]) {
        if (reverse[q][x].size() > 0) {
          active2[q][x] = active[j][x]->add(q);
        }
      }
    }
  }

  // initialize pending
  for (int x = 0; x < sigmaLen; x++) {
    constexpr int j = (active[0][x]->size <= active[1][x]->size) ? 0 : 1;
    pending.push_back(make_shared<IntPair>(j, x));
    pending2->set(x * statesLen + j);
  }

  // process pending until fixed point
  int k = 2;
  // System.out.println("start min");
  while (!pending.empty()) {
    // System.out.println("  cycle pending");
    shared_ptr<IntPair> *const ip = pending.pop_front();
    constexpr int p = ip->n1;
    constexpr int x = ip->n2;
    // System.out.println("    pop n1=" + ip.n1 + " n2=" + ip.n2);
    pending2->clear(x * statesLen + p);
    // find states that need to be split off their blocks
    for (shared_ptr<StateListNode> m = active[p][x]->first; m != nullptr;
         m = m->next) {
      const deque<int> r = reverse[m->q][x];
      if (r.size() > 0) {
        for (auto i : r) {
          if (!split->get(i)) {
            split->set(i);
            constexpr int j = block[i];
            splitblock[j].push_back(i);
            if (!refine2->get(j)) {
              refine2->set(j);
              refine->set(j);
            }
          }
        }
      }
    }

    // refine blocks
    for (int j = refine->nextSetBit(0); j >= 0; j = refine->nextSetBit(j + 1)) {
      const deque<int> sb = splitblock[j];
      if (sb.size() < partition[j].size()) {
        const unordered_set<int> b1 = partition[j];
        const unordered_set<int> b2 = partition[k];
        for (auto s : sb) {
          b1.remove(s);
          b2.insert(s);
          block[s] = k;
          for (int c = 0; c < sigmaLen; c++) {
            shared_ptr<StateListNode> *const sn = active2[s][c];
            if (sn != nullptr && sn->sl == active[j][c]) {
              sn->remove();
              active2[s][c] = active[k][c]->add(s);
            }
          }
        }
        // update pending
        for (int c = 0; c < sigmaLen; c++) {
          constexpr int aj = active[j][c]->size, ak = active[k][c]->size,
                        ofs = c * statesLen;
          if (!pending2->get(ofs + j) && 0 < aj && aj <= ak) {
            pending2->set(ofs + j);
            pending.push_back(make_shared<IntPair>(j, c));
          } else {
            pending2->set(ofs + k);
            pending.push_back(make_shared<IntPair>(k, c));
          }
        }
        k++;
      }
      refine2->clear(j);
      for (auto s : sb) {
        split->clear(s);
      }
      sb.clear();
    }
    refine->clear();
  }

  shared_ptr<Automaton> result = make_shared<Automaton>();

  shared_ptr<Transition> t = make_shared<Transition>();

  // System.out.println("  k=" + k);

  // make a new state for each equivalence class, set initial state
  std::deque<int> stateMap(statesLen);
  std::deque<int> stateRep(k);

  result->createState();

  // System.out.println("min: k=" + k);
  for (int n = 0; n < k; n++) {
    // System.out.println("    n=" + n);

    bool isInitial = false;
    for (auto q : partition[n]) {
      if (q == 0) {
        isInitial = true;
        // System.out.println("    isInitial!");
        break;
      }
    }

    int newState;
    if (isInitial) {
      newState = 0;
    } else {
      newState = result->createState();
    }

    // System.out.println("  newState=" + newState);

    for (auto q : partition[n]) {
      stateMap[q] = newState;
      // System.out.println("      q=" + q + " isAccept?=" + a.isAccept(q));
      result->setAccept(newState, a->isAccept(q));
      stateRep[newState] = q; // select representative
    }
  }

  // build transitions and set acceptance
  for (int n = 0; n < k; n++) {
    int numTransitions = a->initTransition(stateRep[n], t);
    for (int i = 0; i < numTransitions; i++) {
      a->getNextTransition(t);
      // System.out.println("  add trans");
      result->addTransition(n, stateMap[t->dest], t->min, t->max);
    }
  }
  result->finishState();
  // System.out.println(result.getNumStates() + " states");

  return Operations::removeDeadStates(result);
}

MinimizationOperations::IntPair::IntPair(int n1, int n2) : n1(n1), n2(n2) {}

shared_ptr<StateListNode> MinimizationOperations::StateList::add(int q)
{
  return make_shared<StateListNode>(q, shared_from_this());
}

MinimizationOperations::StateListNode::StateListNode(int q,
                                                     shared_ptr<StateList> sl)
    : q(q), sl(sl)
{
  if (sl->size++ == 0) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    sl->first = sl->last = shared_from_this();
  } else {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    sl->last->next = shared_from_this();
    prev = sl->last;
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    sl->last = shared_from_this();
  }
}

void MinimizationOperations::StateListNode::remove()
{
  sl->size--;
  if (sl->first == shared_from_this()) {
    sl->first = next;
  } else {
    prev->next = next;
  }
  if (sl->last == shared_from_this()) {
    sl->last = prev;
  } else {
    next->prev = prev;
  }
}
} // namespace org::apache::lucene::util::automaton