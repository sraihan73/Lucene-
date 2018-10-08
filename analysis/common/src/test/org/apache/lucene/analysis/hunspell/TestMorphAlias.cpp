using namespace std;

#include "TestMorphAlias.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestMorphAlias::beforeClass() 
{
  init(L"morphalias.aff", L"morphalias.dic");
}

void TestMorphAlias::testStemming()
{
  assertStemsTo(L"feet", {L"foot"});
  assertStemsTo(L"feetscratcher", {L"foot"});
  assertStemsTo(L"work", {L"workverb", L"worknoun"});
  assertStemsTo(L"works", {L"workverb", L"worknoun"});
  assertStemsTo(L"notspecial", {L"notspecial"});
  assertStemsTo(L"simplenoun", {L"simplenoun"});
  assertStemsTo(L"simplenouns", {L"simplenoun"});
  assertStemsTo(L"simplenounscratcher");
}
} // namespace org::apache::lucene::analysis::hunspell