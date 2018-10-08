using namespace std;

#include "TestExpressionAggregationFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimple() throws Exception
void TestExpressionAggregationFacetsExample::testSimple() 
{
  shared_ptr<FacetResult> result =
      (make_shared<ExpressionAggregationFacetsExample>())->runSearch();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=A path=[] value=3.9681187 childCount=2\n  B (2.236068)\n  "
               L"C (1.7320508)\n",
               result->toString());
}
} // namespace org::apache::lucene::demo::facet