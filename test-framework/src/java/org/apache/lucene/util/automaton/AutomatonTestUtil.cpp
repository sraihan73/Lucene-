using namespace std;

#include "AutomatonTestUtil.h"

namespace org::apache::lucene::util::automaton
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

wstring AutomatonTestUtil::randomRegexp(shared_ptr<Random> r)
{
  while (true) {
    wstring regexp = randomRegexpString(r);
    // we will also generate some undefined unicode queries
    if (!UnicodeUtil::validUTF16String(regexp)) {
      continue;
    }
    try {
      make_shared<RegExp>(regexp, RegExp::NONE);
      return regexp;
    } catch (const runtime_error &e) {
    }
  }
}

wstring AutomatonTestUtil::randomRegexpString(shared_ptr<Random> r)
{
  constexpr int end = r->nextInt(20);
  if (end == 0) {
    // allow 0 length
    return L"";
  }
  const std::deque<wchar_t> buffer = std::deque<wchar_t>(end);
  for (int i = 0; i < end; i++) {
    int t = r->nextInt(15);
    if (0 == t && i < end - 1) {
      // Make a surrogate pair
      // High surrogate
      buffer[i++] = static_cast<wchar_t>(TestUtil::nextInt(r, 0xd800, 0xdbff));
      // Low surrogate
      buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, 0xdc00, 0xdfff));
    } else if (t <= 1) {
      buffer[i] = static_cast<wchar_t>(r->nextInt(0x80));
    } else if (2 == t) {
      buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, 0x80, 0x800));
    } else if (3 == t) {
      buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, 0x800, 0xd7ff));
    } else if (4 == t) {
      buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, 0xe000, 0xffff));
    } else if (5 == t) {
      buffer[i] = L'.';
    } else if (6 == t) {
      buffer[i] = L'?';
    } else if (7 == t) {
      buffer[i] = L'*';
    } else if (8 == t) {
      buffer[i] = L'+';
    } else if (9 == t) {
      buffer[i] = L'(';
    } else if (10 == t) {
      buffer[i] = L')';
    } else if (11 == t) {
      buffer[i] = L'-';
    } else if (12 == t) {
      buffer[i] = L'[';
    } else if (13 == t) {
      buffer[i] = L']';
    } else if (14 == t) {
      buffer[i] = L'|';
    }
  }
  return wstring(buffer, 0, end);
}

int AutomatonTestUtil::getRandomCodePoint(shared_ptr<Random> r, int min,
                                          int max)
{
  constexpr int code;
  if (max < UnicodeUtil::UNI_SUR_HIGH_START ||
      min > UnicodeUtil::UNI_SUR_HIGH_END) {
    // easy: entire range is before or after surrogates
    code = min + r->nextInt(max - min + 1);
  } else if (min >= UnicodeUtil::UNI_SUR_HIGH_START) {
    if (max > UnicodeUtil::UNI_SUR_LOW_END) {
      // after surrogates
      code = 1 + UnicodeUtil::UNI_SUR_LOW_END +
             r->nextInt(max - UnicodeUtil::UNI_SUR_LOW_END);
    } else {
      throw invalid_argument(L"transition accepts only surrogates: min=" +
                             to_wstring(min) + L" max=" + to_wstring(max));
    }
  } else if (max <= UnicodeUtil::UNI_SUR_LOW_END) {
    if (min < UnicodeUtil::UNI_SUR_HIGH_START) {
      // before surrogates
      code = min + r->nextInt(UnicodeUtil::UNI_SUR_HIGH_START - min);
    } else {
      throw invalid_argument(L"transition accepts only surrogates: min=" +
                             to_wstring(min) + L" max=" + to_wstring(max));
    }
  } else {
    // range includes all surrogates
    int gap1 = UnicodeUtil::UNI_SUR_HIGH_START - min;
    int gap2 = max - UnicodeUtil::UNI_SUR_LOW_END;
    int c = r->nextInt(gap1 + gap2);
    if (c < gap1) {
      code = min + c;
    } else {
      code = UnicodeUtil::UNI_SUR_LOW_END + c - gap1 + 1;
    }
  }

  assert((code >= min && code <= max &&
              (code < UnicodeUtil::UNI_SUR_HIGH_START ||
               code > UnicodeUtil::UNI_SUR_LOW_END),
          L"code=" + to_wstring(code) + L" min=" + to_wstring(min) + L" max=" +
              to_wstring(max)));
  return code;
}

AutomatonTestUtil::RandomAcceptedStrings::ArrivingTransition::
    ArrivingTransition(int from, shared_ptr<Transition> t)
    : from(from), t(t)
{
}

AutomatonTestUtil::RandomAcceptedStrings::RandomAcceptedStrings(
    shared_ptr<Automaton> a)
    : leadsToAccept(unordered_map<>()), a(a),
      transitions(a->getSortedTransitions())
{
  if (a->getNumStates() == 0) {
    throw invalid_argument(L"this automaton accepts nothing");
  }

  const unordered_map<int, deque<std::shared_ptr<ArrivingTransition>>>
      allArriving =
          unordered_map<int, deque<std::shared_ptr<ArrivingTransition>>>();

  const deque<int> q = deque<int>();
  shared_ptr<Set<int>> *const seen = unordered_set<int>();

  // reverse map_obj the transitions, so we can quickly look
  // up all arriving transitions to a given state
  int numStates = a->getNumStates();
  for (int s = 0; s < numStates; s++) {
    for (auto t : transitions[s]) {
      deque<std::shared_ptr<ArrivingTransition>> tl = allArriving[t->dest];
      if (tl.empty()) {
        tl = deque<>();
        allArriving.emplace(t->dest, tl);
      }
      tl.push_back(make_shared<ArrivingTransition>(s, t));
    }
    if (a->isAccept(s)) {
      q.push_back(s);
      seen->add(s);
    }
  }

  // Breadth-first search, from accept states,
  // backwards:
  while (q.empty() == false) {
    constexpr int s = q.pop_front();
    deque<std::shared_ptr<ArrivingTransition>> arriving = allArriving[s];
    if (arriving.size() > 0) {
      for (auto at : arriving) {
        constexpr int from = at->from;
        if (!seen->contains(from)) {
          q.push_back(from);
          seen->add(from);
          leadsToAccept.emplace(at->t, Boolean::TRUE);
        }
      }
    }
  }
}

std::deque<int>
AutomatonTestUtil::RandomAcceptedStrings::getRandomAcceptedString(
    shared_ptr<Random> r)
{
  std::deque<int> codePoints(0);
  int codepointCount = 0;

  int s = 0;

  while (true) {

    if (a->isAccept(s)) {
      if (a->getNumTransitions(s) == 0) {
        // stop now
        break;
      } else {
        if (r->nextBoolean()) {
          break;
        }
      }
    }

    if (a->getNumTransitions(s) == 0) {
      throw runtime_error(L"this automaton has dead states");
    }

    bool cheat = r->nextBoolean();

    shared_ptr<Transition> *const t;
    if (cheat) {
      // pick a transition that we know is the fastest
      // path to an accept state
      deque<std::shared_ptr<Transition>> toAccept =
          deque<std::shared_ptr<Transition>>();
      for (auto t0 : transitions[s]) {
        if (leadsToAccept.find(t0) != leadsToAccept.end()) {
          toAccept.push_back(t0);
        }
      }
      if (toAccept.empty()) {
        // this is OK -- it means we jumped into a cycle
        t = transitions[s][r->nextInt(transitions[s].length)];
      } else {
        t = toAccept[r->nextInt(toAccept.size())];
      }
    } else {
      t = transitions[s][r->nextInt(transitions[s].length)];
    }
    codePoints = ArrayUtil::grow(codePoints, codepointCount + 1);
    codePoints[codepointCount++] = getRandomCodePoint(r, t->min, t->max);
    s = t->dest;
  }

  return Arrays::copyOf(codePoints, codepointCount);
}

shared_ptr<Automaton>
AutomatonTestUtil::randomSingleAutomaton(shared_ptr<Random> random)
{
  while (true) {
    try {
      shared_ptr<Automaton> a1 =
          (make_shared<RegExp>(AutomatonTestUtil::randomRegexp(random),
                               RegExp::NONE))
              ->toAutomaton();
      if (random->nextBoolean()) {
        a1 = Operations::complement(a1, DEFAULT_MAX_DETERMINIZED_STATES);
      }
      return a1;
    } catch (const TooComplexToDeterminizeException &tctde) {
      // This can (rarely) happen if the random regexp is too hard; just try
      // again...
    }
  }
}

shared_ptr<Automaton>
AutomatonTestUtil::randomAutomaton(shared_ptr<Random> random)
{
  // get two random Automata from regexps
  shared_ptr<Automaton> a1 = randomSingleAutomaton(random);
  shared_ptr<Automaton> a2 = randomSingleAutomaton(random);

  // combine them in random ways
  switch (random->nextInt(4)) {
  case 0:
    return Operations::concatenate(a1, a2);
  case 1:
    return Operations::union_(a1, a2);
  case 2:
    return Operations::intersection(a1, a2);
  default:
    return Operations::minus(a1, a2, DEFAULT_MAX_DETERMINIZED_STATES);
  }
}

shared_ptr<Automaton> AutomatonTestUtil::minimizeSimple(shared_ptr<Automaton> a)
{
  shared_ptr<Set<int>> initialSet = unordered_set<int>();
  a = determinizeSimple(Operations::reverse(a, initialSet), initialSet);
  initialSet->clear();
  a = determinizeSimple(Operations::reverse(a, initialSet), initialSet);
  return a;
}

shared_ptr<Automaton>
AutomatonTestUtil::determinizeSimple(shared_ptr<Automaton> a)
{
  shared_ptr<Set<int>> initialset = unordered_set<int>();
  initialset->add(0);
  return determinizeSimple(a, initialset);
}

shared_ptr<Automaton>
AutomatonTestUtil::determinizeSimple(shared_ptr<Automaton> a,
                                     shared_ptr<Set<int>> initialset)
{
  if (a->getNumStates() == 0) {
    return a;
  }
  std::deque<int> points = a->getStartPoints();
  // subset construction
  unordered_map<Set<int>, Set<int>> sets = unordered_map<Set<int>, Set<int>>();
  deque<Set<int>> worklist = deque<Set<int>>();
  unordered_map<Set<int>, int> newstate = unordered_map<Set<int>, int>();
  sets.emplace(initialset, initialset);
  worklist.push_back(initialset);
  shared_ptr<Automaton::Builder> result = make_shared<Automaton::Builder>();
  result->createState();
  newstate.emplace(initialset, 0);
  shared_ptr<Transition> t = make_shared<Transition>();
  while (worklist.size() > 0) {
    shared_ptr<Set<int>> s = worklist.pop_front();
    int r = newstate[s];
    for (auto q : s) {
      if (a->isAccept(q)) {
        result->setAccept(r, true);
        break;
      }
    }
    for (int n = 0; n < points.size(); n++) {
      shared_ptr<Set<int>> p = unordered_set<int>();
      for (auto q : s) {
        int count = a->initTransition(q, t);
        for (int i = 0; i < count; i++) {
          a->getNextTransition(t);
          if (t->min <= points[n] && points[n] <= t->max) {
            p->add(t->dest);
          }
        }
      }

      if (sets.find(p) == sets.end()) {
        sets.emplace(p, p);
        worklist.push_back(p);
        newstate.emplace(p, result->createState());
      }
      int q = newstate[p];
      int min = points[n];
      int max;
      if (n + 1 < points.size()) {
        max = points[n + 1] - 1;
      } else {
        max = Character::MAX_CODE_POINT;
      }
      result->addTransition(r, q, min, max);
    }
  }

  return Operations::removeDeadStates(result->finish());
}

shared_ptr<Set<std::shared_ptr<IntsRef>>>
AutomatonTestUtil::getFiniteStringsRecursive(shared_ptr<Automaton> a, int limit)
{
  unordered_set<std::shared_ptr<IntsRef>> strings =
      unordered_set<std::shared_ptr<IntsRef>>();
  if (!getFiniteStrings(a, 0, unordered_set<int>(), strings,
                        make_shared<IntsRefBuilder>(), limit)) {
    return strings;
  }
  return strings;
}

bool AutomatonTestUtil::getFiniteStrings(
    shared_ptr<Automaton> a, int s, unordered_set<int> &pathstates,
    unordered_set<std::shared_ptr<IntsRef>> &strings,
    shared_ptr<IntsRefBuilder> path, int limit)
{
  pathstates.insert(s);
  shared_ptr<Transition> t = make_shared<Transition>();
  int count = a->initTransition(s, t);
  for (int i = 0; i < count; i++) {
    a->getNextTransition(t);
    if (find(pathstates.begin(), pathstates.end(), t->dest) !=
        pathstates.end()) {
      return false;
    }
    for (int n = t->min; n <= t->max; n++) {
      path->append(n);
      if (a->isAccept(t->dest)) {
        strings.insert(path->toIntsRef());
        if (limit >= 0 && strings.size() > limit) {
          return false;
        }
      }
      if (!getFiniteStrings(a, t->dest, pathstates, strings, path, limit)) {
        return false;
      }
      path->setLength(path->length() - 1);
    }
  }
  pathstates.remove(s);
  return true;
}

bool AutomatonTestUtil::isFiniteSlow(shared_ptr<Automaton> a)
{
  if (a->getNumStates() == 0) {
    return true;
  }
  return isFiniteSlow(a, 0, unordered_set<int>());
}

bool AutomatonTestUtil::isFiniteSlow(shared_ptr<Automaton> a, int s,
                                     unordered_set<int> &path)
{
  path.insert(s);
  shared_ptr<Transition> t = make_shared<Transition>();
  int count = a->initTransition(s, t);
  for (int i = 0; i < count; i++) {
    a->getNextTransition(t);
    if (find(path.begin(), path.end(), t->dest) != path.end() ||
        !isFiniteSlow(a, t->dest, path)) {
      return false;
    }
  }
  path.remove(s);
  return true;
}

void AutomatonTestUtil::assertNoDetachedStates(shared_ptr<Automaton> a)
{
  shared_ptr<Automaton> a2 = Operations::removeDeadStates(a);
  assert((a->getNumStates() == a2->getNumStates(),
          L"automaton has " +
              to_wstring(a->getNumStates() - a2->getNumStates()) +
              L" detached states"));
}

bool AutomatonTestUtil::isDeterministicSlow(shared_ptr<Automaton> a)
{
  shared_ptr<Transition> t = make_shared<Transition>();
  int numStates = a->getNumStates();
  for (int s = 0; s < numStates; s++) {
    int count = a->initTransition(s, t);
    int lastMax = -1;
    for (int i = 0; i < count; i++) {
      a->getNextTransition(t);
      if (t->min <= lastMax) {
        assert(a->isDeterministic() == false);
        return false;
      }
      lastMax = t->max;
    }
  }

  assert(a->isDeterministic() == true);
  return true;
}
} // namespace org::apache::lucene::util::automaton