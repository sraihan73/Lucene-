using namespace std;

#include "TestJUnitRuleOrder.h"

namespace org::apache::lucene::util
{
using org::junit::After;
using org::junit::AfterClass;
using org::junit::Assert;
using org::junit::Before;
using org::junit::BeforeClass;
using org::junit::Rule;
using org::junit::Test;
using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runner::JUnitCore;
using org::junit::runners::model::Statement;
stack<wstring> TestJUnitRuleOrder::stack;

TestJUnitRuleOrder::TestJUnitRuleOrder() : WithNestedTests(true) {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before()
void TestJUnitRuleOrder::Nested::before() { stack.push(L"@Before"); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after()
void TestJUnitRuleOrder::Nested::after() { stack.push(L"@After"); }

TestJUnitRuleOrder::Nested::TestRuleAnonymousInnerClass::
    TestRuleAnonymousInnerClass()
{
}

shared_ptr<Statement>
TestJUnitRuleOrder::Nested::TestRuleAnonymousInnerClass::apply(
    shared_ptr<Statement> base, shared_ptr<Description> description)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), base);
}

TestJUnitRuleOrder::Nested::TestRuleAnonymousInnerClass::
    StatementAnonymousInnerClass::StatementAnonymousInnerClass(
        shared_ptr<TestRuleAnonymousInnerClass> outerInstance,
        shared_ptr<Statement> base)
{
  this->outerInstance = outerInstance;
  this->base = base;
}

void TestJUnitRuleOrder::Nested::TestRuleAnonymousInnerClass::
    StatementAnonymousInnerClass::evaluate() 
{
  stack::push(L"@Rule before");
  base->evaluate();
  stack::push(L"@Rule after");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void test()
void TestJUnitRuleOrder::Nested::test() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClassCleanup()
void TestJUnitRuleOrder::Nested::beforeClassCleanup()
{
  stack = stack<wstring>();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassCheck()
void TestJUnitRuleOrder::Nested::afterClassCheck()
{
  stack.push(L"@AfterClass");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRuleOrder()
void TestJUnitRuleOrder::testRuleOrder()
{
  JUnitCore::runClasses(Nested::typeid);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  Assert::assertEquals(
      Arrays->toString(stack.toArray()),
      L"[@Rule before, @Before, @After, @Rule after, @AfterClass]");
}
} // namespace org::apache::lucene::util