using namespace std;

#include "TestKeepCase.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestKeepCase::beforeClass() 
{
  init(L"keepcase.aff", L"keepcase.dic");
}

void TestKeepCase::testPossibilities()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"Drink", {L"drink"});
  assertStemsTo(L"DRINK", {L"drink"});
  assertStemsTo(L"drinks", {L"drink"});
  assertStemsTo(L"Drinks", {L"drink"});
  assertStemsTo(L"DRINKS", {L"drink"});
  assertStemsTo(L"walk", {L"walk"});
  assertStemsTo(L"walks", {L"walk"});
  assertStemsTo(L"Walk");
  assertStemsTo(L"Walks");
  assertStemsTo(L"WALKS");
  assertStemsTo(L"test", {L"test"});
  assertStemsTo(L"Test");
  assertStemsTo(L"TEST");
}
} // namespace org::apache::lucene::analysis::hunspell