using namespace std;

#include "GeoDegeneratePath.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoDegeneratePath::GeoDegeneratePath(
    shared_ptr<PlanetModel> planetModel,
    std::deque<std::shared_ptr<GeoPoint>> &pathPoints)
    : GeoDegeneratePath(planetModel)
{
  Collections::addAll(points, pathPoints);
  done();
}

GeoDegeneratePath::GeoDegeneratePath(shared_ptr<PlanetModel> planetModel)
    : GeoBasePath(planetModel)
{
}

void GeoDegeneratePath::addPoint(double const lat, double const lon)
{
  if (isDone) {
    throw make_shared<IllegalStateException>(
        L"Can't call addPoint() if done() already called");
  }
  points.push_back(make_shared<GeoPoint>(planetModel, lat, lon));
}

void GeoDegeneratePath::done()
{
  if (isDone) {
    throw make_shared<IllegalStateException>(L"Can't call done() twice");
  }
  if (points.empty()) {
    throw invalid_argument(L"Path must have at least one point");
  }
  isDone = true;

  endPoints = deque<>(points.size());
  segments = deque<>(points.size());

  // First, build all segments.  We'll then go back and build corresponding
  // segment endpoints.
  shared_ptr<GeoPoint> lastPoint = nullptr;
  for (auto end : points) {
    if (lastPoint != nullptr) {
      shared_ptr<Plane> *const normalizedConnectingPlane =
          make_shared<Plane>(lastPoint, end);
      if (normalizedConnectingPlane == nullptr) {
        continue;
      }
      segments.push_back(make_shared<PathSegment>(planetModel, lastPoint, end,
                                                  normalizedConnectingPlane));
    }
    lastPoint = end;
  }

  if (segments.empty()) {
    // Simple circle
    shared_ptr<GeoPoint> *const point = points[0];

    shared_ptr<SegmentEndpoint> *const onlyEndpoint =
        make_shared<SegmentEndpoint>(point);
    endPoints.push_back(onlyEndpoint);
    this->edgePoints = std::deque<std::shared_ptr<GeoPoint>>{point};
    return;
  }

  // Create segment endpoints.  Use an appropriate constructor for the start and
  // end of the path.
  for (int i = 0; i < segments.size(); i++) {
    shared_ptr<PathSegment> *const currentSegment = segments[i];

    if (i == 0) {
      // Starting endpoint
      shared_ptr<SegmentEndpoint> *const startEndpoint =
          make_shared<SegmentEndpoint>(currentSegment->start,
                                       currentSegment->startCutoffPlane);
      endPoints.push_back(startEndpoint);
      this->edgePoints =
          std::deque<std::shared_ptr<GeoPoint>>{currentSegment->start};
      continue;
    }

    endPoints.push_back(make_shared<SegmentEndpoint>(
        currentSegment->start, segments[i - 1]->endCutoffPlane,
        currentSegment->startCutoffPlane));
  }
  // Do final endpoint
  shared_ptr<PathSegment> *const lastSegment = segments[segments.size() - 1];
  endPoints.push_back(make_shared<SegmentEndpoint>(
      lastSegment->end, lastSegment->endCutoffPlane));
}

GeoDegeneratePath::GeoDegeneratePath(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoDegeneratePath(planetModel, SerializableObject::readPointArray(
                                         planetModel, inputStream))
{
}

void GeoDegeneratePath::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writePointArray(outputStream, points);
}

double GeoDegeneratePath::computePathCenterDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  // Walk along path and keep track of the closest distance we find
  double closestDistance = numeric_limits<double>::infinity();
  // Segments first
  for (auto segment : segments) {
    constexpr double segmentDistance =
        segment->pathCenterDistance(planetModel, distanceStyle, x, y, z);
    if (segmentDistance < closestDistance) {
      closestDistance = segmentDistance;
    }
  }
  // Now, endpoints
  for (auto endpoint : endPoints) {
    constexpr double endpointDistance =
        endpoint->pathCenterDistance(distanceStyle, x, y, z);
    if (endpointDistance < closestDistance) {
      closestDistance = endpointDistance;
    }
  }
  return closestDistance;
}

double GeoDegeneratePath::computeNearestDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  double currentDistance = 0.0;
  double minPathCenterDistance = numeric_limits<double>::infinity();
  double bestDistance = numeric_limits<double>::infinity();
  int segmentIndex = 0;

  for (auto endpoint : endPoints) {
    constexpr double endpointPathCenterDistance =
        endpoint->pathCenterDistance(distanceStyle, x, y, z);
    if (endpointPathCenterDistance < minPathCenterDistance) {
      // Use this endpoint
      minPathCenterDistance = endpointPathCenterDistance;
      bestDistance = currentDistance;
    }
    // Look at the following segment, if any
    if (segmentIndex < segments.size()) {
      shared_ptr<PathSegment> *const segment = segments[segmentIndex++];
      constexpr double segmentPathCenterDistance =
          segment->pathCenterDistance(planetModel, distanceStyle, x, y, z);
      if (segmentPathCenterDistance < minPathCenterDistance) {
        minPathCenterDistance = segmentPathCenterDistance;
        bestDistance = distanceStyle->aggregateDistances(
            currentDistance,
            segment->nearestPathDistance(planetModel, distanceStyle, x, y, z));
      }
      currentDistance = distanceStyle->aggregateDistances(
          currentDistance, segment->fullPathDistance(distanceStyle));
    }
  }
  return bestDistance;
}

double GeoDegeneratePath::distance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  // Algorithm:
  // (1) If the point is within any of the segments along the path, return that
  // value. (2) If the point is within any of the segment end circles along the
  // path, return that value.
  double currentDistance = 0.0;
  for (auto segment : segments) {
    double distance =
        segment->pathDistance(planetModel, distanceStyle, x, y, z);
    if (distance != numeric_limits<double>::infinity()) {
      return distanceStyle->fromAggregationForm(
          distanceStyle->aggregateDistances(currentDistance, distance));
    }
    currentDistance = distanceStyle->aggregateDistances(
        currentDistance, segment->fullPathDistance(distanceStyle));
  }

  int segmentIndex = 0;
  currentDistance = 0.0;
  for (auto endpoint : endPoints) {
    double distance = endpoint->pathDistance(distanceStyle, x, y, z);
    if (distance != numeric_limits<double>::infinity()) {
      return distanceStyle->fromAggregationForm(
          distanceStyle->aggregateDistances(currentDistance, distance));
    }
    if (segmentIndex < segments.size()) {
      currentDistance = distanceStyle->aggregateDistances(
          currentDistance,
          segments[segmentIndex++]->fullPathDistance(distanceStyle));
    }
  }

  return numeric_limits<double>::infinity();
}

double GeoDegeneratePath::deltaDistance(shared_ptr<DistanceStyle> distanceStyle,
                                        double const x, double const y,
                                        double const z)
{
  // Since this is always called when a point is within the degenerate path,
  // delta distance is always zero by definition.
  return 0.0;
}

void GeoDegeneratePath::distanceBounds(shared_ptr<Bounds> bounds,
                                       shared_ptr<DistanceStyle> distanceStyle,
                                       double const distanceValue)
{
  // TBD: Compute actual bounds based on distance
  getBounds(bounds);
}

double
GeoDegeneratePath::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  double minDistance = numeric_limits<double>::infinity();
  for (auto endpoint : endPoints) {
    constexpr double newDistance =
        endpoint->outsideDistance(distanceStyle, x, y, z);
    if (newDistance < minDistance) {
      minDistance = newDistance;
    }
  }
  for (auto segment : segments) {
    constexpr double newDistance =
        segment->outsideDistance(planetModel, distanceStyle, x, y, z);
    if (newDistance < minDistance) {
      minDistance = newDistance;
    }
  }
  return minDistance;
}

bool GeoDegeneratePath::isWithin(double const x, double const y, double const z)
{
  for (auto pathPoint : endPoints) {
    if (pathPoint->isWithin(x, y, z)) {
      return true;
    }
  }
  for (auto pathSegment : segments) {
    if (pathSegment->isWithin(x, y, z)) {
      return true;
    }
  }
  return false;
}

std::deque<std::shared_ptr<GeoPoint>> GeoDegeneratePath::getEdgePoints()
{
  return edgePoints;
}

bool GeoDegeneratePath::intersects(
    shared_ptr<Plane> plane,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // We look for an intersection with any of the exterior edges of the path.
  // We also have to look for intersections with the cones described by the
  // endpoints. Return "true" if any such intersections are found.

  // For plane intersections, the basic idea is to come up with an equation of
  // the line that is the intersection (if any).  Then, find the intersections
  // with the unit sphere (if any).  If any of the intersection points are
  // within the bounds, then we've detected an intersection. Well, sort of.  We
  // can detect intersections also due to overlap of segments with each other.
  // But that's an edge case and we won't be optimizing for it.
  // System.err.println(" Looking for intersection of plane "+plane+" with path
  // "+this);

  // Since the endpoints are included in the path segments, we only need to do
  // this if there are no path segments
  if (endPoints.size() == 1) {
    return endPoints[0]->intersects(planetModel, plane, notablePoints, bounds);
  }

  for (auto pathSegment : segments) {
    if (pathSegment->intersects(planetModel, plane, notablePoints, bounds)) {
      return true;
    }
  }

  return false;
}

bool GeoDegeneratePath::intersects(shared_ptr<GeoShape> geoShape)
{
  // Since the endpoints are included in the path segments, we only need to do
  // this if there are no path segments
  if (endPoints.size() == 1) {
    return endPoints[0]->intersects(geoShape);
  }

  for (auto pathSegment : segments) {
    if (pathSegment->intersects(geoShape)) {
      return true;
    }
  }

  return false;
}

void GeoDegeneratePath::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBasePath::getBounds(bounds);
  // For building bounds, order matters.  We want to traverse
  // never more than 180 degrees longitude at a pop or we risk having the
  // bounds object get itself inverted.  So do the edges first.
  for (auto pathSegment : segments) {
    pathSegment->getBounds(planetModel, bounds);
  }
  if (endPoints.size() == 1) {
    endPoints[0]->getBounds(planetModel, bounds);
  }
}

bool GeoDegeneratePath::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoDegeneratePath>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoDegeneratePath> p =
      any_cast<std::shared_ptr<GeoDegeneratePath>>(o);
  if (!GeoBasePath::equals(p)) {
    return false;
  }
  return points.equals(p->points);
}

int GeoDegeneratePath::hashCode()
{
  int result = GeoBasePath::hashCode();
  result = 31 * result + points.hashCode();
  return result;
}

wstring GeoDegeneratePath::toString()
{
  return L"GeoDegeneratePath: {planetmodel=" + planetModel + L", points={" +
         points + L"}}";
}

std::deque<std::shared_ptr<GeoPoint>> const
    GeoDegeneratePath::SegmentEndpoint::circlePoints =
        std::deque<std::shared_ptr<GeoPoint>>(0);
std::deque<std::shared_ptr<Membership>> const
    GeoDegeneratePath::SegmentEndpoint::NO_MEMBERSHIP =
        std::deque<std::shared_ptr<Membership>>(0);

GeoDegeneratePath::SegmentEndpoint::SegmentEndpoint(shared_ptr<GeoPoint> point)
    : point(point), cutoffPlanes(NO_MEMBERSHIP), notablePoints(circlePoints)
{
}

GeoDegeneratePath::SegmentEndpoint::SegmentEndpoint(
    shared_ptr<GeoPoint> point, shared_ptr<SidedPlane> cutoffPlane)
    : point(point), cutoffPlanes(std::deque<std::shared_ptr<Membership>>{
                        make_shared<SidedPlane>(cutoffPlane)}),
      notablePoints(std::deque<std::shared_ptr<GeoPoint>>{point})
{
}

GeoDegeneratePath::SegmentEndpoint::SegmentEndpoint(
    shared_ptr<GeoPoint> point, shared_ptr<SidedPlane> cutoffPlane1,
    shared_ptr<SidedPlane> cutoffPlane2)
    : point(point), cutoffPlanes(std::deque<std::shared_ptr<Membership>>{
                        make_shared<SidedPlane>(cutoffPlane1),
                        make_shared<SidedPlane>(cutoffPlane2)}),
      notablePoints(std::deque<std::shared_ptr<GeoPoint>>{point})
{
}

bool GeoDegeneratePath::SegmentEndpoint::isWithin(shared_ptr<Vector> point)
{
  return this->point->isIdentical(point->x, point->y, point->z);
}

bool GeoDegeneratePath::SegmentEndpoint::isWithin(double const x,
                                                  double const y,
                                                  double const z)
{
  return this->point->isIdentical(x, y, z);
}

double GeoDegeneratePath::SegmentEndpoint::pathDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  if (!isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  return distanceStyle->toAggregationForm(
      distanceStyle->computeDistance(this->point, x, y, z));
}

double GeoDegeneratePath::SegmentEndpoint::nearestPathDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  for (auto m : cutoffPlanes) {
    if (!m->isWithin(x, y, z)) {
      return numeric_limits<double>::infinity();
    }
  }
  return distanceStyle->toAggregationForm(0.0);
}

double GeoDegeneratePath::SegmentEndpoint::pathCenterDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  for (auto m : cutoffPlanes) {
    if (!m->isWithin(x, y, z)) {
      return numeric_limits<double>::infinity();
    }
  }
  return distanceStyle->computeDistance(this->point, x, y, z);
}

double GeoDegeneratePath::SegmentEndpoint::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  return distanceStyle->computeDistance(this->point, x, y, z);
}

bool GeoDegeneratePath::SegmentEndpoint::intersects(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> p,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    std::deque<std::shared_ptr<Membership>> &bounds)
{
  // If not on the plane, no intersection
  if (!p->evaluateIsZero(point)) {
    return false;
  }

  for (auto m : bounds) {
    if (!m->isWithin(point)) {
      return false;
    }
  }
  return true;
}

bool GeoDegeneratePath::SegmentEndpoint::intersects(
    shared_ptr<GeoShape> geoShape)
{
  return geoShape->isWithin(point);
}

void GeoDegeneratePath::SegmentEndpoint::getBounds(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Bounds> bounds)
{
  bounds->addPoint(point);
}

bool GeoDegeneratePath::SegmentEndpoint::equals(any o)
{
  if (!(std::dynamic_pointer_cast<SegmentEndpoint>(o) != nullptr)) {
    return false;
  }
  shared_ptr<SegmentEndpoint> other =
      any_cast<std::shared_ptr<SegmentEndpoint>>(o);
  return point->equals(other->point);
}

int GeoDegeneratePath::SegmentEndpoint::hashCode() { return point->hashCode(); }

wstring GeoDegeneratePath::SegmentEndpoint::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return point->toString();
}

GeoDegeneratePath::PathSegment::PathSegment(
    shared_ptr<PlanetModel> planetModel, shared_ptr<GeoPoint> start,
    shared_ptr<GeoPoint> end, shared_ptr<Plane> normalizedConnectingPlane)
    : start(start), end(end),
      normalizedConnectingPlane(normalizedConnectingPlane),
      startCutoffPlane(
          make_shared<SidedPlane>(end, normalizedConnectingPlane, start)),
      endCutoffPlane(
          make_shared<SidedPlane>(start, normalizedConnectingPlane, end)),
      connectingPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{start, end})
{

  // Cutoff planes use opposite endpoints as correct side examples
}

double GeoDegeneratePath::PathSegment::fullPathDistance(
    shared_ptr<DistanceStyle> distanceStyle)
{
  {
    lock_guard<mutex> lock(fullDistanceCache);
    optional<double> dist = fullDistanceCache[distanceStyle];
    if (!dist) {
      dist = optional<double>(distanceStyle->toAggregationForm(
          distanceStyle->computeDistance(start, end->x, end->y, end->z)));
      fullDistanceCache.emplace(distanceStyle, dist);
    }
    return dist.value();
  }
}

bool GeoDegeneratePath::PathSegment::isWithin(shared_ptr<Vector> point)
{
  return startCutoffPlane->isWithin(point) && endCutoffPlane->isWithin(point) &&
         normalizedConnectingPlane->evaluateIsZero(point);
}

bool GeoDegeneratePath::PathSegment::isWithin(double const x, double const y,
                                              double const z)
{
  return startCutoffPlane->isWithin(x, y, z) &&
         endCutoffPlane->isWithin(x, y, z) &&
         normalizedConnectingPlane->evaluateIsZero(x, y, z);
}

double GeoDegeneratePath::PathSegment::pathCenterDistance(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  // First, if this point is outside the endplanes of the segment, return
  // POSITIVE_INFINITY.
  if (!startCutoffPlane->isWithin(x, y, z) ||
      !endCutoffPlane->isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  // (1) Compute normalizedPerpPlane.  If degenerate, then there is no such
  // plane, which means that the point given is insufficient to distinguish
  // between a family of such planes.  This can happen only if the point is one
  // of the "poles", imagining the normalized plane to be the "equator".  In
  // that case, the distance returned should be zero. Want no allocations or
  // expensive operations!  so we do this the hard way
  constexpr double perpX =
      normalizedConnectingPlane->y * z - normalizedConnectingPlane->z * y;
  constexpr double perpY =
      normalizedConnectingPlane->z * x - normalizedConnectingPlane->x * z;
  constexpr double perpZ =
      normalizedConnectingPlane->x * y - normalizedConnectingPlane->y * x;
  constexpr double magnitude =
      sqrt(perpX * perpX + perpY * perpY + perpZ * perpZ);
  if (abs(magnitude) < Vector::MINIMUM_RESOLUTION) {
    return distanceStyle->computeDistance(start, x, y, z);
  }
  constexpr double normFactor = 1.0 / magnitude;
  shared_ptr<Plane> *const normalizedPerpPlane = make_shared<Plane>(
      perpX * normFactor, perpY * normFactor, perpZ * normFactor, 0.0);

  std::deque<std::shared_ptr<GeoPoint>> intersectionPoints =
      normalizedConnectingPlane->findIntersections(planetModel,
                                                   normalizedPerpPlane);
  shared_ptr<GeoPoint> thePoint;
  if (intersectionPoints.empty()) {
    throw runtime_error(L"Can't find world intersection for point x=" +
                        to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                        to_wstring(z));
  } else if (intersectionPoints.size() == 1) {
    thePoint = intersectionPoints[0];
  } else {
    if (startCutoffPlane->isWithin(intersectionPoints[0]) &&
        endCutoffPlane->isWithin(intersectionPoints[0])) {
      thePoint = intersectionPoints[0];
    } else if (startCutoffPlane->isWithin(intersectionPoints[1]) &&
               endCutoffPlane->isWithin(intersectionPoints[1])) {
      thePoint = intersectionPoints[1];
    } else {
      throw runtime_error(L"Can't find world intersection for point x=" +
                          to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                          to_wstring(z));
    }
  }
  return distanceStyle->computeDistance(thePoint, x, y, z);
}

double GeoDegeneratePath::PathSegment::nearestPathDistance(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  // First, if this point is outside the endplanes of the segment, return
  // POSITIVE_INFINITY.
  if (!startCutoffPlane->isWithin(x, y, z) ||
      !endCutoffPlane->isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  // (1) Compute normalizedPerpPlane.  If degenerate, then there is no such
  // plane, which means that the point given is insufficient to distinguish
  // between a family of such planes.  This can happen only if the point is one
  // of the "poles", imagining the normalized plane to be the "equator".  In
  // that case, the distance returned should be zero. Want no allocations or
  // expensive operations!  so we do this the hard way
  constexpr double perpX =
      normalizedConnectingPlane->y * z - normalizedConnectingPlane->z * y;
  constexpr double perpY =
      normalizedConnectingPlane->z * x - normalizedConnectingPlane->x * z;
  constexpr double perpZ =
      normalizedConnectingPlane->x * y - normalizedConnectingPlane->y * x;
  constexpr double magnitude =
      sqrt(perpX * perpX + perpY * perpY + perpZ * perpZ);
  if (abs(magnitude) < Vector::MINIMUM_RESOLUTION) {
    return distanceStyle->toAggregationForm(0.0);
  }
  constexpr double normFactor = 1.0 / magnitude;
  shared_ptr<Plane> *const normalizedPerpPlane = make_shared<Plane>(
      perpX * normFactor, perpY * normFactor, perpZ * normFactor, 0.0);

  std::deque<std::shared_ptr<GeoPoint>> intersectionPoints =
      normalizedConnectingPlane->findIntersections(planetModel,
                                                   normalizedPerpPlane);
  shared_ptr<GeoPoint> thePoint;
  if (intersectionPoints.empty()) {
    throw runtime_error(L"Can't find world intersection for point x=" +
                        to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                        to_wstring(z));
  } else if (intersectionPoints.size() == 1) {
    thePoint = intersectionPoints[0];
  } else {
    if (startCutoffPlane->isWithin(intersectionPoints[0]) &&
        endCutoffPlane->isWithin(intersectionPoints[0])) {
      thePoint = intersectionPoints[0];
    } else if (startCutoffPlane->isWithin(intersectionPoints[1]) &&
               endCutoffPlane->isWithin(intersectionPoints[1])) {
      thePoint = intersectionPoints[1];
    } else {
      throw runtime_error(L"Can't find world intersection for point x=" +
                          to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                          to_wstring(z));
    }
  }
  return distanceStyle->toAggregationForm(distanceStyle->computeDistance(
      start, thePoint->x, thePoint->y, thePoint->z));
}

double GeoDegeneratePath::PathSegment::pathDistance(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  if (!isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }

  // (1) Compute normalizedPerpPlane.  If degenerate, then return point distance
  // from start to point. Want no allocations or expensive operations!  so we do
  // this the hard way
  constexpr double perpX =
      normalizedConnectingPlane->y * z - normalizedConnectingPlane->z * y;
  constexpr double perpY =
      normalizedConnectingPlane->z * x - normalizedConnectingPlane->x * z;
  constexpr double perpZ =
      normalizedConnectingPlane->x * y - normalizedConnectingPlane->y * x;
  constexpr double magnitude =
      sqrt(perpX * perpX + perpY * perpY + perpZ * perpZ);
  if (abs(magnitude) < Vector::MINIMUM_RESOLUTION) {
    return distanceStyle->toAggregationForm(
        distanceStyle->computeDistance(start, x, y, z));
  }
  constexpr double normFactor = 1.0 / magnitude;
  shared_ptr<Plane> *const normalizedPerpPlane = make_shared<Plane>(
      perpX * normFactor, perpY * normFactor, perpZ * normFactor, 0.0);

  // Old computation: too expensive, because it calculates the intersection
  // point twice.
  // return distanceStyle.computeDistance(planetModel,
  // normalizedConnectingPlane, x, y, z, startCutoffPlane, endCutoffPlane) +
  //  distanceStyle.computeDistance(planetModel, normalizedPerpPlane, start.x,
  //  start.y, start.z, upperConnectingPlane, lowerConnectingPlane);

  std::deque<std::shared_ptr<GeoPoint>> intersectionPoints =
      normalizedConnectingPlane->findIntersections(planetModel,
                                                   normalizedPerpPlane);
  shared_ptr<GeoPoint> thePoint;
  if (intersectionPoints.empty()) {
    throw runtime_error(L"Can't find world intersection for point x=" +
                        to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                        to_wstring(z));
  } else if (intersectionPoints.size() == 1) {
    thePoint = intersectionPoints[0];
  } else {
    if (startCutoffPlane->isWithin(intersectionPoints[0]) &&
        endCutoffPlane->isWithin(intersectionPoints[0])) {
      thePoint = intersectionPoints[0];
    } else if (startCutoffPlane->isWithin(intersectionPoints[1]) &&
               endCutoffPlane->isWithin(intersectionPoints[1])) {
      thePoint = intersectionPoints[1];
    } else {
      throw runtime_error(L"Can't find world intersection for point x=" +
                          to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                          to_wstring(z));
    }
  }
  return distanceStyle->aggregateDistances(
      distanceStyle->toAggregationForm(
          distanceStyle->computeDistance(thePoint, x, y, z)),
      distanceStyle->toAggregationForm(distanceStyle->computeDistance(
          start, thePoint->x, thePoint->y, thePoint->z)));
}

double GeoDegeneratePath::PathSegment::outsideDistance(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  constexpr double distance =
      distanceStyle->computeDistance(planetModel, normalizedConnectingPlane, x,
                                     y, z, {startCutoffPlane, endCutoffPlane});
  constexpr double startDistance =
      distanceStyle->computeDistance(start, x, y, z);
  constexpr double endDistance = distanceStyle->computeDistance(end, x, y, z);
  return min(min(startDistance, endDistance), distance);
}

bool GeoDegeneratePath::PathSegment::intersects(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> p,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    std::deque<std::shared_ptr<Membership>> &bounds)
{
  return normalizedConnectingPlane->intersects(
      planetModel, p, connectingPlanePoints, notablePoints, bounds,
      {startCutoffPlane, endCutoffPlane});
}

bool GeoDegeneratePath::PathSegment::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(normalizedConnectingPlane, connectingPlanePoints,
                              {startCutoffPlane, endCutoffPlane});
}

void GeoDegeneratePath::PathSegment::getBounds(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Bounds> bounds)
{
  // We need to do all bounding planes as well as corner points
  bounds->addPoint(start)->addPoint(end)->addPlane(
      planetModel, normalizedConnectingPlane,
      {startCutoffPlane, endCutoffPlane});
}
} // namespace org::apache::lucene::spatial3d::geom