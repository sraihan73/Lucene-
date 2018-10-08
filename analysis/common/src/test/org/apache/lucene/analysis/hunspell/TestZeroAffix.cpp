using namespace std;

#include "TestZeroAffix.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestZeroAffix::beforeClass() 
{
  init(L"zeroaffix.aff", L"zeroaffix.dic");
}

void TestZeroAffix::testStemming()
{
  assertStemsTo(L"drink", {L"drinksierranevada"});
}
} // namespace org::apache::lucene::analysis::hunspell