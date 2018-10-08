using namespace std;

#include "TestNeedAffix.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestNeedAffix::beforeClass() 
{
  init(L"needaffix.aff", L"needaffix.dic");
}

void TestNeedAffix::testPossibilities()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinks", {L"drink"});
  assertStemsTo(L"walk");
  assertStemsTo(L"walks", {L"walk"});
  assertStemsTo(L"prewalk", {L"walk"});
  assertStemsTo(L"prewalks", {L"walk"});
  assertStemsTo(L"test");
  assertStemsTo(L"pretest");
  assertStemsTo(L"tests");
  assertStemsTo(L"pretests");
}
} // namespace org::apache::lucene::analysis::hunspell