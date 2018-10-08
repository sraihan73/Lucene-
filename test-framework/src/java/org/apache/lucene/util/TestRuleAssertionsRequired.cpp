using namespace std;

#include "TestRuleAssertionsRequired.h"

namespace org::apache::lucene::util
{
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

shared_ptr<Statement>
TestRuleAssertionsRequired::apply(shared_ptr<Statement> base,
                                  shared_ptr<Description> description)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), base);
}

TestRuleAssertionsRequired::StatementAnonymousInnerClass::
    StatementAnonymousInnerClass(
        shared_ptr<TestRuleAssertionsRequired> outerInstance,
        shared_ptr<Statement> base)
{
  this->outerInstance = outerInstance;
  this->base = base;
}

void TestRuleAssertionsRequired::StatementAnonymousInnerClass::evaluate() throw(
    runtime_error)
{
  try {
    // Make sure -ea matches -Dtests.asserts, to catch accidental mis-use:
    if (LuceneTestCase::assertsAreEnabled !=
        LuceneTestCase::TEST_ASSERTS_ENABLED) {
      wstring msg = L"Assertions mismatch: ";
      if (LuceneTestCase::assertsAreEnabled) {
        msg += L"-ea was specified";
      } else {
        msg += L"-ea was not specified";
      }
      if (LuceneTestCase::TEST_ASSERTS_ENABLED) {
        msg += L" but -Dtests.asserts=true";
      } else {
        msg += L" but -Dtests.asserts=false";
      }
      System::err::println(msg);
      throw runtime_error(msg);
    }
  } catch (const AssertionError &e) {
    // Ok, enabled.
  }

  base->evaluate();
}
} // namespace org::apache::lucene::util