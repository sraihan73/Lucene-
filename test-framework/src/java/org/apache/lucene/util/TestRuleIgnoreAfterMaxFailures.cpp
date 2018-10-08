using namespace std;

#include "TestRuleIgnoreAfterMaxFailures.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::RandomizedTest;
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using org::junit::Assert;
using org::junit::internal_::AssumptionViolatedException;
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

TestRuleIgnoreAfterMaxFailures::TestRuleIgnoreAfterMaxFailures(int maxFailures)
{
  Assert::assertTrue(L"maxFailures must be >= 1: " + to_wstring(maxFailures),
                     maxFailures >= 1);
  this->maxFailures = maxFailures;
}

shared_ptr<Statement>
TestRuleIgnoreAfterMaxFailures::apply(shared_ptr<Statement> s,
                                      shared_ptr<Description> d)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), s);
}

TestRuleIgnoreAfterMaxFailures::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<TestRuleIgnoreAfterMaxFailures> outerInstance,
        shared_ptr<Statement> s)
{
  this->outerInstance = outerInstance;
  this->s = s;
}

void TestRuleIgnoreAfterMaxFailures::StatementAnonymousInnerClass::
    evaluate() 
{
  int failuresSoFar = FailureMarker::getFailures();
  if (failuresSoFar >= outerInstance->maxFailures) {
    RandomizedTest::assumeTrue(
        L"Ignored, failures limit reached (" + to_wstring(failuresSoFar) +
            L" >= " + to_wstring(outerInstance->maxFailures) + L").",
        false);
  }

  s->evaluate();
}
} // namespace org::apache::lucene::util