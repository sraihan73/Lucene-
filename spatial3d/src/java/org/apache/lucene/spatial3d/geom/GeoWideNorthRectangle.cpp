using namespace std;

#include "GeoWideNorthRectangle.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoWideNorthRectangle::GeoWideNorthRectangle(
    shared_ptr<PlanetModel> planetModel, double const bottomLat,
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
      eitherBound(make_shared<EitherBound>(shared_from_this())),
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
  if (extent < M_PI) {
    throw invalid_argument(L"Width of rectangle too small");
  }

  constexpr double sinBottomLat = sin(bottomLat);
  constexpr double cosBottomLat = cos(bottomLat);
  constexpr double sinLeftLon = sin(leftLon);
  constexpr double cosLeftLon = cos(leftLon);
  constexpr double sinRightLon = sin(rightLon);
  constexpr double cosRightLon = cos(rightLon);

  // Now build the four points

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

GeoWideNorthRectangle::GeoWideNorthRectangle(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoWideNorthRectangle(planetModel,
                            SerializableObject::readDouble(inputStream),
                            SerializableObject::readDouble(inputStream),
                            SerializableObject::readDouble(inputStream))
{
}

void GeoWideNorthRectangle::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, bottomLat);
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoWideNorthRectangle::expand(double const angle)
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

bool GeoWideNorthRectangle::isWithin(double const x, double const y,
                                     double const z)
{
  return bottomPlane->isWithin(x, y, z) &&
         (leftPlane->isWithin(x, y, z) || rightPlane->isWithin(x, y, z));
}

double GeoWideNorthRectangle::getRadius()
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

shared_ptr<GeoPoint> GeoWideNorthRectangle::getCenter() { return centerPoint; }

std::deque<std::shared_ptr<GeoPoint>> GeoWideNorthRectangle::getEdgePoints()
{
  return edgePoints;
}

bool GeoWideNorthRectangle::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // Right and left bounds are essentially independent hemispheres; crossing
  // into the wrong part of one requires crossing into the right part of the
  // other.  So intersection can ignore the left/right bounds.
  return p->intersects(planetModel, bottomPlane, notablePoints,
                       bottomPlanePoints, bounds, {eitherBound}) ||
         p->intersects(planetModel, leftPlane, notablePoints, leftPlanePoints,
                       bounds, {bottomPlane}) ||
         p->intersects(planetModel, rightPlane, notablePoints, rightPlanePoints,
                       bounds, {bottomPlane});
}

bool GeoWideNorthRectangle::intersects(shared_ptr<GeoShape> geoShape)
{
  // Right and left bounds are essentially independent hemispheres; crossing
  // into the wrong part of one requires crossing into the right part of the
  // other.  So intersection can ignore the left/right bounds.
  return geoShape->intersects(bottomPlane, bottomPlanePoints, {eitherBound}) ||
         geoShape->intersects(leftPlane, leftPlanePoints, {bottomPlane}) ||
         geoShape->intersects(rightPlane, rightPlanePoints, {bottomPlane});
}

void GeoWideNorthRectangle::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->isWide()
      ->addHorizontalPlane(planetModel, bottomLat, bottomPlane, {eitherBound})
      ->addVerticalPlane(planetModel, leftLon, leftPlane, {bottomPlane})
      ->addVerticalPlane(planetModel, rightLon, rightPlane, {bottomPlane})
      ->addIntersection(planetModel, leftPlane, rightPlane, {bottomPlane})
      ->addPoint(LLHC)
      ->addPoint(LRHC)
      ->addPoint(planetModel->NORTH_POLE);
}

double
GeoWideNorthRectangle::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                       double const x, double const y,
                                       double const z)
{
  constexpr double bottomDistance = distanceStyle->computeDistance(
      planetModel, bottomPlane, x, y, z, {eitherBound});
  // Because the rectangle exceeds 180 degrees, it is safe to compute the
  // horizontally unbounded distance to both the left and the right and only
  // take the minimum of the two.
  constexpr double leftDistance = distanceStyle->computeDistance(
      planetModel, leftPlane, x, y, z, {bottomPlane});
  constexpr double rightDistance = distanceStyle->computeDistance(
      planetModel, rightPlane, x, y, z, {bottomPlane});

  constexpr double LRHCDistance = distanceStyle->computeDistance(LRHC, x, y, z);
  constexpr double LLHCDistance = distanceStyle->computeDistance(LLHC, x, y, z);

  return min(min(bottomDistance, min(leftDistance, rightDistance)),
             min(LRHCDistance, LLHCDistance));
}

bool GeoWideNorthRectangle::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoWideNorthRectangle>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoWideNorthRectangle> other =
      any_cast<std::shared_ptr<GeoWideNorthRectangle>>(o);
  return GeoBaseBBox::equals(other) && other->LLHC->equals(LLHC) &&
         other->LRHC->equals(LRHC);
}

int GeoWideNorthRectangle::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  result = 31 * result + LLHC->hashCode();
  result = 31 * result + LRHC->hashCode();
  return result;
}

wstring GeoWideNorthRectangle::toString()
{
  return L"GeoWideNorthRectangle: {planetmodel=" + planetModel +
         L", bottomlat=" + to_wstring(bottomLat) + L"(" +
         to_wstring(bottomLat * 180.0 / M_PI) + L"), leftlon=" +
         to_wstring(leftLon) + L"(" + to_wstring(leftLon * 180.0 / M_PI) +
         L"), rightlon=" + to_wstring(rightLon) + L"(" +
         to_wstring(rightLon * 180.0 / M_PI) + L")}";
}

GeoWideNorthRectangle::EitherBound::EitherBound(
    shared_ptr<GeoWideNorthRectangle> outerInstance)
    : outerInstance(outerInstance)
{
}

bool GeoWideNorthRectangle::EitherBound::isWithin(shared_ptr<Vector> v)
{
  return outerInstance->leftPlane->isWithin(v) ||
         outerInstance->rightPlane->isWithin(v);
}

bool GeoWideNorthRectangle::EitherBound::isWithin(double const x,
                                                  double const y,
                                                  double const z)
{
  return outerInstance->leftPlane->isWithin(x, y, z) ||
         outerInstance->rightPlane->isWithin(x, y, z);
}
} // namespace org::apache::lucene::spatial3d::geom