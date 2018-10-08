using namespace std;

#include "TestOptionalCondition.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestOptionalCondition::beforeClass() 
{
  init(L"optional-condition.aff", L"condition.dic");
}

void TestOptionalCondition::testStemming()
{
  assertStemsTo(L"hello", {L"hello"});
  assertStemsTo(L"try", {L"try"});
  assertStemsTo(L"tried", {L"try"});
  assertStemsTo(L"work", {L"work"});
  assertStemsTo(L"worked", {L"work"});
  assertStemsTo(L"rework", {L"work"});
  assertStemsTo(L"reworked", {L"work"});
  assertStemsTo(L"retried");
  assertStemsTo(L"workied");
  assertStemsTo(L"tryed");
  assertStemsTo(L"tryied");
  assertStemsTo(L"helloed");
}
} // namespace org::apache::lucene::analysis::hunspell