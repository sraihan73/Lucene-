using namespace std;

#include "TestMorphData.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestMorphData::beforeClass() 
{
  init(L"morphdata.aff", L"morphdata.dic");
}

void TestMorphData::testStemming()
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