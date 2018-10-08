using namespace std;

#include "TestRuleSetupTeardownChained.h"

namespace org::apache::lucene::util
{
using org::junit::Assert;
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

shared_ptr<Statement>
TestRuleSetupTeardownChained::apply(shared_ptr<Statement> base,
                                    shared_ptr<Description> description)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), base);
}

TestRuleSetupTeardownChained::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<TestRuleSetupTeardownChained> outerInstance,
        shared_ptr<Statement> base)
{
  this->outerInstance = outerInstance;
  this->base = base;
}

void TestRuleSetupTeardownChained::StatementAnonymousInnerClass::
    evaluate() 
{
  outerInstance->setupCalled = false;
  outerInstance->teardownCalled = false;
  base->evaluate();

  // I assume we don't want to check teardown chaining if something happens in
  // the test because this would obscure the original exception?
  if (!outerInstance->setupCalled) {
    Assert::fail(L"One of the overrides of setUp does not propagate the call.");
  }
  if (!outerInstance->teardownCalled) {
    Assert::fail(
        L"One of the overrides of tearDown does not propagate the call.");
  }
}
} // namespace org::apache::lucene::util