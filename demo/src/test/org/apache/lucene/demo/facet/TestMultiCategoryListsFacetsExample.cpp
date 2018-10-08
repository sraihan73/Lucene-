using namespace std;

#include "TestMultiCategoryListsFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExample() throws Exception
void TestMultiCategoryListsFacetsExample::testExample() 
{
  deque<std::shared_ptr<FacetResult>> results =
      (make_shared<MultiCategoryListsFacetsExample>())->runSearch();
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
} // namespace org::apache::lucene::demo::facet