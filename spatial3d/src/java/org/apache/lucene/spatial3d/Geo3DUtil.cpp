using namespace std;

#include "Geo3DUtil.h"

namespace org::apache::lucene::spatial3d
{
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using GeoPathFactory = org::apache::lucene::spatial3d::geom::GeoPathFactory;
using GeoCircleFactory = org::apache::lucene::spatial3d::geom::GeoCircleFactory;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoPath = org::apache::lucene::spatial3d::geom::GeoPath;
using GeoPolygon = org::apache::lucene::spatial3d::geom::GeoPolygon;
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoCompositePolygon =
    org::apache::lucene::spatial3d::geom::GeoCompositePolygon;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using Polygon = org::apache::lucene::geo::Polygon;
using GeoUtils = org::apache::lucene::geo::GeoUtils;
const double Geo3DUtil::RADIANS_PER_METER =
    1.0 / org::apache::lucene::spatial3d::geom::PlanetModel::WGS84_MEAN;
const double Geo3DUtil::RADIANS_PER_DEGREE = M_PI / 180.0;
const double Geo3DUtil::MAX_VALUE =
    org::apache::lucene::spatial3d::geom::PlanetModel::WGS84
        ->getMaximumMagnitude();
const double Geo3DUtil::MUL = (0x1LL << BITS) / (2 * MAX_VALUE);
const double Geo3DUtil::DECODE = getNextSafeDouble(1 / MUL);

int Geo3DUtil::encodeValue(double x)
{
  if (x > MAX_VALUE) {
    throw invalid_argument(
        L"value=" + to_wstring(x) +
        L" is out-of-bounds (greater than WGS84's planetMax=" +
        to_wstring(MAX_VALUE) + L")");
  }
  if (x < -MAX_VALUE) {
    throw invalid_argument(
        L"value=" + to_wstring(x) +
        L" is out-of-bounds (less than than WGS84's -planetMax=" +
        to_wstring(-MAX_VALUE) + L")");
  }
  int64_t result = static_cast<int64_t>(floor(x / DECODE));
  assert((result >= std, : numeric_limits<int>::min()));
  assert((result <= std, : numeric_limits<int>::max()));
  return static_cast<int>(result);
}

double Geo3DUtil::decodeValue(int x)
{
  double result;
  if (x == MIN_ENCODED_VALUE) {
    // We must special case this, because -MAX_VALUE is not guaranteed to land
    // precisely at a floor value, and we don't ever want to return a value
    // outside of the planet's range (I think?).  The max value is "safe"
    // because we floor during encode:
    result = -MAX_VALUE;
  } else if (x == MAX_ENCODED_VALUE) {
    result = MAX_VALUE;
  } else {
    // We decode to the center value; this keeps the encoding stable
    result = (x + 0.5) * DECODE;
  }
  assert(result >= -MAX_VALUE && result <= MAX_VALUE);
  return result;
}

double Geo3DUtil::decodeValueFloor(int x) { return x * DECODE; }

double Geo3DUtil::getNextSafeDouble(double x)
{

  // Move to double space:
  int64_t bits = Double::doubleToLongBits(x);

  // Make sure we are beyond the actual maximum value:
  bits += numeric_limits<int>::max();

  // Clear the bottom 32 bits:
  bits &= ~(static_cast<int64_t>(numeric_limits<int>::max()));

  // Convert back to double:
  double result = Double::longBitsToDouble(bits);
  assert(result > x);
  return result;
}

double Geo3DUtil::decodeValueCeil(int x)
{
  assert((x < std, : numeric_limits<int>::max()));
  return Math::nextDown((x + 1) * DECODE);
}

double Geo3DUtil::fromDegrees(double const degrees)
{
  return degrees * RADIANS_PER_DEGREE;
}

double Geo3DUtil::fromMeters(double const meters)
{
  return meters * RADIANS_PER_METER;
}

shared_ptr<GeoPolygon> Geo3DUtil::fromPolygon(deque<Polygon> &polygons)
{
  // System.err.println("Creating polygon...");
  if (polygons->length < 1) {
    throw invalid_argument(L"need at least one polygon");
  }
  shared_ptr<GeoPolygon> *const shape;
  if (polygons->length == 1) {
    shared_ptr<GeoPolygon> *const component = fromPolygon({polygons[0]});
    if (component == nullptr) {
      // Polygon is degenerate
      shape = make_shared<GeoCompositePolygon>(PlanetModel::WGS84);
    } else {
      shape = component;
    }
  } else {
    shared_ptr<GeoCompositePolygon> *const poly =
        make_shared<GeoCompositePolygon>(PlanetModel::WGS84);
    for (shared_ptr<Polygon> p : polygons) {
      shared_ptr<GeoPolygon> *const component = fromPolygon({p});
      if (component != nullptr) {
        poly->addShape(component);
      }
    }
    shape = poly;
  }
  return shape;
  // System.err.println("...done");
}

shared_ptr<GeoPolygon> Geo3DUtil::fromLargePolygon(deque<Polygon> &polygons)
{
  if (polygons->length < 1) {
    throw invalid_argument(L"need at least one polygon");
  }
  return GeoPolygonFactory::makeLargeGeoPolygon(
      PlanetModel::WGS84, convertToDescription({polygons}));
}

shared_ptr<GeoPath> Geo3DUtil::fromPath(std::deque<double> &pathLatitudes,
                                        std::deque<double> &pathLongitudes,
                                        double const pathWidthMeters)
{
  if (pathLatitudes.size() != pathLongitudes.size()) {
    throw invalid_argument(L"same number of latitudes and longitudes required");
  }
  std::deque<std::shared_ptr<GeoPoint>> points(pathLatitudes.size());
  for (int i = 0; i < pathLatitudes.size(); i++) {
    constexpr double latitude = pathLatitudes[i];
    constexpr double longitude = pathLongitudes[i];
    GeoUtils::checkLatitude(latitude);
    GeoUtils::checkLongitude(longitude);
    points[i] = make_shared<GeoPoint>(PlanetModel::WGS84, fromDegrees(latitude),
                                      fromDegrees(longitude));
  }
  return GeoPathFactory::makeGeoPath(PlanetModel::WGS84,
                                     fromMeters(pathWidthMeters), points);
}

shared_ptr<GeoCircle> Geo3DUtil::fromDistance(double const latitude,
                                              double const longitude,
                                              double const radiusMeters)
{
  GeoUtils::checkLatitude(latitude);
  GeoUtils::checkLongitude(longitude);
  return GeoCircleFactory::makeGeoCircle(
      PlanetModel::WGS84, fromDegrees(latitude), fromDegrees(longitude),
      fromMeters(radiusMeters));
}

shared_ptr<GeoBBox> Geo3DUtil::fromBox(double const minLatitude,
                                       double const maxLatitude,
                                       double const minLongitude,
                                       double const maxLongitude)
{
  GeoUtils::checkLatitude(minLatitude);
  GeoUtils::checkLongitude(minLongitude);
  GeoUtils::checkLatitude(maxLatitude);
  GeoUtils::checkLongitude(maxLongitude);
  return GeoBBoxFactory::makeGeoBBox(
      PlanetModel::WGS84, Geo3DUtil::fromDegrees(maxLatitude),
      Geo3DUtil::fromDegrees(minLatitude), Geo3DUtil::fromDegrees(minLongitude),
      Geo3DUtil::fromDegrees(maxLongitude));
}

shared_ptr<GeoPolygon> Geo3DUtil::fromPolygon(shared_ptr<Polygon> polygon)
{
  // First, assemble the "holes".  The geo3d convention is to use the same
  // polygon sense on the inner ring as the outer ring, so we process these
  // recursively with reverseMe flipped.
  std::deque<std::shared_ptr<Polygon>> theHoles = polygon->getHoles();
  const deque<std::shared_ptr<GeoPolygon>> holeList =
      deque<std::shared_ptr<GeoPolygon>>(theHoles.size());
  for (auto hole : theHoles) {
    // System.out.println("Hole: "+hole);
    shared_ptr<GeoPolygon> *const component = fromPolygon({hole});
    if (component != nullptr) {
      holeList.push_back(component);
    }
  }

  // Now do the polygon itself
  const std::deque<double> polyLats = polygon->getPolyLats();
  const std::deque<double> polyLons = polygon->getPolyLons();

  // I presume the arguments have already been checked
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>(polyLats.size() - 1);
  // We skip the last point anyway because the API requires it to be repeated,
  // and geo3d doesn't repeat it.
  for (int i = 0; i < polyLats.size() - 1; i++) {
    constexpr int index = polyLats.size() - 2 - i;
    points.push_back(make_shared<GeoPoint>(PlanetModel::WGS84,
                                           fromDegrees(polyLats[index]),
                                           fromDegrees(polyLons[index])));
  }
  // System.err.println(" building polygon with "+points.size()+" points...");
  shared_ptr<GeoPolygon> *const rval =
      GeoPolygonFactory::makeGeoPolygon(PlanetModel::WGS84, points, holeList);
  // System.err.println(" ...done");
  return rval;
}

deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>
Geo3DUtil::convertToDescription(deque<Polygon> &polygons)
{
  const deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>
      descriptions =
          deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>(
              polygons->length);
  for (shared_ptr<Polygon> polygon : polygons) {
    std::deque<std::shared_ptr<Polygon>> theHoles = polygon->getHoles();
    const deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>> holes =
        convertToDescription(theHoles);

    // Now do the polygon itself
    const std::deque<double> polyLats = polygon->getPolyLats();
    const std::deque<double> polyLons = polygon->getPolyLons();

    // I presume the arguments have already been checked
    const deque<std::shared_ptr<GeoPoint>> points =
        deque<std::shared_ptr<GeoPoint>>(polyLats.size() - 1);
    // We skip the last point anyway because the API requires it to be repeated,
    // and geo3d doesn't repeat it.
    for (int i = 0; i < polyLats.size() - 1; i++) {
      constexpr int index = polyLats.size() - 2 - i;
      points.push_back(make_shared<GeoPoint>(PlanetModel::WGS84,
                                             fromDegrees(polyLats[index]),
                                             fromDegrees(polyLons[index])));
    }

    descriptions.push_back(
        make_shared<GeoPolygonFactory::PolygonDescription>(points, holes));
  }
  return descriptions;
}
} // namespace org::apache::lucene::spatial3d