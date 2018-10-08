using namespace std;

#include "TestCircumfix.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestCircumfix::beforeClass() 
{
  init(L"circumfix.aff", L"circumfix.dic");
}

void TestCircumfix::testCircumfix()
{
  assertStemsTo(L"nagy", {L"nagy"});
  assertStemsTo(L"nagyobb", {L"nagy"});
  assertStemsTo(L"legnagyobb", {L"nagy"});
  assertStemsTo(L"legeslegnagyobb", {L"nagy"});
  assertStemsTo(L"nagyobbobb");
  assertStemsTo(L"legnagy");
  assertStemsTo(L"legeslegnagy");
}
} // namespace org::apache::lucene::analysis::hunspell