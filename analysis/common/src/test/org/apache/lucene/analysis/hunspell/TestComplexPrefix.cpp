using namespace std;

#include "TestComplexPrefix.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestComplexPrefix::beforeClass() 
{
  init(L"complexprefix.aff", L"complexprefix.dic");
}

void TestComplexPrefix::testPrefixes()
{
  assertStemsTo(L"ptwofoo", {L"foo"});
  assertStemsTo(L"poneptwofoo", {L"foo"});
  assertStemsTo(L"foosuf", {L"foo"});
  assertStemsTo(L"ptwofoosuf", {L"foo"});
  assertStemsTo(L"poneptwofoosuf", {L"foo"});
  assertStemsTo(L"ponefoo");
  assertStemsTo(L"ponefoosuf");
  assertStemsTo(L"ptwoponefoo");
  assertStemsTo(L"ptwoponefoosuf");
}
} // namespace org::apache::lucene::analysis::hunspell