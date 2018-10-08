using namespace std;

#include "Geo3DDocValuesField.h"

namespace org::apache::lucene::spatial3d
{
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using SortField = org::apache::lucene::search::SortField;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using Polygon = org::apache::lucene::geo::Polygon;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoDistanceShape = org::apache::lucene::spatial3d::geom::GeoDistanceShape;
using GeoOutsideDistance =
    org::apache::lucene::spatial3d::geom::GeoOutsideDistance;
const double Geo3DDocValuesField::inverseMaximumValue =
    1.0 / static_cast<double>(0x1FFFFF);
const double Geo3DDocValuesField::inverseXFactor =
    (org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
         ->getMaximumXValue() -
     org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
         ->getMinimumXValue()) *
    inverseMaximumValue;
const double Geo3DDocValuesField::inverseYFactor =
    (org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
         ->getMaximumYValue() -
     org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
         ->getMinimumYValue()) *
    inverseMaximumValue;
const double Geo3DDocValuesField::inverseZFactor =
    (org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
         ->getMaximumZValue() -
     org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
         ->getMinimumZValue()) *
    inverseMaximumValue;
const double Geo3DDocValuesField::xFactor = 1.0 / inverseXFactor;
const double Geo3DDocValuesField::yFactor = 1.0 / inverseYFactor;
const double Geo3DDocValuesField::zFactor = 1.0 / inverseZFactor;
const double Geo3DDocValuesField::xStep = inverseXFactor * STEP_FUDGE;
const double Geo3DDocValuesField::yStep = inverseYFactor * STEP_FUDGE;
const double Geo3DDocValuesField::zStep = inverseZFactor * STEP_FUDGE;
const shared_ptr<org::apache::lucene::document::FieldType>
    Geo3DDocValuesField::TYPE =
        make_shared<org::apache::lucene::document::FieldType>();

Geo3DDocValuesField::StaticConstructor::StaticConstructor()
{
  TYPE->setDocValuesType(DocValuesType::SORTED_NUMERIC);
  TYPE->freeze();
}

Geo3DDocValuesField::StaticConstructor Geo3DDocValuesField::staticConstructor;

Geo3DDocValuesField::Geo3DDocValuesField(const wstring &name,
                                         shared_ptr<GeoPoint> point)
    : org::apache::lucene::document::Field(name, TYPE)
{
  setLocationValue(point);
}

Geo3DDocValuesField::Geo3DDocValuesField(const wstring &name, double const x,
                                         double const y, double const z)
    : org::apache::lucene::document::Field(name, TYPE)
{
  setLocationValue(x, y, z);
}

void Geo3DDocValuesField::setLocationValue(shared_ptr<GeoPoint> point)
{
  fieldsData = static_cast<int64_t>(encodePoint(point));
}

void Geo3DDocValuesField::setLocationValue(double const x, double const y,
                                           double const z)
{
  fieldsData = static_cast<int64_t>(encodePoint(x, y, z));
}

int64_t Geo3DDocValuesField::encodePoint(shared_ptr<GeoPoint> point)
{
  return encodePoint(point->x, point->y, point->z);
}

int64_t Geo3DDocValuesField::encodePoint(double const x, double const y,
                                           double const z)
{
  int XEncoded = encodeX(x);
  int YEncoded = encodeY(y);
  int ZEncoded = encodeZ(z);
  return ((static_cast<int64_t>(XEncoded & 0x1FFFFF)) << 42) |
         ((static_cast<int64_t>(YEncoded & 0x1FFFFF)) << 21) |
         (static_cast<int64_t>(ZEncoded & 0x1FFFFF));
}

shared_ptr<GeoPoint> Geo3DDocValuesField::decodePoint(int64_t const docValue)
{
  return make_shared<GeoPoint>(
      decodeX((static_cast<int>(docValue >> 42)) & 0x1FFFFF),
      decodeY((static_cast<int>(docValue >> 21)) & 0x1FFFFF),
      decodeZ((static_cast<int>(docValue)) & 0x1FFFFF));
}

double Geo3DDocValuesField::decodeXValue(int64_t const docValue)
{
  return decodeX((static_cast<int>(docValue >> 42)) & 0x1FFFFF);
}

double Geo3DDocValuesField::decodeYValue(int64_t const docValue)
{
  return decodeY((static_cast<int>(docValue >> 21)) & 0x1FFFFF);
}

double Geo3DDocValuesField::decodeZValue(int64_t const docValue)
{
  return decodeZ((static_cast<int>(docValue)) & 0x1FFFFF);
}

double Geo3DDocValuesField::roundDownX(double const startValue)
{
  return startValue - xStep;
}

double Geo3DDocValuesField::roundUpX(double const startValue)
{
  return startValue + xStep;
}

double Geo3DDocValuesField::roundDownY(double const startValue)
{
  return startValue - yStep;
}

double Geo3DDocValuesField::roundUpY(double const startValue)
{
  return startValue + yStep;
}

double Geo3DDocValuesField::roundDownZ(double const startValue)
{
  return startValue - zStep;
}

double Geo3DDocValuesField::roundUpZ(double const startValue)
{
  return startValue + zStep;
}

int Geo3DDocValuesField::encodeX(double const x)
{
  if (x > PlanetModel::WGS84->getMaximumXValue()) {
    throw invalid_argument(L"x value exceeds WGS84 maximum");
  } else if (x < PlanetModel::WGS84->getMinimumXValue()) {
    throw invalid_argument(L"x value less than WGS84 minimum");
  }
  return static_cast<int>(
      floor((x - PlanetModel::WGS84->getMinimumXValue()) * xFactor + 0.5));
}

double Geo3DDocValuesField::decodeX(int const x)
{
  return x * inverseXFactor + PlanetModel::WGS84->getMinimumXValue();
}

int Geo3DDocValuesField::encodeY(double const y)
{
  if (y > PlanetModel::WGS84->getMaximumYValue()) {
    throw invalid_argument(L"y value exceeds WGS84 maximum");
  } else if (y < PlanetModel::WGS84->getMinimumYValue()) {
    throw invalid_argument(L"y value less than WGS84 minimum");
  }
  return static_cast<int>(
      floor((y - PlanetModel::WGS84->getMinimumYValue()) * yFactor + 0.5));
}

double Geo3DDocValuesField::decodeY(int const y)
{
  return y * inverseYFactor + PlanetModel::WGS84->getMinimumYValue();
}

int Geo3DDocValuesField::encodeZ(double const z)
{
  if (z > PlanetModel::WGS84->getMaximumZValue()) {
    throw invalid_argument(L"z value exceeds WGS84 maximum");
  } else if (z < PlanetModel::WGS84->getMinimumZValue()) {
    throw invalid_argument(L"z value less than WGS84 minimum");
  }
  return static_cast<int>(
      floor((z - PlanetModel::WGS84->getMinimumZValue()) * zFactor + 0.5));
}

double Geo3DDocValuesField::decodeZ(int const z)
{
  return z * inverseZFactor + PlanetModel::WGS84->getMinimumZValue();
}

void Geo3DDocValuesField::checkCompatible(shared_ptr<FieldInfo> fieldInfo)
{
  // dv properties could be "unset", if you e.g. used only StoredField with this
  // same name in the segment.
  if (fieldInfo->getDocValuesType() != DocValuesType::NONE &&
      fieldInfo->getDocValuesType() != TYPE->docValuesType()) {
    throw invalid_argument(
        L"field=\"" + fieldInfo->name + L"\" was indexed with docValuesType=" +
        fieldInfo->getDocValuesType() + L" but this type has docValuesType=" +
        TYPE->docValuesType() +
        L", is the field really a Geo3DDocValuesField?");
  }
}

wstring Geo3DDocValuesField::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  result->append(getClass().getSimpleName());
  result->append(L" <");
  result->append(name_);
  result->append(L':');

  int64_t currentValue = any_cast<optional<int64_t>>(fieldsData);

  result->append(decodeXValue(currentValue));
  result->append(L',');
  result->append(decodeYValue(currentValue));
  result->append(L',');
  result->append(decodeZValue(currentValue));

  result->append(L'>');
  return result->toString();
}

shared_ptr<SortField> Geo3DDocValuesField::newDistanceSort(
    const wstring &field, double const latitude, double const longitude,
    double const maxRadiusMeters)
{
  shared_ptr<GeoDistanceShape> *const shape =
      Geo3DUtil::fromDistance(latitude, longitude, maxRadiusMeters);
  return make_shared<Geo3DPointSortField>(field, shape);
}

shared_ptr<SortField> Geo3DDocValuesField::newPathSort(
    const wstring &field, std::deque<double> &pathLatitudes,
    std::deque<double> &pathLongitudes, double const pathWidthMeters)
{
  shared_ptr<GeoDistanceShape> *const shape =
      Geo3DUtil::fromPath(pathLatitudes, pathLongitudes, pathWidthMeters);
  return make_shared<Geo3DPointSortField>(field, shape);
}

shared_ptr<SortField> Geo3DDocValuesField::newOutsideDistanceSort(
    const wstring &field, double const latitude, double const longitude,
    double const maxRadiusMeters)
{
  shared_ptr<GeoOutsideDistance> *const shape =
      Geo3DUtil::fromDistance(latitude, longitude, maxRadiusMeters);
  return make_shared<Geo3DPointOutsideSortField>(field, shape);
}

shared_ptr<SortField> Geo3DDocValuesField::newOutsideBoxSort(
    const wstring &field, double const minLatitude, double const maxLatitude,
    double const minLongitude, double const maxLongitude)
{
  shared_ptr<GeoOutsideDistance> *const shape =
      Geo3DUtil::fromBox(minLatitude, maxLatitude, minLongitude, maxLongitude);
  return make_shared<Geo3DPointOutsideSortField>(field, shape);
}

shared_ptr<SortField>
Geo3DDocValuesField::newOutsidePolygonSort(const wstring &field,
                                           deque<Polygon> &polygons)
{
  shared_ptr<GeoOutsideDistance> *const shape =
      Geo3DUtil::fromPolygon({polygons});
  return make_shared<Geo3DPointOutsideSortField>(field, shape);
}

shared_ptr<SortField>
Geo3DDocValuesField::newOutsideLargePolygonSort(const wstring &field,
                                                deque<Polygon> &polygons)
{
  shared_ptr<GeoOutsideDistance> *const shape =
      Geo3DUtil::fromLargePolygon({polygons});
  return make_shared<Geo3DPointOutsideSortField>(field, shape);
}

shared_ptr<SortField> Geo3DDocValuesField::newOutsidePathSort(
    const wstring &field, std::deque<double> &pathLatitudes,
    std::deque<double> &pathLongitudes, double const pathWidthMeters)
{
  shared_ptr<GeoOutsideDistance> *const shape =
      Geo3DUtil::fromPath(pathLatitudes, pathLongitudes, pathWidthMeters);
  return make_shared<Geo3DPointOutsideSortField>(field, shape);
}
} // namespace org::apache::lucene::spatial3d