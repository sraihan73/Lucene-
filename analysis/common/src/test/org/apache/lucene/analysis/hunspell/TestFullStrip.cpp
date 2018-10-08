using namespace std;

#include "TestFullStrip.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFullStrip::beforeClass() 
{
  init(L"fullstrip.aff", L"fullstrip.dic");
}

void TestFullStrip::testStemming() { assertStemsTo(L"tasty", {L"beer"}); }
} // namespace org::apache::lucene::analysis::hunspell