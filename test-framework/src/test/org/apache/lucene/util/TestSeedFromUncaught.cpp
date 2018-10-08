using namespace std;

#include "TestSeedFromUncaught.h"

namespace org::apache::lucene::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFoo() throws Exception
void TestSeedFromUncaught::ThrowInUncaught::testFoo() 
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this());
  t->start();
  t->join();
}

TestSeedFromUncaught::ThrowInUncaught::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<ThrowInUncaught> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestSeedFromUncaught::ThrowInUncaught::ThreadAnonymousInnerClass::run()
{
  throw runtime_error(L"foobar");
}

TestSeedFromUncaught::TestSeedFromUncaught() : WithNestedTests(true) {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testUncaughtDumpsSeed()
void TestSeedFromUncaught::testUncaughtDumpsSeed()
{
  shared_ptr<Result> result = JUnitCore::runClasses(ThrowInUncaught::typeid);
  assertFailureCount(1, result);
  shared_ptr<Failure> f = result->getFailures()->get(0);
  wstring trace = f->getTrace();
  Assert::assertTrue(trace.find(L"SeedInfo.seed(") != wstring::npos);
  Assert::assertTrue(trace.find(L"foobar") != wstring::npos);
}
} // namespace org::apache::lucene::util