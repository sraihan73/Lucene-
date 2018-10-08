using namespace std;

#include "TestTwoSuffixes.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestTwoSuffixes::beforeClass() 
{
  init(L"twosuffixes.aff", L"twosuffixes.dic");
}

void TestTwoSuffixes::testExamples()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinkable", {L"drink"});
  assertStemsTo(L"drinks", {L"drink"});
  assertStemsTo(L"drinkableable");
  assertStemsTo(L"drinkss");
}
} // namespace org::apache::lucene::analysis::hunspell