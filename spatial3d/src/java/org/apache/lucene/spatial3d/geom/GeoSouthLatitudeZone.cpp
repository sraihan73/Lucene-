using namespace std;

#include "GeoSouthLatitudeZone.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const GeoSouthLatitudeZone::planePoints =
    std::deque<std::shared_ptr<GeoPoint>>(0);

GeoSouthLatitudeZone::GeoSouthLatitudeZone(shared_ptr<PlanetModel> planetModel,
                                           double const topLat)
    : GeoBaseBBox(planetModel), topLat(topLat), cosTopLat(cos(topLat)),
      topPlane(make_shared<SidedPlane>(interiorPoint, planetModel, sinTopLat)),
      interiorPoint(
          make_shared<GeoPoint>(planetModel, sinMiddleLat, 0.0,
                                sqrt(1.0 - sinMiddleLat * sinMiddleLat), 1.0)),
      topBoundaryPoint(make_shared<GeoPoint>(
          planetModel, sinTopLat, 0.0, sqrt(1.0 - sinTopLat * sinTopLat), 1.0)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{topBoundaryPoint})
{

  constexpr double sinTopLat = sin(topLat);

  // Compute an interior point.  Pick one whose lat is between top and bottom.
  constexpr double middleLat = (topLat - M_PI * 0.5) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
}

GeoSouthLatitudeZone::GeoSouthLatitudeZone(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoSouthLatitudeZone(planetModel,
                           SerializableObject::readDouble(inputStream))
{
}

void GeoSouthLatitudeZone::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, topLat);
}

shared_ptr<GeoBBox> GeoSouthLatitudeZone::expand(double const angle)
{
  constexpr double newTopLat = topLat + angle;
  constexpr double newBottomLat = -M_PI * 0.5;
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     -M_PI, M_PI);
}

bool GeoSouthLatitudeZone::isWithin(double const x, double const y,
                                    double const z)
{
  return topPlane->isWithin(x, y, z);
}

double GeoSouthLatitudeZone::getRadius()
{
  // This is a bit tricky.  I guess we should interpret this as meaning the
  // angle of a circle that would contain all the bounding box points, when
  // starting in the "center".
  if (topLat > 0.0) {
    return M_PI;
  }
  double maxCosLat = cosTopLat;
  return maxCosLat * M_PI;
}

shared_ptr<GeoPoint> GeoSouthLatitudeZone::getCenter() { return interiorPoint; }

std::deque<std::shared_ptr<GeoPoint>> GeoSouthLatitudeZone::getEdgePoints()
{
  return edgePoints;
}

bool GeoSouthLatitudeZone::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, topPlane, notablePoints, planePoints,
                       bounds);
}

bool GeoSouthLatitudeZone::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(topPlane, planePoints);
}

void GeoSouthLatitudeZone::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->addHorizontalPlane(planetModel, topLat, topPlane);
}

double
GeoSouthLatitudeZone::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                      double const x, double const y,
                                      double const z)
{
  return distanceStyle->computeDistance(planetModel, topPlane, x, y, z);
}

bool GeoSouthLatitudeZone::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoSouthLatitudeZone>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoSouthLatitudeZone> other =
      any_cast<std::shared_ptr<GeoSouthLatitudeZone>>(o);
  return GeoBaseBBox::equals(other) &&
         other->topBoundaryPoint->equals(topBoundaryPoint);
}

int GeoSouthLatitudeZone::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + topBoundaryPoint->hashCode();
  return result;
}

wstring GeoSouthLatitudeZone::toString()
{
  return L"GeoSouthLatitudeZone: {planetmodel=" + planetModel + L", toplat=" +
         to_wstring(topLat) + L"(" + to_wstring(topLat * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom