using namespace std;

#include "TestConv.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestConv::beforeClass() 
{
  init(L"conv.aff", L"conv.dic");
}

void TestConv::testConversion()
{
  assertStemsTo(L"drink", {L"drInk"});
  assertStemsTo(L"drInk", {L"drInk"});
  assertStemsTo(L"drInkAble", {L"drInk"});
  assertStemsTo(L"drInkABle", {L"drInk"});
  assertStemsTo(L"drinkABle", {L"drInk"});
}
} // namespace org::apache::lucene::analysis::hunspell