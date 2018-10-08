using namespace std;

#include "TestSimpleFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFacetOnly() throws Exception
void TestSimpleFacetsExample::testFacetOnly() 
{
  deque<std::shared_ptr<FacetResult>> results =
      (make_shared<SimpleFacetsExample>())->runFacetOnly();
  assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa (2)\n  Bob "
               L"(1)\n  Susan (1)\n  Frank (1)\n",
               results[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=Publish Date path=[] value=5 childCount=3\n  2010 (2)\n  "
               L"2012 (2)\n  1999 (1)\n",
               results[1]->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimple() throws Exception
void TestSimpleFacetsExample::testSimple() 
{
  deque<std::shared_ptr<FacetResult>> results =
      (make_shared<SimpleFacetsExample>())->runSearch();
  assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa (2)\n  Bob "
               L"(1)\n  Susan (1)\n  Frank (1)\n",
               results[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=Publish Date path=[] value=5 childCount=3\n  2010 (2)\n  "
               L"2012 (2)\n  1999 (1)\n",
               results[1]->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDrillDown() throws Exception
void TestSimpleFacetsExample::testDrillDown() 
{
  shared_ptr<FacetResult> result =
      (make_shared<SimpleFacetsExample>())->runDrillDown();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"dim=Author path=[] value=2 childCount=2\n  Bob (1)\n  Lisa (1)\n",
      result->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDrillSideways() throws Exception
void TestSimpleFacetsExample::testDrillSideways() 
{
  deque<std::shared_ptr<FacetResult>> result =
      (make_shared<SimpleFacetsExample>())->runDrillSideways();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=Publish Date path=[] value=5 childCount=3\n  2010 (2)\n  "
               L"2012 (2)\n  1999 (1)\n",
               result[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"dim=Author path=[] value=2 childCount=2\n  Bob (1)\n  Lisa (1)\n",
      result[1]->toString());
}
} // namespace org::apache::lucene::demo::facet