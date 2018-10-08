using namespace std;

#include "TestRangeFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using TopDocs = org::apache::lucene::search::TopDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimple() throws Exception
void TestRangeFacetsExample::testSimple() 
{
  shared_ptr<RangeFacetsExample> example = make_shared<RangeFacetsExample>();
  example->index();
  shared_ptr<FacetResult> result = example->search();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=timestamp path=[] value=87 childCount=3\n  Past hour "
               L"(4)\n  Past six hours (22)\n  Past day (87)\n",
               result->toString());
  delete example;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @SuppressWarnings("unchecked") public void
// testDrillDown() throws Exception
void TestRangeFacetsExample::testDrillDown() 
{
  shared_ptr<RangeFacetsExample> example = make_shared<RangeFacetsExample>();
  example->index();
  shared_ptr<TopDocs> hits = example->drillDown(example->PAST_SIX_HOURS);
  assertEquals(22, hits->totalHits);
  delete example;
}
} // namespace org::apache::lucene::demo::facet