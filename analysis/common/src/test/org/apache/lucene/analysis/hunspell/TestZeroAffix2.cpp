using namespace std;

#include "TestZeroAffix2.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestZeroAffix2::beforeClass() 
{
  init(L"zeroaffix2.aff", L"zeroaffix2.dic");
}

void TestZeroAffix2::testStemming() { assertStemsTo(L"b", {L"beer"}); }
} // namespace org::apache::lucene::analysis::hunspell