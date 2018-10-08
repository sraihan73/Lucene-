using namespace std;

#include "TestFailIfDirectoryNotClosed.h"

namespace org::apache::lucene::util
{
using Directory = org::apache::lucene::store::Directory;
using com::carrotsearch::randomizedtesting::RandomizedTest;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;

TestFailIfDirectoryNotClosed::TestFailIfDirectoryNotClosed()
    : WithNestedTests(true)
{
}

void TestFailIfDirectoryNotClosed::Nested1::testDummy() 
{
  shared_ptr<Directory> dir = newDirectory();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wcout << dir->toString() << endl;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailIfDirectoryNotClosed()
void TestFailIfDirectoryNotClosed::testFailIfDirectoryNotClosed()
{
  shared_ptr<Result> r = JUnitCore::runClasses(Nested1::typeid);
  RandomizedTest::assumeTrue(
      L"Ignoring nested test, very likely zombie threads present.",
      r->getIgnoreCount() == 0);
  assertFailureCount(1, r);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  Assert::assertTrue(r->getFailures()->get(0)->toString()->contains(
      L"Resource in scope SUITE failed to close"));
}
} // namespace org::apache::lucene::util