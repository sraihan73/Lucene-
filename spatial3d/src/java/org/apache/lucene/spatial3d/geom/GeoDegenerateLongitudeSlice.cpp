using namespace std;

#include "GeoDegenerateLongitudeSlice.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoDegenerateLongitudeSlice::GeoDegenerateLongitudeSlice(
    shared_ptr<PlanetModel> planetModel, double const longitude)
    : GeoBaseBBox(planetModel), longitude(longitude),
      boundingPlane(
          make_shared<SidedPlane>(interiorPoint, -sinLongitude, cosLongitude)),
      plane(make_shared<Plane>(cosLongitude, sinLongitude)),
      interiorPoint(make_shared<GeoPoint>(planetModel, 0.0, sinLongitude, 1.0,
                                          cosLongitude)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{interiorPoint}),
      planePoints(std::deque<std::shared_ptr<GeoPoint>>{
          planetModel->NORTH_POLE, planetModel->SOUTH_POLE})
{
  // Argument checking
  if (longitude < -M_PI || longitude > M_PI) {
    throw invalid_argument(L"Longitude out of range");
  }

  constexpr double sinLongitude = sin(longitude);
  constexpr double cosLongitude = cos(longitude);

  // We need a bounding plane too, which is perpendicular to the longitude plane
  // and sided so that the point (0.0, longitude) is inside.
}

GeoDegenerateLongitudeSlice::GeoDegenerateLongitudeSlice(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoDegenerateLongitudeSlice(planetModel,
                                  SerializableObject::readDouble(inputStream))
{
}

void GeoDegenerateLongitudeSlice::write(
    shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, longitude);
}

shared_ptr<GeoBBox> GeoDegenerateLongitudeSlice::expand(double const angle)
{
  // Figuring out when we escalate to a special case requires some prefiguring
  double newLeftLon = longitude - angle;
  double newRightLon = longitude + angle;
  double currentLonSpan = 2.0 * angle;
  if (currentLonSpan + 2.0 * angle >= M_PI * 2.0) {
    newLeftLon = -M_PI;
    newRightLon = M_PI;
  }
  return GeoBBoxFactory::makeGeoBBox(planetModel, M_PI * 0.5, -M_PI * 0.5,
                                     newLeftLon, newRightLon);
}

bool GeoDegenerateLongitudeSlice::isWithin(double const x, double const y,
                                           double const z)
{
  return plane->evaluateIsZero(x, y, z) && boundingPlane->isWithin(x, y, z);
}

double GeoDegenerateLongitudeSlice::getRadius() { return M_PI * 0.5; }

shared_ptr<GeoPoint> GeoDegenerateLongitudeSlice::getCenter()
{
  return interiorPoint;
}

std::deque<std::shared_ptr<GeoPoint>>
GeoDegenerateLongitudeSlice::getEdgePoints()
{
  return edgePoints;
}

bool GeoDegenerateLongitudeSlice::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, plane, notablePoints, planePoints, bounds,
                       {boundingPlane});
}

bool GeoDegenerateLongitudeSlice::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(plane, planePoints, {boundingPlane});
}

void GeoDegenerateLongitudeSlice::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->addVerticalPlane(planetModel, longitude, plane, {boundingPlane})
      ->addPoint(planetModel->NORTH_POLE)
      ->addPoint(planetModel->SOUTH_POLE);
}

int GeoDegenerateLongitudeSlice::getRelationship(shared_ptr<GeoShape> path)
{
  // Look for intersections.
  if (intersects(path)) {
    return OVERLAPS;
  }

  if (path->isWithin(interiorPoint)) {
    return CONTAINS;
  }

  return DISJOINT;
}

double GeoDegenerateLongitudeSlice::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  constexpr double distance = distanceStyle->computeDistance(
      planetModel, plane, x, y, z, {boundingPlane});

  constexpr double northDistance =
      distanceStyle->computeDistance(planetModel->NORTH_POLE, x, y, z);
  constexpr double southDistance =
      distanceStyle->computeDistance(planetModel->SOUTH_POLE, x, y, z);

  return min(distance, min(northDistance, southDistance));
}

bool GeoDegenerateLongitudeSlice::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoDegenerateLongitudeSlice>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoDegenerateLongitudeSlice> other =
      any_cast<std::shared_ptr<GeoDegenerateLongitudeSlice>>(o);
  return GeoBaseBBox::equals(other) && other->longitude == longitude;
}

int GeoDegenerateLongitudeSlice::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  int64_t temp = Double::doubleToLongBits(longitude);
  result = result * 31 +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring GeoDegenerateLongitudeSlice::toString()
{
  return L"GeoDegenerateLongitudeSlice: {planetmodel=" + planetModel +
         L", longitude=" + to_wstring(longitude) + L"(" +
         to_wstring(longitude * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom