using namespace std;

#include "TestRuleStoreClassName.h"

namespace org::apache::lucene::util
{
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

shared_ptr<Statement> TestRuleStoreClassName::apply(shared_ptr<Statement> s,
                                                    shared_ptr<Description> d)
{
  if (!d->isSuite()) {
    throw invalid_argument(L"This is a @ClassRule (applies to suites only).");
  }

  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), s, d);
}

TestRuleStoreClassName::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<TestRuleStoreClassName> outerInstance,
        shared_ptr<Statement> s, shared_ptr<Description> d)
{
  this->outerInstance = outerInstance;
  this->s = s;
  this->d = d;
}

void TestRuleStoreClassName::StatementAnonymousInnerClass::evaluate() throw(
    runtime_error)
{
  try {
    outerInstance->description = d;
    s->evaluate();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    outerInstance->description.reset();
  }
}

type_info TestRuleStoreClassName::getTestClass()
{
  shared_ptr<Description> localDescription = description;
  if (localDescription == nullptr) {
    throw runtime_error(L"The rule is not currently executing.");
  }
  return localDescription->getTestClass();
}
} // namespace org::apache::lucene::util