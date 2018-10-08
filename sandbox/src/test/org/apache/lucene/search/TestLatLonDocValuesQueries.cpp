using namespace std;

#include "TestLatLonDocValuesQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LatLonDocValuesField =
    org::apache::lucene::document::LatLonDocValuesField;
using BaseGeoPointTestCase = org::apache::lucene::geo::BaseGeoPointTestCase;
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using Polygon = org::apache::lucene::geo::Polygon;

bool TestLatLonDocValuesQueries::supportsPolygons() { return false; }

void TestLatLonDocValuesQueries::addPointToDoc(const wstring &field,
                                               shared_ptr<Document> doc,
                                               double lat, double lon)
{
  doc->push_back(make_shared<LatLonDocValuesField>(field, lat, lon));
}

shared_ptr<Query> TestLatLonDocValuesQueries::newRectQuery(const wstring &field,
                                                           double minLat,
                                                           double maxLat,
                                                           double minLon,
                                                           double maxLon)
{
  return LatLonDocValuesField::newSlowBoxQuery(field, minLat, maxLat, minLon,
                                               maxLon);
}

shared_ptr<Query>
TestLatLonDocValuesQueries::newDistanceQuery(const wstring &field,
                                             double centerLat, double centerLon,
                                             double radiusMeters)
{
  return LatLonDocValuesField::newSlowDistanceQuery(field, centerLat, centerLon,
                                                    radiusMeters);
}

shared_ptr<Query>
TestLatLonDocValuesQueries::newPolygonQuery(const wstring &field,
                                            deque<Polygon> &polygons)
{
  fail();
  return nullptr;
}

double TestLatLonDocValuesQueries::quantizeLat(double latRaw)
{
  return GeoEncodingUtils::decodeLatitude(
      GeoEncodingUtils::encodeLatitude(latRaw));
}

double TestLatLonDocValuesQueries::quantizeLon(double lonRaw)
{
  return GeoEncodingUtils::decodeLongitude(
      GeoEncodingUtils::encodeLongitude(lonRaw));
}
} // namespace org::apache::lucene::search