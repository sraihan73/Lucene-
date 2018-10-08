using namespace std;

#include "TestDoubleEscape.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestDoubleEscape::beforeClass() 
{
  init(L"double-escaped.aff", L"double-escaped.dic");
}

void TestDoubleEscape::testStemming() { assertStemsTo(L"adubo", {L"adubar"}); }
} // namespace org::apache::lucene::analysis::hunspell