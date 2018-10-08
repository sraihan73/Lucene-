using namespace std;

#include "TestEscaped.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestEscaped::beforeClass() 
{
  init(L"escaped.aff", L"escaped.dic");
}

void TestEscaped::testStemming()
{
  assertStemsTo(L"works", {L"work"});
  assertStemsTo(L"work", {L"work"});
  assertStemsTo(L"R2/D2", {L"R2/D2"});
  assertStemsTo(L"R2/D2s", {L"R2/D2"});
  assertStemsTo(L"N/A", {L"N/A"});
  assertStemsTo(L"N/As");
}
} // namespace org::apache::lucene::analysis::hunspell