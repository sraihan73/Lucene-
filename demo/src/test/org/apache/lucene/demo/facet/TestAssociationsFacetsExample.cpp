using namespace std;

#include "TestAssociationsFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExamples() throws Exception
void TestAssociationsFacetsExample::testExamples() 
{
  deque<std::shared_ptr<FacetResult>> res =
      (make_shared<AssociationsFacetsExample>())->runSumAssociations();
  assertEquals(L"Wrong number of results", 2, res.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"dim=tags path=[] value=-1 childCount=2\n  lucene (4)\n  solr (2)\n",
      res[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=genre path=[] value=-1.0 childCount=2\n  computing "
               L"(1.62)\n  software (0.34)\n",
               res[1]->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDrillDown() throws Exception
void TestAssociationsFacetsExample::testDrillDown() 
{
  shared_ptr<FacetResult> result =
      (make_shared<AssociationsFacetsExample>())->runDrillDown();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=genre path=[] value=-1.0 childCount=2\n  computing "
               L"(0.75)\n  software (0.34)\n",
               result->toString());
}
} // namespace org::apache::lucene::demo::facet