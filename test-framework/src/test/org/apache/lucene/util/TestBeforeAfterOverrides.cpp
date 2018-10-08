using namespace std;

#include "TestBeforeAfterOverrides.h"

namespace org::apache::lucene::util
{
using org::junit::After;
using org::junit::Assert;
using org::junit::Before;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;

TestBeforeAfterOverrides::TestBeforeAfterOverrides() : WithNestedTests(true) {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before()
void TestBeforeAfterOverrides::Before1::before() {}

void TestBeforeAfterOverrides::Before1::testEmpty() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void before()
void TestBeforeAfterOverrides::Before3::before() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after()
void TestBeforeAfterOverrides::After1::after() {}

void TestBeforeAfterOverrides::After1::testEmpty() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after()
void TestBeforeAfterOverrides::After3::after() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBefore()
void TestBeforeAfterOverrides::testBefore()
{
  shared_ptr<Result> result = JUnitCore::runClasses(Before3::typeid);
  Assert::assertEquals(1, result->getFailureCount());
  Assert::assertTrue(result->getFailures()->get(0).getTrace()->contains(
      L"There are overridden methods"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAfter()
void TestBeforeAfterOverrides::testAfter()
{
  shared_ptr<Result> result = JUnitCore::runClasses(Before3::typeid);
  Assert::assertEquals(1, result->getFailureCount());
  Assert::assertTrue(result->getFailures()->get(0).getTrace()->contains(
      L"There are overridden methods"));
}
} // namespace org::apache::lucene::util