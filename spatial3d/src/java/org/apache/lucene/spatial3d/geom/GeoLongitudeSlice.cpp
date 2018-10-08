using namespace std;

#include "GeoLongitudeSlice.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoLongitudeSlice::GeoLongitudeSlice(shared_ptr<PlanetModel> planetModel,
                                     double const leftLon, double rightLon)
    : GeoBaseBBox(planetModel), leftLon(leftLon), rightLon(rightLon),
      leftPlane(make_shared<SidedPlane>(centerPoint, cosLeftLon, sinLeftLon)),
      rightPlane(
          make_shared<SidedPlane>(centerPoint, cosRightLon, sinRightLon)),
      planePoints(std::deque<std::shared_ptr<GeoPoint>>{
          planetModel->NORTH_POLE, planetModel->SOUTH_POLE}),
      centerPoint(make_shared<GeoPoint>(planetModel, 0.0, middleLon)),
      edgePoints(
          std::deque<std::shared_ptr<GeoPoint>>{planetModel->NORTH_POLE})
{
  // Argument checking
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

  constexpr double sinLeftLon = sin(leftLon);
  constexpr double cosLeftLon = cos(leftLon);
  constexpr double sinRightLon = sin(rightLon);
  constexpr double cosRightLon = cos(rightLon);

  // Normalize
  while (leftLon > rightLon) {
    rightLon += M_PI * 2.0;
  }
  constexpr double middleLon = (leftLon + rightLon) * 0.5;
}

GeoLongitudeSlice::GeoLongitudeSlice(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoLongitudeSlice(planetModel,
                        SerializableObject::readDouble(inputStream),
                        SerializableObject::readDouble(inputStream))
{
}

void GeoLongitudeSlice::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, leftLon);
  SerializableObject::writeDouble(outputStream, rightLon);
}

shared_ptr<GeoBBox> GeoLongitudeSlice::expand(double const angle)
{
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
  return GeoBBoxFactory::makeGeoBBox(planetModel, M_PI * 0.5, -M_PI * 0.5,
                                     newLeftLon, newRightLon);
}

bool GeoLongitudeSlice::isWithin(double const x, double const y, double const z)
{
  return leftPlane->isWithin(x, y, z) && rightPlane->isWithin(x, y, z);
}

double GeoLongitudeSlice::getRadius()
{
  // Compute the extent and divide by two
  double extent = rightLon - leftLon;
  if (extent < 0.0) {
    extent += M_PI * 2.0;
  }
  return max(M_PI * 0.5, extent * 0.5);
}

shared_ptr<GeoPoint> GeoLongitudeSlice::getCenter() { return centerPoint; }

std::deque<std::shared_ptr<GeoPoint>> GeoLongitudeSlice::getEdgePoints()
{
  return edgePoints;
}

bool GeoLongitudeSlice::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, leftPlane, notablePoints, planePoints,
                       bounds, {rightPlane}) ||
         p->intersects(planetModel, rightPlane, notablePoints, planePoints,
                       bounds, {leftPlane});
}

bool GeoLongitudeSlice::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(leftPlane, planePoints, {rightPlane}) ||
         geoShape->intersects(rightPlane, planePoints, {leftPlane});
}

void GeoLongitudeSlice::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  bounds->addVerticalPlane(planetModel, leftLon, leftPlane, {rightPlane})
      ->addVerticalPlane(planetModel, rightLon, rightPlane, {leftPlane})
      ->addIntersection(planetModel, rightPlane, leftPlane)
      .addPoint(planetModel->NORTH_POLE)
      .addPoint(planetModel->SOUTH_POLE);
}

double
GeoLongitudeSlice::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  constexpr double leftDistance = distanceStyle->computeDistance(
      planetModel, leftPlane, x, y, z, {rightPlane});
  constexpr double rightDistance = distanceStyle->computeDistance(
      planetModel, rightPlane, x, y, z, {leftPlane});

  constexpr double northDistance =
      distanceStyle->computeDistance(planetModel->NORTH_POLE, x, y, z);
  constexpr double southDistance =
      distanceStyle->computeDistance(planetModel->SOUTH_POLE, x, y, z);

  return min(min(northDistance, southDistance),
             min(leftDistance, rightDistance));
}

bool GeoLongitudeSlice::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoLongitudeSlice>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoLongitudeSlice> other =
      any_cast<std::shared_ptr<GeoLongitudeSlice>>(o);
  return GeoBaseBBox::equals(other) && other->leftLon == leftLon &&
         other->rightLon == rightLon;
}

int GeoLongitudeSlice::hashCode()
{
  int result = GeoBaseBBox::hashCode();
  int64_t temp = Double::doubleToLongBits(leftLon);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(rightLon);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring GeoLongitudeSlice::toString()
{
  return L"GeoLongitudeSlice: {planetmodel=" + planetModel + L", leftlon=" +
         to_wstring(leftLon) + L"(" + to_wstring(leftLon * 180.0 / M_PI) +
         L"), rightlon=" + to_wstring(rightLon) + L"(" +
         to_wstring(rightLon * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom