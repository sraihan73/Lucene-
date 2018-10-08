using namespace std;

#include "TestOnlyInCompound.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestOnlyInCompound::beforeClass() 
{
  init(L"onlyincompound.aff", L"onlyincompound.dic");
}

void TestOnlyInCompound::testPossibilities()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinks", {L"drink"});
  assertStemsTo(L"drinked");
  assertStemsTo(L"predrink");
  assertStemsTo(L"predrinked");
  assertStemsTo(L"walk");
}
} // namespace org::apache::lucene::analysis::hunspell