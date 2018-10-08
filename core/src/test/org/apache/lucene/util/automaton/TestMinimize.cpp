using namespace std;

#include "TestMinimize.h"

namespace org::apache::lucene::util::automaton
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMinimize::testBasic()
{
  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    shared_ptr<Automaton> la = Operations::determinize(
        Operations::removeDeadStates(a), numeric_limits<int>::max());
    shared_ptr<Automaton> lb =
        MinimizationOperations::minimize(a, numeric_limits<int>::max());
    assertTrue(Operations::sameLanguage(la, lb));
  }
}

void TestMinimize::testAgainstBrzozowski()
{
  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random());
    a = AutomatonTestUtil::minimizeSimple(a);
    shared_ptr<Automaton> b =
        MinimizationOperations::minimize(a, numeric_limits<int>::max());
    assertTrue(Operations::sameLanguage(a, b));
    assertEquals(a->getNumStates(), b->getNumStates());
    int numStates = a->getNumStates();

    int sum1 = 0;
    for (int s = 0; s < numStates; s++) {
      sum1 += a->getNumTransitions(s);
    }
    int sum2 = 0;
    for (int s = 0; s < numStates; s++) {
      sum2 += b->getNumTransitions(s);
    }

    assertEquals(sum1, sum2);
  }
}

void TestMinimize::testMinimizeHuge()
{
  (make_shared<RegExp>(L"+-*(A|.....|BC)*]", RegExp::NONE))
      ->toAutomaton(1000000);
}
} // namespace org::apache::lucene::util::automaton