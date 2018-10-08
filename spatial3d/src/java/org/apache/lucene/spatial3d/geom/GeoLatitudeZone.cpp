using namespace std;

#include "GeoLatitudeZone.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const GeoLatitudeZone::planePoints =
    std::deque<std::shared_ptr<GeoPoint>>(0);

GeoLatitudeZone::GeoLatitudeZone(shared_ptr<PlanetModel> planetModel,
                                 double const topLat, double const bottomLat)
    : GeoBaseBBox(planetModel), topLat(topLat), bottomLat(bottomLat),
      cosTopLat(cos(topLat)), cosBottomLat(cos(bottomLat)),
      topPlane(make_shared<SidedPlane>(interiorPoint, planetModel, sinTopLat)),
      bottomPlane(
          make_shared<SidedPlane>(interiorPoint, planetModel, sinBottomLat)),
      interiorPoint(
          make_shared<GeoPoint>(planetModel, sinMiddleLat, 0.0,
                                sqrt(1.0 - sinMiddleLat * sinMiddleLat), 1.0)),
      topBoundaryPoint(make_shared<GeoPoint>(
          planetModel, sinTopLat, 0.0, sqrt(1.0 - sinTopLat * sinTopLat), 1.0)),
      bottomBoundaryPoint(
          make_shared<GeoPoint>(planetModel, sinBottomLat, 0.0,
                                sqrt(1.0 - sinBottomLat * sinBottomLat), 1.0)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{topBoundaryPoint,
                                                        bottomBoundaryPoint})
{

  constexpr double sinTopLat = sin(topLat);
  constexpr double sinBottomLat = sin(bottomLat);

  // Compute an interior point.  Pick one whose lat is between top and bottom.
  constexpr double middleLat = (topLat + bottomLat) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
}

GeoLatitudeZone::GeoLatitudeZone(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoLatitudeZone(planetModel, SerializableObject::readDouble(inputStream),
                      SerializableObject::readDouble(inputStream))
{
}

void GeoLatitudeZone::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, topLat);
  SerializableObject::writeDouble(outputStream, bottomLat);
}

shared_ptr<GeoBBox> GeoLatitudeZone::expand(double const angle)
{
  constexpr double newTopLat = topLat + angle;
  constexpr double newBottomLat = bottomLat - angle;
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     -M_PI, M_PI);
}

bool GeoLatitudeZone::isWithin(double const x, double const y, double const z)
{
  return topPlane->isWithin(x, y, z) && bottomPlane->isWithin(x, y, z);
}

double GeoLatitudeZone::getRadius()
{
  // This is a bit tricky.  I guess we should interpret this as meaning the
  // angle of a circle that would contain all the bounding box points, when
  // starting in the "center".
  if (topLat > 0.0 && bottomLat < 0.0) {
    return M_PI;
  }
  double maxCosLat = cosTopLat;
  if (maxCosLat < cosBottomLat) {
    maxCosLat = cosBottomLat;
  }
  return maxCosLat * M_PI;
}

shared_ptr<GeoPoint> GeoLatitudeZone::getCenter()
{
  // This is totally arbitrary and only a cartesian could agree with it.
  return interiorPoint;
}

std::deque<std::shared_ptr<GeoPoint>> GeoLatitudeZone::getEdgePoints()
{
  return edgePoints;
}

bool GeoLatitudeZone::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, topPlane, notablePoints, planePoints,
                       bounds, {bottomPlane}) ||
         p->intersects(planetModel, bottomPlane, notablePoints, planePoints,
                       bounds, {topPlane});
}

bool GeoLatitudeZone::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(topPlane, planePoints, {bottomPlane}) ||
         geoShape->intersects(bottomPlane, planePoints, {topPlane});
}

void GeoLatitudeZone::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->noLongitudeBound()
      ->addHorizontalPlane(planetModel, topLat, topPlane)
      .addHorizontalPlane(planetModel, bottomLat, bottomPlane);
}

double GeoLatitudeZone::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                        double const x, double const y,
                                        double const z)
{
  constexpr double topDistance = distanceStyle->computeDistance(
      planetModel, topPlane, x, y, z, {bottomPlane});
  constexpr double bottomDistance = distanceStyle->computeDistance(
      planetModel, bottomPlane, x, y, z, {topPlane});

  return min(topDistance, bottomDistance);
}

bool GeoLatitudeZone::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoLatitudeZone>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoLatitudeZone> other =
      any_cast<std::shared_ptr<GeoLatitudeZone>>(o);
  return GeoBaseBBox::equals(other) &&
         other->topBoundaryPoint->equals(topBoundaryPoint) &&
         other->bottomBoundaryPoint->equals(bottomBoundaryPoint);
}

int GeoLatitudeZone::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + topBoundaryPoint->hashCode();
  result = 31 * result + bottomBoundaryPoint->hashCode();
  return result;
}

wstring GeoLatitudeZone::toString()
{
  return L"GeoLatitudeZone: {planetmodel=" + planetModel + L", toplat=" +
         to_wstring(topLat) + L"(" + to_wstring(topLat * 180.0 / M_PI) +
         L"), bottomlat=" + to_wstring(bottomLat) + L"(" +
         to_wstring(bottomLat * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom