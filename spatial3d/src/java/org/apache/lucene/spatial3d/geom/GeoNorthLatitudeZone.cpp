using namespace std;

#include "GeoNorthLatitudeZone.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const GeoNorthLatitudeZone::planePoints =
    std::deque<std::shared_ptr<GeoPoint>>(0);

GeoNorthLatitudeZone::GeoNorthLatitudeZone(shared_ptr<PlanetModel> planetModel,
                                           double const bottomLat)
    : GeoBaseBBox(planetModel), bottomLat(bottomLat),
      cosBottomLat(cos(bottomLat)),
      bottomPlane(
          make_shared<SidedPlane>(interiorPoint, planetModel, sinBottomLat)),
      interiorPoint(
          make_shared<GeoPoint>(planetModel, sinMiddleLat, 0.0,
                                sqrt(1.0 - sinMiddleLat * sinMiddleLat), 1.0)),
      bottomBoundaryPoint(
          make_shared<GeoPoint>(planetModel, sinBottomLat, 0.0,
                                sqrt(1.0 - sinBottomLat * sinBottomLat), 1.0)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{bottomBoundaryPoint})
{

  constexpr double sinBottomLat = sin(bottomLat);

  // Compute an interior point.  Pick one whose lat is between top and bottom.
  constexpr double middleLat = (M_PI * 0.5 + bottomLat) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
}

GeoNorthLatitudeZone::GeoNorthLatitudeZone(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoNorthLatitudeZone(planetModel,
                           SerializableObject::readDouble(inputStream))
{
}

void GeoNorthLatitudeZone::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, bottomLat);
}

shared_ptr<GeoBBox> GeoNorthLatitudeZone::expand(double const angle)
{
  constexpr double newTopLat = M_PI * 0.5;
  constexpr double newBottomLat = bottomLat - angle;
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     -M_PI, M_PI);
}

bool GeoNorthLatitudeZone::isWithin(double const x, double const y,
                                    double const z)
{
  return bottomPlane->isWithin(x, y, z);
}

double GeoNorthLatitudeZone::getRadius()
{
  // This is a bit tricky.  I guess we should interpret this as meaning the
  // angle of a circle that would contain all the bounding box points, when
  // starting in the "center".
  if (bottomLat < 0.0) {
    return M_PI;
  }
  double maxCosLat = cosBottomLat;
  return maxCosLat * M_PI;
}

shared_ptr<GeoPoint> GeoNorthLatitudeZone::getCenter() { return interiorPoint; }

std::deque<std::shared_ptr<GeoPoint>> GeoNorthLatitudeZone::getEdgePoints()
{
  return edgePoints;
}

bool GeoNorthLatitudeZone::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, bottomPlane, notablePoints, planePoints,
                       bounds);
}

bool GeoNorthLatitudeZone::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(bottomPlane, planePoints);
}

void GeoNorthLatitudeZone::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->addHorizontalPlane(planetModel, bottomLat, bottomPlane);
}

double
GeoNorthLatitudeZone::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                      double const x, double const y,
                                      double const z)
{
  return distanceStyle->computeDistance(planetModel, bottomPlane, x, y, z);
}

bool GeoNorthLatitudeZone::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoNorthLatitudeZone>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoNorthLatitudeZone> other =
      any_cast<std::shared_ptr<GeoNorthLatitudeZone>>(o);
  return GeoBaseBBox::equals(other) &&
         other->bottomBoundaryPoint->equals(bottomBoundaryPoint);
}

int GeoNorthLatitudeZone::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + bottomBoundaryPoint->hashCode();
  return result;
}

wstring GeoNorthLatitudeZone::toString()
{
  return L"GeoNorthLatitudeZone: {planetmodel=" + planetModel +
         L", bottomlat=" + to_wstring(bottomLat) + L"(" +
         to_wstring(bottomLat * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom