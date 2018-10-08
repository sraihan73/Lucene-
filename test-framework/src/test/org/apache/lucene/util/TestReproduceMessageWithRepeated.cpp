using namespace std;

#include "TestReproduceMessageWithRepeated.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::JUnitCore;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testMe()
void TestReproduceMessageWithRepeated::Nested::testMe()
{
  throw runtime_error(L"bad");
}

TestReproduceMessageWithRepeated::TestReproduceMessageWithRepeated()
    : WithNestedTests(true)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRepeatedMessage() throws Exception
void TestReproduceMessageWithRepeated::testRepeatedMessage() throw(
    runtime_error)
{
  wstring syserr = runAndReturnSyserr();
  Assert::assertTrue(syserr.find(L" -Dtests.method=testMe ") != wstring::npos);
}

wstring TestReproduceMessageWithRepeated::runAndReturnSyserr()
{
  JUnitCore::runClasses(Nested::typeid);
  wstring err = getSysErr();
  return err;
}
} // namespace org::apache::lucene::util