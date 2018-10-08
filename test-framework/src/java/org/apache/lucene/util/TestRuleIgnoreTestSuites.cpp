using namespace std;

#include "TestRuleIgnoreTestSuites.h"

namespace org::apache::lucene::util
{
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;
const wstring TestRuleIgnoreTestSuites::PROPERTY_RUN_NESTED =
    L"tests.runnested";

shared_ptr<Statement> TestRuleIgnoreTestSuites::apply(shared_ptr<Statement> s,
                                                      shared_ptr<Description> d)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), s, d);
}

TestRuleIgnoreTestSuites::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<TestRuleIgnoreTestSuites> outerInstance,
        shared_ptr<Statement> s, shared_ptr<Description> d)
{
  this->outerInstance = outerInstance;
  this->s = s;
  this->d = d;
}

void TestRuleIgnoreTestSuites::StatementAnonymousInnerClass::evaluate() throw(
    runtime_error)
{
  if (NestedTestSuite::typeid->isAssignableFrom(d->getTestClass())) {
    LuceneTestCase::assumeTrue(
        L"Nested suite class ignored (started as stand-alone).",
        isRunningNested());
  }
  s->evaluate();
}

bool TestRuleIgnoreTestSuites::isRunningNested()
{
  return Boolean::getBoolean(PROPERTY_RUN_NESTED);
}
} // namespace org::apache::lucene::util