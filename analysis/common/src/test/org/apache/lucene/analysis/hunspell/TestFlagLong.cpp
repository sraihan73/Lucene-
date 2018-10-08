using namespace std;

#include "TestFlagLong.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFlagLong::beforeClass() 
{
  init(L"flaglong.aff", L"flaglong.dic");
}

void TestFlagLong::testLongFlags()
{
  assertStemsTo(L"foo", {L"foo"});
  assertStemsTo(L"foos", {L"foo"});
  assertStemsTo(L"fooss");
  assertStemsTo(L"foobogus");
}
} // namespace org::apache::lucene::analysis::hunspell