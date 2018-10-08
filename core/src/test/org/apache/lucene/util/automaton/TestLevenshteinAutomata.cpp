using namespace std;

#include "TestLevenshteinAutomata.h"

namespace org::apache::lucene::util::automaton
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestLevenshteinAutomata::testLev0() 
{
  assertLev(L"", 0);
  assertCharVectors(0);
}

void TestLevenshteinAutomata::testLev1() 
{
  assertLev(L"", 1);
  assertCharVectors(1);
}

void TestLevenshteinAutomata::testLev2() 
{
  assertLev(L"", 2);
  assertCharVectors(2);
}

void TestLevenshteinAutomata::testNoWastedStates() 
{
  assertFalse(Operations::hasDeadStatesFromInitial(
      (make_shared<LevenshteinAutomata>(L"abc", false))->toAutomaton(1)));
}

void TestLevenshteinAutomata::assertCharVectors(int n)
{
  int k = 2 * n + 1;
  // use k + 2 as the exponent: the formula generates different transitions
  // for w, w-1, w-2
  int limit = static_cast<int>(pow(2, k + 2));
  for (int i = 0; i < limit; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring encoded = Integer::toString(i, 2);
    assertLev(encoded, n);
  }
}

void TestLevenshteinAutomata::assertLev(const wstring &s, int maxDistance)
{
  shared_ptr<LevenshteinAutomata> builder =
      make_shared<LevenshteinAutomata>(s, false);
  shared_ptr<LevenshteinAutomata> tbuilder =
      make_shared<LevenshteinAutomata>(s, true);
  std::deque<std::shared_ptr<Automaton>> automata(maxDistance + 1);
  std::deque<std::shared_ptr<Automaton>> tautomata(maxDistance + 1);
  for (int n = 0; n < automata.size(); n++) {
    automata[n] = builder->toAutomaton(n);
    tautomata[n] = tbuilder->toAutomaton(n);
    assertNotNull(automata[n]);
    assertNotNull(tautomata[n]);
    assertTrue(automata[n]->isDeterministic());
    assertTrue(tautomata[n]->isDeterministic());
    assertTrue(Operations::isFinite(automata[n]));
    assertTrue(Operations::isFinite(tautomata[n]));
    assertFalse(Operations::hasDeadStatesFromInitial(automata[n]));
    assertFalse(Operations::hasDeadStatesFromInitial(tautomata[n]));
    // check that the dfa for n-1 accepts a subset of the dfa for n
    if (n > 0) {
      assertTrue(
          Operations::subsetOf(Operations::removeDeadStates(automata[n - 1]),
                               Operations::removeDeadStates(automata[n])));
      assertTrue(
          Operations::subsetOf(Operations::removeDeadStates(automata[n - 1]),
                               Operations::removeDeadStates(tautomata[n])));
      assertTrue(
          Operations::subsetOf(Operations::removeDeadStates(tautomata[n - 1]),
                               Operations::removeDeadStates(automata[n])));
      assertTrue(
          Operations::subsetOf(Operations::removeDeadStates(tautomata[n - 1]),
                               Operations::removeDeadStates(tautomata[n])));
      assertNotSame(automata[n - 1], automata[n]);
    }
    // check that Lev(N) is a subset of LevT(N)
    assertTrue(
        Operations::subsetOf(Operations::removeDeadStates(automata[n]),
                             Operations::removeDeadStates(tautomata[n])));
    // special checks for specific n
    switch (n) {
    case 0:
      // easy, matches the string itself
      assertTrue(Operations::sameLanguage(
          Automata::makeString(s), Operations::removeDeadStates(automata[0])));
      assertTrue(Operations::sameLanguage(
          Automata::makeString(s), Operations::removeDeadStates(tautomata[0])));
      break;
    case 1:
      // generate a lev1 naively, and check the accepted lang is the same.
      assertTrue(Operations::sameLanguage(
          naiveLev1(s), Operations::removeDeadStates(automata[1])));
      assertTrue(Operations::sameLanguage(
          naiveLev1T(s), Operations::removeDeadStates(tautomata[1])));
      break;
    default:
      assertBruteForce(s, automata[n], n);
      assertBruteForceT(s, tautomata[n], n);
      break;
    }
  }
}

shared_ptr<Automaton> TestLevenshteinAutomata::naiveLev1(const wstring &s)
{
  shared_ptr<Automaton> a = Automata::makeString(s);
  a = Operations::union_(a, insertionsOf(s));
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  a = Operations::union_(a, deletionsOf(s));
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  a = Operations::union_(a, substitutionsOf(s));
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);

  return a;
}

shared_ptr<Automaton> TestLevenshteinAutomata::naiveLev1T(const wstring &s)
{
  shared_ptr<Automaton> a = naiveLev1(s);
  a = Operations::union_(a, transpositionsOf(s));
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  return a;
}

shared_ptr<Automaton> TestLevenshteinAutomata::insertionsOf(const wstring &s)
{
  deque<std::shared_ptr<Automaton>> deque =
      deque<std::shared_ptr<Automaton>>();

  for (int i = 0; i <= s.length(); i++) {
    shared_ptr<Automaton> a = Automata::makeString(s.substr(0, i));
    a = Operations::concatenate(a, Automata::makeAnyChar());
    a = Operations::concatenate(a, Automata::makeString(s.substr(i)));
    deque.push_back(a);
  }

  shared_ptr<Automaton> a = Operations::union_(deque);
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  return a;
}

shared_ptr<Automaton> TestLevenshteinAutomata::deletionsOf(const wstring &s)
{
  deque<std::shared_ptr<Automaton>> deque =
      deque<std::shared_ptr<Automaton>>();

  for (int i = 0; i < s.length(); i++) {
    shared_ptr<Automaton> a = Automata::makeString(s.substr(0, i));
    a = Operations::concatenate(a, Automata::makeString(s.substr(i + 1)));
    deque.push_back(a);
  }

  shared_ptr<Automaton> a = Operations::union_(deque);
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  return a;
}

shared_ptr<Automaton> TestLevenshteinAutomata::substitutionsOf(const wstring &s)
{
  deque<std::shared_ptr<Automaton>> deque =
      deque<std::shared_ptr<Automaton>>();

  for (int i = 0; i < s.length(); i++) {
    shared_ptr<Automaton> a = Automata::makeString(s.substr(0, i));
    a = Operations::concatenate(a, Automata::makeAnyChar());
    a = Operations::concatenate(a, Automata::makeString(s.substr(i + 1)));
    deque.push_back(a);
  }

  shared_ptr<Automaton> a = Operations::union_(deque);
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  return a;
}

shared_ptr<Automaton>
TestLevenshteinAutomata::transpositionsOf(const wstring &s)
{
  if (s.length() < 2) {
    return Automata::makeEmpty();
  }
  deque<std::shared_ptr<Automaton>> deque =
      deque<std::shared_ptr<Automaton>>();
  for (int i = 0; i < s.length() - 1; i++) {
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    sb->append(s.substr(0, i));
    sb->append(s[i + 1]);
    sb->append(s[i]);
    sb->append(s.substr(i + 2, s.length() - (i + 2)));
    wstring st = sb->toString();
    if (st != s) {
      deque.push_back(Automata::makeString(st));
    }
  }
  shared_ptr<Automaton> a = Operations::union_(deque);
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  return a;
}

void TestLevenshteinAutomata::assertBruteForce(const wstring &input,
                                               shared_ptr<Automaton> dfa,
                                               int distance)
{
  shared_ptr<CharacterRunAutomaton> ra =
      make_shared<CharacterRunAutomaton>(dfa);
  int maxLen = input.length() + distance + 1;
  int maxNum = static_cast<int>(pow(2, maxLen));
  for (int i = 0; i < maxNum; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring encoded = Integer::toString(i, 2);
    bool accepts = ra->run(encoded);
    if (accepts) {
      assertTrue(getDistance(input, encoded) <= distance);
    } else {
      assertTrue(getDistance(input, encoded) > distance);
    }
  }
}

void TestLevenshteinAutomata::assertBruteForceT(const wstring &input,
                                                shared_ptr<Automaton> dfa,
                                                int distance)
{
  shared_ptr<CharacterRunAutomaton> ra =
      make_shared<CharacterRunAutomaton>(dfa);
  int maxLen = input.length() + distance + 1;
  int maxNum = static_cast<int>(pow(2, maxLen));
  for (int i = 0; i < maxNum; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring encoded = Integer::toString(i, 2);
    bool accepts = ra->run(encoded);
    if (accepts) {
      assertTrue(getTDistance(input, encoded) <= distance);
    } else {
      assertTrue(getTDistance(input, encoded) > distance);
    }
  }
}

int TestLevenshteinAutomata::getDistance(const wstring &target,
                                         const wstring &other)
{
  std::deque<wchar_t> sa;
  int n;
  std::deque<int> p;  //'previous' cost array, horizontally
  std::deque<int> d;  // cost array, horizontally
  std::deque<int> _d; // placeholder to assist in swapping p and d

  /*
     The difference between this impl. and the previous is that, rather
     than creating and retaining a matrix of size s.length()+1 by t.length()+1,
     we maintain two single-dimensional arrays of length s.length()+1.  The
     first, d, is the 'current working' distance array that maintains the newest
     distance cost counts as we iterate through the characters of std::wstring s. Each
     time we increment the index of std::wstring t we are comparing, d is copied to p,
     the second int[].  Doing so allows us to retain the previous cost counts as
     required by the algorithm (taking the minimum of the cost count to the
     left, up one, and diagonally up and to the left of the current cost count
     being calculated).  (Note that the arrays aren't really copied anymore,
     just switched...this is clearly much better than cloning an array or doing
     a System.arraycopy() each time  through the outer loop.)

     Effectively, the difference between the two implementations is this one
     does not cause an out of memory condition when calculating the LD over two
     very large strings.
   */

  sa = target.toCharArray();
  n = sa.size();
  p = std::deque<int>(n + 1);
  d = std::deque<int>(n + 1);

  constexpr int m = other.length();
  if (n == 0 || m == 0) {
    if (n == m) {
      return 0;
    } else {
      return max(n, m);
    }
  }

  // indexes into strings s and t
  int i; // iterates through s
  int j; // iterates through t

  wchar_t t_j; // jth character of t

  int cost; // cost

  for (i = 0; i <= n; i++) {
    p[i] = i;
  }

  for (j = 1; j <= m; j++) {
    t_j = other[j - 1];
    d[0] = j;

    for (i = 1; i <= n; i++) {
      cost = sa[i - 1] == t_j ? 0 : 1;
      // minimum of cell to the left+1, to the top+1, diagonally left and up
      // +cost
      d[i] = min(min(d[i - 1] + 1, p[i] + 1), p[i - 1] + cost);
    }

    // copy current distance counts to 'previous row' distance counts
    _d = p;
    p = d;
    d = _d;
  }

  // our last action in the above loop was to switch d and p, so p now
  // actually has the most recent cost counts
  return abs(p[n]);
}

int TestLevenshteinAutomata::getTDistance(const wstring &target,
                                          const wstring &other)
{
  std::deque<wchar_t> sa;
  int n;
  std::deque<std::deque<int>> d; // cost array

  sa = target.toCharArray();
  n = sa.size();
  constexpr int m = other.length();
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: d = new int[n+1][m+1];
  d = RectangularVectors::ReturnRectangularIntVector(n + 1, m + 1);

  if (n == 0 || m == 0) {
    if (n == m) {
      return 0;
    } else {
      return max(n, m);
    }
  }

  // indexes into strings s and t
  int i; // iterates through s
  int j; // iterates through t

  wchar_t t_j; // jth character of t

  int cost; // cost

  for (i = 0; i <= n; i++) {
    d[i][0] = i;
  }

  for (j = 0; j <= m; j++) {
    d[0][j] = j;
  }

  for (j = 1; j <= m; j++) {
    t_j = other[j - 1];

    for (i = 1; i <= n; i++) {
      cost = sa[i - 1] == t_j ? 0 : 1;
      // minimum of cell to the left+1, to the top+1, diagonally left and up
      // +cost
      d[i][j] =
          min(min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);
      // transposition
      if (i > 1 && j > 1 && target[i - 1] == other[j - 2] &&
          target[i - 2] == other[j - 1]) {
        d[i][j] = min(d[i][j], d[i - 2][j - 2] + cost);
      }
    }
  }

  // our last action in the above loop was to switch d and p, so p now
  // actually has the most recent cost counts
  return abs(d[n][m]);
}
} // namespace org::apache::lucene::util::automaton