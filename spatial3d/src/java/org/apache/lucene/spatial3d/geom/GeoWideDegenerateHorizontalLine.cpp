using namespace std;

#include "GeoWideDegenerateHorizontalLine.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoWideDegenerateHorizontalLine::GeoWideDegenerateHorizontalLine(
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
      eitherBound(make_shared<EitherBound>(shared_from_this())),
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
  if (extent < M_PI) {
    throw invalid_argument(L"Width of rectangle too small");
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
  double middleLon = (leftLon + rightLon) * 0.5;
  double sinMiddleLon = sin(middleLon);
  double cosMiddleLon = cos(middleLon);
}

GeoWideDegenerateHorizontalLine::GeoWideDegenerateHorizontalLine(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoWideDegenerateHorizontalLine(
          planetModel, SerializableObject::readDouble(inputStream),
          SerializableObject::readDouble(inputStream),
          SerializableObject::readDouble(inputStream))
{
}

void GeoWideDegenerateHorizontalLine::write(
    shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, latitude);
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoWideDegenerateHorizontalLine::expand(double const angle)
{
  constexpr double newTopLat = latitude + angle;
  constexpr double newBottomLat = latitude - angle;
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

bool GeoWideDegenerateHorizontalLine::isWithin(double const x, double const y,
                                               double const z)
{
  return plane->evaluateIsZero(x, y, z) &&
         (leftPlane->isWithin(x, y, z) || rightPlane->isWithin(x, y, z));
}

double GeoWideDegenerateHorizontalLine::getRadius()
{
  // Here we compute the distance from the middle point to one of the corners.
  // However, we need to be careful to use the longest of three distances: the
  // distance to a corner on the top; the distnace to a corner on the bottom,
  // and the distance to the right or left edge from the center.
  constexpr double topAngle = centerPoint->arcDistance(RHC);
  constexpr double bottomAngle = centerPoint->arcDistance(LHC);
  return max(topAngle, bottomAngle);
}

shared_ptr<GeoPoint> GeoWideDegenerateHorizontalLine::getCenter()
{
  return centerPoint;
}

std::deque<std::shared_ptr<GeoPoint>>
GeoWideDegenerateHorizontalLine::getEdgePoints()
{
  return edgePoints;
}

bool GeoWideDegenerateHorizontalLine::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // Right and left bounds are essentially independent hemispheres; crossing
  // into the wrong part of one requires crossing into the right part of the
  // other.  So intersection can ignore the left/right bounds.
  return p->intersects(planetModel, plane, notablePoints, planePoints, bounds,
                       {eitherBound});
}

bool GeoWideDegenerateHorizontalLine::intersects(shared_ptr<GeoShape> geoShape)
{
  // Right and left bounds are essentially independent hemispheres; crossing
  // into the wrong part of one requires crossing into the right part of the
  // other.  So intersection can ignore the left/right bounds.
  return geoShape->intersects(plane, planePoints, {eitherBound});
}

void GeoWideDegenerateHorizontalLine::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->isWide()
      ->addHorizontalPlane(planetModel, latitude, plane, {eitherBound})
      ->addPoint(LHC)
      ->addPoint(RHC);
}

int GeoWideDegenerateHorizontalLine::getRelationship(shared_ptr<GeoShape> path)
{
  if (intersects(path)) {
    return OVERLAPS;
  }

  if (path->isWithin(centerPoint)) {
    return CONTAINS;
  }

  return DISJOINT;
}

double GeoWideDegenerateHorizontalLine::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  constexpr double distance = distanceStyle->computeDistance(
      planetModel, plane, x, y, z, {eitherBound});

  constexpr double LHCDistance = distanceStyle->computeDistance(LHC, x, y, z);
  constexpr double RHCDistance = distanceStyle->computeDistance(RHC, x, y, z);

  return min(distance, min(LHCDistance, RHCDistance));
}

bool GeoWideDegenerateHorizontalLine::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoWideDegenerateHorizontalLine>(o) !=
        nullptr)) {
    return false;
  }
  shared_ptr<GeoWideDegenerateHorizontalLine> other =
      any_cast<std::shared_ptr<GeoWideDegenerateHorizontalLine>>(o);
  return GeoBaseBBox::equals(other) && other->LHC->equals(LHC) &&
         other->RHC->equals(RHC);
}

int GeoWideDegenerateHorizontalLine::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + LHC->hashCode();
  result = 31 * result + RHC->hashCode();
  return result;
}

wstring GeoWideDegenerateHorizontalLine::toString()
{
  return L"GeoWideDegenerateHorizontalLine: {planetmodel=" + planetModel +
         L", latitude=" + to_wstring(latitude) + L"(" +
         to_wstring(latitude * 180.0 / M_PI) + L"), leftlon=" +
         to_wstring(leftLon) + L"(" + to_wstring(leftLon * 180.0 / M_PI) +
         L"), rightLon=" + to_wstring(rightLon) + L"(" +
         to_wstring(rightLon * 180.0 / M_PI) + L")}";
}

GeoWideDegenerateHorizontalLine::EitherBound::EitherBound(
    shared_ptr<GeoWideDegenerateHorizontalLine> outerInstance)
    : outerInstance(outerInstance)
{
}

bool GeoWideDegenerateHorizontalLine::EitherBound::isWithin(double const x,
                                                            double const y,
                                                            double const z)
{
  return outerInstance->leftPlane->isWithin(x, y, z) ||
         outerInstance->rightPlane->isWithin(x, y, z);
}
} // namespace org::apache::lucene::spatial3d::geom