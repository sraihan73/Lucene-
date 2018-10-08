using namespace std;

#include "SerializedStrategyTest.h"

namespace org::apache::lucene::spatial::serialized
{
using SpatialMatchConcern = org::apache::lucene::spatial::SpatialMatchConcern;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using org::junit::Before;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before @Override public void setUp() throws Exception
void SerializedStrategyTest::setUp() 
{
  StrategyTestCase::setUp();
  this->ctx = SpatialContext::GEO;
  this->strategy = make_shared<SerializedDVStrategy>(ctx, L"serialized");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicOperaions() throws
// java.io.IOException
void SerializedStrategyTest::testBasicOperaions() 
{
  getAddAndVerifyIndexedDocuments(DATA_SIMPLE_BBOX);

  executeQueries(SpatialMatchConcern::EXACT, {QTEST_Simple_Queries_BBox});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testStatesBBox() throws java.io.IOException
void SerializedStrategyTest::testStatesBBox() 
{
  getAddAndVerifyIndexedDocuments(DATA_STATES_BBOX);

  executeQueries(SpatialMatchConcern::FILTER, {QTEST_States_IsWithin_BBox});
  executeQueries(SpatialMatchConcern::FILTER, {QTEST_States_Intersects_BBox});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCitiesIntersectsBBox() throws
// java.io.IOException
void SerializedStrategyTest::testCitiesIntersectsBBox() 
{
  getAddAndVerifyIndexedDocuments(DATA_WORLD_CITIES_POINTS);

  executeQueries(SpatialMatchConcern::FILTER, {QTEST_Cities_Intersects_BBox});
}
} // namespace org::apache::lucene::spatial::serialized