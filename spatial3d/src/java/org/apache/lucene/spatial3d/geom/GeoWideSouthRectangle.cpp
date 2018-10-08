using namespace std;

#include "GeoWideSouthRectangle.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoWideSouthRectangle::GeoWideSouthRectangle(
    shared_ptr<PlanetModel> planetModel, double const topLat,
    double const leftLon, double rightLon)
    : GeoBaseBBox(planetModel), topLat(topLat), leftLon(leftLon),
      rightLon(rightLon), cosMiddleLat(cos(middleLat)),
      ULHC(make_shared<GeoPoint>(planetModel, sinTopLat, sinLeftLon, cosTopLat,
                                 cosLeftLon, topLat, leftLon)),
      URHC(make_shared<GeoPoint>(planetModel, sinTopLat, sinRightLon, cosTopLat,
                                 cosRightLon, topLat, rightLon)),
      topPlane(make_shared<SidedPlane>(centerPoint, planetModel, sinTopLat)),
      leftPlane(make_shared<SidedPlane>(centerPoint, cosLeftLon, sinLeftLon)),
      rightPlane(
          make_shared<SidedPlane>(centerPoint, cosRightLon, sinRightLon)),
      topPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{ULHC, URHC}),
      leftPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{
          ULHC, planetModel->SOUTH_POLE}),
      rightPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{
          URHC, planetModel->SOUTH_POLE}),
      centerPoint(make_shared<GeoPoint>(planetModel, sinMiddleLat, sinMiddleLon,
                                        cosMiddleLat, cosMiddleLon)),
      eitherBound(make_shared<EitherBound>(shared_from_this())),
      edgePoints(
          std::deque<std::shared_ptr<GeoPoint>>{planetModel->SOUTH_POLE})
{
  // Argument checking
  if (topLat > M_PI * 0.5 || topLat < -M_PI * 0.5) {
    throw invalid_argument(L"Top latitude out of range");
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

  constexpr double sinTopLat = sin(topLat);
  constexpr double cosTopLat = cos(topLat);
  constexpr double sinLeftLon = sin(leftLon);
  constexpr double cosLeftLon = cos(leftLon);
  constexpr double sinRightLon = sin(rightLon);
  constexpr double cosRightLon = cos(rightLon);

  // Now build the four points

  constexpr double middleLat = (topLat - M_PI * 0.5) * 0.5;
  constexpr double sinMiddleLat = sin(middleLat);
  // Normalize
  while (leftLon > rightLon) {
    rightLon += M_PI * 2.0;
  }
  constexpr double middleLon = (leftLon + rightLon) * 0.5;
  constexpr double sinMiddleLon = sin(middleLon);
  constexpr double cosMiddleLon = cos(middleLon);
}

GeoWideSouthRectangle::GeoWideSouthRectangle(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoWideSouthRectangle(planetModel,
                            SerializableObject::readDouble(inputStream),
                            SerializableObject::readDouble(inputStream),
                            SerializableObject::readDouble(inputStream))
{
}

void GeoWideSouthRectangle::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, topLat);
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoWideSouthRectangle::expand(double const angle)
{
  constexpr double newTopLat = topLat + angle;
  constexpr double newBottomLat = -M_PI * 0.5;
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

bool GeoWideSouthRectangle::isWithin(double const x, double const y,
                                     double const z)
{
  return topPlane->isWithin(x, y, z) &&
         (leftPlane->isWithin(x, y, z) || rightPlane->isWithin(x, y, z));
}

double GeoWideSouthRectangle::getRadius()
{
  // Here we compute the distance from the middle point to one of the corners.
  // However, we need to be careful to use the longest of three distances: the
  // distance to a corner on the top; the distnace to a corner on the bottom,
  // and the distance to the right or left edge from the center.
  constexpr double centerAngle =
      (rightLon - (rightLon + leftLon) * 0.5) * cosMiddleLat;
  constexpr double topAngle = centerPoint->arcDistance(URHC);
  return max(centerAngle, topAngle);
}

shared_ptr<GeoPoint> GeoWideSouthRectangle::getCenter() { return centerPoint; }

std::deque<std::shared_ptr<GeoPoint>> GeoWideSouthRectangle::getEdgePoints()
{
  return edgePoints;
}

bool GeoWideSouthRectangle::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // Right and left bounds are essentially independent hemispheres; crossing
  // into the wrong part of one requires crossing into the right part of the
  // other.  So intersection can ignore the left/right bounds.
  return p->intersects(planetModel, topPlane, notablePoints, topPlanePoints,
                       bounds, {eitherBound}) ||
         p->intersects(planetModel, leftPlane, notablePoints, leftPlanePoints,
                       bounds, {topPlane}) ||
         p->intersects(planetModel, rightPlane, notablePoints, rightPlanePoints,
                       bounds, {topPlane});
}

bool GeoWideSouthRectangle::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(topPlane, topPlanePoints, {eitherBound}) ||
         geoShape->intersects(leftPlane, leftPlanePoints, {topPlane}) ||
         geoShape->intersects(rightPlane, rightPlanePoints, {topPlane});
}

void GeoWideSouthRectangle::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->isWide()
      ->addHorizontalPlane(planetModel, topLat, topPlane, {eitherBound})
      ->addVerticalPlane(planetModel, rightLon, rightPlane, {topPlane})
      ->addVerticalPlane(planetModel, leftLon, leftPlane, {topPlane})
      ->addIntersection(planetModel, leftPlane, rightPlane, {topPlane})
      ->addPoint(ULHC)
      ->addPoint(URHC)
      ->addPoint(planetModel->SOUTH_POLE);
}

double
GeoWideSouthRectangle::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                       double const x, double const y,
                                       double const z)
{
  constexpr double topDistance = distanceStyle->computeDistance(
      planetModel, topPlane, x, y, z, {eitherBound});
  // Because the rectangle exceeds 180 degrees, it is safe to compute the
  // horizontally unbounded distance to both the left and the right and only
  // take the minimum of the two.
  constexpr double leftDistance = distanceStyle->computeDistance(
      planetModel, leftPlane, x, y, z, {topPlane});
  constexpr double rightDistance = distanceStyle->computeDistance(
      planetModel, rightPlane, x, y, z, {topPlane});

  constexpr double ULHCDistance = distanceStyle->computeDistance(ULHC, x, y, z);
  constexpr double URHCDistance = distanceStyle->computeDistance(URHC, x, y, z);

  return min(min(topDistance, min(leftDistance, rightDistance)),
             min(ULHCDistance, URHCDistance));
}

bool GeoWideSouthRectangle::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoWideSouthRectangle>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoWideSouthRectangle> other =
      any_cast<std::shared_ptr<GeoWideSouthRectangle>>(o);
  return GeoBaseBBox::equals(o) && other->ULHC->equals(ULHC) &&
         other->URHC->equals(URHC);
}

int GeoWideSouthRectangle::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + ULHC->hashCode();
  result = 31 * result + URHC->hashCode();
  return result;
}

wstring GeoWideSouthRectangle::toString()
{
  return L"GeoWideSouthRectangle: {planetmodel=" + planetModel + L", toplat=" +
         to_wstring(topLat) + L"(" + to_wstring(topLat * 180.0 / M_PI) +
         L"), leftlon=" + to_wstring(leftLon) + L"(" +
         to_wstring(leftLon * 180.0 / M_PI) + L"), rightlon=" +
         to_wstring(rightLon) + L"(" + to_wstring(rightLon * 180.0 / M_PI) +
         L")}";
}

GeoWideSouthRectangle::EitherBound::EitherBound(
    shared_ptr<GeoWideSouthRectangle> outerInstance)
    : outerInstance(outerInstance)
{
}

bool GeoWideSouthRectangle::EitherBound::isWithin(shared_ptr<Vector> v)
{
  return outerInstance->leftPlane->isWithin(v) ||
         outerInstance->rightPlane->isWithin(v);
}

bool GeoWideSouthRectangle::EitherBound::isWithin(double const x,
                                                  double const y,
                                                  double const z)
{
  return outerInstance->leftPlane->isWithin(x, y, z) ||
         outerInstance->rightPlane->isWithin(x, y, z);
}
} // namespace org::apache::lucene::spatial3d::geom