using namespace std;

#include "GeoExactCircle.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoExactCircle::GeoExactCircle(shared_ptr<PlanetModel> planetModel,
                               double const lat, double const lon,
                               double const radius, double const accuracy)
    : GeoBaseCircle(planetModel),
      center(make_shared<GeoPoint>(planetModel, lat, lon)), radius(radius),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{edgePoint}),
      circleSlices(deque<>())
{
  if (lat < -M_PI * 0.5 || lat > M_PI * 0.5) {
    throw invalid_argument(L"Latitude out of bounds");
  }
  if (lon < -M_PI || lon > M_PI) {
    throw invalid_argument(L"Longitude out of bounds");
  }
  if (radius < 0.0) {
    throw invalid_argument(L"Radius out of bounds");
  }
  if (radius < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Radius cannot be effectively zero");
  }
  if (planetModel->minimumPoleDistance - radius < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Radius out of bounds. It cannot be bigger than " +
                           to_wstring(planetModel->minimumPoleDistance) +
                           L" for this planet model");
  }

  if (accuracy < Vector::MINIMUM_RESOLUTION) {
    actualAccuracy = Vector::MINIMUM_RESOLUTION;
  } else {
    actualAccuracy = accuracy;
  }

  // We construct approximation planes until we have a low enough error estimate
  const deque<std::shared_ptr<ApproximationSlice>> slices =
      deque<std::shared_ptr<ApproximationSlice>>(100);
  // Construct four cardinal points, and then we'll build the first two planes
  shared_ptr<GeoPoint> *const northPoint =
      planetModel->surfacePointOnBearing(center, radius, 0.0);
  shared_ptr<GeoPoint> *const southPoint =
      planetModel->surfacePointOnBearing(center, radius, M_PI);
  shared_ptr<GeoPoint> *const eastPoint =
      planetModel->surfacePointOnBearing(center, radius, M_PI * 0.5);
  shared_ptr<GeoPoint> *const westPoint =
      planetModel->surfacePointOnBearing(center, radius, M_PI * 1.5);

  shared_ptr<GeoPoint> *const edgePoint;
  if (planetModel->c > planetModel->ab) {
    // z can be greater than x or y, so ellipse is longer in height than width
    slices.push_back(make_shared<ApproximationSlice>(
        center, eastPoint, M_PI * 0.5, westPoint, M_PI * -0.5, northPoint, 0.0,
        true));
    slices.push_back(make_shared<ApproximationSlice>(
        center, westPoint, M_PI * 1.5, eastPoint, M_PI * 0.5, southPoint, M_PI,
        true));
    edgePoint = eastPoint;
  } else {
    // z will be less than x or y, so ellipse is shorter than it is tall
    slices.push_back(
        make_shared<ApproximationSlice>(center, northPoint, 0.0, southPoint,
                                        M_PI, eastPoint, M_PI * 0.5, true));
    slices.push_back(make_shared<ApproximationSlice>(
        center, southPoint, M_PI, northPoint, M_PI * 2.0, westPoint, M_PI * 1.5,
        true));
    edgePoint = northPoint;
  }
  // System.out.println("Edgepoint = " + edgePoint);

  // Now, iterate over slices until we have converted all of them into safe
  // SidedPlanes.
  while (slices.size() > 0) {
    // Peel off a slice from the back
    shared_ptr<ApproximationSlice> *const thisSlice = slices.pop_back();
    // Assess it to see if it is OK as it is, or needs to be split.
    // To do this, we need to look at the part of the circle that will have the
    // greatest error. We will need to compute bearing points for these.
    constexpr double interpPoint1Bearing =
        (thisSlice->point1Bearing + thisSlice->middlePointBearing) * 0.5;
    shared_ptr<GeoPoint> *const interpPoint1 =
        planetModel->surfacePointOnBearing(center, radius, interpPoint1Bearing);
    constexpr double interpPoint2Bearing =
        (thisSlice->point2Bearing + thisSlice->middlePointBearing) * 0.5;
    shared_ptr<GeoPoint> *const interpPoint2 =
        planetModel->surfacePointOnBearing(center, radius, interpPoint2Bearing);

    // Is this point on the plane? (that is, is the approximation good enough?)
    if (!thisSlice->mustSplit &&
        abs(thisSlice->plane->evaluate(interpPoint1)) < actualAccuracy &&
        abs(thisSlice->plane->evaluate(interpPoint2)) < actualAccuracy) {
      circleSlices.push_back(make_shared<CircleSlice>(
          thisSlice->plane, thisSlice->endPoint1, thisSlice->endPoint2, center,
          thisSlice->middlePoint));
      // assert thisSlice.plane.isWithin(center);
    } else {
      // Split the plane into two, and add it back to the end
      slices.push_back(make_shared<ApproximationSlice>(
          center, thisSlice->endPoint1, thisSlice->point1Bearing,
          thisSlice->middlePoint, thisSlice->middlePointBearing, interpPoint1,
          interpPoint1Bearing, false));
      slices.push_back(make_shared<ApproximationSlice>(
          center, thisSlice->middlePoint, thisSlice->middlePointBearing,
          thisSlice->endPoint2, thisSlice->point2Bearing, interpPoint2,
          interpPoint2Bearing, false));
    }
  }

  // System.out.println("Is edgepoint within? "+isWithin(edgePoint));
}

GeoExactCircle::GeoExactCircle(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoExactCircle(planetModel, SerializableObject::readDouble(inputStream),
                     SerializableObject::readDouble(inputStream),
                     SerializableObject::readDouble(inputStream),
                     SerializableObject::readDouble(inputStream))
{
}

void GeoExactCircle::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, center->getLatitude());
  SerializableObject::writeDouble(outputStream, center->getLongitude());
  SerializableObject::writeDouble(outputStream, radius);
  SerializableObject::writeDouble(outputStream, actualAccuracy);
}

double GeoExactCircle::getRadius() { return radius; }

shared_ptr<GeoPoint> GeoExactCircle::getCenter() { return center; }

double GeoExactCircle::distance(shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y, double const z)
{
  return distanceStyle->computeDistance(this->center, x, y, z);
}

void GeoExactCircle::distanceBounds(shared_ptr<Bounds> bounds,
                                    shared_ptr<DistanceStyle> distanceStyle,
                                    double const distanceValue)
{
  // TBD: Compute actual bounds based on distance
  getBounds(bounds);
}

double GeoExactCircle::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                       double const x, double const y,
                                       double const z)
{
  double outsideDistance = numeric_limits<double>::infinity();
  for (auto slice : circleSlices) {
    constexpr double distance =
        distanceStyle->computeDistance(planetModel, slice->circlePlane, x, y, z,
                                       {slice->plane1, slice->plane2});
    if (distance < outsideDistance) {
      outsideDistance = distance;
    }
  }
  return outsideDistance;
}

bool GeoExactCircle::isWithin(double const x, double const y, double const z)
{
  for (auto slice : circleSlices) {
    if (slice->circlePlane->isWithin(x, y, z) &&
        slice->plane1->isWithin(x, y, z) && slice->plane2->isWithin(x, y, z)) {
      return true;
    }
  }
  return false;
}

std::deque<std::shared_ptr<GeoPoint>> GeoExactCircle::getEdgePoints()
{
  return edgePoints;
}

bool GeoExactCircle::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  for (auto slice : circleSlices) {
    if (slice->circlePlane->intersects(planetModel, p, notablePoints,
                                       slice->notableEdgePoints, bounds,
                                       {slice->plane1, slice->plane2})) {
      return true;
    }
  }
  return false;
}

bool GeoExactCircle::intersects(shared_ptr<GeoShape> geoShape)
{
  for (auto slice : circleSlices) {
    if (geoShape->intersects(slice->circlePlane, slice->notableEdgePoints,
                             {slice->plane1, slice->plane2})) {
      return true;
    }
  }
  return false;
}

void GeoExactCircle::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseCircle::getBounds(bounds);
  bounds->addPoint(center);
  for (auto slice : circleSlices) {
    bounds->addPlane(planetModel, slice->circlePlane,
                     {slice->plane1, slice->plane2});
    for (auto point : slice->notableEdgePoints) {
      bounds->addPoint(point);
    }
  }
}

bool GeoExactCircle::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoExactCircle>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoExactCircle> other =
      any_cast<std::shared_ptr<GeoExactCircle>>(o);
  return GeoBaseCircle::equals(other) && other->center->equals(center) &&
         other->radius == radius && other->actualAccuracy == actualAccuracy;
}

int GeoExactCircle::hashCode()
{
  int result = GeoBaseCircle::hashCode();
  result = 31 * result + center->hashCode();
  int64_t temp = Double::doubleToLongBits(radius);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(actualAccuracy);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring GeoExactCircle::toString()
{
  return L"GeoExactCircle: {planetmodel=" + planetModel + L", center=" +
         center + L", radius=" + to_wstring(radius) + L"(" +
         to_wstring(radius * 180.0 / M_PI) + L"), accuracy=" +
         to_wstring(actualAccuracy) + L"}";
}

GeoExactCircle::ApproximationSlice::ApproximationSlice(
    shared_ptr<GeoPoint> center, shared_ptr<GeoPoint> endPoint1,
    double const point1Bearing, shared_ptr<GeoPoint> endPoint2,
    double const point2Bearing, shared_ptr<GeoPoint> middlePoint,
    double const middlePointBearing, bool const mustSplit)
    : plane(SidedPlane::constructNormalizedThreePointSidedPlane(
          center, endPoint1, endPoint2, middlePoint)),
      endPoint1(endPoint1), point1Bearing(point1Bearing), endPoint2(endPoint2),
      point2Bearing(point2Bearing), middlePoint(middlePoint),
      middlePointBearing(middlePointBearing), mustSplit(mustSplit)
{
  // Construct the plane going through the three given points
  if (this->plane == nullptr) {
    throw invalid_argument(
        L"Either circle is too small or accuracy is too high; could not "
        L"construct a plane with endPoint1=" +
        endPoint1 + L" bearing " + to_wstring(point1Bearing) + L", endPoint2=" +
        endPoint2 + L" bearing " + to_wstring(point2Bearing) + L", middle=" +
        middlePoint + L" bearing " + to_wstring(middlePointBearing));
  }
  if (this->plane->isWithin(-center->x, -center->y, -center->z)) {
    // Plane is bogus, we cannot build the circle
    throw invalid_argument(
        L"Could not construct a valid plane for this planet model with "
        L"endPoint1=" +
        endPoint1 + L" bearing " + to_wstring(point1Bearing) + L", endPoint2=" +
        endPoint2 + L" bearing " + to_wstring(point2Bearing) + L", middle=" +
        middlePoint + L" bearing " + to_wstring(middlePointBearing));
  }
}

wstring GeoExactCircle::ApproximationSlice::toString()
{
  return L"{end point 1 = " + endPoint1 + L" bearing 1 = " +
         to_wstring(point1Bearing) + L" end point 2 = " + endPoint2 +
         L" bearing 2 = " + to_wstring(point2Bearing) + L" middle point = " +
         middlePoint + L" middle bearing = " + to_wstring(middlePointBearing) +
         L"}";
}

GeoExactCircle::CircleSlice::CircleSlice(shared_ptr<SidedPlane> circlePlane,
                                         shared_ptr<GeoPoint> endPoint1,
                                         shared_ptr<GeoPoint> endPoint2,
                                         shared_ptr<GeoPoint> center,
                                         shared_ptr<GeoPoint> check)
    : notableEdgePoints(
          std::deque<std::shared_ptr<GeoPoint>>{endPoint1, endPoint2}),
      circlePlane(circlePlane),
      plane1(make_shared<SidedPlane>(check, endPoint1, center)),
      plane2(make_shared<SidedPlane>(check, endPoint2, center))
{
}

wstring GeoExactCircle::CircleSlice::toString()
{
  return L"{circle plane = " + circlePlane + L" plane 1 = " + plane1 +
         L" plane 2 = " + plane2 + L" notable edge points = " +
         notableEdgePoints + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom