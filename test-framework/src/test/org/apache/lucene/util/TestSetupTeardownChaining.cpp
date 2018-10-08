using namespace std;

#include "TestSetupTeardownChaining.h"

namespace org::apache::lucene::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;

void TestSetupTeardownChaining::NestedSetupChain::setUp() 
{
  // missing call.
  wcout << L"Hello." << endl;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMe()
void TestSetupTeardownChaining::NestedSetupChain::testMe() {}

void TestSetupTeardownChaining::NestedTeardownChain::tearDown() throw(
    runtime_error)
{
  // missing call.
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMe()
void TestSetupTeardownChaining::NestedTeardownChain::testMe() {}

TestSetupTeardownChaining::TestSetupTeardownChaining() : WithNestedTests(true)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSetupChaining()
void TestSetupTeardownChaining::testSetupChaining()
{
  shared_ptr<Result> result = JUnitCore::runClasses(NestedSetupChain::typeid);
  Assert::assertEquals(1, result->getFailureCount());
  shared_ptr<Failure> failure = result->getFailures()->get(0);
  Assert::assertTrue(failure->getMessage()->contains(
      L"One of the overrides of setUp does not propagate the call."));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTeardownChaining()
void TestSetupTeardownChaining::testTeardownChaining()
{
  shared_ptr<Result> result =
      JUnitCore::runClasses(NestedTeardownChain::typeid);
  Assert::assertEquals(1, result->getFailureCount());
  shared_ptr<Failure> failure = result->getFailures()->get(0);
  Assert::assertTrue(failure->getMessage()->contains(
      L"One of the overrides of tearDown does not propagate the call."));
}
} // namespace org::apache::lucene::util