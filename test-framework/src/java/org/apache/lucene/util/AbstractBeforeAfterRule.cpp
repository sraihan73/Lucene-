using namespace std;

#include "AbstractBeforeAfterRule.h"

namespace org::apache::lucene::util
{
using org::junit::After;
using org::junit::AfterClass;
using org::junit::rules::RuleChain;
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::MultipleFailureException;
using org::junit::runners::model::Statement;

shared_ptr<Statement> AbstractBeforeAfterRule::apply(shared_ptr<Statement> s,
                                                     shared_ptr<Description> d)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), s);
}

AbstractBeforeAfterRule::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<AbstractBeforeAfterRule> outerInstance,
        shared_ptr<Statement> s)
{
  this->outerInstance = outerInstance;
  this->s = s;
}

void AbstractBeforeAfterRule::StatementAnonymousInnerClass::evaluate() throw(
    runtime_error)
{
  const deque<runtime_error> errors = deque<runtime_error>();

  try {
    outerInstance->before();
    s->evaluate();
  } catch (const runtime_error &t) {
    errors.push_back(t);
  }

  try {
    outerInstance->after();
  } catch (const runtime_error &t) {
    errors.push_back(t);
  }

  MultipleFailureException::assertEmpty(errors);
}

void AbstractBeforeAfterRule::before()  {}

void AbstractBeforeAfterRule::after()  {}
} // namespace org::apache::lucene::util