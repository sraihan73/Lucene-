using namespace std;

#include "TestFlagNum.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFlagNum::beforeClass() 
{
  init(L"flagnum.aff", L"flagnum.dic");
}

void TestFlagNum::testNumFlags()
{
  assertStemsTo(L"foo", {L"foo"});
  assertStemsTo(L"foos", {L"foo"});
  assertStemsTo(L"fooss");
}
} // namespace org::apache::lucene::analysis::hunspell