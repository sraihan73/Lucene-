using namespace std;

#include "GeoNorthRectangle.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoNorthRectangle::GeoNorthRectangle(shared_ptr<PlanetModel> planetModel,
                                     double const bottomLat,
                                     double const leftLon, double rightLon)
    : GeoBaseBBox(planetModel), bottomLat(bottomLat), leftLon(leftLon),
      rightLon(rightLon), cosMiddleLat(cos(middleLat)),
      LRHC(make_shared<GeoPoint>(planetModel, sinBottomLat, sinRightLon,
                                 cosBottomLat, cosRightLon, bottomLat,
                                 rightLon)),
      LLHC(make_shared<GeoPoint>(planetModel, sinBottomLat, sinLeftLon,
                                 cosBottomLat, cosLeftLon, bottomLat, leftLon)),
      bottomPlane(
          make_shared<SidedPlane>(centerPoint, planetModel, sinBottomLat)),
      leftPlane(make_shared<SidedPlane>(centerPoint, cosLeftLon, sinLeftLon)),
      rightPlane(
          make_shared<SidedPlane>(centerPoint, cosRightLon, sinRightLon)),
      bottomPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{LLHC, LRHC}),
      leftPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{
          planetModel->NORTH_POLE, LLHC}),
      rightPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{
          planetModel->NORTH_POLE, LRHC}),
      centerPoint(make_shared<GeoPoint>(planetModel, sinMiddleLat, sinMiddleLon,
                                        cosMiddleLat, cosMiddleLon)),
      edgePoints(
          std::deque<std::shared_ptr<GeoPoint>>{planetModel->NORTH_POLE})
{
  // Argument checking
  if (bottomLat > M_PI * 0.5 || bottomLat < -M_PI * 0.5) {
    throw invalid_argument(L"Bottom latitude out of range");
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

  constexpr double sinBottomLat = sin(bottomLat);
  constexpr double cosBottomLat = cos(bottomLat);
  constexpr double sinLeftLon = sin(leftLon);
  constexpr double cosLeftLon = cos(leftLon);
  constexpr double sinRightLon = sin(rightLon);
  constexpr double cosRightLon = cos(rightLon);

  // Now build the points

  constexpr double middleLat = (M_PI * 0.5 + bottomLat) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
  // Normalize
  while (leftLon > rightLon) {
    rightLon += M_PI * 2.0;
  }
  constexpr double middleLon = (leftLon + rightLon) * 0.5;
  constexpr double sinMiddleLon = sin(middleLon);
  constexpr double cosMiddleLon = cos(middleLon);
}

GeoNorthRectangle::GeoNorthRectangle(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoNorthRectangle(planetModel,
                        SerializableObject::readDouble(inputStream),
                        SerializableObject::readDouble(inputStream),
                        SerializableObject::readDouble(inputStream))
{
}

void GeoNorthRectangle::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, bottomLat);
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoNorthRectangle::expand(double const angle)
{
  constexpr double newTopLat = M_PI * 0.5;
  constexpr double newBottomLat = bottomLat - angle;
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

bool GeoNorthRectangle::isWithin(double const x, double const y, double const z)
{
  return bottomPlane->isWithin(x, y, z) && leftPlane->isWithin(x, y, z) &&
         rightPlane->isWithin(x, y, z);
}

double GeoNorthRectangle::getRadius()
{
  // Here we compute the distance from the middle point to one of the corners.
  // However, we need to be careful to use the longest of three distances: the
  // distance to a corner on the top; the distnace to a corner on the bottom,
  // and the distance to the right or left edge from the center.
  constexpr double centerAngle =
      (rightLon - (rightLon + leftLon) * 0.5) * cosMiddleLat;
  constexpr double bottomAngle = centerPoint->arcDistance(LLHC);
  return max(centerAngle, bottomAngle);
}

std::deque<std::shared_ptr<GeoPoint>> GeoNorthRectangle::getEdgePoints()
{
  return edgePoints;
}

shared_ptr<GeoPoint> GeoNorthRectangle::getCenter() { return centerPoint; }

bool GeoNorthRectangle::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, bottomPlane, notablePoints,
                       bottomPlanePoints, bounds, {leftPlane, rightPlane}) ||
         p->intersects(planetModel, leftPlane, notablePoints, leftPlanePoints,
                       bounds, {rightPlane, bottomPlane}) ||
         p->intersects(planetModel, rightPlane, notablePoints, rightPlanePoints,
                       bounds, {leftPlane, bottomPlane});
}

bool GeoNorthRectangle::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(bottomPlane, bottomPlanePoints,
                              {leftPlane, rightPlane}) ||
         geoShape->intersects(leftPlane, leftPlanePoints,
                              {rightPlane, bottomPlane}) ||
         geoShape->intersects(rightPlane, rightPlanePoints,
                              {leftPlane, bottomPlane});
}

void GeoNorthRectangle::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds
      ->addHorizontalPlane(planetModel, bottomLat, bottomPlane,
                           {leftPlane, rightPlane})
      ->addVerticalPlane(planetModel, leftLon, leftPlane,
                         {bottomPlane, rightPlane})
      ->addVerticalPlane(planetModel, rightLon, rightPlane,
                         {bottomPlane, leftPlane})
      ->addIntersection(planetModel, rightPlane, leftPlane, {bottomPlane})
      ->addPoint(LLHC)
      ->addPoint(LRHC)
      ->addPoint(planetModel->NORTH_POLE);
}

double
GeoNorthRectangle::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  constexpr double bottomDistance = distanceStyle->computeDistance(
      planetModel, bottomPlane, x, y, z, {leftPlane, rightPlane});
  constexpr double leftDistance = distanceStyle->computeDistance(
      planetModel, leftPlane, x, y, z, {rightPlane, bottomPlane});
  constexpr double rightDistance = distanceStyle->computeDistance(
      planetModel, rightPlane, x, y, z, {leftPlane, bottomPlane});

  constexpr double LRHCDistance = distanceStyle->computeDistance(LRHC, x, y, z);
  constexpr double LLHCDistance = distanceStyle->computeDistance(LLHC, x, y, z);

  return min(bottomDistance, min(min(leftDistance, rightDistance),
                                 min(LRHCDistance, LLHCDistance)));
}

bool GeoNorthRectangle::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoNorthRectangle>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoNorthRectangle> other =
      any_cast<std::shared_ptr<GeoNorthRectangle>>(o);
  return GeoBaseBBox::equals(other) && other->LLHC->equals(LLHC) &&
         other->LRHC->equals(LRHC);
}

int GeoNorthRectangle::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + LLHC->hashCode();
  result = 31 * result + LRHC->hashCode();
  return result;
}

wstring GeoNorthRectangle::toString()
{
  return L"GeoNorthRectangle: {planetmodel=" + planetModel + L", bottomlat=" +
         to_wstring(bottomLat) + L"(" + to_wstring(bottomLat * 180.0 / M_PI) +
         L"), leftlon=" + to_wstring(leftLon) + L"(" +
         to_wstring(leftLon * 180.0 / M_PI) + L"), rightlon=" +
         to_wstring(rightLon) + L"(" + to_wstring(rightLon * 180.0 / M_PI) +
         L")}";
}
} // namespace org::apache::lucene::spatial3d::geom