using namespace std;

#include "GeoDegenerateLatitudeZone.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const
    GeoDegenerateLatitudeZone::planePoints =
        std::deque<std::shared_ptr<GeoPoint>>(0);

GeoDegenerateLatitudeZone::GeoDegenerateLatitudeZone(
    shared_ptr<PlanetModel> planetModel, double const latitude)
    : GeoBaseBBox(planetModel), latitude(latitude), sinLatitude(sin(latitude)),
      plane(make_shared<Plane>(planetModel, sinLatitude)),
      interiorPoint(make_shared<GeoPoint>(planetModel, sinLatitude, 0.0,
                                          cosLatitude, 1.0)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{interiorPoint})
{

  double cosLatitude = cos(latitude);
  // Compute an interior point.
}

GeoDegenerateLatitudeZone::GeoDegenerateLatitudeZone(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoDegenerateLatitudeZone(planetModel,
                                SerializableObject::readDouble(inputStream))
{
}

void GeoDegenerateLatitudeZone::write(
    shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, latitude);
}

shared_ptr<GeoBBox> GeoDegenerateLatitudeZone::expand(double const angle)
{
  double newTopLat = latitude + angle;
  double newBottomLat = latitude - angle;
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     -M_PI, M_PI);
}

bool GeoDegenerateLatitudeZone::isWithin(double const x, double const y,
                                         double const z)
{
  return abs(z - this->sinLatitude) < 1e-10;
}

double GeoDegenerateLatitudeZone::getRadius() { return M_PI; }

shared_ptr<GeoPoint> GeoDegenerateLatitudeZone::getCenter()
{
  // Totally arbitrary
  return interiorPoint;
}

std::deque<std::shared_ptr<GeoPoint>>
GeoDegenerateLatitudeZone::getEdgePoints()
{
  return edgePoints;
}

bool GeoDegenerateLatitudeZone::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, plane, notablePoints, planePoints, bounds);
}

bool GeoDegenerateLatitudeZone::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(plane, planePoints);
}

void GeoDegenerateLatitudeZone::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->noLongitudeBound()->addHorizontalPlane(planetModel, latitude, plane);
}

int GeoDegenerateLatitudeZone::getRelationship(shared_ptr<GeoShape> path)
{
  // Second, the shortcut of seeing whether endpoints are in/out is not going to
  // work with no area endpoints.  So we rely entirely on intersections.
  // System.out.println("Got here! latitude="+latitude+" path="+path);

  if (intersects(path)) {
    return OVERLAPS;
  }

  if (path->isWithin(interiorPoint)) {
    return CONTAINS;
  }

  return DISJOINT;
}

double GeoDegenerateLatitudeZone::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  return distanceStyle->computeDistance(planetModel, plane, x, y, z);
}

bool GeoDegenerateLatitudeZone::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoDegenerateLatitudeZone>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoDegenerateLatitudeZone> other =
      any_cast<std::shared_ptr<GeoDegenerateLatitudeZone>>(o);
  return GeoBaseBBox::equals(other) && other->latitude == latitude;
}

int GeoDegenerateLatitudeZone::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  int64_t temp = Double::doubleToLongBits(latitude);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring GeoDegenerateLatitudeZone::toString()
{
  return L"GeoDegenerateLatitudeZone: {planetmodel=" + planetModel + L", lat=" +
         to_wstring(latitude) + L"(" + to_wstring(latitude * 180.0 / M_PI) +
         L")}";
}
} // namespace org::apache::lucene::spatial3d::geom