using namespace std;

#include "LimitedFiniteStringsIteratorTest.h"

namespace org::apache::lucene::util::automaton
{
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.util.automaton.FiniteStringsIteratorTest.getFiniteStrings;

void LimitedFiniteStringsIteratorTest::testRandomFiniteStrings()
{
  // Just makes sure we can run on any random finite
  // automaton:
  int iters = atLeast(100);
  for (int i = 0; i < iters; i++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    try {
      // Must pass a limit because the random automaton
      // can accept MANY strings:
      getFiniteStrings(make_shared<LimitedFiniteStringsIterator>(
          a, TestUtil::nextInt(random(), 1, 1000)));
      // NOTE: cannot do this, because the method is not
      // guaranteed to detect cycles when you have a limit
      // assertTrue(Operations.isFinite(a));
    } catch (const invalid_argument &iae) {
      assertFalse(Operations::isFinite(a));
    }
  }
}

void LimitedFiniteStringsIteratorTest::testInvalidLimitNegative()
{
  shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<LimitedFiniteStringsIterator>(a, -7);
    fail(L"did not hit exception");
  });
}

void LimitedFiniteStringsIteratorTest::testInvalidLimitNull()
{
  shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<LimitedFiniteStringsIterator>(a, 0); });
}

void LimitedFiniteStringsIteratorTest::testSingleton()
{
  shared_ptr<Automaton> a = Automata::makeString(L"foobar");
  deque<std::shared_ptr<IntsRef>> actual =
      getFiniteStrings(make_shared<LimitedFiniteStringsIterator>(a, 1));
  TestUtil::assertEquals(1, actual.size());
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  Util::toUTF32((wstring(L"foobar")).toCharArray(), 0, 6, scratch);
      assertTrue(find(actual.begin(), actual.end(), scratch->get()) != actual.end()));
}

void LimitedFiniteStringsIteratorTest::testLimit()
{
  shared_ptr<Automaton> a = Operations::union_(Automata::makeString(L"foo"),
                                               Automata::makeString(L"bar"));

  // Test without limit
  shared_ptr<FiniteStringsIterator> withoutLimit =
      make_shared<LimitedFiniteStringsIterator>(a, -1);
  TestUtil::assertEquals(2, getFiniteStrings(withoutLimit)->size());

  // Test with limit
  shared_ptr<FiniteStringsIterator> withLimit =
      make_shared<LimitedFiniteStringsIterator>(a, 1);
  TestUtil::assertEquals(1, getFiniteStrings(withLimit)->size());
}

void LimitedFiniteStringsIteratorTest::testSize()
{
  shared_ptr<Automaton> a = Operations::union_(Automata::makeString(L"foo"),
                                               Automata::makeString(L"bar"));
  shared_ptr<LimitedFiniteStringsIterator> iterator =
      make_shared<LimitedFiniteStringsIterator>(a, -1);
  deque<std::shared_ptr<IntsRef>> actual = getFiniteStrings(iterator);
  TestUtil::assertEquals(2, actual.size());
  TestUtil::assertEquals(2, iterator->size());
}
} // namespace org::apache::lucene::util::automaton