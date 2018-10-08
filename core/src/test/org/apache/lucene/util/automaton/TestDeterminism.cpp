using namespace std;

#include "TestDeterminism.h"

namespace org::apache::lucene::util::automaton
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestDeterminism::testRegexps() 
{
  int num = atLeast(500);
  for (int i = 0; i < num; i++) {
    assertAutomaton(
        (make_shared<RegExp>(AutomatonTestUtil::randomRegexp(random()),
                             RegExp::NONE))
            ->toAutomaton());
  }
}

void TestDeterminism::testAgainstSimple() 
{
  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    a = AutomatonTestUtil::determinizeSimple(a);
    shared_ptr<Automaton> b =
        Operations::determinize(a, numeric_limits<int>::max());
    // TODO: more verifications possible?
    assertTrue(Operations::sameLanguage(a, b));
  }
}

void TestDeterminism::assertAutomaton(shared_ptr<Automaton> a)
{
  a = Operations::determinize(Operations::removeDeadStates(a),
                              DEFAULT_MAX_DETERMINIZED_STATES);

  // complement(complement(a)) = a
  shared_ptr<Automaton> equivalent = Operations::complement(
      Operations::complement(a, DEFAULT_MAX_DETERMINIZED_STATES),
      DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::sameLanguage(a, equivalent));

  // a union a = a
  equivalent = Operations::determinize(
      Operations::removeDeadStates(Operations::union_(a, a)),
      DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::sameLanguage(a, equivalent));

  // a intersect a = a
  equivalent = Operations::determinize(
      Operations::removeDeadStates(Operations::intersection(a, a)),
      DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::sameLanguage(a, equivalent));

  // a minus a = empty
  shared_ptr<Automaton> empty =
      Operations::minus(a, a, DEFAULT_MAX_DETERMINIZED_STATES);
  assertTrue(Operations::isEmpty(empty));

  // as long as don't accept the empty string
  // then optional(a) - empty = a
  if (!Operations::run(a, L"")) {
    // System.out.println("test " + a);
    shared_ptr<Automaton> optional = Operations::optional(a);
    // System.out.println("optional " + optional);
    equivalent = Operations::minus(optional, Automata::makeEmptyString(),
                                   DEFAULT_MAX_DETERMINIZED_STATES);
    // System.out.println("equiv " + equivalent);
    assertTrue(Operations::sameLanguage(a, equivalent));
  }
}
} // namespace org::apache::lucene::util::automaton