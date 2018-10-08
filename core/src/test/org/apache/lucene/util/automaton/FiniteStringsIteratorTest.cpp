using namespace std;

#include "FiniteStringsIteratorTest.h"

namespace org::apache::lucene::util::automaton
{
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void FiniteStringsIteratorTest::testRandomFiniteStrings1()
{
  int numStrings = atLeast(100);
  if (VERBOSE) {
    wcout << L"TEST: numStrings=" << numStrings << endl;
  }

  shared_ptr<Set<std::shared_ptr<IntsRef>>> strings =
      unordered_set<std::shared_ptr<IntsRef>>();
  deque<std::shared_ptr<Automaton>> automata =
      deque<std::shared_ptr<Automaton>>();
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  for (int i = 0; i < numStrings; i++) {
    wstring s = TestUtil::randomSimpleString(random(), 1, 200);
    Util::toUTF32(s.toCharArray(), 0, s.length(), scratch);
    if (strings->add(scratch->toIntsRef())) {
      automata.push_back(Automata::makeString(s));
      if (VERBOSE) {
        wcout << L"  add string=" << s << endl;
      }
    }
  }

  // TODO: we could sometimes use
  // DaciukMihovAutomatonBuilder here

  // TODO: what other random things can we do here...
  shared_ptr<Automaton> a = Operations::union_(automata);
  if (random()->nextBoolean()) {
    a = MinimizationOperations::minimize(a, 1000000);
    if (VERBOSE) {
      wcout << L"TEST: a.minimize numStates=" << a->getNumStates() << endl;
    }
  } else if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"TEST: a.determinize" << endl;
    }
    a = Operations::determinize(a, 1000000);
  } else if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"TEST: a.removeDeadStates" << endl;
    }
    a = Operations::removeDeadStates(a);
  }

  shared_ptr<FiniteStringsIterator> iterator =
      make_shared<FiniteStringsIterator>(a);
  deque<std::shared_ptr<IntsRef>> actual = getFiniteStrings(iterator);
  assertFiniteStringsRecursive(a, actual);

  if (!strings->equals(unordered_set<>(actual))) {
    wcout << L"strings.size()=" << strings->size() << L" actual.size="
          << actual.size() << endl;
    deque<std::shared_ptr<IntsRef>> x =
        deque<std::shared_ptr<IntsRef>>(strings);
    sort(x.begin(), x.end());
    deque<std::shared_ptr<IntsRef>> y =
        deque<std::shared_ptr<IntsRef>>(actual);
    sort(y.begin(), y.end());
    int end = min(x.size(), y.size());
    for (int i = 0; i < end; i++) {
      wcout << L"  i=" << i << L" string=" << toString(x[i]) << L" actual="
            << toString(y[i]) << endl;
    }
    fail(L"wrong strings found");
  }
}

void FiniteStringsIteratorTest::testFiniteStringsBasic()
{
  shared_ptr<Automaton> a = Operations::union_(Automata::makeString(L"dog"),
                                               Automata::makeString(L"duck"));
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  shared_ptr<FiniteStringsIterator> iterator =
      make_shared<FiniteStringsIterator>(a);
  deque<std::shared_ptr<IntsRef>> actual = getFiniteStrings(iterator);
  assertFiniteStringsRecursive(a, actual);
  TestUtil::assertEquals(2, actual.size());
  shared_ptr<IntsRefBuilder> dog = make_shared<IntsRefBuilder>();
  Util::toIntsRef(make_shared<BytesRef>(L"dog"), dog);
      assertTrue(find(actual.begin(), actual.end(), dog->get()) != actual.end()));
      shared_ptr<IntsRefBuilder> duck = make_shared<IntsRefBuilder>();
      Util::toIntsRef(make_shared<BytesRef>(L"duck"), duck);
      assertTrue(find(actual.begin(), actual.end(), duck->get()) != actual.end()));
}

void FiniteStringsIteratorTest::testFiniteStringsEatsStack()
{
  std::deque<wchar_t> chars(50000);
  TestUtil::randomFixedLengthUnicodeString(random(), chars, 0, chars.size());
  wstring bigString1 = wstring(chars);
  TestUtil::randomFixedLengthUnicodeString(random(), chars, 0, chars.size());
  wstring bigString2 = wstring(chars);
  shared_ptr<Automaton> a = Operations::union_(
      Automata::makeString(bigString1), Automata::makeString(bigString2));
  shared_ptr<FiniteStringsIterator> iterator =
      make_shared<FiniteStringsIterator>(a);
  deque<std::shared_ptr<IntsRef>> actual = getFiniteStrings(iterator);
  TestUtil::assertEquals(2, actual.size());
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  Util::toUTF32(bigString1.toCharArray(), 0, bigString1.length(), scratch);
      assertTrue(find(actual.begin(), actual.end(), scratch->get()) != actual.end()));
      Util::toUTF32(bigString2.toCharArray(), 0, bigString2.length(), scratch);
      assertTrue(find(actual.begin(), actual.end(), scratch->get()) != actual.end()));
}

void FiniteStringsIteratorTest::testWithCycle() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<Automaton> a =
        (make_shared<RegExp>(L"abc.*", RegExp::NONE))->toAutomaton();
    shared_ptr<FiniteStringsIterator> iterator =
        make_shared<FiniteStringsIterator>(a);
    getFiniteStrings(iterator);
  });
}

void FiniteStringsIteratorTest::testSingletonNoLimit()
{
  shared_ptr<Automaton> a = Automata::makeString(L"foobar");
  shared_ptr<FiniteStringsIterator> iterator =
      make_shared<FiniteStringsIterator>(a);
  deque<std::shared_ptr<IntsRef>> actual = getFiniteStrings(iterator);
  TestUtil::assertEquals(1, actual.size());
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  Util::toUTF32((wstring(L"foobar")).toCharArray(), 0, 6, scratch);
      assertTrue(find(actual.begin(), actual.end(), scratch->get()) != actual.end()));
}

void FiniteStringsIteratorTest::testShortAccept()
{
  shared_ptr<Automaton> a = Operations::union_(Automata::makeString(L"x"),
                                               Automata::makeString(L"xy"));
  a = MinimizationOperations::minimize(a, DEFAULT_MAX_DETERMINIZED_STATES);
  shared_ptr<FiniteStringsIterator> iterator =
      make_shared<FiniteStringsIterator>(a);
  deque<std::shared_ptr<IntsRef>> actual = getFiniteStrings(iterator);
  TestUtil::assertEquals(2, actual.size());
  shared_ptr<IntsRefBuilder> x = make_shared<IntsRefBuilder>();
  Util::toIntsRef(make_shared<BytesRef>(L"x"), x);
      assertTrue(find(actual.begin(), actual.end(), x->get()) != actual.end()));
      shared_ptr<IntsRefBuilder> xy = make_shared<IntsRefBuilder>();
      Util::toIntsRef(make_shared<BytesRef>(L"xy"), xy);
      assertTrue(find(actual.begin(), actual.end(), xy->get()) != actual.end()));
}

void FiniteStringsIteratorTest::testSingleString()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int start = a->createState();
  int end = a->createState();
  a->setAccept(end, true);
  a->addTransition(start, end, L'a', L'a');
  a->finishState();
  shared_ptr<Set<std::shared_ptr<IntsRef>>> accepted =
      TestOperations::getFiniteStrings(a);
  TestUtil::assertEquals(1, accepted->size());
  shared_ptr<IntsRefBuilder> intsRef = make_shared<IntsRefBuilder>();
  intsRef->append(L'a');
  assertTrue(accepted->contains(intsRef->toIntsRef()));
}

deque<std::shared_ptr<IntsRef>> FiniteStringsIteratorTest::getFiniteStrings(
    shared_ptr<FiniteStringsIterator> iterator)
{
  deque<std::shared_ptr<IntsRef>> result = deque<std::shared_ptr<IntsRef>>();
  for (IntsRef finiteString; (finiteString = iterator->next()) != nullptr;) {
    result.push_back(IntsRef::deepCopyOf(finiteString));
  }

  return result;
}

void FiniteStringsIteratorTest::assertFiniteStringsRecursive(
    shared_ptr<Automaton> automaton, deque<std::shared_ptr<IntsRef>> &actual)
{
  shared_ptr<Set<std::shared_ptr<IntsRef>>> expected =
      AutomatonTestUtil::getFiniteStringsRecursive(automaton, -1);
  // Check that no string is emitted twice.
  TestUtil::assertEquals(expected->size(), actual.size());
  TestUtil::assertEquals(expected, unordered_set<>(actual));
}

wstring FiniteStringsIteratorTest::toString(shared_ptr<IntsRef> ints)
{
  shared_ptr<BytesRef> br = make_shared<BytesRef>(ints->length);
  for (int i = 0; i < ints->length; i++) {
    br->bytes[i] = static_cast<char>(ints->ints[i]);
  }
  br->length = ints->length;
  return br->utf8ToString();
}
} // namespace org::apache::lucene::util::automaton