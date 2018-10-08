using namespace std;

#include "TestCaseInsensitive.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestCaseInsensitive::beforeClass() 
{
  init(true, L"simple.aff", L"mixedcase.dic");
}

void TestCaseInsensitive::testCaseInsensitivity()
{
  assertStemsTo(L"lucene", {L"lucene", L"lucen"});
  assertStemsTo(L"LuCeNe", {L"lucene", L"lucen"});
  assertStemsTo(L"mahoute", {L"mahout"});
  assertStemsTo(L"MaHoUte", {L"mahout"});
}

void TestCaseInsensitive::testSimplePrefix()
{
  assertStemsTo(L"solr", {L"olr"});
}

void TestCaseInsensitive::testRecursiveSuffix()
{
  // we should not recurse here! as the suffix has no continuation!
  assertStemsTo(L"abcd");
}

void TestCaseInsensitive::testAllStems()
{
  assertStemsTo(L"ab", {L"ab"});
  assertStemsTo(L"abc", {L"ab"});
  assertStemsTo(L"apach", {L"apach"});
  assertStemsTo(L"apache", {L"apach"});
  assertStemsTo(L"foo", {L"foo", L"foo"});
  assertStemsTo(L"food", {L"foo"});
  assertStemsTo(L"foos", {L"foo"});
  assertStemsTo(L"lucen", {L"lucen"});
  assertStemsTo(L"lucene", {L"lucen", L"lucene"});
  assertStemsTo(L"mahout", {L"mahout"});
  assertStemsTo(L"mahoute", {L"mahout"});
  assertStemsTo(L"moo", {L"moo"});
  assertStemsTo(L"mood", {L"moo"});
  assertStemsTo(L"olr", {L"olr"});
  assertStemsTo(L"solr", {L"olr"});
}

void TestCaseInsensitive::testBogusStems()
{
  assertStemsTo(L"abs");
  assertStemsTo(L"abe");
  assertStemsTo(L"sab");
  assertStemsTo(L"sapach");
  assertStemsTo(L"sapache");
  assertStemsTo(L"apachee");
  assertStemsTo(L"sfoo");
  assertStemsTo(L"sfoos");
  assertStemsTo(L"fooss");
  assertStemsTo(L"lucenee");
  assertStemsTo(L"solre");
}
} // namespace org::apache::lucene::analysis::hunspell