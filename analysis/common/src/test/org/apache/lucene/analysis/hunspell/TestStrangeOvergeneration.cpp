using namespace std;

#include "TestStrangeOvergeneration.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestStrangeOvergeneration::beforeClass() 
{
  init(L"strange-overgeneration.aff", L"strange-overgeneration.dic");
}

void TestStrangeOvergeneration::testStemming()
{
  assertStemsTo(L"btasty", {L"beer"});
  assertStemsTo(L"tasty");
  assertStemsTo(L"yuck");
  assertStemsTo(L"foo");
}
} // namespace org::apache::lucene::analysis::hunspell