using namespace std;

#include "TestPointVectorStrategy.h"

namespace org::apache::lucene::spatial::deque
{
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using SpatialMatchConcern = org::apache::lucene::spatial::SpatialMatchConcern;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::junit::Before;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before @Override public void setUp() throws Exception
void TestPointVectorStrategy::setUp() 
{
  StrategyTestCase::setUp();
  this->ctx = SpatialContext::GEO;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCircleShapeSupport()
void TestPointVectorStrategy::testCircleShapeSupport()
{
  this->strategy =
      PointVectorStrategy::newInstance(ctx, getClass().getSimpleName());
  shared_ptr<Circle> circle = ctx->makeCircle(ctx->makePoint(0, 0), 10);
  shared_ptr<SpatialArgs> args =
      make_shared<SpatialArgs>(SpatialOperation::Intersects, circle);
  shared_ptr<Query> query = this->strategy->makeQuery(args);

  assertNotNull(query);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = UnsupportedOperationException.class) public
// void testInvalidQueryShape()
void TestPointVectorStrategy::testInvalidQueryShape()
{
  this->strategy =
      PointVectorStrategy::newInstance(ctx, getClass().getSimpleName());
  shared_ptr<Point> point = ctx->makePoint(0, 0);
  shared_ptr<SpatialArgs> args =
      make_shared<SpatialArgs>(SpatialOperation::Intersects, point);
  this->strategy->makeQuery(args);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCitiesIntersectsBBox() throws
// java.io.IOException
void TestPointVectorStrategy::testCitiesIntersectsBBox() 
{
  // note: does not require docValues
  this->strategy =
      PointVectorStrategy::newInstance(ctx, getClass().getSimpleName());
  getAddAndVerifyIndexedDocuments(DATA_WORLD_CITIES_POINTS);
  executeQueries(SpatialMatchConcern::FILTER, {QTEST_Cities_Intersects_BBox});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFieldOptions() throws
// java.io.IOException, java.text.ParseException
void TestPointVectorStrategy::testFieldOptions() throw(IOException,
                                                       ParseException)
{
  // It's not stored; test it isn't.
  this->strategy =
      PointVectorStrategy::newInstance(ctx, getClass().getSimpleName());
  adoc(L"99", L"POINT(-5.0 8.2)");
  commit();
  shared_ptr<SearchResults> results =
      executeQuery(make_shared<MatchAllDocsQuery>(), 1);
  shared_ptr<Document> document = results->results[0]->document;
  assertNull(L"not stored", document->getField(strategy->getFieldName() +
                                               PointVectorStrategy::SUFFIX_X));
  assertNull(L"not stored", document->getField(strategy->getFieldName() +
                                               PointVectorStrategy::SUFFIX_Y));
  deleteAll();

  // Now we mark it stored.  We also disable pointvalues...
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(PointVectorStrategy::DEFAULT_FIELDTYPE);
  fieldType->setStored(true);
  fieldType->setDimensions(0, 0); // disable point values
  this->strategy = make_shared<PointVectorStrategy>(
      ctx, getClass().getSimpleName(), fieldType);
  adoc(L"99", L"POINT(-5.0 8.2)");
  commit();
  results = executeQuery(make_shared<MatchAllDocsQuery>(), 1);
  document = results->results[0]->document;
  assertEquals(
      L"stored", -5.0,
      document
          ->getField(strategy->getFieldName() + PointVectorStrategy::SUFFIX_X)
          ->numericValue());
  assertEquals(
      L"stored", 8.2,
      document
          ->getField(strategy->getFieldName() + PointVectorStrategy::SUFFIX_Y)
          ->numericValue());

  // Test a query fails without point values
  expectThrows(UnsupportedOperationException::typeid, [&]() {
    shared_ptr<SpatialArgs> args =
        make_shared<SpatialArgs>(SpatialOperation::Intersects,
                                 ctx->makeRectangle(-10.0, 10.0, -5.0, 5.0));
    this->strategy->makeQuery(args);
  });
}
} // namespace org::apache::lucene::spatial::deque