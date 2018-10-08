using namespace std;

#include "TestCondition2.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestCondition2::beforeClass() 
{
  init(L"condition2.aff", L"condition2.dic");
}

void TestCondition2::testStemming()
{
  assertStemsTo(L"monopolies", {L"monopoly"});
}
} // namespace org::apache::lucene::analysis::hunspell