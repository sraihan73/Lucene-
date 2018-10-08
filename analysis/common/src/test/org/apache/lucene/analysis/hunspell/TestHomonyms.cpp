using namespace std;

#include "TestHomonyms.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestHomonyms::beforeClass() 
{
  init(L"homonyms.aff", L"homonyms.dic");
}

void TestHomonyms::testExamples()
{
  assertStemsTo(L"works", {L"work", L"work"});
}
} // namespace org::apache::lucene::analysis::hunspell