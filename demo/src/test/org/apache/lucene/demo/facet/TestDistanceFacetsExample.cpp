using namespace std;

#include "TestDistanceFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using TopDocs = org::apache::lucene::search::TopDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDistanceFacetsExample::testSimple() 
{
  shared_ptr<DistanceFacetsExample> example =
      make_shared<DistanceFacetsExample>();
  example->index();
  shared_ptr<FacetResult> result = example->search();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=field path=[] value=3 childCount=4\n  < 1 km (1)\n  < 2 "
               L"km (2)\n  < 5 km (2)\n  < 10 km (3)\n",
               result->toString());
  delete example;
}

void TestDistanceFacetsExample::testDrillDown() 
{
  shared_ptr<DistanceFacetsExample> example =
      make_shared<DistanceFacetsExample>();
  example->index();
  shared_ptr<TopDocs> hits = example->drillDown(example->FIVE_KM);
  assertEquals(2, hits->totalHits);
  delete example;
}
} // namespace org::apache::lucene::demo::facet