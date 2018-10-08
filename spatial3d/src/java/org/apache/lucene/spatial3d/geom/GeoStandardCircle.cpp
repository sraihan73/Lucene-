using namespace std;

#include "GeoStandardCircle.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const GeoStandardCircle::circlePoints =
    std::deque<std::shared_ptr<GeoPoint>>(0);

GeoStandardCircle::GeoStandardCircle(shared_ptr<PlanetModel> planetModel,
                                     double const lat, double const lon,
                                     double const cutoffAngle)
    : GeoBaseCircle(planetModel),
      center(make_shared<GeoPoint>(planetModel, lat, lon)),
      cutoffAngle(cutoffAngle)
{
  if (lat < -M_PI * 0.5 || lat > M_PI * 0.5) {
    throw invalid_argument(L"Latitude out of bounds");
  }
  if (lon < -M_PI || lon > M_PI) {
    throw invalid_argument(L"Longitude out of bounds");
  }
  if (cutoffAngle < 0.0 || cutoffAngle > M_PI) {
    throw invalid_argument(L"Cutoff angle out of bounds");
  }
  if (cutoffAngle < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Cutoff angle cannot be effectively zero");
  }
  // In an ellipsoidal world, cutoff distances make no sense, unfortunately.
  // Only membership can be used to make in/out determination. Compute two
  // points on the circle, with the right angle from the center.  We'll use
  // these to obtain the perpendicular plane to the circle.
  double upperLat = lat + cutoffAngle;
  double upperLon = lon;
  if (upperLat > M_PI * 0.5) {
    upperLon += M_PI;
    if (upperLon > M_PI) {
      upperLon -= 2.0 * M_PI;
    }
    upperLat = M_PI - upperLat;
  }
  double lowerLat = lat - cutoffAngle;
  double lowerLon = lon;
  if (lowerLat < -M_PI * 0.5) {
    lowerLon += M_PI;
    if (lowerLon > M_PI) {
      lowerLon -= 2.0 * M_PI;
    }
    lowerLat = -M_PI - lowerLat;
  }
  shared_ptr<GeoPoint> *const upperPoint =
      make_shared<GeoPoint>(planetModel, upperLat, upperLon);
  shared_ptr<GeoPoint> *const lowerPoint =
      make_shared<GeoPoint>(planetModel, lowerLat, lowerLon);
  if (abs(cutoffAngle - M_PI) < Vector::MINIMUM_RESOLUTION) {
    // Circle is the whole world
    this->circlePlane.reset();
    this->edgePoints = std::deque<std::shared_ptr<GeoPoint>>(0);
  } else {
    // Construct normal plane
    shared_ptr<Plane> *const normalPlane =
        Plane::constructNormalizedZPlane({upperPoint, lowerPoint, center});
    // Construct a sided plane that goes through the two points and whose normal
    // is in the normalPlane.
    this->circlePlane = SidedPlane::constructNormalizedPerpendicularSidedPlane(
        center, normalPlane, upperPoint, lowerPoint);
    if (circlePlane == nullptr) {
      throw invalid_argument(L"Couldn't construct circle plane, probably too "
                             L"small?  Cutoff angle = " +
                             to_wstring(cutoffAngle) + L"; upperPoint = " +
                             upperPoint + L"; lowerPoint = " + lowerPoint);
    }
    shared_ptr<GeoPoint> *const recomputedIntersectionPoint =
        circlePlane->getSampleIntersectionPoint(planetModel, normalPlane);
    if (recomputedIntersectionPoint == nullptr) {
      throw invalid_argument(L"Couldn't construct intersection point, probably "
                             L"circle too small?  Plane = " +
                             circlePlane);
    }
    this->edgePoints =
        std::deque<std::shared_ptr<GeoPoint>>{recomputedIntersectionPoint};
  }
}

GeoStandardCircle::GeoStandardCircle(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoStandardCircle(planetModel,
                        SerializableObject::readDouble(inputStream),
                        SerializableObject::readDouble(inputStream),
                        SerializableObject::readDouble(inputStream))
{
}

void GeoStandardCircle::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, center->getLatitude());
  SerializableObject::writeDouble(outputStream, center->getLongitude());
  SerializableObject::writeDouble(outputStream, cutoffAngle);
}

double GeoStandardCircle::getRadius() { return cutoffAngle; }

shared_ptr<GeoPoint> GeoStandardCircle::getCenter() { return center; }

double GeoStandardCircle::distance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  return distanceStyle->computeDistance(this->center, x, y, z);
}

void GeoStandardCircle::distanceBounds(shared_ptr<Bounds> bounds,
                                       shared_ptr<DistanceStyle> distanceStyle,
                                       double const distanceValue)
{
  // TBD: Compute actual bounds based on distance
  getBounds(bounds);
}

double
GeoStandardCircle::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  return distanceStyle->computeDistance(planetModel, circlePlane, x, y, z);
}

bool GeoStandardCircle::isWithin(double const x, double const y, double const z)
{
  if (circlePlane == nullptr) {
    return true;
  }
  // Fastest way of determining membership
  return circlePlane->isWithin(x, y, z);
}

std::deque<std::shared_ptr<GeoPoint>> GeoStandardCircle::getEdgePoints()
{
  return edgePoints;
}

bool GeoStandardCircle::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  if (circlePlane == nullptr) {
    return false;
  }
  return circlePlane->intersects(planetModel, p, notablePoints, circlePoints,
                                 bounds);
}

bool GeoStandardCircle::intersects(shared_ptr<GeoShape> geoShape)
{
  if (circlePlane == nullptr) {
    return false;
  }
  return geoShape->intersects(circlePlane, circlePoints);
}

int GeoStandardCircle::getRelationship(shared_ptr<GeoShape> geoShape)
{
  if (circlePlane == nullptr) {
    // same as GeoWorld
    if (geoShape->getEdgePoints().size() > 0) {
      return WITHIN;
    }
    return OVERLAPS;
  }
  return GeoBaseCircle::getRelationship(geoShape);
}

void GeoStandardCircle::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseCircle::getBounds(bounds);
  if (circlePlane == nullptr) {
    // Entire world; should already be covered
    return;
  }
  bounds->addPoint(center);
  bounds->addPlane(planetModel, circlePlane);
}

bool GeoStandardCircle::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoStandardCircle>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoStandardCircle> other =
      any_cast<std::shared_ptr<GeoStandardCircle>>(o);
  return GeoBaseCircle::equals(other) && other->center->equals(center) &&
         other->cutoffAngle == cutoffAngle;
}

int GeoStandardCircle::hashCode()
{
  int result = GeoBaseCircle::hashCode();
  result = 31 * result + center->hashCode();
  int64_t temp = Double::doubleToLongBits(cutoffAngle);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring GeoStandardCircle::toString()
{
  return L"GeoStandardCircle: {planetmodel=" + planetModel + L", center=" +
         center + L", radius=" + to_wstring(cutoffAngle) + L"(" +
         to_wstring(cutoffAngle * 180.0 / M_PI) + L")}";
}
} // namespace org::apache::lucene::spatial3d::geom