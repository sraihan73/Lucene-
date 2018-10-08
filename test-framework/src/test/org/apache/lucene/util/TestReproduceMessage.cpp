using namespace std;

#include "TestReproduceMessage.h"

namespace org::apache::lucene::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
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
SorePoint TestReproduceMessage::where = (SorePoint)0;
SoreType TestReproduceMessage::type = (SoreType)0;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass()
void TestReproduceMessage::Nested::beforeClass()
{
  if (isRunningNested()) {
    triggerOn(SorePoint::BEFORE_CLASS);
  }
}

TestReproduceMessage::Nested::TestRuleAnonymousInnerClass::
    TestRuleAnonymousInnerClass()
{
}

shared_ptr<Statement>
TestReproduceMessage::Nested::TestRuleAnonymousInnerClass::apply(
    shared_ptr<Statement> base, shared_ptr<Description> description)
{
  return make_shared<StatementAnonymousInnerClass>(shared_from_this(), base);
}

TestReproduceMessage::Nested::TestRuleAnonymousInnerClass::
    StatementAnonymousInnerClass::StatementAnonymousInnerClass(
        shared_ptr<TestRuleAnonymousInnerClass> outerInstance,
        shared_ptr<Statement> base)
{
  this->outerInstance = outerInstance;
  this->base = base;
}

void TestReproduceMessage::Nested::TestRuleAnonymousInnerClass::
    StatementAnonymousInnerClass::evaluate() 
{
  triggerOn(SorePoint::RULE);
  base->evaluate();
}

TestReproduceMessage::Nested::Nested() { triggerOn(SorePoint::INITIALIZER); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before()
void TestReproduceMessage::Nested::before() { triggerOn(SorePoint::BEFORE); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void test()
void TestReproduceMessage::Nested::test() { triggerOn(SorePoint::TEST); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after()
void TestReproduceMessage::Nested::after() { triggerOn(SorePoint::AFTER); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestReproduceMessage::Nested::afterClass()
{
  if (isRunningNested()) {
    triggerOn(SorePoint::AFTER_CLASS);
  }
}

void TestReproduceMessage::Nested::triggerOn(SorePoint pt)
{
  if (pt == where) {
    switch (type) {
    case org::apache::lucene::util::SoreType::ASSUMPTION:
      LuceneTestCase::assumeTrue(pt.toString(), false);
      throw runtime_error(L"unreachable");
    case org::apache::lucene::util::SoreType::ERROR:
      throw runtime_error(pt.toString());
    case org::apache::lucene::util::SoreType::FAILURE:
      Assert::assertTrue(pt.toString(), false);
      throw runtime_error(L"unreachable");
    }
  }
}

TestReproduceMessage::TestReproduceMessage() : WithNestedTests(true) {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeBeforeClass() throws Exception
void TestReproduceMessage::testAssumeBeforeClass() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::BEFORE_CLASS;
  Assert::assertTrue(runAndReturnSyserr().isEmpty());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeInitializer() throws Exception
void TestReproduceMessage::testAssumeInitializer() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::INITIALIZER;
  Assert::assertTrue(runAndReturnSyserr().isEmpty());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeRule() throws Exception
void TestReproduceMessage::testAssumeRule() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::RULE;
  Assert::assertEquals(L"", runAndReturnSyserr());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeBefore() throws Exception
void TestReproduceMessage::testAssumeBefore() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::BEFORE;
  Assert::assertTrue(runAndReturnSyserr().isEmpty());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeTest() throws Exception
void TestReproduceMessage::testAssumeTest() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::TEST;
  Assert::assertTrue(runAndReturnSyserr().isEmpty());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeAfter() throws Exception
void TestReproduceMessage::testAssumeAfter() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::AFTER;
  Assert::assertTrue(runAndReturnSyserr().isEmpty());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAssumeAfterClass() throws Exception
void TestReproduceMessage::testAssumeAfterClass() 
{
  type = SoreType::ASSUMPTION;
  where = SorePoint::AFTER_CLASS;
  Assert::assertTrue(runAndReturnSyserr().isEmpty());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureBeforeClass() throws Exception
void TestReproduceMessage::testFailureBeforeClass() 
{
  type = SoreType::FAILURE;
  where = SorePoint::BEFORE_CLASS;
  Assert::assertTrue(runAndReturnSyserr().find(L"NOTE: reproduce with:") !=
                     wstring::npos);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureInitializer() throws Exception
void TestReproduceMessage::testFailureInitializer() 
{
  type = SoreType::FAILURE;
  where = SorePoint::INITIALIZER;
  Assert::assertTrue(runAndReturnSyserr().find(L"NOTE: reproduce with:") !=
                     wstring::npos);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureRule() throws Exception
void TestReproduceMessage::testFailureRule() 
{
  type = SoreType::FAILURE;
  where = SorePoint::RULE;

  const wstring syserr = runAndReturnSyserr();

  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureBefore() throws Exception
void TestReproduceMessage::testFailureBefore() 
{
  type = SoreType::FAILURE;
  where = SorePoint::BEFORE;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureTest() throws Exception
void TestReproduceMessage::testFailureTest() 
{
  type = SoreType::FAILURE;
  where = SorePoint::TEST;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureAfter() throws Exception
void TestReproduceMessage::testFailureAfter() 
{
  type = SoreType::FAILURE;
  where = SorePoint::AFTER;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureAfterClass() throws Exception
void TestReproduceMessage::testFailureAfterClass() 
{
  type = SoreType::FAILURE;
  where = SorePoint::AFTER_CLASS;
  Assert::assertTrue(runAndReturnSyserr().find(L"NOTE: reproduce with:") !=
                     wstring::npos);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorBeforeClass() throws Exception
void TestReproduceMessage::testErrorBeforeClass() 
{
  type = SoreType::ERROR;
  where = SorePoint::BEFORE_CLASS;
  Assert::assertTrue(runAndReturnSyserr().find(L"NOTE: reproduce with:") !=
                     wstring::npos);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorInitializer() throws Exception
void TestReproduceMessage::testErrorInitializer() 
{
  type = SoreType::ERROR;
  where = SorePoint::INITIALIZER;
  Assert::assertTrue(runAndReturnSyserr().find(L"NOTE: reproduce with:") !=
                     wstring::npos);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorRule() throws Exception
void TestReproduceMessage::testErrorRule() 
{
  type = SoreType::ERROR;
  where = SorePoint::RULE;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorBefore() throws Exception
void TestReproduceMessage::testErrorBefore() 
{
  type = SoreType::ERROR;
  where = SorePoint::BEFORE;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorTest() throws Exception
void TestReproduceMessage::testErrorTest() 
{
  type = SoreType::ERROR;
  where = SorePoint::TEST;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorAfter() throws Exception
void TestReproduceMessage::testErrorAfter() 
{
  type = SoreType::ERROR;
  where = SorePoint::AFTER;
  const wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L"NOTE: reproduce with:") != wstring::npos);
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))->contains(L"-Dtests.method=test"));
  Assert::assertTrue(
      Arrays::asList(syserr.split(L"\\s"))
          ->contains(L"-Dtestcase=" + Nested::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testErrorAfterClass() throws Exception
void TestReproduceMessage::testErrorAfterClass() 
{
  type = SoreType::ERROR;
  where = SorePoint::AFTER_CLASS;
  Assert::assertTrue(runAndReturnSyserr().find(L"NOTE: reproduce with:") !=
                     wstring::npos);
}

wstring TestReproduceMessage::runAndReturnSyserr()
{
  JUnitCore::runClasses(Nested::typeid);

  wstring err = getSysErr();
  // super.prevSysErr.println("Type: " + type + ", point: " + where + " resulted
  // in:\n" + err); super.prevSysErr.println("---");
  return err;
}
} // namespace org::apache::lucene::util