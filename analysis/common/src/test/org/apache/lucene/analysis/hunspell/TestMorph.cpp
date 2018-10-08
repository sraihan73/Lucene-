using namespace std;

#include "TestMorph.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestMorph::beforeClass() 
{
  init(L"morph.aff", L"morph.dic");
}

void TestMorph::testExamples()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinkable", {L"drink"});
  assertStemsTo(L"drinkableable");
}
} // namespace org::apache::lucene::analysis::hunspell