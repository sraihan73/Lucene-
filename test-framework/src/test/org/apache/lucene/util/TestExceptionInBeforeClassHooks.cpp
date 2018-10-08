using namespace std;

#include "TestExceptionInBeforeClassHooks.h"

namespace org::apache::lucene::util
{
using junit::framework::Assert;
using org::junit::Before;
using org::junit::BeforeClass;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;

TestExceptionInBeforeClassHooks::TestExceptionInBeforeClassHooks()
    : WithNestedTests(true)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestExceptionInBeforeClassHooks::Nested1::beforeClass() throw(
    runtime_error)
{
  shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass>();
  t->start();
  t->join();
}

TestExceptionInBeforeClassHooks::Nested1::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass()
{
}

void TestExceptionInBeforeClassHooks::Nested1::ThreadAnonymousInnerClass::run()
{
  throw runtime_error(L"foobar");
}

void TestExceptionInBeforeClassHooks::Nested1::test() {}

void TestExceptionInBeforeClassHooks::Nested2::test1() 
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this());
  t->start();
  t->join();
}

TestExceptionInBeforeClassHooks::Nested2::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<Nested2> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestExceptionInBeforeClassHooks::Nested2::ThreadAnonymousInnerClass::run()
{
  throw runtime_error(L"foobar1");
}

void TestExceptionInBeforeClassHooks::Nested2::test2() 
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass2>(shared_from_this());
  t->start();
  t->join();
}

TestExceptionInBeforeClassHooks::Nested2::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(shared_ptr<Nested2> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestExceptionInBeforeClassHooks::Nested2::ThreadAnonymousInnerClass2::run()
{
  throw runtime_error(L"foobar2");
}

void TestExceptionInBeforeClassHooks::Nested2::test3() 
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass3>(shared_from_this());
  t->start();
  t->join();
}

TestExceptionInBeforeClassHooks::Nested2::ThreadAnonymousInnerClass3::
    ThreadAnonymousInnerClass3(shared_ptr<Nested2> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestExceptionInBeforeClassHooks::Nested2::ThreadAnonymousInnerClass3::run()
{
  throw runtime_error(L"foobar3");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void runBeforeTest() throws Exception
void TestExceptionInBeforeClassHooks::Nested3::runBeforeTest() throw(
    runtime_error)
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this());
  t->start();
  t->join();
}

TestExceptionInBeforeClassHooks::Nested3::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<Nested3> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestExceptionInBeforeClassHooks::Nested3::ThreadAnonymousInnerClass::run()
{
  throw runtime_error(L"foobar");
}

void TestExceptionInBeforeClassHooks::Nested3::test1()  {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExceptionInBeforeClassFailsTheTest()
void TestExceptionInBeforeClassHooks::testExceptionInBeforeClassFailsTheTest()
{
  shared_ptr<Result> runClasses = JUnitCore::runClasses(Nested1::typeid);
  assertFailureCount(1, runClasses);
  Assert::assertEquals(1, runClasses->getRunCount());
  Assert::assertTrue(
      runClasses->getFailures()->get(0).getTrace()->contains(L"foobar"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExceptionWithinTestFailsTheTest()
void TestExceptionInBeforeClassHooks::testExceptionWithinTestFailsTheTest()
{
  shared_ptr<Result> runClasses = JUnitCore::runClasses(Nested2::typeid);
  assertFailureCount(3, runClasses);
  Assert::assertEquals(3, runClasses->getRunCount());

  deque<wstring> foobars = deque<wstring>();
  for (shared_ptr<Failure> f : runClasses->getFailures()) {
    shared_ptr<Matcher> m =
        Pattern::compile(L"foobar[0-9]+").matcher(f->getTrace());
    while (m->find()) {
      foobars.push_back(m->group());
    }
  }

  sort(foobars.begin(), foobars.end());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  Assert::assertEquals(L"[foobar1, foobar2, foobar3]",
                       Arrays->toString(foobars.toArray()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExceptionWithinBefore()
void TestExceptionInBeforeClassHooks::testExceptionWithinBefore()
{
  shared_ptr<Result> runClasses = JUnitCore::runClasses(Nested3::typeid);
  assertFailureCount(1, runClasses);
  Assert::assertEquals(1, runClasses->getRunCount());
  Assert::assertTrue(
      runClasses->getFailures()->get(0).getTrace()->contains(L"foobar"));
}
} // namespace org::apache::lucene::util