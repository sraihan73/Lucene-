using namespace std;

#include "TestPolygon.h"

namespace org::apache::lucene::geo
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPolygon::testPolygonNullPolyLats()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Polygon>(nullptr, std::deque<double>{-66, -65, -65, -66, -66});
  });
  assertTrue(expected.what()->contains(L"polyLats must not be null"));
}

void TestPolygon::testPolygonNullPolyLons()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Polygon>(std::deque<double>{18, 18, 19, 19, 18}, nullptr);
  });
  assertTrue(expected.what()->contains(L"polyLons must not be null"));
}

void TestPolygon::testPolygonLine()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Polygon>(std::deque<double>{18, 18, 18},
                         std::deque<double>{-66, -65, -66});
  });
  assertTrue(expected.what()->contains(L"at least 4 polygon points required"));
}

void TestPolygon::testPolygonBogus()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Polygon>(std::deque<double>{18, 18, 19, 19},
                         std::deque<double>{-66, -65, -65, -66, -66});
  });
  assertTrue(expected.what()->contains(L"must be equal length"));
}

void TestPolygon::testPolygonNotClosed()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Polygon>(std::deque<double>{18, 18, 19, 19, 19},
                         std::deque<double>{-66, -65, -65, -66, -67});
  });
  assertTrue(expected.what(),
             expected.what()->contains(L"it must close itself"));
}

void TestPolygon::testGeoJSONPolygon() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{\n");
  b->append(L"  \"type\": \"Polygon\",\n");
  b->append(L"  \"coordinates\": [\n");
  b->append(L"    [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"      [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"  ]\n");
  b->append(L"}\n");

  std::deque<std::shared_ptr<Polygon>> polygons =
      Polygon::fromGeoJSON(b->toString());
  assertEquals(1, polygons.size());
  assertEquals(make_shared<Polygon>(
                   std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
                   std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0}),
               polygons[0]);
}

void TestPolygon::testGeoJSONPolygonWithHole() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{\n");
  b->append(L"  \"type\": \"Polygon\",\n");
  b->append(L"  \"coordinates\": [\n");
  b->append(L"    [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"      [100.0, 1.0], [100.0, 0.0] ],\n");
  b->append(L"    [ [100.5, 0.5], [100.5, 0.75], [100.75, 0.75], [100.75, "
            L"0.5], [100.5, 0.5]]\n");
  b->append(L"  ]\n");
  b->append(L"}\n");

  shared_ptr<Polygon> hole = make_shared<Polygon>(
      std::deque<double>{0.5, 0.75, 0.75, 0.5, 0.5},
      std::deque<double>{100.5, 100.5, 100.75, 100.75, 100.5});
  shared_ptr<Polygon> expected = make_shared<Polygon>(
      std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
      std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0}, hole);
  std::deque<std::shared_ptr<Polygon>> polygons =
      Polygon::fromGeoJSON(b->toString());

  assertEquals(1, polygons.size());
  assertEquals(expected, polygons[0]);
}

void TestPolygon::testGeoJSONMultiPolygon() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{\n");
  b->append(L"  \"type\": \"MultiPolygon\",\n");
  b->append(L"  \"coordinates\": [\n");
  b->append(L"    [\n");
  b->append(L"      [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"        [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"    ],\n");
  b->append(L"    [\n");
  b->append(L"      [ [10.0, 2.0], [11.0, 2.0], [11.0, 3.0],\n");
  b->append(L"        [10.0, 3.0], [10.0, 2.0] ]\n");
  b->append(L"    ]\n");
  b->append(L"  ],\n");
  b->append(L"}\n");

  std::deque<std::shared_ptr<Polygon>> polygons =
      Polygon::fromGeoJSON(b->toString());
  assertEquals(2, polygons.size());
  assertEquals(make_shared<Polygon>(
                   std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
                   std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0}),
               polygons[0]);
  assertEquals(
      make_shared<Polygon>(std::deque<double>{2.0, 2.0, 3.0, 3.0, 2.0},
                           std::deque<double>{10.0, 11.0, 11.0, 10.0, 10.0}),
      polygons[1]);
}

void TestPolygon::testGeoJSONTypeComesLast() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{\n");
  b->append(L"  \"coordinates\": [\n");
  b->append(L"    [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"      [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"  ],\n");
  b->append(L"  \"type\": \"Polygon\",\n");
  b->append(L"}\n");

  std::deque<std::shared_ptr<Polygon>> polygons =
      Polygon::fromGeoJSON(b->toString());
  assertEquals(1, polygons.size());
  assertEquals(make_shared<Polygon>(
                   std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
                   std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0}),
               polygons[0]);
}

void TestPolygon::testGeoJSONPolygonFeature() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{ \"type\": \"Feature\",\n");
  b->append(L"  \"geometry\": {\n");
  b->append(L"    \"type\": \"Polygon\",\n");
  b->append(L"    \"coordinates\": [\n");
  b->append(L"      [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"        [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"      ]\n");
  b->append(L"  },\n");
  b->append(L"  \"properties\": {\n");
  b->append(L"    \"prop0\": \"value0\",\n");
  b->append(L"    \"prop1\": {\"this\": \"that\"}\n");
  b->append(L"  }\n");
  b->append(L"}\n");

  std::deque<std::shared_ptr<Polygon>> polygons =
      Polygon::fromGeoJSON(b->toString());
  assertEquals(1, polygons.size());
  assertEquals(make_shared<Polygon>(
                   std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
                   std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0}),
               polygons[0]);
}

void TestPolygon::testGeoJSONMultiPolygonFeature() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{ \"type\": \"Feature\",\n");
  b->append(L"  \"geometry\": {\n");
  b->append(L"      \"type\": \"MultiPolygon\",\n");
  b->append(L"      \"coordinates\": [\n");
  b->append(L"        [\n");
  b->append(L"          [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"            [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"        ],\n");
  b->append(L"        [\n");
  b->append(L"          [ [10.0, 2.0], [11.0, 2.0], [11.0, 3.0],\n");
  b->append(L"            [10.0, 3.0], [10.0, 2.0] ]\n");
  b->append(L"        ]\n");
  b->append(L"      ]\n");
  b->append(L"  },\n");
  b->append(L"  \"properties\": {\n");
  b->append(L"    \"prop0\": \"value0\",\n");
  b->append(L"    \"prop1\": {\"this\": \"that\"}\n");
  b->append(L"  }\n");
  b->append(L"}\n");

  std::deque<std::shared_ptr<Polygon>> polygons =
      Polygon::fromGeoJSON(b->toString());
  assertEquals(2, polygons.size());
  assertEquals(make_shared<Polygon>(
                   std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
                   std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0}),
               polygons[0]);
  assertEquals(
      make_shared<Polygon>(std::deque<double>{2.0, 2.0, 3.0, 3.0, 2.0},
                           std::deque<double>{10.0, 11.0, 11.0, 10.0, 10.0}),
      polygons[1]);
}

void TestPolygon::testGeoJSONFeatureCollectionWithSinglePolygon() throw(
    runtime_error)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{ \"type\": \"FeatureCollection\",\n");
  b->append(L"  \"features\": [\n");
  b->append(L"    { \"type\": \"Feature\",\n");
  b->append(L"      \"geometry\": {\n");
  b->append(L"        \"type\": \"Polygon\",\n");
  b->append(L"        \"coordinates\": [\n");
  b->append(L"          [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"            [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"          ]\n");
  b->append(L"      },\n");
  b->append(L"      \"properties\": {\n");
  b->append(L"        \"prop0\": \"value0\",\n");
  b->append(L"        \"prop1\": {\"this\": \"that\"}\n");
  b->append(L"      }\n");
  b->append(L"    }\n");
  b->append(L"  ]\n");
  b->append(L"}    \n");

  shared_ptr<Polygon> expected = make_shared<Polygon>(
      std::deque<double>{0.0, 0.0, 1.0, 1.0, 0.0},
      std::deque<double>{100.0, 101.0, 101.0, 100.0, 100.0});
  std::deque<std::shared_ptr<Polygon>> actual =
      Polygon::fromGeoJSON(b->toString());
  assertEquals(1, actual.size());
  assertEquals(expected, actual[0]);
}

void TestPolygon::testIllegalGeoJSONExtraCrapAtEnd() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{\n");
  b->append(L"  \"type\": \"Polygon\",\n");
  b->append(L"  \"coordinates\": [\n");
  b->append(L"    [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"      [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"  ]\n");
  b->append(L"}\n");
  b->append(L"foo\n");

  runtime_error e = expectThrows(
      ParseException::typeid, [&]() { Polygon::fromGeoJSON(b->toString()); });
  assertTrue(e.what()->contains(
      L"unexpected character 'f' after end of GeoJSON object"));
}

void TestPolygon::testIllegalGeoJSONLinkedCRS() 
{

  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{\n");
  b->append(L"  \"type\": \"Polygon\",\n");
  b->append(L"  \"coordinates\": [\n");
  b->append(L"    [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"      [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"  ],\n");
  b->append(L"  \"crs\": {\n");
  b->append(L"    \"type\": \"link\",\n");
  b->append(L"    \"properties\": {\n");
  b->append(L"      \"href\": \"http://example.com/crs/42\",\n");
  b->append(L"      \"type\": \"proj4\"\n");
  b->append(L"    }\n");
  b->append(L"  }    \n");
  b->append(L"}\n");
  runtime_error e = expectThrows(
      ParseException::typeid, [&]() { Polygon::fromGeoJSON(b->toString()); });
  assertTrue(e.what()->contains(L"cannot handle linked crs"));
}

void TestPolygon::testIllegalGeoJSONMultipleFeatures() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"{ \"type\": \"FeatureCollection\",\n");
  b->append(L"  \"features\": [\n");
  b->append(L"    { \"type\": \"Feature\",\n");
  b->append(L"      \"geometry\": {\"type\": \"Point\", \"coordinates\": "
            L"[102.0, 0.5]},\n");
  b->append(L"      \"properties\": {\"prop0\": \"value0\"}\n");
  b->append(L"    },\n");
  b->append(L"    { \"type\": \"Feature\",\n");
  b->append(L"      \"geometry\": {\n");
  b->append(L"      \"type\": \"LineString\",\n");
  b->append(L"      \"coordinates\": [\n");
  b->append(
      L"        [102.0, 0.0], [103.0, 1.0], [104.0, 0.0], [105.0, 1.0]\n");
  b->append(L"        ]\n");
  b->append(L"      },\n");
  b->append(L"      \"properties\": {\n");
  b->append(L"        \"prop0\": \"value0\",\n");
  b->append(L"        \"prop1\": 0.0\n");
  b->append(L"      }\n");
  b->append(L"    },\n");
  b->append(L"    { \"type\": \"Feature\",\n");
  b->append(L"      \"geometry\": {\n");
  b->append(L"        \"type\": \"Polygon\",\n");
  b->append(L"        \"coordinates\": [\n");
  b->append(L"          [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],\n");
  b->append(L"            [100.0, 1.0], [100.0, 0.0] ]\n");
  b->append(L"          ]\n");
  b->append(L"      },\n");
  b->append(L"      \"properties\": {\n");
  b->append(L"        \"prop0\": \"value0\",\n");
  b->append(L"        \"prop1\": {\"this\": \"that\"}\n");
  b->append(L"      }\n");
  b->append(L"    }\n");
  b->append(L"  ]\n");
  b->append(L"}    \n");

  runtime_error e = expectThrows(
      ParseException::typeid, [&]() { Polygon::fromGeoJSON(b->toString()); });
  assertTrue(e.what()->contains(
      L"can only handle type FeatureCollection (if it has a single polygon "
      L"geometry), Feature, Polygon or MutiPolygon, but got Point"));
}
} // namespace org::apache::lucene::geo