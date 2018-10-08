using namespace std;

#include "TestAutomaton.h"

namespace org::apache::lucene::util::automaton
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using RandomAcceptedStrings = org::apache::lucene::util::automaton::
    AutomatonTestUtil::RandomAcceptedStrings;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestAutomaton::testBasic() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int start = a->createState();
  int x = a->createState();
  int y = a->createState();
  int end = a->createState();
  a->setAccept(end, true);

  a->addTransition(start, x, L'a', L'a');
  a->addTransition(start, end, L'd', L'd');
  a->addTransition(x, y, L'b', L'b');
  a->addTransition(y, end, L'c', L'c');
  a->finishState();
}

void TestAutomaton::testReduceBasic() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int start = a->createState();
  int end = a->createState();
  a->setAccept(end, true);
  // Should collapse to a-b:
  a->addTransition(start, end, L'a', L'a');
  a->addTransition(start, end, L'b', L'b');
  a->addTransition(start, end, L'm', L'm');
  // Should collapse to x-y:
  a->addTransition(start, end, L'x', L'x');
  a->addTransition(start, end, L'y', L'y');

  a->finishState();
  TestUtil::assertEquals(3, a->getNumTransitions(start));
  shared_ptr<Transition> scratch = make_shared<Transition>();
  a->initTransition(start, scratch);
  a->getNextTransition(scratch);
  TestUtil::assertEquals(L'a', scratch->min);
  TestUtil::assertEquals(L'b', scratch->max);
  a->getNextTransition(scratch);
  TestUtil::assertEquals(L'm', scratch->min);
  TestUtil::assertEquals(L'm', scratch->max);
  a->getNextTransition(scratch);
  TestUtil::assertEquals(L'x', scratch->min);
  TestUtil::assertEquals(L'y', scratch->max);
}

void TestAutomaton::testSameLanguage() 
{
  shared_ptr<Automaton> a1 = Automata::makeString(L"foobar");
  shared_ptr<Automaton> a2 =
      Operations::removeDeadStates(Operations::concatenate(
          Automata::makeString(L"foo"), Automata::makeString(L"bar")));
  assertTrue(Operations::sameLanguage(a1, a2));
}

void TestAutomaton::testCommonPrefix() 
{
  shared_ptr<Automaton> a = Operations::concatenate(
      Automata::makeString(L"foobar"), Automata::makeAnyString());
  TestUtil::assertEquals(L"foobar", Operations::getCommonPrefix(a));
}

void TestAutomaton::testConcatenate1() 
{
  shared_ptr<Automaton> a = Operations::concatenate(Automata::makeString(L"m"),
                                                    Automata::makeAnyString());
  assertTrue(Operations::run(a, L"m"));
  assertTrue(Operations::run(a, L"me"));
  assertTrue(Operations::run(a, L"me too"));
}

void TestAutomaton::testConcatenate2() 
{
  shared_ptr<Automaton> a = Operations::concatenate(
      Arrays::asList(Automata::makeString(L"m"), Automata::makeAnyString(),
                     Automata::makeString(L"n"), Automata::makeAnyString()));
  a = Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a, L"mn"));
  assertTrue(Operations::run(a, L"mone"));
  assertFalse(Operations::run(a, L"m"));
  assertFalse(Operations::isFinite(a));
}

void TestAutomaton::testUnion1() 
{
  shared_ptr<Automaton> a = Operations::union_(Arrays::asList(
      Automata::makeString(L"foobar"), Automata::makeString(L"barbaz")));
  a = Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a, L"foobar"));
  assertTrue(Operations::run(a, L"barbaz"));

  assertMatches(a, {L"foobar", L"barbaz"});
}

void TestAutomaton::testUnion2() 
{
  shared_ptr<Automaton> a = Operations::union_(
      Arrays::asList(Automata::makeString(L"foobar"), Automata::makeString(L""),
                     Automata::makeString(L"barbaz")));
  a = Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a, L"foobar"));
  assertTrue(Operations::run(a, L"barbaz"));
  assertTrue(Operations::run(a, L""));

  assertMatches(a, {L"", L"foobar", L"barbaz"});
}

void TestAutomaton::testMinimizeSimple() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"foobar");
  shared_ptr<Automaton> aMin =
      MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);

  assertTrue(Operations::sameLanguage(a, aMin));
}

void TestAutomaton::testMinimize2() 
{
  shared_ptr<Automaton> a = Operations::union_(Arrays::asList(
      Automata::makeString(L"foobar"), Automata::makeString(L"boobar")));
  shared_ptr<Automaton> aMin =
      MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::sameLanguage(
      Operations::determinize(Operations::removeDeadStates(a),
                              DEFAULT_MAX_DETERMINIZED_STATES),
      aMin));
}

void TestAutomaton::testReverse() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"foobar");
  shared_ptr<Automaton> ra = Operations::reverse(a);
  shared_ptr<Automaton> a2 = Operations::determinize(
      Operations::reverse(ra), DEFAULT_MAX_DETERMINIZED_STATES);

  assertTrue(Operations::sameLanguage(a, a2));
}

void TestAutomaton::testOptional() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"foobar");
  shared_ptr<Automaton> a2 = Operations::optional(a);
  a2 = Operations::determinize(a2, DEFAULT_MAX_DETERMINIZED_STATES);

  assertTrue(Operations::run(a, L"foobar"));
  assertFalse(Operations::run(a, L""));
  assertTrue(Operations::run(a2, L"foobar"));
  assertTrue(Operations::run(a2, L""));
}

void TestAutomaton::testRepeatAny() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"zee");
  shared_ptr<Automaton> a2 = Operations::determinize(
      Operations::repeat(a), DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a2, L""));
  assertTrue(Operations::run(a2, L"zee"));
  assertTrue(Operations::run(a2, L"zeezee"));
  assertTrue(Operations::run(a2, L"zeezeezee"));
}

void TestAutomaton::testRepeatMin() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"zee");
  shared_ptr<Automaton> a2 = Operations::determinize(
      Operations::repeat(a, 2), DEFAULT_MAX_DETERMINIZED_STATES);
  assertFalse(Operations::run(a2, L""));
  assertFalse(Operations::run(a2, L"zee"));
  assertTrue(Operations::run(a2, L"zeezee"));
  assertTrue(Operations::run(a2, L"zeezeezee"));
}

void TestAutomaton::testRepeatMinMax1() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"zee");
  shared_ptr<Automaton> a2 = Operations::determinize(
      Operations::repeat(a, 0, 2), DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a2, L""));
  assertTrue(Operations::run(a2, L"zee"));
  assertTrue(Operations::run(a2, L"zeezee"));
  assertFalse(Operations::run(a2, L"zeezeezee"));
}

void TestAutomaton::testRepeatMinMax2() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"zee");
  shared_ptr<Automaton> a2 = Operations::determinize(
      Operations::repeat(a, 2, 4), DEFAULT_MAX_DETERMINIZED_STATES);
  assertFalse(Operations::run(a2, L""));
  assertFalse(Operations::run(a2, L"zee"));
  assertTrue(Operations::run(a2, L"zeezee"));
  assertTrue(Operations::run(a2, L"zeezeezee"));
  assertTrue(Operations::run(a2, L"zeezeezeezee"));
  assertFalse(Operations::run(a2, L"zeezeezeezeezee"));
}

void TestAutomaton::testComplement() 
{
  shared_ptr<Automaton> a = Automata::makeString(L"zee");
  shared_ptr<Automaton> a2 = Operations::determinize(
      Operations::complement(a, DEFAULT_MAX_DETERMINIZED_STATES),
      DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a2, L""));
  assertFalse(Operations::run(a2, L"zee"));
  assertTrue(Operations::run(a2, L"zeezee"));
  assertTrue(Operations::run(a2, L"zeezeezee"));
}

void TestAutomaton::testInterval() 
{
  shared_ptr<Automaton> a =
      Operations::determinize(Automata::makeDecimalInterval(17, 100, 3),
                              DEFAULT_MAX_DETERMINIZED_STATES);
  assertFalse(Operations::run(a, L""));
  assertTrue(Operations::run(a, L"017"));
  assertTrue(Operations::run(a, L"100"));
  assertTrue(Operations::run(a, L"073"));
}

void TestAutomaton::testCommonSuffix() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int init = a->createState();
  int fini = a->createState();
  a->setAccept(init, true);
  a->setAccept(fini, true);
  a->addTransition(init, fini, L'm');
  a->addTransition(fini, fini, L'm');
  a->finishState();
  TestUtil::assertEquals(
      0, Operations::getCommonSuffixBytesRef(a, DEFAULT_MAX_DETERMINIZED_STATES)
             ->length);
}

void TestAutomaton::testReverseRandom1() 
{
  int ITERS = atLeast(100);
  for (int i = 0; i < ITERS; i++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    shared_ptr<Automaton> ra = Operations::reverse(a);
    shared_ptr<Automaton> rra = Operations::reverse(ra);
    assertTrue(Operations::sameLanguage(
        Operations::determinize(Operations::removeDeadStates(a),
                                numeric_limits<int>::max()),
        Operations::determinize(Operations::removeDeadStates(rra),
                                numeric_limits<int>::max())));
  }
}

void TestAutomaton::testReverseRandom2() 
{
  int ITERS = atLeast(100);
  for (int iter = 0; iter < ITERS; iter++) {
    // System.out.println("TEST: iter=" + iter);
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    if (random()->nextBoolean()) {
      a = Operations::removeDeadStates(a);
    }
    shared_ptr<Automaton> ra = Operations::reverse(a);
    shared_ptr<Automaton> rda =
        Operations::determinize(ra, numeric_limits<int>::max());

    if (Operations::isEmpty(a)) {
      assertTrue(Operations::isEmpty(rda));
      continue;
    }

    shared_ptr<RandomAcceptedStrings> ras =
        make_shared<RandomAcceptedStrings>(a);

    for (int iter2 = 0; iter2 < 20; iter2++) {
      // Find string accepted by original automaton
      std::deque<int> s = ras->getRandomAcceptedString(random());

      // Reverse it
      for (int j = 0; j < s.size() / 2; j++) {
        int x = s[j];
        s[j] = s[s.size() - j - 1];
        s[s.size() - j - 1] = x;
      }
      // System.out.println("TEST:   iter2=" + iter2 + " s=" +
      // Arrays.toString(s));

      // Make sure reversed automaton accepts it
      assertTrue(Operations::run(rda, make_shared<IntsRef>(s, 0, s.size())));
    }
  }
}

void TestAutomaton::testAnyStringEmptyString() 
{
  shared_ptr<Automaton> a = Operations::determinize(
      Automata::makeAnyString(), DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a, L""));
}

void TestAutomaton::testBasicIsEmpty() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->createState();
  assertTrue(Operations::isEmpty(a));
}

void TestAutomaton::testRemoveDeadTransitionsEmpty() 
{
  shared_ptr<Automaton> a = Automata::makeEmpty();
  shared_ptr<Automaton> a2 = Operations::removeDeadStates(a);
  assertTrue(Operations::isEmpty(a2));
}

void TestAutomaton::testInvalidAddTransition() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int s1 = a->createState();
  int s2 = a->createState();
  a->addTransition(s1, s2, L'a');
  a->addTransition(s2, s2, L'a');
  expectThrows(IllegalStateException::typeid,
               [&]() { a->addTransition(s1, s2, L'b'); });
}

void TestAutomaton::testBuilderRandom() 
{
  int ITERS = atLeast(100);
  for (int iter = 0; iter < ITERS; iter++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());

    // Just get all transitions, shuffle, and build a new automaton with the
    // same transitions:
    deque<std::shared_ptr<Transition>> allTrans =
        deque<std::shared_ptr<Transition>>();
    int numStates = a->getNumStates();
    for (int s = 0; s < numStates; s++) {
      int count = a->getNumTransitions(s);
      for (int i = 0; i < count; i++) {
        shared_ptr<Transition> t = make_shared<Transition>();
        a->getTransition(s, i, t);
        allTrans.push_back(t);
      }
    }

    shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();
    for (int i = 0; i < numStates; i++) {
      int s = builder->createState();
      builder->setAccept(s, a->isAccept(s));
    }

    Collections::shuffle(allTrans, random());
    for (auto t : allTrans) {
      builder->addTransition(t->source, t->dest, t->min, t->max);
    }

    assertTrue(Operations::sameLanguage(
        Operations::determinize(Operations::removeDeadStates(a),
                                numeric_limits<int>::max()),
        Operations::determinize(Operations::removeDeadStates(builder->finish()),
                                numeric_limits<int>::max())));
  }
}

void TestAutomaton::testIsTotal() 
{
  assertFalse(Operations::isTotal(make_shared<Automaton>()));
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int init = a->createState();
  int fini = a->createState();
  a->setAccept(fini, true);
  a->addTransition(init, fini, Character::MIN_CODE_POINT,
                   Character::MAX_CODE_POINT);
  a->finishState();
  assertFalse(Operations::isTotal(a));
  a->addTransition(fini, fini, Character::MIN_CODE_POINT,
                   Character::MAX_CODE_POINT);
  a->finishState();
  assertFalse(Operations::isTotal(a));
  a->setAccept(init, true);
  assertTrue(Operations::isTotal(
      MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES)));
}

void TestAutomaton::testMinimizeEmpty() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int init = a->createState();
  int fini = a->createState();
  a->addTransition(init, fini, L'a');
  a->finishState();
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  TestUtil::assertEquals(0, a->getNumStates());
}

void TestAutomaton::testMinus() 
{
  shared_ptr<Automaton> a1 = Automata::makeString(L"foobar");
  shared_ptr<Automaton> a2 = Automata::makeString(L"boobar");
  shared_ptr<Automaton> a3 = Automata::makeString(L"beebar");
  shared_ptr<Automaton> a = Operations::union_(Arrays::asList(a1, a2, a3));
  if (random()->nextBoolean()) {
    a = Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  } else if (random()->nextBoolean()) {
    a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  }
  assertMatches(a, {L"foobar", L"beebar", L"boobar"});

  shared_ptr<Automaton> a4 = Operations::determinize(
      Operations::minus(a, a2, DEFAULT_MAX_DETERMINIZED_STATES),
      DEFAULT_MAX_DETERMINIZED_STATES);

  assertTrue(Operations::run(a4, L"foobar"));
  assertFalse(Operations::run(a4, L"boobar"));
  assertTrue(Operations::run(a4, L"beebar"));
  assertMatches(a4, {L"foobar", L"beebar"});

  a4 = Operations::determinize(
      Operations::minus(a4, a1, DEFAULT_MAX_DETERMINIZED_STATES),
      DEFAULT_MAX_DETERMINIZED_STATES);
  assertFalse(Operations::run(a4, L"foobar"));
  assertFalse(Operations::run(a4, L"boobar"));
  assertTrue(Operations::run(a4, L"beebar"));
  assertMatches(a4, {L"beebar"});

  a4 = Operations::determinize(
      Operations::minus(a4, a3, DEFAULT_MAX_DETERMINIZED_STATES),
      DEFAULT_MAX_DETERMINIZED_STATES);
  assertFalse(Operations::run(a4, L"foobar"));
  assertFalse(Operations::run(a4, L"boobar"));
  assertFalse(Operations::run(a4, L"beebar"));
  assertMatches(a4);
}

void TestAutomaton::testOneInterval() 
{
  shared_ptr<Automaton> a = Automata::makeDecimalInterval(999, 1032, 0);
  a = Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a, L"0999"));
  assertTrue(Operations::run(a, L"00999"));
  assertTrue(Operations::run(a, L"000999"));
}

void TestAutomaton::testAnotherInterval() 
{
  shared_ptr<Automaton> a = Automata::makeDecimalInterval(1, 2, 0);
  a = Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::run(a, L"01"));
}

void TestAutomaton::testIntervalRandom() 
{
  int ITERS = atLeast(100);
  for (int iter = 0; iter < ITERS; iter++) {
    int min = TestUtil::nextInt(random(), 0, 100000);
    int max = TestUtil::nextInt(random(), min, min + 100000);
    int digits;
    if (random()->nextBoolean()) {
      digits = 0;
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring s = Integer::toString(max);
      digits = TestUtil::nextInt(random(), s.length(), 2 * s.length());
    }
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (int i = 0; i < digits; i++) {
      b->append(L'0');
    }
    wstring prefix = b->toString();

    shared_ptr<Automaton> a =
        Operations::determinize(Automata::makeDecimalInterval(min, max, digits),
                                DEFAULT_MAX_DETERMINIZED_STATES);
    if (random()->nextBoolean()) {
      a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring mins = Integer::toString(min);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring maxs = Integer::toString(max);
    if (digits > 0) {
      mins = prefix.substr(mins.length()) + mins;
      maxs = prefix.substr(maxs.length()) + maxs;
    }
    assertTrue(Operations::run(a, mins));
    assertTrue(Operations::run(a, maxs));

    for (int iter2 = 0; iter2 < 100; iter2++) {
      int x = random()->nextInt(2 * max);
      bool expected = x >= min && x <= max;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring sx = Integer::toString(x);
      if (sx.length() < digits) {
        // Left-fill with 0s
        sx = b->substr(sx.length()) + sx;
      } else if (digits == 0) {
        // Left-fill with random number of 0s:
        int numZeros = random()->nextInt(10);
        shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
        for (int i = 0; i < numZeros; i++) {
          sb->append(L'0');
        }
        sb->append(sx);
        sx = sb->toString();
      }
      TestUtil::assertEquals(expected, Operations::run(a, sx));
    }
  }
}

void TestAutomaton::assertMatches(shared_ptr<Automaton> a,
                                  deque<wstring> &strings)
{
  shared_ptr<Set<std::shared_ptr<IntsRef>>> expected =
      unordered_set<std::shared_ptr<IntsRef>>();
  for (wstring s : strings) {
    shared_ptr<IntsRefBuilder> ints = make_shared<IntsRefBuilder>();
    expected->add(Util::toUTF32(s, ints));
  }

  TestUtil::assertEquals(
      expected, TestOperations::getFiniteStrings(Operations::determinize(
                    a, DEFAULT_MAX_DETERMINIZED_STATES)));
}

void TestAutomaton::testConcatenatePreservesDet() 
{
  shared_ptr<Automaton> a1 = Automata::makeString(L"foobar");
  assertTrue(a1->isDeterministic());
  shared_ptr<Automaton> a2 = Automata::makeString(L"baz");
  assertTrue(a2->isDeterministic());
  assertTrue(
      (Operations::concatenate(Arrays::asList(a1, a2))->isDeterministic()));
}

void TestAutomaton::testRemoveDeadStates() 
{
  shared_ptr<Automaton> a = Operations::concatenate(
      Arrays::asList(Automata::makeString(L"x"), Automata::makeString(L"y")));
  TestUtil::assertEquals(4, a->getNumStates());
  a = Operations::removeDeadStates(a);
  TestUtil::assertEquals(3, a->getNumStates());
}

void TestAutomaton::testRemoveDeadStatesEmpty1() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->finishState();
  assertTrue(Operations::isEmpty(a));
  assertTrue(Operations::isEmpty(Operations::removeDeadStates(a)));
}

void TestAutomaton::testRemoveDeadStatesEmpty2() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->finishState();
  assertTrue(Operations::isEmpty(a));
  assertTrue(Operations::isEmpty(Operations::removeDeadStates(a)));
}

void TestAutomaton::testRemoveDeadStatesEmpty3() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int init = a->createState();
  int fini = a->createState();
  a->addTransition(init, fini, L'a');
  shared_ptr<Automaton> a2 = Operations::removeDeadStates(a);
  TestUtil::assertEquals(0, a2->getNumStates());
}

void TestAutomaton::testConcatEmpty() 
{
  // If you concat empty automaton to anything the result should still be empty:
  shared_ptr<Automaton> a = Operations::concatenate(
      Automata::makeEmpty(), Automata::makeString(L"foo"));
  TestUtil::assertEquals(unordered_set<std::shared_ptr<IntsRef>>(),
                         TestOperations::getFiniteStrings(a));

  a = Operations::concatenate(Automata::makeString(L"foo"),
                              Automata::makeEmpty());
  TestUtil::assertEquals(unordered_set<std::shared_ptr<IntsRef>>(),
                         TestOperations::getFiniteStrings(a));
}

void TestAutomaton::testSeemsNonEmptyButIsNot1() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  // Init state has a transition but doesn't lead to accept
  int init = a->createState();
  int s = a->createState();
  a->addTransition(init, s, L'a');
  a->finishState();
  assertTrue(Operations::isEmpty(a));
}

void TestAutomaton::testSeemsNonEmptyButIsNot2() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int init = a->createState();
  int s = a->createState();
  a->addTransition(init, s, L'a');
  // An orphan'd accept state
  s = a->createState();
  a->setAccept(s, true);
  a->finishState();
  assertTrue(Operations::isEmpty(a));
}

void TestAutomaton::testSameLanguage1() 
{
  shared_ptr<Automaton> a = Automata::makeEmptyString();
  shared_ptr<Automaton> a2 = Automata::makeEmptyString();
  int state = a2->createState();
  a2->addTransition(0, state, L'a');
  a2->finishState();
  assertTrue(Operations::sameLanguage(Operations::removeDeadStates(a),
                                      Operations::removeDeadStates(a2)));
}

shared_ptr<Automaton> TestAutomaton::randomNoOp(shared_ptr<Automaton> a)
{
  switch (random()->nextInt(7)) {
  case 0:
    if (VERBOSE) {
      wcout << L"  randomNoOp: determinize" << endl;
    }
    return Operations::determinize(a, numeric_limits<int>::max());
  case 1:
    if (a->getNumStates() < 100) {
      if (VERBOSE) {
        wcout << L"  randomNoOp: minimize" << endl;
      }
      return MinimizationOperations::minimize(a,
                                              DEFAULT_MAX_DETERMINIZED_STATES);
    } else {
      if (VERBOSE) {
        wcout << L"  randomNoOp: skip op=minimize: too many states ("
              << a->getNumStates() << L")" << endl;
      }
      return a;
    }
  case 2:
    if (VERBOSE) {
      wcout << L"  randomNoOp: removeDeadStates" << endl;
    }
    return Operations::removeDeadStates(a);
  case 3:
    if (VERBOSE) {
      wcout << L"  randomNoOp: reverse reverse" << endl;
    }
    a = Operations::reverse(a);
    a = randomNoOp(a);
    return Operations::reverse(a);
  case 4:
    if (VERBOSE) {
      wcout << L"  randomNoOp: concat empty string" << endl;
    }
    return Operations::concatenate(a, Automata::makeEmptyString());
  case 5:
    if (VERBOSE) {
      wcout << L"  randomNoOp: union empty automaton" << endl;
    }
    return Operations::union_(a, Automata::makeEmpty());
  case 6:
    if (VERBOSE) {
      wcout << L"  randomNoOp: do nothing!" << endl;
    }
    return a;
  }
  assert(false);
  return nullptr;
}

shared_ptr<Automaton> TestAutomaton::unionTerms(
    shared_ptr<deque<std::shared_ptr<BytesRef>>> terms)
{
  shared_ptr<Automaton> a;
  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"TEST: unionTerms: use union" << endl;
    }
    deque<std::shared_ptr<Automaton>> as =
        deque<std::shared_ptr<Automaton>>();
    for (auto term : terms) {
      as.push_back(Automata::makeString(term->utf8ToString()));
    }
    a = Operations::union_(as);
  } else {
    if (VERBOSE) {
      wcout << L"TEST: unionTerms: use makeStringUnion" << endl;
    }
    deque<std::shared_ptr<BytesRef>> termsList =
        deque<std::shared_ptr<BytesRef>>(terms);
    sort(termsList.begin(), termsList.end());
    a = Automata::makeStringUnion(termsList);
  }

  return randomNoOp(a);
}

wstring TestAutomaton::getRandomString()
{
  // return TestUtil.randomSimpleString(random());
  return TestUtil::randomRealisticUnicodeString(random());
}

void TestAutomaton::testRandomFinite() 
{

  int numTerms = atLeast(10);
  int iters = atLeast(100);

  if (VERBOSE) {
    wcout << L"TEST: numTerms=" << numTerms << L" iters=" << iters << endl;
  }

  shared_ptr<Set<std::shared_ptr<BytesRef>>> terms =
      unordered_set<std::shared_ptr<BytesRef>>();
  while (terms->size() < numTerms) {
    terms->add(make_shared<BytesRef>(getRandomString()));
  }

  shared_ptr<Automaton> a = unionTerms(terms);
  assertSame(terms, a);

  for (int iter = 0; iter < iters; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << L" numTerms=" << terms->size()
            << L" a.numStates=" << a->getNumStates() << endl;
      /*
      System.out.println("  terms:");
      for(BytesRef term : terms) {
        System.out.println("    " + term);
      }
      */
    }
    switch (random()->nextInt(15)) {

    case 0: {
      // concatenate prefix
      if (VERBOSE) {
        wcout << L"  op=concat prefix" << endl;
      }
      shared_ptr<Set<std::shared_ptr<BytesRef>>> newTerms =
          unordered_set<std::shared_ptr<BytesRef>>();
      shared_ptr<BytesRef> prefix = make_shared<BytesRef>(getRandomString());
      shared_ptr<BytesRefBuilder> newTerm = make_shared<BytesRefBuilder>();
      for (auto term : terms) {
        newTerm->copyBytes(prefix);
        newTerm->append(term);
        newTerms->add(newTerm->toBytesRef());
      }
      terms = newTerms;
      bool wasDeterministic1 = a->isDeterministic();
      a = Operations::concatenate(Automata::makeString(prefix->utf8ToString()),
                                  a);
      TestUtil::assertEquals(wasDeterministic1, a->isDeterministic());
    } break;

    case 1: {
      // concatenate suffix
      shared_ptr<BytesRef> suffix = make_shared<BytesRef>(getRandomString());
      if (VERBOSE) {
        wcout << L"  op=concat suffix " << suffix << endl;
      }
      shared_ptr<Set<std::shared_ptr<BytesRef>>> newTerms =
          unordered_set<std::shared_ptr<BytesRef>>();
      shared_ptr<BytesRefBuilder> newTerm = make_shared<BytesRefBuilder>();
      for (auto term : terms) {
        newTerm->copyBytes(term);
        newTerm->append(suffix);
        newTerms->add(newTerm->toBytesRef());
      }
      terms = newTerms;
      a = Operations::concatenate(a,
                                  Automata::makeString(suffix->utf8ToString()));
    } break;

    case 2:
      // determinize
      if (VERBOSE) {
        wcout << L"  op=determinize" << endl;
      }
      a = Operations::determinize(a, numeric_limits<int>::max());
      assertTrue(a->isDeterministic());
      break;

    case 3:
      if (a->getNumStates() < 100) {
        if (VERBOSE) {
          wcout << L"  op=minimize" << endl;
        }
        // minimize
        a = MinimizationOperations::minimize(a,
                                             DEFAULT_MAX_DETERMINIZED_STATES);
      } else if (VERBOSE) {
        wcout << L"  skip op=minimize: too many states (" << a->getNumStates()
              << L")" << endl;
      }
      break;

    case 4: {
      // union
      if (VERBOSE) {
        wcout << L"  op=union" << endl;
      }
      shared_ptr<Set<std::shared_ptr<BytesRef>>> newTerms =
          unordered_set<std::shared_ptr<BytesRef>>();
      int numNewTerms = random()->nextInt(5);
      while (newTerms->size() < numNewTerms) {
        newTerms->add(make_shared<BytesRef>(getRandomString()));
      }
      terms->addAll(newTerms);
      shared_ptr<Automaton> newA = unionTerms(newTerms);
      a = Operations::union_(a, newA);
    } break;

    case 5: {
      // optional
      if (VERBOSE) {
        wcout << L"  op=optional" << endl;
      }
      // NOTE: This can add a dead state:
      a = Operations::optional(a);
      terms->add(make_shared<BytesRef>());
    } break;

    case 6: {
      // minus finite
      if (VERBOSE) {
        wcout << L"  op=minus finite" << endl;
      }
      if (terms->size() > 0) {
        shared_ptr<RandomAcceptedStrings> rasl =
            make_shared<RandomAcceptedStrings>(Operations::removeDeadStates(a));
        shared_ptr<Set<std::shared_ptr<BytesRef>>> toRemove =
            unordered_set<std::shared_ptr<BytesRef>>();
        int numToRemove =
            TestUtil::nextInt(random(), 1, (terms->size() + 1) / 2);
        while (toRemove->size() < numToRemove) {
          std::deque<int> ints = rasl->getRandomAcceptedString(random());
          shared_ptr<BytesRef> term = make_shared<BytesRef>(
              UnicodeUtil::newString(ints, 0, ints.size()));
          if (toRemove->contains(term) == false) {
            toRemove->add(term);
          }
        }
        for (auto term : toRemove) {
          bool removed = terms->remove(term);
          assertTrue(removed);
        }
        shared_ptr<Automaton> a2 = unionTerms(toRemove);
        a = Operations::minus(a, a2, numeric_limits<int>::max());
      }
    } break;

    case 7: {
      // minus infinite
      deque<std::shared_ptr<Automaton>> as =
          deque<std::shared_ptr<Automaton>>();
      int count = TestUtil::nextInt(random(), 1, 5);
      shared_ptr<Set<int>> prefixes = unordered_set<int>();
      while (prefixes->size() < count) {
        // prefix is a leading ascii byte; we remove <prefix>* from a
        int prefix = random()->nextInt(128);
        prefixes->add(prefix);
      }

      if (VERBOSE) {
        wcout << L"  op=minus infinite prefixes=" << prefixes << endl;
      }

      for (auto prefix : prefixes) {
        // prefix is a leading ascii byte; we remove <prefix>* from a
        shared_ptr<Automaton> a2 = make_shared<Automaton>();
        int init = a2->createState();
        int state = a2->createState();
        a2->addTransition(init, state, prefix);
        a2->setAccept(state, true);
        a2->addTransition(state, state, Character::MIN_CODE_POINT,
                          Character::MAX_CODE_POINT);
        a2->finishState();
        as.push_back(a2);
        Set<std::shared_ptr<BytesRef>>::const_iterator it = terms->begin();
        while (it != terms->end()) {
          shared_ptr<BytesRef> term = *it;
          if (term->length > 0 &&
              (term->bytes[term->offset] & 0xFF) == prefix) {
            (*it)->remove();
          }
          it++;
        }
      }
      shared_ptr<Automaton> a2 = randomNoOp(Operations::union_(as));
      a = Operations::minus(a, a2, DEFAULT_MAX_DETERMINIZED_STATES);
    } break;

    case 8: {
      int count = TestUtil::nextInt(random(), 10, 20);
      if (VERBOSE) {
        wcout << L"  op=intersect infinite count=" << count << endl;
      }
      // intersect infinite
      deque<std::shared_ptr<Automaton>> as =
          deque<std::shared_ptr<Automaton>>();

      shared_ptr<Set<int>> prefixes = unordered_set<int>();
      while (prefixes->size() < count) {
        int prefix = random()->nextInt(128);
        prefixes->add(prefix);
      }
      if (VERBOSE) {
        wcout << L"  prefixes=" << prefixes << endl;
      }

      for (auto prefix : prefixes) {
        // prefix is a leading ascii byte; we retain <prefix>* in a
        shared_ptr<Automaton> a2 = make_shared<Automaton>();
        int init = a2->createState();
        int state = a2->createState();
        a2->addTransition(init, state, prefix);
        a2->setAccept(state, true);
        a2->addTransition(state, state, Character::MIN_CODE_POINT,
                          Character::MAX_CODE_POINT);
        a2->finishState();
        as.push_back(a2);
        prefixes->add(prefix);
      }

      shared_ptr<Automaton> a2 = Operations::union_(as);
      if (random()->nextBoolean()) {
        a2 = Operations::determinize(a2, DEFAULT_MAX_DETERMINIZED_STATES);
      } else if (random()->nextBoolean()) {
        a2 = MinimizationOperations::minimize(a2,
                                              DEFAULT_MAX_DETERMINIZED_STATES);
      }
      a = Operations::intersection(a, a2);

      Set<std::shared_ptr<BytesRef>>::const_iterator it = terms->begin();
      while (it != terms->end()) {
        shared_ptr<BytesRef> term = *it;
        if (term->length == 0 ||
            prefixes->contains(term->bytes[term->offset] & 0xff) == false) {
          if (VERBOSE) {
            wcout << L"  drop term=" << term << endl;
          }
          (*it)->remove();
        } else {
          if (VERBOSE) {
            wcout << L"  keep term=" << term << endl;
          }
        }
        it++;
      }
    } break;

    case 9: {
      // reverse
      if (VERBOSE) {
        wcout << L"  op=reverse" << endl;
      }
      a = Operations::reverse(a);
      shared_ptr<Set<std::shared_ptr<BytesRef>>> newTerms =
          unordered_set<std::shared_ptr<BytesRef>>();
      for (auto term : terms) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        newTerms->add(make_shared<BytesRef>(
            (make_shared<StringBuilder>(term->utf8ToString()))
                ->reverse()
                ->toString()));
      }
      terms = newTerms;
    } break;

    case 10:
      if (VERBOSE) {
        wcout << L"  op=randomNoOp" << endl;
      }
      a = randomNoOp(a);
      break;

    case 11: {
      // interval
      int min = random()->nextInt(1000);
      int max = min + random()->nextInt(50);
      // digits must be non-zero else we make cycle
      // C++ TODO: There is no native C++ equivalent to 'toString':
      int digits = Integer::toString(max)->length();
      if (VERBOSE) {
        wcout << L"  op=union interval min=" << min << L" max=" << max
              << L" digits=" << digits << endl;
      }
      a = Operations::union_(a,
                             Automata::makeDecimalInterval(min, max, digits));
      shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
      for (int i = 0; i < digits; i++) {
        b->append(L'0');
      }
      wstring prefix = b->toString();
      for (int i = min; i <= max; i++) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wstring s = Integer::toString(i);
        if (s.length() < digits) {
          // Left-fill with 0s
          s = prefix.substr(s.length()) + s;
        }
        terms->add(make_shared<BytesRef>(s));
      }
    } break;

    case 12:
      if (VERBOSE) {
        wcout << L"  op=remove the empty string" << endl;
      }
      a = Operations::minus(a, Automata::makeEmptyString(),
                            DEFAULT_MAX_DETERMINIZED_STATES);
      terms->remove(make_shared<BytesRef>());
      break;

    case 13:
      if (VERBOSE) {
        wcout << L"  op=add the empty string" << endl;
      }
      a = Operations::union_(a, Automata::makeEmptyString());
      terms->add(make_shared<BytesRef>());
      break;

    case 14:
      // Safety in case we are really unlucky w/ the dice:
      if (terms->size() <= numTerms * 3) {
        if (VERBOSE) {
          wcout << L"  op=concat finite automaton" << endl;
        }
        int count = random()->nextBoolean() ? 2 : 3;
        shared_ptr<Set<std::shared_ptr<BytesRef>>> addTerms =
            unordered_set<std::shared_ptr<BytesRef>>();
        while (addTerms->size() < count) {
          addTerms->add(make_shared<BytesRef>(getRandomString()));
        }
        if (VERBOSE) {
          for (auto term : addTerms) {
            wcout << L"    term=" << term << endl;
          }
        }
        shared_ptr<Automaton> a2 = unionTerms(addTerms);
        shared_ptr<Set<std::shared_ptr<BytesRef>>> newTerms =
            unordered_set<std::shared_ptr<BytesRef>>();
        if (random()->nextBoolean()) {
          // suffix
          if (VERBOSE) {
            wcout << L"  do suffix" << endl;
          }
          a = Operations::concatenate(a, randomNoOp(a2));
          shared_ptr<BytesRefBuilder> newTerm = make_shared<BytesRefBuilder>();
          for (auto term : terms) {
            for (auto suffix : addTerms) {
              newTerm->copyBytes(term);
              newTerm->append(suffix);
              newTerms->add(newTerm->toBytesRef());
            }
          }
        } else {
          // prefix
          if (VERBOSE) {
            wcout << L"  do prefix" << endl;
          }
          a = Operations::concatenate(randomNoOp(a2), a);
          shared_ptr<BytesRefBuilder> newTerm = make_shared<BytesRefBuilder>();
          for (auto term : terms) {
            for (auto prefix : addTerms) {
              newTerm->copyBytes(prefix);
              newTerm->append(term);
              newTerms->add(newTerm->toBytesRef());
            }
          }
        }

        terms = newTerms;
      }
      break;
    default:
      throw make_shared<AssertionError>();
    }

    assertSame(terms, a);
    TestUtil::assertEquals(AutomatonTestUtil::isDeterministicSlow(a),
                           a->isDeterministic());

    if (random()->nextInt(10) == 7) {
      a = verifyTopoSort(a);
    }
  }

  assertSame(terms, a);
}

shared_ptr<Automaton> TestAutomaton::verifyTopoSort(shared_ptr<Automaton> a)
{
  std::deque<int> sorted = Operations::topoSortStates(a);
  // This can be < if we removed dead states:
  assertTrue(sorted.size() <= a->getNumStates());
  shared_ptr<Automaton> a2 = make_shared<Automaton>();
  std::deque<int> stateMap(a->getNumStates());
  Arrays::fill(stateMap, -1);
  shared_ptr<Transition> transition = make_shared<Transition>();
  for (auto state : sorted) {
    int newState = a2->createState();
    a2->setAccept(newState, a->isAccept(state));

    // Each state should only appear once in the sort:
    TestUtil::assertEquals(-1, stateMap[state]);
    stateMap[state] = newState;
  }

  // 2nd pass: add new transitions
  for (auto state : sorted) {
    int count = a->initTransition(state, transition);
    for (int i = 0; i < count; i++) {
      a->getNextTransition(transition);
      assert(stateMap[transition->dest] > stateMap[state]);
      a2->addTransition(stateMap[state], stateMap[transition->dest],
                        transition->min, transition->max);
    }
  }

  a2->finishState();
  return a2;
}

void TestAutomaton::assertSame(
    shared_ptr<deque<std::shared_ptr<BytesRef>>> terms,
    shared_ptr<Automaton> a)
{

  try {
    assertTrue(Operations::isFinite(a));
    assertFalse(Operations::isTotal(a));

    shared_ptr<Automaton> detA =
        Operations::determinize(a, DEFAULT_MAX_DETERMINIZED_STATES);

    // Make sure all terms are accepted:
    shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
    for (auto term : terms) {
      Util::toIntsRef(term, scratch);
      assertTrue(L"failed to accept term=" + term->utf8ToString(),
                 Operations::run(detA, term->utf8ToString()));
    }

    // Use getFiniteStrings:
    shared_ptr<Set<std::shared_ptr<IntsRef>>> expected =
        unordered_set<std::shared_ptr<IntsRef>>();
    for (auto term : terms) {
      shared_ptr<IntsRefBuilder> intsRef = make_shared<IntsRefBuilder>();
      Util::toUTF32(term->utf8ToString(), intsRef);
      expected->add(intsRef->toIntsRef());
    }
    shared_ptr<Set<std::shared_ptr<IntsRef>>> actual =
        TestOperations::getFiniteStrings(a);

    if (expected->equals(actual) == false) {
      wcout << L"FAILED:" << endl;
      for (auto term : expected) {
        if (actual->contains(term) == false) {
          wcout << L"  term=" << term << L" should be accepted but isn't"
                << endl;
        }
      }
      for (auto term : actual) {
        if (expected->contains(term) == false) {
          wcout << L"  term=" << term << L" is accepted but should not be"
                << endl;
        }
      }
      throw make_shared<AssertionError>(L"mismatch");
    }

    // Use sameLanguage:
    shared_ptr<Automaton> a2 = Operations::removeDeadStates(
        Operations::determinize(unionTerms(terms), numeric_limits<int>::max()));
    assertTrue(Operations::sameLanguage(
        a2, Operations::removeDeadStates(
                Operations::determinize(a, numeric_limits<int>::max()))));

    // Do same check, in UTF8 space
    shared_ptr<Automaton> utf8 =
        randomNoOp((make_shared<UTF32ToUTF8>())->convert(a));

    shared_ptr<Set<std::shared_ptr<IntsRef>>> expected2 =
        unordered_set<std::shared_ptr<IntsRef>>();
    for (auto term : terms) {
      shared_ptr<IntsRefBuilder> intsRef = make_shared<IntsRefBuilder>();
      Util::toIntsRef(term, intsRef);
      expected2->add(intsRef->toIntsRef());
    }
    TestUtil::assertEquals(expected2, TestOperations::getFiniteStrings(utf8));
  } catch (const AssertionError &ae) {
    wcout << L"TEST: FAILED: not same" << endl;
    wcout << L"  terms (count=" << terms->size() << L"):" << endl;
    for (auto term : terms) {
      wcout << L"    " << term << endl;
    }
    wcout << L"  automaton:" << endl;
    wcout << a->toDot() << endl;
    // a.writeDot("fail");
    throw ae;
  }
}

bool TestAutomaton::accepts(shared_ptr<Automaton> a, shared_ptr<BytesRef> b)
{
  shared_ptr<IntsRefBuilder> intsBuilder = make_shared<IntsRefBuilder>();
  Util::toIntsRef(b, intsBuilder);
  return Operations::run(a, intsBuilder->toIntsRef());
}

shared_ptr<Automaton> TestAutomaton::makeBinaryInterval(
    shared_ptr<BytesRef> minTerm, bool minInclusive,
    shared_ptr<BytesRef> maxTerm, bool maxInclusive)
{

  if (VERBOSE) {
    wcout << L"TEST: minTerm=" << minTerm << L" minInclusive=" << minInclusive
          << L" maxTerm=" << maxTerm << L" maxInclusive=" << maxInclusive
          << endl;
  }

  shared_ptr<Automaton> a = Automata::makeBinaryInterval(minTerm, minInclusive,
                                                         maxTerm, maxInclusive);

  shared_ptr<Automaton> minA =
      MinimizationOperations::minimize(a, numeric_limits<int>::max());
  if (minA->getNumStates() != a->getNumStates()) {
    assertTrue(minA->getNumStates() < a->getNumStates());
    wcout << L"Original was not minimal:" << endl;
    wcout << L"Original:\n" << a->toDot() << endl;
    wcout << L"Minimized:\n" << minA->toDot() << endl;
    wcout << L"minTerm=" << minTerm << L" minInclusive=" << minInclusive
          << endl;
    wcout << L"maxTerm=" << maxTerm << L" maxInclusive=" << maxInclusive
          << endl;
    fail(L"automaton was not minimal");
  }

  if (VERBOSE) {
    wcout << a->toDot() << endl;
  }

  return a;
}

void TestAutomaton::testMakeBinaryIntervalFiniteCasesBasic() throw(
    runtime_error)
{
  // 0 (incl) - 00 (incl)
  std::deque<char> zeros(3);
  shared_ptr<Automaton> a =
      makeBinaryInterval(make_shared<BytesRef>(zeros, 0, 1), true,
                         make_shared<BytesRef>(zeros, 0, 2), true);
  assertTrue(Operations::isFinite(a));
  assertFalse(accepts(a, make_shared<BytesRef>()));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 1)));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 2)));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 3)));

  // '' (incl) - 00 (incl)
  a = makeBinaryInterval(make_shared<BytesRef>(), true,
                         make_shared<BytesRef>(zeros, 0, 2), true);
  assertTrue(Operations::isFinite(a));
  assertTrue(accepts(a, make_shared<BytesRef>()));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 1)));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 2)));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 3)));

  // '' (excl) - 00 (incl)
  a = makeBinaryInterval(make_shared<BytesRef>(), false,
                         make_shared<BytesRef>(zeros, 0, 2), true);
  assertTrue(Operations::isFinite(a));
  assertFalse(accepts(a, make_shared<BytesRef>()));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 1)));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 2)));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 3)));

  // 0 (excl) - 00 (incl)
  a = makeBinaryInterval(make_shared<BytesRef>(zeros, 0, 1), false,
                         make_shared<BytesRef>(zeros, 0, 2), true);
  assertTrue(Operations::isFinite(a));
  assertFalse(accepts(a, make_shared<BytesRef>()));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 1)));
  assertTrue(accepts(a, make_shared<BytesRef>(zeros, 0, 2)));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 3)));

  // 0 (excl) - 00 (excl)
  a = makeBinaryInterval(make_shared<BytesRef>(zeros, 0, 1), false,
                         make_shared<BytesRef>(zeros, 0, 2), false);
  assertTrue(Operations::isFinite(a));
  assertFalse(accepts(a, make_shared<BytesRef>()));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 1)));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 2)));
  assertFalse(accepts(a, make_shared<BytesRef>(zeros, 0, 3)));
}

void TestAutomaton::testMakeBinaryIntervalFiniteCasesRandom() throw(
    runtime_error)
{
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<BytesRef> prefix =
        make_shared<BytesRef>(TestUtil::randomRealisticUnicodeString(random()));

    shared_ptr<BytesRefBuilder> b = make_shared<BytesRefBuilder>();
    b->append(prefix);
    int numZeros = random()->nextInt(10);
    for (int i = 0; i < numZeros; i++) {
      b->append(static_cast<char>(0));
    }
    shared_ptr<BytesRef> minTerm = b->get();

    b = make_shared<BytesRefBuilder>();
    b->append(minTerm);
    numZeros = random()->nextInt(10);
    for (int i = 0; i < numZeros; i++) {
      b->append(static_cast<char>(0));
    }
    shared_ptr<BytesRef> maxTerm = b->get();

    bool minInclusive = random()->nextBoolean();
    bool maxInclusive = random()->nextBoolean();
    shared_ptr<Automaton> a =
        makeBinaryInterval(minTerm, minInclusive, maxTerm, maxInclusive);
    assertTrue(Operations::isFinite(a));
    int expectedCount = maxTerm->length - minTerm->length + 1;
    if (minInclusive == false) {
      expectedCount--;
    }
    if (maxInclusive == false) {
      expectedCount--;
    }

    if (expectedCount <= 0) {
      assertTrue(Operations::isEmpty(a));
      continue;
    } else {
      // Enumerate all finite strings and verify the count matches what we
      // expect:
      TestUtil::assertEquals(
          expectedCount,
          TestOperations::getFiniteStrings(a, expectedCount)->size());
    }

    b = make_shared<BytesRefBuilder>();
    b->append(minTerm);
    if (minInclusive == false) {
      assertFalse(accepts(a, b->get()));
      b->append(static_cast<char>(0));
    }
    while (b->length() < maxTerm->length) {
      b->append(static_cast<char>(0));

      bool expected;
      if (b->length() == maxTerm->length) {
        expected = maxInclusive;
      } else {
        expected = true;
      }
      TestUtil::assertEquals(expected, accepts(a, b->get()));
    }
  }
}

void TestAutomaton::testMakeBinaryIntervalRandom() 
{
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<BytesRef> minTerm = TestUtil::randomBinaryTerm(random());
    bool minInclusive = random()->nextBoolean();
    shared_ptr<BytesRef> maxTerm = TestUtil::randomBinaryTerm(random());
    bool maxInclusive = random()->nextBoolean();

    shared_ptr<Automaton> a =
        makeBinaryInterval(minTerm, minInclusive, maxTerm, maxInclusive);

    for (int iter2 = 0; iter2 < 500; iter2++) {
      shared_ptr<BytesRef> term = TestUtil::randomBinaryTerm(random());
      int minCmp = minTerm->compareTo(term);
      int maxCmp = maxTerm->compareTo(term);

      bool expected;
      if (minCmp > 0 || maxCmp < 0) {
        expected = false;
      } else if (minCmp == 0 && maxCmp == 0) {
        expected = minInclusive && maxInclusive;
      } else if (minCmp == 0) {
        expected = minInclusive;
      } else if (maxCmp == 0) {
        expected = maxInclusive;
      } else {
        expected = true;
      }

      if (VERBOSE) {
        wcout << L"  check term=" << term << L" expected=" << expected << endl;
      }
      shared_ptr<IntsRefBuilder> intsBuilder = make_shared<IntsRefBuilder>();
      Util::toIntsRef(term, intsBuilder);
      TestUtil::assertEquals(expected,
                             Operations::run(a, intsBuilder->toIntsRef()));
    }
  }
}

shared_ptr<IntsRef> TestAutomaton::intsRef(const wstring &s)
{
  shared_ptr<IntsRefBuilder> intsBuilder = make_shared<IntsRefBuilder>();
  Util::toIntsRef(make_shared<BytesRef>(s), intsBuilder);
  return intsBuilder->toIntsRef();
}

void TestAutomaton::testMakeBinaryIntervalBasic() 
{
  shared_ptr<Automaton> a = Automata::makeBinaryInterval(
      make_shared<BytesRef>(L"bar"), true, make_shared<BytesRef>(L"foo"), true);
  assertTrue(Operations::run(a, intsRef(L"bar")));
  assertTrue(Operations::run(a, intsRef(L"foo")));
  assertTrue(Operations::run(a, intsRef(L"beep")));
  assertFalse(Operations::run(a, intsRef(L"baq")));
  assertTrue(Operations::run(a, intsRef(L"bara")));
}

void TestAutomaton::testMakeBinaryIntervalEqual() 
{
  shared_ptr<Automaton> a = Automata::makeBinaryInterval(
      make_shared<BytesRef>(L"bar"), true, make_shared<BytesRef>(L"bar"), true);
  assertTrue(Operations::run(a, intsRef(L"bar")));
  assertTrue(Operations::isFinite(a));
  TestUtil::assertEquals(1, TestOperations::getFiniteStrings(a)->size());
}

void TestAutomaton::testMakeBinaryIntervalCommonPrefix() 
{
  shared_ptr<Automaton> a =
      Automata::makeBinaryInterval(make_shared<BytesRef>(L"bar"), true,
                                   make_shared<BytesRef>(L"barfoo"), true);
  assertFalse(Operations::run(a, intsRef(L"bam")));
  assertTrue(Operations::run(a, intsRef(L"bar")));
  assertTrue(Operations::run(a, intsRef(L"bara")));
  assertTrue(Operations::run(a, intsRef(L"barf")));
  assertTrue(Operations::run(a, intsRef(L"barfo")));
  assertTrue(Operations::run(a, intsRef(L"barfoo")));
  assertTrue(Operations::run(a, intsRef(L"barfonz")));
  assertFalse(Operations::run(a, intsRef(L"barfop")));
  assertFalse(Operations::run(a, intsRef(L"barfoop")));
}

void TestAutomaton::testMakeBinaryIntervalOpenMax() 
{
  shared_ptr<Automaton> a = Automata::makeBinaryInterval(
      make_shared<BytesRef>(L"bar"), true, nullptr, true);
  assertFalse(Operations::run(a, intsRef(L"bam")));
  assertTrue(Operations::run(a, intsRef(L"bar")));
  assertTrue(Operations::run(a, intsRef(L"bara")));
  assertTrue(Operations::run(a, intsRef(L"barf")));
  assertTrue(Operations::run(a, intsRef(L"barfo")));
  assertTrue(Operations::run(a, intsRef(L"barfoo")));
  assertTrue(Operations::run(a, intsRef(L"barfonz")));
  assertTrue(Operations::run(a, intsRef(L"barfop")));
  assertTrue(Operations::run(a, intsRef(L"barfoop")));
  assertTrue(Operations::run(a, intsRef(L"zzz")));
}

void TestAutomaton::testMakeBinaryIntervalOpenMin() 
{
  shared_ptr<Automaton> a = Automata::makeBinaryInterval(
      nullptr, true, make_shared<BytesRef>(L"foo"), true);
  assertFalse(Operations::run(a, intsRef(L"foz")));
  assertFalse(Operations::run(a, intsRef(L"zzz")));
  assertTrue(Operations::run(a, intsRef(L"foo")));
  assertTrue(Operations::run(a, intsRef(L"")));
  assertTrue(Operations::run(a, intsRef(L"a")));
  assertTrue(Operations::run(a, intsRef(L"aaa")));
  assertTrue(Operations::run(a, intsRef(L"bz")));
}

void TestAutomaton::testMakeBinaryIntervalOpenBoth() 
{
  shared_ptr<Automaton> a =
      Automata::makeBinaryInterval(nullptr, true, nullptr, true);
  assertTrue(Operations::run(a, intsRef(L"foz")));
  assertTrue(Operations::run(a, intsRef(L"zzz")));
  assertTrue(Operations::run(a, intsRef(L"foo")));
  assertTrue(Operations::run(a, intsRef(L"")));
  assertTrue(Operations::run(a, intsRef(L"a")));
  assertTrue(Operations::run(a, intsRef(L"aaa")));
  assertTrue(Operations::run(a, intsRef(L"bz")));
}

void TestAutomaton::testAcceptAllEmptyStringMin() 
{
  shared_ptr<Automaton> a = Automata::makeBinaryInterval(
      make_shared<BytesRef>(), true, nullptr, true);
  assertTrue(Operations::sameLanguage(Automata::makeAnyBinary(), a));
}

shared_ptr<IntsRef> TestAutomaton::toIntsRef(const wstring &s)
{
  shared_ptr<IntsRefBuilder> b = make_shared<IntsRefBuilder>();
  for (int i = 0, cp = 0; i < s.length(); i += Character::charCount(cp)) {
    cp = s.codePointAt(i);
    b->append(cp);
  }

  return b->get();
}

void TestAutomaton::testGetSingleton()
{
  int iters = atLeast(10000);
  for (int iter = 0; iter < iters; iter++) {
    wstring s = TestUtil::randomRealisticUnicodeString(random());
    shared_ptr<Automaton> a = Automata::makeString(s);
    TestUtil::assertEquals(toIntsRef(s), Operations::getSingleton(a));
  }
}

void TestAutomaton::testGetSingletonEmptyString()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int s = a->createState();
  a->setAccept(s, true);
  a->finishState();
  TestUtil::assertEquals(make_shared<IntsRef>(), Operations::getSingleton(a));
}

void TestAutomaton::testGetSingletonNothing()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->createState();
  a->finishState();
  assertNull(Operations::getSingleton(a));
}

void TestAutomaton::testGetSingletonTwo()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int s = a->createState();
  int x = a->createState();
  a->setAccept(x, true);
  a->addTransition(s, x, 55);
  int y = a->createState();
  a->setAccept(y, true);
  a->addTransition(s, y, 58);
  a->finishState();
  assertNull(Operations::getSingleton(a));
}
} // namespace org::apache::lucene::util::automaton