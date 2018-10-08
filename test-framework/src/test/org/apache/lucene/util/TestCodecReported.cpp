using namespace std;

#include "TestCodecReported.h"

namespace org::apache::lucene::util
{
using Codec = org::apache::lucene::codecs::Codec;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;

TestCodecReported::TestCodecReported() : WithNestedTests(true) {}

wstring TestCodecReported::Nested1::codecName;

void TestCodecReported::Nested1::testDummy()
{
  codecName = Codec::getDefault()->getName();
  fail();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCorrectCodecReported()
void TestCodecReported::testCorrectCodecReported()
{
  shared_ptr<Result> r = JUnitCore::runClasses(Nested1::typeid);
  Assert::assertEquals(1, r->getFailureCount());
  Assert::assertTrue(WithNestedTests::getSysErr(),
                     WithNestedTests::getSysErr().find(
                         L"codec=" + Nested1::codecName) != wstring::npos);
}
} // namespace org::apache::lucene::util