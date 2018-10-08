using namespace std;

#include "GeoDegenerateHorizontalLine.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoDegenerateHorizontalLine::GeoDegenerateHorizontalLine(
    shared_ptr<PlanetModel> planetModel, double const latitude,
    double const leftLon, double rightLon)
    : GeoBaseBBox(planetModel), latitude(latitude), leftLon(leftLon),
      rightLon(rightLon),
      LHC(make_shared<GeoPoint>(planetModel, sinLatitude, sinLeftLon,
                                cosLatitude, cosLeftLon, latitude, leftLon)),
      RHC(make_shared<GeoPoint>(planetModel, sinLatitude, sinRightLon,
                                cosLatitude, cosRightLon, latitude, rightLon)),
      plane(make_shared<Plane>(planetModel, sinLatitude)),
      leftPlane(make_shared<SidedPlane>(centerPoint, cosLeftLon, sinLeftLon)),
      rightPlane(
          make_shared<SidedPlane>(centerPoint, cosRightLon, sinRightLon)),
      planePoints(std::deque<std::shared_ptr<GeoPoint>>{LHC, RHC}),
      centerPoint(make_shared<GeoPoint>(planetModel, sinLatitude, sinMiddleLon,
                                        cosLatitude, cosMiddleLon)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{centerPoint})
{
  // Argument checking
  if (latitude > M_PI * 0.5 || latitude < -M_PI * 0.5) {
    throw invalid_argument(L"Latitude out of range");
  }
  if (leftLon < -M_PI || leftLon > M_PI) {
    throw invalid_argument(L"Left longitude out of range");
  }
  if (rightLon < -M_PI || rightLon > M_PI) {
    throw invalid_argument(L"Right longitude out of range");
  }
  double extent = rightLon - leftLon;
  if (extent < 0.0) {
    extent += 2.0 * M_PI;
  }
  if (extent > M_PI) {
    throw invalid_argument(L"Width of rectangle too great");
  }

  constexpr double sinLatitude = sin(latitude);
  constexpr double cosLatitude = cos(latitude);
  constexpr double sinLeftLon = sin(leftLon);
  constexpr double cosLeftLon = cos(leftLon);
  constexpr double sinRightLon = sin(rightLon);
  constexpr double cosRightLon = cos(rightLon);

  // Now build the two points

  // Normalize
  while (leftLon > rightLon) {
    rightLon += M_PI * 2.0;
  }
  constexpr double middleLon = (leftLon + rightLon) * 0.5;
  constexpr double sinMiddleLon = sin(middleLon);
  constexpr double cosMiddleLon = cos(middleLon);
}

GeoDegenerateHorizontalLine::GeoDegenerateHorizontalLine(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoDegenerateHorizontalLine(planetModel,
                                  SerializableObject::readDouble(inputStream),
                                  SerializableObject::readDouble(inputStream),
                                  SerializableObject::readDouble(inputStream))
{
}

void GeoDegenerateHorizontalLine::write(
    shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, latitude);
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoDegenerateHorizontalLine::expand(double const angle)
{
  double newTopLat = latitude + angle;
  double newBottomLat = latitude - angle;
  // Figuring out when we escalate to a special case requires some prefiguring
  double currentLonSpan = rightLon - leftLon;
  if (currentLonSpan < 0.0) {
    currentLonSpan += M_PI * 2.0;
  }
  double newLeftLon = leftLon - angle;
  double newRightLon = rightLon + angle;
  if (currentLonSpan + 2.0 * angle >= M_PI * 2.0) {
    newLeftLon = -M_PI;
    newRightLon = M_PI;
  }
  return GeoBBoxFactory::makeGeoBBox(planetModel, newTopLat, newBottomLat,
                                     newLeftLon, newRightLon);
}

bool GeoDegenerateHorizontalLine::isWithin(double const x, double const y,
                                           double const z)
{
  return plane->evaluateIsZero(x, y, z) && leftPlane->isWithin(x, y, z) &&
         rightPlane->isWithin(x, y, z);
}

double GeoDegenerateHorizontalLine::getRadius()
{
  double topAngle = centerPoint->arcDistance(RHC);
  double bottomAngle = centerPoint->arcDistance(LHC);
  return max(topAngle, bottomAngle);
}

shared_ptr<GeoPoint> GeoDegenerateHorizontalLine::getCenter()
{
  return centerPoint;
}

std::deque<std::shared_ptr<GeoPoint>>
GeoDegenerateHorizontalLine::getEdgePoints()
{
  return edgePoints;
}

bool GeoDegenerateHorizontalLine::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, plane, notablePoints, planePoints, bounds,
                       {leftPlane, rightPlane});
}

bool GeoDegenerateHorizontalLine::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(plane, planePoints, {leftPlane, rightPlane});
}

void GeoDegenerateHorizontalLine::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds
      ->addHorizontalPlane(planetModel, latitude, plane,
                           {leftPlane, rightPlane})
      ->addPoint(LHC)
      ->addPoint(RHC);
}

int GeoDegenerateHorizontalLine::getRelationship(shared_ptr<GeoShape> path)
{
  // System.err.println("getting relationship between "+this+" and "+path);
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

double GeoDegenerateHorizontalLine::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  constexpr double distance = distanceStyle->computeDistance(
      planetModel, plane, x, y, z, {leftPlane, rightPlane});

  constexpr double LHCDistance = distanceStyle->computeDistance(LHC, x, y, z);
  constexpr double RHCDistance = distanceStyle->computeDistance(RHC, x, y, z);

  return min(distance, min(LHCDistance, RHCDistance));
}

bool GeoDegenerateHorizontalLine::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoDegenerateHorizontalLine>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoDegenerateHorizontalLine> other =
      any_cast<std::shared_ptr<GeoDegenerateHorizontalLine>>(o);
  return GeoBaseBBox::equals(other) && other->LHC->equals(LHC) &&
         other->RHC->equals(RHC);
}

int GeoDegenerateHorizontalLine::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + LHC->hashCode();
  result = 31 * result + RHC->hashCode();
  return result;
}

wstring GeoDegenerateHorizontalLine::toString()
{
  return L"GeoDegenerateHorizontalLine: {planetmodel=" + planetModel +
         L", latitude=" + to_wstring(latitude) + L"(" +
         to_wstring(latitude * 180.0 / M_PI) + L"), leftlon=" +
         to_wstring(leftLon) + L"(" + to_wstring(leftLon * 180.0 / M_PI) +
         L"), rightLon=" + to_wstring(rightLon) + L"(" +
         to_wstring(rightLon * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom