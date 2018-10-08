using namespace std;

#include "TestDependencies.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestDependencies::beforeClass() 
{
  init(L"dependencies.aff", L"dependencies.dic");
}

void TestDependencies::testDependencies()
{
  assertStemsTo(L"drink", {L"drink", L"drink"});
  assertStemsTo(L"drinks", {L"drink", L"drink"});
  assertStemsTo(L"drinkable", {L"drink"});
  assertStemsTo(L"drinkables", {L"drink"});
  assertStemsTo(L"undrinkable", {L"drink"});
  assertStemsTo(L"undrinkables", {L"drink"});
  assertStemsTo(L"undrink");
  assertStemsTo(L"undrinks");
}
} // namespace org::apache::lucene::analysis::hunspell