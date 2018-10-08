using namespace std;

#include "TestCompiledAutomaton.h"

namespace org::apache::lucene::util::automaton
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<CompiledAutomaton>
TestCompiledAutomaton::build(int maxDeterminizedStates,
                             deque<wstring> &strings)
{
  const deque<std::shared_ptr<BytesRef>> terms =
      deque<std::shared_ptr<BytesRef>>();
  for (wstring s : strings) {
    terms.push_back(make_shared<BytesRef>(s));
  }
  sort(terms.begin(), terms.end());
  shared_ptr<Automaton> *const a = DaciukMihovAutomatonBuilder::build(terms);
  return make_shared<CompiledAutomaton>(a, true, false, maxDeterminizedStates,
                                        false);
}

void TestCompiledAutomaton::testFloor(shared_ptr<CompiledAutomaton> c,
                                      const wstring &input,
                                      const wstring &expected)
{
  shared_ptr<BytesRef> *const b = make_shared<BytesRef>(input);
  shared_ptr<BytesRef> *const result =
      c->floor(b, make_shared<BytesRefBuilder>());
  if (expected == L"") {
    assertNull(result);
  } else {
    assertNotNull(result);
    assertEquals(L"actual=" + result->utf8ToString() + L" vs expected=" +
                     expected + L" (input=" + input + L")",
                 result, make_shared<BytesRef>(expected));
  }
}

void TestCompiledAutomaton::testTerms(
    int maxDeterminizedStates, std::deque<wstring> &terms) 
{
  shared_ptr<CompiledAutomaton> *const c = build(maxDeterminizedStates, terms);
  std::deque<std::shared_ptr<BytesRef>> termBytes(terms.size());
  for (int idx = 0; idx < terms.size(); idx++) {
    termBytes[idx] = make_shared<BytesRef>(terms[idx]);
  }
  Arrays::sort(termBytes);

  if (VERBOSE) {
    wcout << L"\nTEST: terms in unicode order" << endl;
    for (auto t : termBytes) {
      wcout << L"  " << t->utf8ToString() << endl;
    }
    // System.out.println(c.utf8.toDot());
  }

  for (int iter = 0; iter < 100 * RANDOM_MULTIPLIER; iter++) {
    const wstring s = random()->nextInt(10) == 1
                          ? terms[random()->nextInt(terms.size())]
                          : randomString();
    if (VERBOSE) {
      wcout << L"\nTEST: floor(" << s << L")" << endl;
    }
    int loc = Arrays::binarySearch(termBytes, make_shared<BytesRef>(s));
    const wstring expected;
    if (loc >= 0) {
      expected = s;
    } else {
      // term doesn't exist
      loc = -(loc + 1);
      if (loc == 0) {
        expected = L"";
      } else {
        expected = termBytes[loc - 1]->utf8ToString();
      }
    }
    if (VERBOSE) {
      wcout << L"  expected=" << expected << endl;
    }
    testFloor(c, s, expected);
  }
}

void TestCompiledAutomaton::testRandom() 
{
  constexpr int numTerms = atLeast(400);
  shared_ptr<Set<wstring>> *const terms = unordered_set<wstring>();
  while (terms->size() != numTerms) {
    terms->add(randomString());
  }
  testTerms(numTerms * 100,
            terms->toArray(std::deque<wstring>(terms->size())));
}

wstring TestCompiledAutomaton::randomString()
{
  // return _TestUtil.randomSimpleString(random);
  return TestUtil::randomRealisticUnicodeString(random());
}

void TestCompiledAutomaton::testBasic() 
{
  shared_ptr<CompiledAutomaton> c = build(
      Operations::DEFAULT_MAX_DETERMINIZED_STATES, {L"fob", L"foo", L"goo"});
  testFloor(c, L"goo", L"goo");
  testFloor(c, L"ga", L"foo");
  testFloor(c, L"g", L"foo");
  testFloor(c, L"foc", L"fob");
  testFloor(c, L"foz", L"foo");
  testFloor(c, L"f", L"");
  testFloor(c, L"", L"");
  testFloor(c, L"aa", L"");
  testFloor(c, L"zzz", L"goo");
}

void TestCompiledAutomaton::testBinaryAll() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int state = a->createState();
  a->setAccept(state, true);
  a->addTransition(state, state, 0, 0xff);
  a->finishState();

  shared_ptr<CompiledAutomaton> ca = make_shared<CompiledAutomaton>(
      a, nullptr, true, numeric_limits<int>::max(), true);
  TestUtil::assertEquals(CompiledAutomaton::AUTOMATON_TYPE::ALL, ca->type);
}

void TestCompiledAutomaton::testUnicodeAll() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int state = a->createState();
  a->setAccept(state, true);
  a->addTransition(state, state, 0, Character::MAX_CODE_POINT);
  a->finishState();

  shared_ptr<CompiledAutomaton> ca = make_shared<CompiledAutomaton>(
      a, nullptr, true, numeric_limits<int>::max(), false);
  TestUtil::assertEquals(CompiledAutomaton::AUTOMATON_TYPE::ALL, ca->type);
}

void TestCompiledAutomaton::testBinarySingleton() 
{
  // This is just ascii so we can pretend it's binary:
  shared_ptr<Automaton> a = Automata::makeString(L"foobar");
  shared_ptr<CompiledAutomaton> ca = make_shared<CompiledAutomaton>(
      a, nullptr, true, numeric_limits<int>::max(), true);
  TestUtil::assertEquals(CompiledAutomaton::AUTOMATON_TYPE::SINGLE, ca->type);
}

void TestCompiledAutomaton::testUnicodeSingleton() 
{
  shared_ptr<Automaton> a =
      Automata::makeString(TestUtil::randomRealisticUnicodeString(random()));
  shared_ptr<CompiledAutomaton> ca = make_shared<CompiledAutomaton>(
      a, nullptr, true, numeric_limits<int>::max(), false);
  TestUtil::assertEquals(CompiledAutomaton::AUTOMATON_TYPE::SINGLE, ca->type);
}
} // namespace org::apache::lucene::util::automaton