using namespace std;

#include "Geo3dRptTest.h"

namespace org::apache::lucene::spatial::spatial4j
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using SpatialTestData = org::apache::lucene::spatial::SpatialTestData;
using CompositeSpatialStrategy =
    org::apache::lucene::spatial::composite::CompositeSpatialStrategy;
using RandomSpatialOpStrategyTestCase =
    org::apache::lucene::spatial::prefix::RandomSpatialOpStrategyTestCase;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using S2PrefixTree = org::apache::lucene::spatial::prefix::tree::S2PrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
using GeoAreaShape = org::apache::lucene::spatial3d::geom::GeoAreaShape;
using GeoPath = org::apache::lucene::spatial3d::geom::GeoPath;
using GeoPathFactory = org::apache::lucene::spatial3d::geom::GeoPathFactory;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPointShape = org::apache::lucene::spatial3d::geom::GeoPointShape;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using RandomGeo3dShapeGenerator =
    org::apache::lucene::spatial3d::geom::RandomGeo3dShapeGenerator;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
//    import static
//    org.locationtech.spatial4j.distance.DistanceUtils.DEGREES_TO_RADIANS;

void Geo3dRptTest::setupGrid()
{
  int type = random()->nextInt(4);
  if (type == 0) {
    this->grid = make_shared<GeohashPrefixTree>(ctx, 2);
  } else if (type == 1) {
    this->grid = make_shared<QuadPrefixTree>(ctx, 5);
  } else {
    int arity = random()->nextInt(3) + 1;
    this->grid = make_shared<S2PrefixTree>(ctx, 5 - arity, arity);
  }
  this->rptStrategy = newRPT();
  this->rptStrategy->setPruneLeafyBranches(random()->nextBoolean());
}

shared_ptr<RecursivePrefixTreeStrategy> Geo3dRptTest::newRPT()
{
  shared_ptr<RecursivePrefixTreeStrategy> *const rpt =
      make_shared<RecursivePrefixTreeStrategy>(
          this->grid, getClass().getSimpleName() + L"_rpt");
  rpt->setDistErrPct(0.10); // not too many cells
  return rpt;
}

void Geo3dRptTest::setupStrategy()
{
  shapeGenerator = make_shared<RandomGeo3dShapeGenerator>();
  planetModel = shapeGenerator->randomPlanetModel();
  shared_ptr<Geo3dSpatialContextFactory> factory =
      make_shared<Geo3dSpatialContextFactory>();
  factory->planetModel = planetModel;
  ctx = factory->newSpatialContext();

  setupGrid();

  shared_ptr<SerializedDVStrategy> serializedDVStrategy =
      make_shared<SerializedDVStrategy>(ctx,
                                        getClass().getSimpleName() + L"_sdv");
  this->strategy = make_shared<CompositeSpatialStrategy>(
      L"composite_" + getClass().getSimpleName(), rptStrategy,
      serializedDVStrategy);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure1() throws java.io.IOException
void Geo3dRptTest::testFailure1() 
{
  setupStrategy();
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(make_shared<GeoPoint>(planetModel, 18 * DEGREES_TO_RADIANS,
                                         -27 * DEGREES_TO_RADIANS));
  points.push_back(make_shared<GeoPoint>(planetModel, -57 * DEGREES_TO_RADIANS,
                                         146 * DEGREES_TO_RADIANS));
  points.push_back(make_shared<GeoPoint>(planetModel, 14 * DEGREES_TO_RADIANS,
                                         -180 * DEGREES_TO_RADIANS));
  points.push_back(make_shared<GeoPoint>(planetModel, -15 * DEGREES_TO_RADIANS,
                                         153 * DEGREES_TO_RADIANS));

  shared_ptr<Shape> *const triangle = make_shared<Geo3dShape<>>(
      GeoPolygonFactory::makeGeoPolygon(planetModel, points), ctx);
  shared_ptr<Rectangle> *const rect = ctx->makeRectangle(-49, -45, 73, 86);
  testOperation(rect, SpatialOperation::Intersects, triangle, false);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailureLucene6535() throws
// java.io.IOException
void Geo3dRptTest::testFailureLucene6535() 
{
  setupStrategy();

  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(make_shared<GeoPoint>(planetModel, 18 * DEGREES_TO_RADIANS,
                                         -27 * DEGREES_TO_RADIANS));
  points.push_back(make_shared<GeoPoint>(planetModel, -57 * DEGREES_TO_RADIANS,
                                         146 * DEGREES_TO_RADIANS));
  points.push_back(make_shared<GeoPoint>(planetModel, 14 * DEGREES_TO_RADIANS,
                                         -180 * DEGREES_TO_RADIANS));
  points.push_back(make_shared<GeoPoint>(planetModel, -15 * DEGREES_TO_RADIANS,
                                         153 * DEGREES_TO_RADIANS));
  std::deque<std::shared_ptr<GeoPoint>> pathPoints = {
      make_shared<GeoPoint>(planetModel, 55.0 * DEGREES_TO_RADIANS,
                            -26.0 * DEGREES_TO_RADIANS),
      make_shared<GeoPoint>(planetModel, -90.0 * DEGREES_TO_RADIANS, 0.0),
      make_shared<GeoPoint>(planetModel, 54.0 * DEGREES_TO_RADIANS,
                            165.0 * DEGREES_TO_RADIANS),
      make_shared<GeoPoint>(planetModel, -90.0 * DEGREES_TO_RADIANS, 0.0)};
  shared_ptr<GeoPath> *const path = GeoPathFactory::makeGeoPath(
      planetModel, 29 * DEGREES_TO_RADIANS, pathPoints);
  shared_ptr<Shape> *const shape = make_shared<Geo3dShape<>>(path, ctx);
  shared_ptr<Rectangle> *const rect = ctx->makeRectangle(131, 143, 39, 54);
  testOperation(rect, SpatialOperation::Intersects, shape, true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 30) public void testOperations()
// throws java.io.IOException
void Geo3dRptTest::testOperations() 
{
  setupStrategy();

  testOperationRandomShapes(SpatialOperation::Intersects);
}

shared_ptr<Shape> Geo3dRptTest::randomIndexedShape()
{
  int type = shapeGenerator->randomShapeType();
  shared_ptr<GeoAreaShape> areaShape =
      shapeGenerator->randomGeoAreaShape(type, planetModel);
  if (std::dynamic_pointer_cast<GeoPointShape>(areaShape) != nullptr) {
    return make_shared<Geo3dPointShape>(
        std::static_pointer_cast<GeoPointShape>(areaShape), ctx);
  }
  return make_shared<Geo3dShape<>>(areaShape, ctx);
}

shared_ptr<Shape> Geo3dRptTest::randomQueryShape()
{
  int type = shapeGenerator->randomShapeType();
  shared_ptr<GeoAreaShape> areaShape =
      shapeGenerator->randomGeoAreaShape(type, planetModel);
  return make_shared<Geo3dShape<>>(areaShape, ctx);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOperationsFromFile() throws
// java.io.IOException
void Geo3dRptTest::testOperationsFromFile() 
{
  setupStrategy();
  const Iterator<std::shared_ptr<SpatialTestData>> indexedSpatialData =
      getSampleData(L"states-poly.txt");
  const deque<std::shared_ptr<Shape>> indexedShapes =
      deque<std::shared_ptr<Shape>>();
  while (indexedSpatialData->hasNext()) {
    indexedShapes.push_back(indexedSpatialData->next().shape);
    indexedSpatialData++;
  }
  const Iterator<std::shared_ptr<SpatialTestData>> querySpatialData =
      getSampleData(L"states-bbox.txt");
  const deque<std::shared_ptr<Shape>> queryShapes =
      deque<std::shared_ptr<Shape>>();
  while (querySpatialData->hasNext()) {
    queryShapes.push_back(querySpatialData->next().shape);
    queryShapes.push_back(randomQueryShape());
    querySpatialData++;
  }
  testOperation(SpatialOperation::Intersects, indexedShapes, queryShapes,
                random()->nextBoolean());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWKT() throws Exception
void Geo3dRptTest::testWKT() 
{
  shared_ptr<Geo3dSpatialContextFactory> factory =
      make_shared<Geo3dSpatialContextFactory>();
  shared_ptr<SpatialContext> ctx = factory->newSpatialContext();
  wstring wkt =
      L"POLYGON ((20.0 -60.4, 20.1 -60.4, 20.1 -60.3, 20.0  -60.3,20.0 -60.4))";
  shared_ptr<Shape> s = ctx->getFormats().getWktReader().read(wkt);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"POINT (30 10)";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"LINESTRING (30 10, 10 30, 40 40)";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"POLYGON ((35 10, 45 45, 15 40, 10 20, 35 10), (20 30, 35 35, 30 "
            L"20, 20 30))";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"MULTIPOINT ((10 40), (40 30), (20 20), (30 10))";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"MULTILINESTRING ((10 10, 20 20, 10 40),(40 40, 30 30, 40 20, 30 "
            L"10))";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)), ((20 35, 10 30, 10 "
            L"10, 30 5, 45 20, 20 35),(30 20, 20 15, 20 25, 30 20)))";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"GEOMETRYCOLLECTION(POINT(4 6),LINESTRING(4 6,7 10))";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"ENVELOPE(1, 2, 4, 3)";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      wkt = L"BUFFER(POINT(-10 30), 5.2)";
      s = ctx->getFormats().getWktReader().read(wkt);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: assertTrue(s instanceof Geo3dShape<?>);
      assertTrue(std::dynamic_pointer_cast<Geo3dShape<?>>(s) != nullptr);
      // wkt = "BUFFER(LINESTRING(1 2, 3 4), 0.5)";
      // s = ctx.getFormats().getWktReader().read(wkt);
      // assertTrue(s instanceof  Geo3dShape<?>);
}
} // namespace org::apache::lucene::spatial::spatial4j