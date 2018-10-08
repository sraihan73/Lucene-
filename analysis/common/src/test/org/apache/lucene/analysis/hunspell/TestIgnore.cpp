using namespace std;

#include "TestIgnore.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestIgnore::beforeClass() 
{
  init(L"ignore.aff", L"ignore.dic");
}

void TestIgnore::testExamples()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinkable", {L"drink"});
  assertStemsTo(L"dr'ink-able", {L"drink"});
  assertStemsTo(L"drank-able", {L"drank"});
  assertStemsTo(L"'-'-'-");
}
} // namespace org::apache::lucene::analysis::hunspell