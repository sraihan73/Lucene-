using namespace std;

#include "GeoRectangle.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoRectangle::GeoRectangle(shared_ptr<PlanetModel> planetModel,
                           double const topLat, double const bottomLat,
                           double const leftLon, double rightLon)
    : GeoBaseBBox(planetModel), topLat(topLat), bottomLat(bottomLat),
      leftLon(leftLon), rightLon(rightLon), cosMiddleLat(cos(middleLat)),
      ULHC(make_shared<GeoPoint>(planetModel, sinTopLat, sinLeftLon, cosTopLat,
                                 cosLeftLon, topLat, leftLon)),
      URHC(make_shared<GeoPoint>(planetModel, sinTopLat, sinRightLon, cosTopLat,
                                 cosRightLon, topLat, rightLon)),
      LRHC(make_shared<GeoPoint>(planetModel, sinBottomLat, sinRightLon,
                                 cosBottomLat, cosRightLon, bottomLat,
                                 rightLon)),
      LLHC(make_shared<GeoPoint>(planetModel, sinBottomLat, sinLeftLon,
                                 cosBottomLat, cosLeftLon, bottomLat, leftLon)),
      topPlane(make_shared<SidedPlane>(centerPoint, planetModel, sinTopLat)),
      bottomPlane(
          make_shared<SidedPlane>(centerPoint, planetModel, sinBottomLat)),
      leftPlane(make_shared<SidedPlane>(centerPoint, cosLeftLon, sinLeftLon)),
      rightPlane(
          make_shared<SidedPlane>(centerPoint, cosRightLon, sinRightLon)),
      topPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{ULHC, URHC}),
      bottomPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{LLHC, LRHC}),
      leftPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{ULHC, LLHC}),
      rightPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{URHC, LRHC}),
      centerPoint(make_shared<GeoPoint>(planetModel, sinMiddleLat, sinMiddleLon,
                                        cosMiddleLat, cosMiddleLon)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{ULHC})
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

  constexpr double sinTopLat = sin(topLat);
  constexpr double cosTopLat = cos(topLat);
  constexpr double sinBottomLat = sin(bottomLat);
  constexpr double cosBottomLat = cos(bottomLat);
  constexpr double sinLeftLon = sin(leftLon);
  constexpr double cosLeftLon = cos(leftLon);
  constexpr double sinRightLon = sin(rightLon);
  constexpr double cosRightLon = cos(rightLon);

  // Now build the four points

  constexpr double middleLat = (topLat + bottomLat) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
  // Normalize
  while (leftLon > rightLon) {
    rightLon += M_PI * 2.0;
  }
  constexpr double middleLon = (leftLon + rightLon) * 0.5;
  constexpr double sinMiddleLon = sin(middleLon);
  constexpr double cosMiddleLon = cos(middleLon);
}

GeoRectangle::GeoRectangle(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoRectangle(planetModel, SerializableObject::readDouble(inputStream),
                   SerializableObject::readDouble(inputStream),
                   SerializableObject::readDouble(inputStream),
                   SerializableObject::readDouble(inputStream))
{
}

void GeoRectangle::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, topLat);
  SerializableObject::writeDouble(outputStream, bottomLat);
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoRectangle::expand(double const angle)
{
  constexpr double newTopLat = topLat + angle;
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

bool GeoRectangle::isWithin(double const x, double const y, double const z)
{
  return topPlane->isWithin(x, y, z) && bottomPlane->isWithin(x, y, z) &&
         leftPlane->isWithin(x, y, z) && rightPlane->isWithin(x, y, z);
}

double GeoRectangle::getRadius()
{
  // Here we compute the distance from the middle point to one of the corners.
  // However, we need to be careful to use the longest of three distances: the
  // distance to a corner on the top; the distnace to a corner on the bottom,
  // and the distance to the right or left edge from the center.
  constexpr double centerAngle =
      (rightLon - (rightLon + leftLon) * 0.5) * cosMiddleLat;
  constexpr double topAngle = centerPoint->arcDistance(URHC);
  constexpr double bottomAngle = centerPoint->arcDistance(LLHC);
  return max(centerAngle, max(topAngle, bottomAngle));
}

std::deque<std::shared_ptr<GeoPoint>> GeoRectangle::getEdgePoints()
{
  return edgePoints;
}

shared_ptr<GeoPoint> GeoRectangle::getCenter() { return centerPoint; }

bool GeoRectangle::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, topPlane, notablePoints, topPlanePoints,
                       bounds, {bottomPlane, leftPlane, rightPlane}) ||
         p->intersects(planetModel, bottomPlane, notablePoints,
                       bottomPlanePoints, bounds,
                       {topPlane, leftPlane, rightPlane}) ||
         p->intersects(planetModel, leftPlane, notablePoints, leftPlanePoints,
                       bounds, {rightPlane, topPlane, bottomPlane}) ||
         p->intersects(planetModel, rightPlane, notablePoints, rightPlanePoints,
                       bounds, {leftPlane, topPlane, bottomPlane});
}

bool GeoRectangle::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(topPlane, topPlanePoints,
                              {bottomPlane, leftPlane, rightPlane}) ||
         geoShape->intersects(bottomPlane, bottomPlanePoints,
                              {topPlane, leftPlane, rightPlane}) ||
         geoShape->intersects(leftPlane, leftPlanePoints,
                              {rightPlane, topPlane, bottomPlane}) ||
         geoShape->intersects(rightPlane, rightPlanePoints,
                              {leftPlane, topPlane, bottomPlane});
}

void GeoRectangle::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds
      ->addHorizontalPlane(planetModel, topLat, topPlane,
                           {bottomPlane, leftPlane, rightPlane})
      ->addVerticalPlane(planetModel, rightLon, rightPlane,
                         {topPlane, bottomPlane, leftPlane})
      ->addHorizontalPlane(planetModel, bottomLat, bottomPlane,
                           {topPlane, leftPlane, rightPlane})
      ->addVerticalPlane(planetModel, leftLon, leftPlane,
                         {topPlane, bottomPlane, rightPlane})
      ->addIntersection(planetModel, leftPlane, rightPlane,
                        {topPlane, bottomPlane})
      ->addPoint(ULHC)
      ->addPoint(URHC)
      ->addPoint(LLHC)
      ->addPoint(LRHC);
}

double GeoRectangle::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                     double const x, double const y,
                                     double const z)
{
  constexpr double topDistance = distanceStyle->computeDistance(
      planetModel, topPlane, x, y, z, {bottomPlane, leftPlane, rightPlane});
  constexpr double bottomDistance = distanceStyle->computeDistance(
      planetModel, bottomPlane, x, y, z, {topPlane, leftPlane, rightPlane});
  constexpr double leftDistance = distanceStyle->computeDistance(
      planetModel, leftPlane, x, y, z, {rightPlane, topPlane, bottomPlane});
  constexpr double rightDistance = distanceStyle->computeDistance(
      planetModel, rightPlane, x, y, z, {leftPlane, topPlane, bottomPlane});

  constexpr double ULHCDistance = distanceStyle->computeDistance(ULHC, x, y, z);
  constexpr double URHCDistance = distanceStyle->computeDistance(URHC, x, y, z);
  constexpr double LRHCDistance = distanceStyle->computeDistance(LRHC, x, y, z);
  constexpr double LLHCDistance = distanceStyle->computeDistance(LLHC, x, y, z);

  return min(
      min(min(topDistance, bottomDistance), min(leftDistance, rightDistance)),
      min(min(ULHCDistance, URHCDistance), min(LRHCDistance, LLHCDistance)));
}

bool GeoRectangle::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoRectangle>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoRectangle> other = any_cast<std::shared_ptr<GeoRectangle>>(o);
  return GeoBaseBBox::equals(other) && other->ULHC->equals(ULHC) &&
         other->LRHC->equals(LRHC);
}

int GeoRectangle::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + ULHC->hashCode();
  result = 31 * result + LRHC->hashCode();
  return result;
}

wstring GeoRectangle::toString()
{
  return L"GeoRectangle: {planetmodel=" + planetModel + L", toplat=" +
         to_wstring(topLat) + L"(" + to_wstring(topLat * 180.0 / M_PI) +
         L"), bottomlat=" + to_wstring(bottomLat) + L"(" +
         to_wstring(bottomLat * 180.0 / M_PI) + L"), leftlon=" +
         to_wstring(leftLon) + L"(" + to_wstring(leftLon * 180.0 / M_PI) +
         L"), rightlon=" + to_wstring(rightLon) + L"(" +
         to_wstring(rightLon * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom