using namespace std;

#include "TestRuleThreadAndTestName.h"

namespace org::apache::lucene::util
{
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

shared_ptr<Statement>
TestRuleThreadAndTestName::apply(shared_ptr<Statement> base,
                                 shared_ptr<Description> description)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), base,
                                                   description);
}

TestRuleThreadAndTestName::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<TestRuleThreadAndTestName> outerInstance,
        shared_ptr<Statement> base, shared_ptr<Description> description)
{
  this->outerInstance = outerInstance;
  this->base = base;
  this->description = description;
}

void TestRuleThreadAndTestName::StatementAnonymousInnerClass::evaluate() throw(
    runtime_error)
{
  try {
    shared_ptr<Thread> current = Thread::currentThread();
    outerInstance->testCaseThread = current;
    outerInstance->testMethodName = description->getMethodName();

    base->evaluate();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    outerInstance->testCaseThread.reset();
    outerInstance->testMethodName = L"";
  }
}
} // namespace org::apache::lucene::util