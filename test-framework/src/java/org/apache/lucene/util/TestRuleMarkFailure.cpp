using namespace std;

#include "TestRuleMarkFailure.h"

namespace org::apache::lucene::util
{
using org::junit::internal_::AssumptionViolatedException;
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

TestRuleMarkFailure::TestRuleMarkFailure(deque<TestRuleMarkFailure> &chained)
    : chained(chained)
{
}

shared_ptr<Statement> TestRuleMarkFailure::apply(shared_ptr<Statement> s,
                                                 shared_ptr<Description> d)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), s);
}

TestRuleMarkFailure::StatementAnonymousInnerClass::StatementAnonymousInnerClass(
    shared_ptr<TestRuleMarkFailure> outerInstance, shared_ptr<Statement> s)
{
  this->outerInstance = outerInstance;
  this->s = s;
}

void TestRuleMarkFailure::StatementAnonymousInnerClass::evaluate() throw(
    runtime_error)
{
  // Clear status at start.
  outerInstance->failures = false;

  try {
    s->evaluate();
  } catch (const runtime_error &t) {
    if (!isAssumption(t)) {
      outerInstance->markFailed();
    }
    throw t;
  }
}

bool TestRuleMarkFailure::isAssumption(runtime_error t)
{
  for (auto t2 : expandFromMultiple(t)) {
    if (!(std::dynamic_pointer_cast<AssumptionViolatedException>(t2) !=
          nullptr)) {
      return false;
    }
  }
  return true;
}

deque<runtime_error> TestRuleMarkFailure::expandFromMultiple(runtime_error t)
{
  return expandFromMultiple(t, deque<runtime_error>());
}

deque<runtime_error>
TestRuleMarkFailure::expandFromMultiple(runtime_error t,
                                        deque<runtime_error> &deque)
{
  if (std::dynamic_pointer_cast<
          org::junit::runners::model::MultipleFailureException>(t) != nullptr) {
    for (runtime_error sub :
         (std::static_pointer_cast<
              org::junit::runners::model::MultipleFailureException>(t))
             ->getFailures()) {
      expandFromMultiple(sub, deque);
    }
  } else {
    deque.push_back(t);
  }

  return deque;
}

void TestRuleMarkFailure::markFailed()
{
  failures = true;
  for (auto next : chained) {
    next->markFailed();
  }
}

bool TestRuleMarkFailure::hadFailures() { return failures; }

bool TestRuleMarkFailure::wasSuccessful() { return !hadFailures(); }
} // namespace org::apache::lucene::util