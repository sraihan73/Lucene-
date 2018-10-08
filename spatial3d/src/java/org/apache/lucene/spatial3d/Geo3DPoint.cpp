using namespace std;

#include "Geo3DPoint.h"

namespace org::apache::lucene::spatial3d
{
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using PointValues = org::apache::lucene::index::PointValues;
using Polygon = org::apache::lucene::geo::Polygon;
using GeoUtils = org::apache::lucene::geo::GeoUtils;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;
const shared_ptr<org::apache::lucene::document::FieldType> Geo3DPoint::TYPE =
    make_shared<org::apache::lucene::document::FieldType>();

Geo3DPoint::StaticConstructor::StaticConstructor()
{
  TYPE->setDimensions(3, Integer::BYTES);
  TYPE->freeze();
}

Geo3DPoint::StaticConstructor Geo3DPoint::staticConstructor;

Geo3DPoint::Geo3DPoint(const wstring &name, double latitude, double longitude)
    : org::apache::lucene::document::Field(name, TYPE)
{
  GeoUtils::checkLatitude(latitude);
  GeoUtils::checkLongitude(longitude);
  // Translate latitude/longitude to x,y,z:
  shared_ptr<GeoPoint> *const point = make_shared<GeoPoint>(
      PlanetModel::WGS84, Geo3DUtil::fromDegrees(latitude),
      Geo3DUtil::fromDegrees(longitude));
  fillFieldsData(point->x, point->y, point->z);
}

shared_ptr<Query> Geo3DPoint::newDistanceQuery(const wstring &field,
                                               double const latitude,
                                               double const longitude,
                                               double const radiusMeters)
{
  shared_ptr<GeoShape> *const shape =
      Geo3DUtil::fromDistance(latitude, longitude, radiusMeters);
  return newShapeQuery(field, shape);
}

shared_ptr<Query> Geo3DPoint::newBoxQuery(const wstring &field,
                                          double const minLatitude,
                                          double const maxLatitude,
                                          double const minLongitude,
                                          double const maxLongitude)
{
  shared_ptr<GeoShape> *const shape =
      Geo3DUtil::fromBox(minLatitude, maxLatitude, minLongitude, maxLongitude);
  return newShapeQuery(field, shape);
}

shared_ptr<Query> Geo3DPoint::newPolygonQuery(const wstring &field,
                                              deque<Polygon> &polygons)
{
  shared_ptr<GeoShape> *const shape = Geo3DUtil::fromPolygon({polygons});
  return newShapeQuery(field, shape);
}

shared_ptr<Query> Geo3DPoint::newLargePolygonQuery(const wstring &field,
                                                   deque<Polygon> &polygons)
{
  shared_ptr<GeoShape> *const shape = Geo3DUtil::fromLargePolygon({polygons});
  return newShapeQuery(field, shape);
}

shared_ptr<Query> Geo3DPoint::newPathQuery(const wstring &field,
                                           std::deque<double> &pathLatitudes,
                                           std::deque<double> &pathLongitudes,
                                           double const pathWidthMeters)
{
  shared_ptr<GeoShape> *const shape =
      Geo3DUtil::fromPath(pathLatitudes, pathLongitudes, pathWidthMeters);
  return newShapeQuery(field, shape);
}

Geo3DPoint::Geo3DPoint(const wstring &name, double x, double y, double z)
    : org::apache::lucene::document::Field(name, TYPE)
{
  fillFieldsData(x, y, z);
}

void Geo3DPoint::fillFieldsData(double x, double y, double z)
{
  std::deque<char> bytes(12);
  encodeDimension(x, bytes, 0);
  encodeDimension(y, bytes, Integer::BYTES);
  encodeDimension(z, bytes, 2 * Integer::BYTES);
  fieldsData = make_shared<BytesRef>(bytes);
}

void Geo3DPoint::encodeDimension(double value, std::deque<char> &bytes,
                                 int offset)
{
  NumericUtils::intToSortableBytes(Geo3DUtil::encodeValue(value), bytes,
                                   offset);
}

double Geo3DPoint::decodeDimension(std::deque<char> &value, int offset)
{
  return Geo3DUtil::decodeValue(
      NumericUtils::sortableBytesToInt(value, offset));
}

shared_ptr<Query> Geo3DPoint::newShapeQuery(const wstring &field,
                                            shared_ptr<GeoShape> shape)
{
  return make_shared<PointInGeo3DShapeQuery>(field, shape);
}

wstring Geo3DPoint::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  result->append(getClass().getSimpleName());
  result->append(L" <");
  result->append(name_);
  result->append(L':');

  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  result->append(L" x=" +
                 to_wstring(decodeDimension(bytes->bytes, bytes->offset)));
  result->append(L" y=" + to_wstring(decodeDimension(
                              bytes->bytes, bytes->offset + Integer::BYTES)));
  result->append(
      L" z=" + to_wstring(decodeDimension(bytes->bytes,
                                          bytes->offset + 2 * Integer::BYTES)));
  result->append(L'>');
  return result->toString();
}
} // namespace org::apache::lucene::spatial3d