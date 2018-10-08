using namespace std;

#include "GeoDegenerateVerticalLine.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoDegenerateVerticalLine::GeoDegenerateVerticalLine(
    shared_ptr<PlanetModel> planetModel, double const topLat,
    double const bottomLat, double const longitude)
    : GeoBaseBBox(planetModel), topLat(topLat), bottomLat(bottomLat),
      longitude(longitude),
      UHC(make_shared<GeoPoint>(planetModel, sinTopLat, sinLongitude, cosTopLat,
                                cosLongitude, topLat, longitude)),
      LHC(make_shared<GeoPoint>(planetModel, sinBottomLat, sinLongitude,
                                cosBottomLat, cosLongitude, bottomLat,
                                longitude)),
      topPlane(make_shared<SidedPlane>(centerPoint, planetModel, sinTopLat)),
      bottomPlane(
          make_shared<SidedPlane>(centerPoint, planetModel, sinBottomLat)),
      boundingPlane(
          make_shared<SidedPlane>(centerPoint, -sinLongitude, cosLongitude)),
      plane(make_shared<Plane>(cosLongitude, sinLongitude)),
      planePoints(std::deque<std::shared_ptr<GeoPoint>>{UHC, LHC}),
      centerPoint(make_shared<GeoPoint>(planetModel, sinMiddleLat, sinLongitude,
                                        cosMiddleLat, cosLongitude)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{centerPoint})
{
  // Argument checking
  if (topLat > M_PI * 0.5 || topLat < -M_PI * 0.5) {
    throw invalid_argument(L"Top latitude out of range");
  }
  if (bottomLat > M_PI * 0.5 || bottomLat < -M_PI * 0.5) {
    throw invalid_argument(L"Bottom latitude out of range");
  }
  if (topLat < bottomLat) {
    throw invalid_argument(L"Top latitude less than bottom latitude");
  }
  if (longitude < -M_PI || longitude > M_PI) {
    throw invalid_argument(L"Longitude out of range");
  }

  constexpr double sinTopLat = sin(topLat);
  constexpr double cosTopLat = cos(topLat);
  constexpr double sinBottomLat = sin(bottomLat);
  constexpr double cosBottomLat = cos(bottomLat);
  constexpr double sinLongitude = sin(longitude);
  constexpr double cosLongitude = cos(longitude);

  // Now build the two points

  constexpr double middleLat = (topLat + bottomLat) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
  constexpr double cosMiddleLat = cos(middleLat);
}

GeoDegenerateVerticalLine::GeoDegenerateVerticalLine(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoDegenerateVerticalLine(planetModel,
                                SerializableObject::readDouble(inputStream),
                                SerializableObject::readDouble(inputStream),
                                SerializableObject::readDouble(inputStream))
{
}

void GeoDegenerateVerticalLine::write(
    shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, topLat);
  SerializableObject::writeDouble(outputStream, bottomLat);
  SerializableObject::writeDouble(outputStream, longitude);
}

shared_ptr<GeoBBox> GeoDegenerateVerticalLine::expand(double const angle)
{
  constexpr double newTopLat = topLat + angle;
  constexpr double newBottomLat = bottomLat - angle;
  double newLeftLon = longitude - angle;
  double newRightLon = longitude + angle;
  double currentLonSpan = 2.0 * angle;
  if (currentLonSpan + 2.0 * angle >= M_PI * 2.0) {
    newLeftLon = -M_PI;
    newRightLon = M_PI;
  }
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     newLeftLon, newRightLon);
}

bool GeoDegenerateVerticalLine::isWithin(double const x, double const y,
                                         double const z)
{
  return plane->evaluateIsZero(x, y, z) && boundingPlane->isWithin(x, y, z) &&
         topPlane->isWithin(x, y, z) && bottomPlane->isWithin(x, y, z);
}

double GeoDegenerateVerticalLine::getRadius()
{
  // Here we compute the distance from the middle point to one of the corners.
  // However, we need to be careful to use the longest of three distances: the
  // distance to a corner on the top; the distnace to a corner on the bottom,
  // and the distance to the right or left edge from the center.
  constexpr double topAngle = centerPoint->arcDistance(UHC);
  constexpr double bottomAngle = centerPoint->arcDistance(LHC);
  return max(topAngle, bottomAngle);
}

shared_ptr<GeoPoint> GeoDegenerateVerticalLine::getCenter()
{
  return centerPoint;
}

std::deque<std::shared_ptr<GeoPoint>>
GeoDegenerateVerticalLine::getEdgePoints()
{
  return edgePoints;
}

bool GeoDegenerateVerticalLine::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, plane, notablePoints, planePoints, bounds,
                       {boundingPlane, topPlane, bottomPlane});
}

bool GeoDegenerateVerticalLine::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(plane, planePoints,
                              {boundingPlane, topPlane, bottomPlane});
}

void GeoDegenerateVerticalLine::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds
      ->addVerticalPlane(planetModel, longitude, plane,
                         {boundingPlane, topPlane, bottomPlane})
      ->addPoint(UHC)
      ->addPoint(LHC);
}

int GeoDegenerateVerticalLine::getRelationship(shared_ptr<GeoShape> path)
{
  // System.err.println(this+" relationship to "+path);
  if (intersects(path)) {
    // System.err.println(" overlaps");
    return OVERLAPS;
  }

  if (path->isWithin(centerPoint)) {
    // System.err.println(" contains");
    return CONTAINS;
  }

  // System.err.println(" disjoint");
  return DISJOINT;
}

double GeoDegenerateVerticalLine::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  constexpr double distance = distanceStyle->computeDistance(
      planetModel, plane, x, y, z, {topPlane, bottomPlane, boundingPlane});

  constexpr double UHCDistance = distanceStyle->computeDistance(UHC, x, y, z);
  constexpr double LHCDistance = distanceStyle->computeDistance(LHC, x, y, z);

  return min(distance, min(UHCDistance, LHCDistance));
}

bool GeoDegenerateVerticalLine::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoDegenerateVerticalLine>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoDegenerateVerticalLine> other =
      any_cast<std::shared_ptr<GeoDegenerateVerticalLine>>(o);
  return GeoBaseBBox::equals(other) && other->UHC->equals(UHC) &&
         other->LHC->equals(LHC);
}

int GeoDegenerateVerticalLine::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + UHC->hashCode();
  result = 31 * result + LHC->hashCode();
  return result;
}

wstring GeoDegenerateVerticalLine::toString()
{
  return L"GeoDegenerateVerticalLine: {longitude=" + to_wstring(longitude) +
         L"(" + to_wstring(longitude * 180.0 / M_PI) + L"), toplat=" +
         to_wstring(topLat) + L"(" + to_wstring(topLat * 180.0 / M_PI) +
         L"), bottomlat=" + to_wstring(bottomLat) + L"(" +
         to_wstring(bottomLat * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom