using namespace std;

#include "TestSpaces.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestSpaces::beforeClass() 
{
  init(L"spaces.aff", L"spaces.dic");
}

void TestSpaces::testStemming()
{
  assertStemsTo(L"four", {L"four"});
  assertStemsTo(L"fours", {L"four"});
  assertStemsTo(L"five", {L"five"});
  assertStemsTo(L"forty four", {L"forty four"});
  assertStemsTo(L"forty fours", {L"forty four"});
  assertStemsTo(L"forty five", {L"forty five"});
  assertStemsTo(L"fifty", {L"50"});
  assertStemsTo(L"fiftys", {L"50"});
  assertStemsTo(L"sixty", {L"60"});
  assertStemsTo(L"sixty four", {L"64"});
  assertStemsTo(L"fifty four", {L"54"});
  assertStemsTo(L"fifty fours", {L"54"});
}
} // namespace org::apache::lucene::analysis::hunspell