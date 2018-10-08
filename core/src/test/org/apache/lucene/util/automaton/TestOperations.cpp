using namespace std;

#include "TestOperations.h"

namespace org::apache::lucene::util::automaton
{
using namespace org::apache::lucene::util;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestOperations::testStringUnion()
{
  deque<std::shared_ptr<BytesRef>> strings =
      deque<std::shared_ptr<BytesRef>>();
  for (int i = RandomNumbers::randomIntBetween(random(), 0, 1000); --i >= 0;) {
    strings.push_back(
        make_shared<BytesRef>(TestUtil::randomUnicodeString(random())));
  }

  sort(strings.begin(), strings.end());
  shared_ptr<Automaton> union_ = Automata::makeStringUnion(strings);
  assertTrue(union_->isDeterministic());
  assertFalse(Operations::hasDeadStatesFromInitial(union_));

  shared_ptr<Automaton> naiveUnion = TestOperations::naiveUnion(strings);
  assertTrue(naiveUnion->isDeterministic());
  assertFalse(Operations::hasDeadStatesFromInitial(naiveUnion));

  assertTrue(Operations::sameLanguage(union_, naiveUnion));
}

shared_ptr<Automaton>
TestOperations::naiveUnion(deque<std::shared_ptr<BytesRef>> &strings)
{
  std::deque<std::shared_ptr<Automaton>> eachIndividual(strings.size());
  int i = 0;
  for (auto bref : strings) {
    eachIndividual[i++] = Automata::makeString(bref->utf8ToString());
  }
  return Operations::determinize(
      Operations::union_(Arrays::asList(eachIndividual)),
      DEFAULT_MAX_DETERMINIZED_STATES);
}

void TestOperations::testEmptyLanguageConcatenate()
{
  shared_ptr<Automaton> a = Automata::makeString(L"a");
  shared_ptr<Automaton> concat =
      Operations::concatenate(a, Automata::makeEmpty());
  assertTrue(Operations::isEmpty(concat));
}

void TestOperations::testEmptySingletonNFAConcatenate()
{
  shared_ptr<Automaton> singleton = Automata::makeString(L"");
  shared_ptr<Automaton> expandedSingleton = singleton;
  // an NFA (two transitions for 't' from initial state)
  shared_ptr<Automaton> nfa = Operations::union_(
      Automata::makeString(L"this"), Automata::makeString(L"three"));
  shared_ptr<Automaton> concat1 =
      Operations::concatenate(expandedSingleton, nfa);
  shared_ptr<Automaton> concat2 = Operations::concatenate(singleton, nfa);
  assertFalse(concat2->isDeterministic());
  assertTrue(Operations::sameLanguage(Operations::determinize(concat1, 100),
                                      Operations::determinize(concat2, 100)));
  assertTrue(Operations::sameLanguage(Operations::determinize(nfa, 100),
                                      Operations::determinize(concat1, 100)));
  assertTrue(Operations::sameLanguage(Operations::determinize(nfa, 100),
                                      Operations::determinize(concat2, 100)));
}

void TestOperations::testGetRandomAcceptedString() 
{
  constexpr int ITER1 = atLeast(100);
  constexpr int ITER2 = atLeast(100);
  for (int i = 0; i < ITER1; i++) {

    shared_ptr<RegExp> *const re = make_shared<RegExp>(
        AutomatonTestUtil::randomRegexp(random()), RegExp::NONE);
    // System.out.println("TEST i=" + i + " re=" + re);
    shared_ptr<Automaton> *const a = Operations::determinize(
        re->toAutomaton(), DEFAULT_MAX_DETERMINIZED_STATES);
    assertFalse(Operations::isEmpty(a));

    shared_ptr<AutomatonTestUtil::RandomAcceptedStrings> *const rx =
        make_shared<AutomatonTestUtil::RandomAcceptedStrings>(a);
    for (int j = 0; j < ITER2; j++) {
      // System.out.println("TEST: j=" + j);
      std::deque<int> acc;
      try {
        acc = rx->getRandomAcceptedString(random());
        const wstring s = UnicodeUtil::newString(acc, 0, acc.size());
        // a.writeDot("adot");
        assertTrue(Operations::run(a, s));
      } catch (const runtime_error &t) {
        wcout << L"regexp: " << re << endl;
        if (acc.size() > 0) {
          wcout << L"fail acc re=" << re << L" count=" << acc.size() << endl;
          for (int k = 0; k < acc.size(); k++) {
            wcout << L"  " << Integer::toHexString(acc[k]) << endl;
          }
        }
        throw t;
      }
    }
  }
}

void TestOperations::testIsFinite()
{
  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    assertEquals(AutomatonTestUtil::isFiniteSlow(a), Operations::isFinite(a));
  }
}

void TestOperations::testIsFiniteEatsStack()
{
  std::deque<wchar_t> chars(50000);
  TestUtil::randomFixedLengthUnicodeString(random(), chars, 0, chars.size());
  wstring bigString1 = wstring(chars);
  TestUtil::randomFixedLengthUnicodeString(random(), chars, 0, chars.size());
  wstring bigString2 = wstring(chars);
  shared_ptr<Automaton> a = Operations::union_(
      Automata::makeString(bigString1), Automata::makeString(bigString2));
  invalid_argument exc = expectThrows(invalid_argument::typeid,
                                      [&]() { Operations::isFinite(a); });
  assertTrue(exc.what()->contains(L"input automaton is too large"));
}

void TestOperations::testTopoSortEatsStack()
{
  std::deque<wchar_t> chars(50000);
  TestUtil::randomFixedLengthUnicodeString(random(), chars, 0, chars.size());
  wstring bigString1 = wstring(chars);
  TestUtil::randomFixedLengthUnicodeString(random(), chars, 0, chars.size());
  wstring bigString2 = wstring(chars);
  shared_ptr<Automaton> a = Operations::union_(
      Automata::makeString(bigString1), Automata::makeString(bigString2));
  invalid_argument exc = expectThrows(invalid_argument::typeid,
                                      [&]() { Operations::topoSortStates(a); });
  assertTrue(exc.what()->contains(L"input automaton is too large"));
}

shared_ptr<Set<std::shared_ptr<IntsRef>>>
TestOperations::getFiniteStrings(shared_ptr<Automaton> a)
{
  return getFiniteStrings(make_shared<FiniteStringsIterator>(a));
}

shared_ptr<Set<std::shared_ptr<IntsRef>>>
TestOperations::getFiniteStrings(shared_ptr<Automaton> a, int limit)
{
  return getFiniteStrings(make_shared<LimitedFiniteStringsIterator>(a, limit));
}

shared_ptr<Set<std::shared_ptr<IntsRef>>>
TestOperations::getFiniteStrings(shared_ptr<FiniteStringsIterator> iterator)
{
  shared_ptr<Set<std::shared_ptr<IntsRef>>> result =
      unordered_set<std::shared_ptr<IntsRef>>();
  for (IntsRef finiteString; (finiteString = iterator->next()) != nullptr;) {
    result->add(IntsRef::deepCopyOf(finiteString));
  }

  return result;
}
} // namespace org::apache::lucene::util::automaton