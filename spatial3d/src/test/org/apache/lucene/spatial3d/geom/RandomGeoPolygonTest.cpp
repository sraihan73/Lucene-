using namespace std;

#include "RandomGeoPolygonTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using com::carrotsearch::randomizedtesting::generators::BiasedNumbers;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testRandomLUCENE8157()
void RandomGeoPolygonTest::testRandomLUCENE8157()
{
  shared_ptr<PlanetModel> *const planetModel = randomPlanetModel();
  shared_ptr<GeoPoint> *const startPoint = randomGeoPoint(planetModel);
  double d = random()->nextDouble();
  constexpr double distanceSmall =
      d * 1e-9 + Vector::MINIMUM_ANGULAR_RESOLUTION;
  constexpr double distanceBig = d * 1e-7 + Vector::MINIMUM_ANGULAR_RESOLUTION;
  constexpr double bearing = random()->nextDouble() * M_PI;
  shared_ptr<GeoPoint> point1 = planetModel->surfacePointOnBearing(
      startPoint, distanceSmall, bearing * 1.001);
  shared_ptr<GeoPoint> point2 =
      planetModel->surfacePointOnBearing(startPoint, distanceBig, bearing);
  shared_ptr<GeoPoint> point3 = planetModel->surfacePointOnBearing(
      startPoint, distanceBig, bearing - 0.5 * M_PI);
  deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(startPoint);
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  try {
    shared_ptr<GeoPolygon> polygon =
        GeoPolygonFactory::makeGeoPolygon(planetModel, points);
    assertTrue(polygon != nullptr);
  } catch (const runtime_error &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    fail(points.toString());
  }
}

void RandomGeoPolygonTest::testLUCENE8157()
{
  shared_ptr<GeoPoint> point1 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, 0.281855362988772, -0.7816673189809037);
  shared_ptr<GeoPoint> point2 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, 0.28185536309057774, -0.7816673188511931);
  shared_ptr<GeoPoint> point3 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, 0.28186535556824205, -0.7816546103463846);
  shared_ptr<GeoPoint> point4 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, 0.28186757010406716, -0.7816777221140381);
  deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  try {
    shared_ptr<GeoPolygon> polygon =
        GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points);
  } catch (const runtime_error &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    fail(points.toString());
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCoplanarityTilePolygon()
void RandomGeoPolygonTest::testCoplanarityTilePolygon()
{
  // POLYGON((-90.55764 -0.34907,-90.55751 -0.34868,-90.55777 -0.34842,-90.55815
  // -0.34766,-90.55943 -0.34842, -90.55918 -0.34842,-90.55764 -0.34907))
  deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(make_shared<GeoPoint>(PlanetModel::SPHERE,
                                         Geo3DUtil::fromDegrees(-0.34907),
                                         Geo3DUtil::fromDegrees(-90.55764)));
  points.push_back(make_shared<GeoPoint>(PlanetModel::SPHERE,
                                         Geo3DUtil::fromDegrees(-0.34868),
                                         Geo3DUtil::fromDegrees(-90.55751)));
  points.push_back(make_shared<GeoPoint>(PlanetModel::SPHERE,
                                         Geo3DUtil::fromDegrees(-0.34842),
                                         Geo3DUtil::fromDegrees(-90.55777)));
  points.push_back(make_shared<GeoPoint>(PlanetModel::SPHERE,
                                         Geo3DUtil::fromDegrees(-0.34766),
                                         Geo3DUtil::fromDegrees(-90.55815)));
  points.push_back(make_shared<GeoPoint>(PlanetModel::SPHERE,
                                         Geo3DUtil::fromDegrees(-0.34842),
                                         Geo3DUtil::fromDegrees(-90.55943)));
  points.push_back(make_shared<GeoPoint>(PlanetModel::SPHERE,
                                         Geo3DUtil::fromDegrees(-0.34842),
                                         Geo3DUtil::fromDegrees(-90.55918)));
  shared_ptr<GeoCompositePolygon> polygon =
      std::static_pointer_cast<GeoCompositePolygon>(
          GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points));
  assertTrue(polygon->size() == 3);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testCompareBigPolygons()
void RandomGeoPolygonTest::testCompareBigPolygons()
{
  testComparePolygons(M_PI);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testCompareSmallPolygons()
void RandomGeoPolygonTest::testCompareSmallPolygons()
{
  testComparePolygons(1e-4 * M_PI);
}

void RandomGeoPolygonTest::testComparePolygons(double limitDistance)
{
  shared_ptr<PlanetModel> *const planetModel = randomPlanetModel();
  // Create polygon points using a reference point and a maximum distance to the
  // point
  shared_ptr<GeoPoint> *const referencePoint;
  if (random()->nextBoolean()) {
    referencePoint = getBiasedPoint(planetModel);
  } else {
    referencePoint = randomGeoPoint(planetModel);
  }
  constexpr int n = random()->nextInt(4) + 4;

  deque<std::shared_ptr<GeoPoint>> orderedPoints;
  shared_ptr<GeoPolygon> polygon = nullptr;
  shared_ptr<GeoPolygon> largePolygon = nullptr;
  do {
    const deque<std::shared_ptr<GeoPoint>> points =
        deque<std::shared_ptr<GeoPoint>>(n);
    double maxDistance = random()->nextDouble() * limitDistance;
    // if distance is too small we can fail
    // building the polygon.
    while (maxDistance < 1e-7) {
      maxDistance = random()->nextDouble() * limitDistance;
    }
    for (int i = 0; i < n; i++) {
      while (true) {
        constexpr double distance = BiasedNumbers::randomDoubleBetween(
            random(), 0, maxDistance); // random().nextDouble() * maxDistance;
        constexpr double bearing = random()->nextDouble() * 2 * M_PI;
        shared_ptr<GeoPoint> *const p = planetModel->surfacePointOnBearing(
            referencePoint, distance, bearing);
        if (!contains(p, points)) {
          if (points.size() > 1 &&
              Plane::arePointsCoplanar(points[points.size() - 1],
                                       points[points.size() - 2], p)) {
            continue;
          }
          points.push_back(p);
          break;
        }
      }
    }
    // order points so we don't get crossing edges
    orderedPoints = orderPoints(points);
    if (random()->nextBoolean() && random()->nextBoolean()) {
      reverse(orderedPoints.begin(), orderedPoints.end());
    }
    shared_ptr<GeoPolygonFactory::PolygonDescription>
        *const polygonDescription =
            make_shared<GeoPolygonFactory::PolygonDescription>(orderedPoints);

    try {
      polygon =
          GeoPolygonFactory::makeGeoPolygon(planetModel, polygonDescription);
    } catch (const runtime_error &e) {
      shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>(
          L"Polygon failed to build with an exception:\n");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(points.toString() + L"\n");
      buffer->append(L"WKT:" + getWKT(orderedPoints));
      buffer->append(e.what());
      fail(buffer->toString());
    }
    if (polygon == nullptr) {
      shared_ptr<StringBuilder> *const buffer =
          make_shared<StringBuilder>(L"Polygon failed to build:\n");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(points.toString() + L"\n");
      buffer->append(L"WKT:" + getWKT(orderedPoints));
      fail(buffer->toString());
    }
    try {
      largePolygon = GeoPolygonFactory::makeLargeGeoPolygon(
          planetModel, Collections::singletonList(polygonDescription));
    } catch (const runtime_error &e) {
      shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>(
          L"Large polygon failed to build with an exception:\n");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(points.toString() + L"\n");
      buffer->append(L"WKT:" + getWKT(orderedPoints));
      buffer->append(e.what());
      fail(buffer->toString());
    }
    if (largePolygon == nullptr) {
      shared_ptr<StringBuilder> buffer =
          make_shared<StringBuilder>(L"Large polygon failed to build:\n");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(points.toString() + L"\n");
      buffer->append(L"WKT:" + getWKT(orderedPoints));
      fail(buffer->toString());
    }
  } while (polygon->getClass().equals(largePolygon->getClass()));
  // Some of these do not work but it seems it s from the way the point is
  // created GeoPoint centerOfMass = getCenterOfMass(planetModel, orderedPoints);
  // checkPoint(polygon, largePolygon, centerOfMass, orderedPoints);
  // checkPoint(polygon, largePolygon, new GeoPoint(-centerOfMass.x,
  // -centerOfMass.y, -centerOfMass.z), orderedPoints); checkPoint(polygon,
  // largePolygon, new GeoPoint(centerOfMass.x, -centerOfMass.y,
  // -centerOfMass.z), orderedPoints); checkPoint(polygon, largePolygon, new
  // GeoPoint(centerOfMass.x, centerOfMass.y, -centerOfMass.z), orderedPoints);
  // checkPoint(polygon, largePolygon, new GeoPoint(-centerOfMass.x,
  // -centerOfMass.y, centerOfMass.z), orderedPoints); checkPoint(polygon,
  // largePolygon, new GeoPoint(-centerOfMass.x, centerOfMass.y,
  // -centerOfMass.z), orderedPoints); checkPoint(polygon, largePolygon, new
  // GeoPoint(centerOfMass.x, -centerOfMass.y, centerOfMass.z), orderedPoints);
  for (int i = 0; i < 100000; i++) {
    shared_ptr<GeoPoint> *const point;
    if (random()->nextBoolean()) {
      point = getBiasedPoint(planetModel);
    } else {
      point = randomGeoPoint(planetModel);
    }
    checkPoint(polygon, largePolygon, point, orderedPoints);
  }
}

void RandomGeoPolygonTest::checkPoint(
    shared_ptr<GeoPolygon> polygon, shared_ptr<GeoPolygon> largePolygon,
    shared_ptr<GeoPoint> point,
    deque<std::shared_ptr<GeoPoint>> &orderedPoints)
{
  constexpr bool withIn1 = polygon->isWithin(point);
  constexpr bool withIn2 = largePolygon->isWithin(point);
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (withIn1 != withIn2) {
    // NOTE: Standard and large polygon are mathematically slightly different
    // close to the edges (due to bounding planes). Nothing we can do about that
    // so we filter the differences.
    constexpr double d1 =
        polygon->computeOutsideDistance(DistanceStyle::ARC, point);
    constexpr double d2 =
        largePolygon->computeOutsideDistance(DistanceStyle::ARC, point);
    if (d1 == 0 && d2 == 0) {
      return;
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer =
        buffer->append(L"\nStandard polygon: " + polygon->toString() + L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer =
        buffer->append(L"\nLarge polygon: " + largePolygon->toString() + L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer = buffer->append(L"\nPoint: " + point->toString() + L"\n");
    buffer->append(L"\nWKT: " + getWKT(orderedPoints));
    buffer->append(
        L"\nWKT: POINT(" +
        to_wstring(Geo3DUtil::toDegrees(point->getLongitude())) + L" " +
        to_wstring(Geo3DUtil::toDegrees(point->getLatitude())) + L")\n");
    buffer->append(L"normal polygon: " + StringHelper::toString(withIn1) +
                   L"\n");
    buffer->append(L"large polygon: " + StringHelper::toString(withIn2) +
                   L"\n");
  }
  assertTrue(buffer->toString(), withIn1 == withIn2);
}

shared_ptr<GeoPoint>
RandomGeoPolygonTest::getBiasedPoint(shared_ptr<PlanetModel> planetModel)
{
  double lat = BiasedNumbers::randomDoubleBetween(random(), 0, M_PI / 2);
  if (random()->nextBoolean()) {
    lat = (-1) * lat;
  }
  double lon = BiasedNumbers::randomDoubleBetween(random(), 0, M_PI);
  if (random()->nextBoolean()) {
    lon = (-1) * lon;
  }
  return make_shared<GeoPoint>(planetModel, lat, lon);
}

wstring RandomGeoPolygonTest::getWKT(deque<std::shared_ptr<GeoPoint>> &points)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>(L"POLYGON((");
  for (auto point : points) {
    buffer->append(
        to_wstring(Geo3DUtil::toDegrees(point->getLongitude())) + L" " +
        to_wstring(Geo3DUtil::toDegrees(point->getLatitude())) + L",");
  }
  buffer->append(
      to_wstring(Geo3DUtil::toDegrees(points[0]->getLongitude())) + L" " +
      to_wstring(Geo3DUtil::toDegrees(points[0]->getLatitude())) + L"))\n");
  return buffer->toString();
}

bool RandomGeoPolygonTest::contains(shared_ptr<GeoPoint> p,
                                    deque<std::shared_ptr<GeoPoint>> &points)
{
  for (auto point : points) {
    if (point->isNumericallyIdentical(p)) {
      return true;
    }
  }
  return false;
}

shared_ptr<GeoPoint>
RandomGeoPolygonTest::getCenterOfMass(shared_ptr<PlanetModel> planetModel,
                                      deque<std::shared_ptr<GeoPoint>> &points)
{
  double x = 0;
  double y = 0;
  double z = 0;
  // get center of mass
  for (auto point : points) {
    x += point->x;
    y += point->y;
    z += point->z;
  }
  // Normalization is not needed because createSurfacePoint does the scaling
  // anyway.
  return planetModel->createSurfacePoint(x, y, z);
}
} // namespace org::apache::lucene::spatial3d::geom