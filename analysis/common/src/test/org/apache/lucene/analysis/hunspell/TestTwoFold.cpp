using namespace std;

#include "TestTwoFold.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestTwoFold::beforeClass() 
{
  init(L"twofold.aff", L"morph.dic");
}

void TestTwoFold::testExamples()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinkable", {L"drink"});
  assertStemsTo(L"drinkables", {L"drink"});
  assertStemsTo(L"drinksable");
  assertStemsTo(L"drinkableable");
  assertStemsTo(L"drinks");
}
} // namespace org::apache::lucene::analysis::hunspell