using namespace std;

#include "GeoDegeneratePoint.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoDegeneratePoint::GeoDegeneratePoint(shared_ptr<PlanetModel> planetModel,
                                       double const lat, double const lon)
    : GeoPoint(planetModel, lat, lon), planetModel(planetModel),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{shared_from_this()})
{
}

GeoDegeneratePoint::GeoDegeneratePoint(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoPoint(planetModel, inputStream), planetModel(planetModel),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{shared_from_this()})
{
}

shared_ptr<PlanetModel> GeoDegeneratePoint::getPlanetModel()
{
  return planetModel;
}

shared_ptr<GeoBBox> GeoDegeneratePoint::expand(double const angle)
{
  constexpr double newTopLat = latitude + angle;
  constexpr double newBottomLat = latitude - angle;
  constexpr double newLeftLon = longitude - angle;
  constexpr double newRightLon = longitude + angle;
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     newLeftLon, newRightLon);
}

std::deque<std::shared_ptr<GeoPoint>> GeoDegeneratePoint::getEdgePoints()
{
  return edgePoints;
}

bool GeoDegeneratePoint::intersects(
    shared_ptr<Plane> plane,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // If not on the plane, no intersection
  if (!plane->evaluateIsZero(shared_from_this())) {
    return false;
  }

  for (shared_ptr<Membership> m : bounds) {
    if (!m->isWithin(shared_from_this())) {
      return false;
    }
  }
  return true;
}

bool GeoDegeneratePoint::intersects(shared_ptr<GeoShape> geoShape)
{
  // We have no way of computing this properly, so return isWithin(), as we are
  // allowed by contract.
  return geoShape->isWithin(shared_from_this());
}

void GeoDegeneratePoint::getBounds(shared_ptr<Bounds> bounds)
{
  bounds->addPoint(shared_from_this());
}

double GeoDegeneratePoint::computeOutsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, shared_ptr<GeoPoint> point)
{
  return distanceStyle->computeDistance(shared_from_this(), point);
}

double GeoDegeneratePoint::computeOutsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  return distanceStyle->computeDistance(shared_from_this(), x, y, z);
}

bool GeoDegeneratePoint::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoDegeneratePoint>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoDegeneratePoint> other =
      any_cast<std::shared_ptr<GeoDegeneratePoint>>(o);
  return GeoPoint::equals(other) && other->latitude == latitude &&
         other->longitude == longitude;
}

wstring GeoDegeneratePoint::toString()
{
  return L"GeoDegeneratePoint: {planetmodel=" + planetModel + L", lat=" +
         to_wstring(latitude) + L"(" + to_wstring(latitude * 180.0 / M_PI) +
         L"), lon=" + to_wstring(longitude) + L"(" +
         to_wstring(longitude * 180.0 / M_PI) + L")}";
}

bool GeoDegeneratePoint::isWithin(shared_ptr<Vector> point)
{
  return isWithin(point->x, {point->y, point->z});
}

bool GeoDegeneratePoint::isWithin(double const x, double const y,
                                  double const z)
{
  return this->isIdentical(x, y, z);
}

double GeoDegeneratePoint::getRadius() { return 0.0; }

shared_ptr<GeoPoint> GeoDegeneratePoint::getCenter()
{
  return shared_from_this();
}

int GeoDegeneratePoint::getRelationship(shared_ptr<GeoShape> shape)
{
  if (shape->isWithin(shared_from_this())) {
    // System.err.println("Degenerate point "+this+" is WITHIN shape "+shape);
    return CONTAINS;
  }

  // System.err.println("Degenerate point "+this+" is NOT within shape "+shape);
  return DISJOINT;
}

double
GeoDegeneratePoint::computeDistance(shared_ptr<DistanceStyle> distanceStyle,
                                    double const x, double const y,
                                    double const z)
{
  if (isWithin(x, {y, z})) {
    return 0.0;
  }
  return numeric_limits<double>::infinity();
}

void GeoDegeneratePoint::getDistanceBounds(
    shared_ptr<Bounds> bounds, shared_ptr<DistanceStyle> distanceStyle,
    double const distanceValue)
{
  getBounds(bounds);
}
} // namespace org::apache::lucene::spatial3d::geom