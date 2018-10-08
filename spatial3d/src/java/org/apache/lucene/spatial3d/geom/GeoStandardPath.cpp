using namespace std;

#include "GeoStandardPath.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoStandardPath::GeoStandardPath(
    shared_ptr<PlanetModel> planetModel, double const maxCutoffAngle,
    std::deque<std::shared_ptr<GeoPoint>> &pathPoints)
    : GeoStandardPath(planetModel, maxCutoffAngle)
{
  Collections::addAll(points, pathPoints);
  done();
}

GeoStandardPath::GeoStandardPath(shared_ptr<PlanetModel> planetModel,
                                 double const maxCutoffAngle)
    : GeoBasePath(planetModel), cutoffAngle(maxCutoffAngle),
      sinAngle(sin(maxCutoffAngle)), cosAngle(cos(maxCutoffAngle))
{
  if (maxCutoffAngle <= 0.0 || maxCutoffAngle > M_PI * 0.5) {
    throw invalid_argument(L"Cutoff angle out of bounds");
  }
}

void GeoStandardPath::addPoint(double const lat, double const lon)
{
  if (isDone) {
    throw make_shared<IllegalStateException>(
        L"Can't call addPoint() if done() already called");
  }
  points.push_back(make_shared<GeoPoint>(planetModel, lat, lon));
}

void GeoStandardPath::done()
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
  // Compute an offset to use for all segments.  This will be based on the
  // minimum magnitude of the entire ellipsoid.
  constexpr double cutoffOffset =
      this->sinAngle * planetModel->getMinimumMagnitude();

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
                                                  normalizedConnectingPlane,
                                                  cutoffOffset));
    }
    lastPoint = end;
  }

  if (segments.empty()) {
    // Simple circle
    double lat = points[0]->getLatitude();
    double lon = points[0]->getLongitude();
    // Compute two points on the circle, with the right angle from the center.
    // We'll use these to obtain the perpendicular plane to the circle.
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
    shared_ptr<GeoPoint> *const point = points[0];

    // Construct normal plane
    shared_ptr<Plane> *const normalPlane =
        Plane::constructNormalizedZPlane({upperPoint, lowerPoint, point});

    shared_ptr<SegmentEndpoint> *const onlyEndpoint =
        make_shared<SegmentEndpoint>(point, normalPlane, upperPoint,
                                     lowerPoint);
    endPoints.push_back(onlyEndpoint);
    this->edgePoints = std::deque<std::shared_ptr<GeoPoint>>{
        onlyEndpoint->circlePlane->getSampleIntersectionPoint(planetModel,
                                                              normalPlane)};
    return;
  }

  // Create segment endpoints.  Use an appropriate constructor for the start and
  // end of the path.
  for (int i = 0; i < segments.size(); i++) {
    shared_ptr<PathSegment> *const currentSegment = segments[i];

    if (i == 0) {
      // Starting endpoint
      shared_ptr<SegmentEndpoint> *const startEndpoint =
          make_shared<SegmentEndpoint>(
              currentSegment->start, currentSegment->startCutoffPlane,
              currentSegment->ULHC, currentSegment->LLHC);
      endPoints.push_back(startEndpoint);
      this->edgePoints =
          std::deque<std::shared_ptr<GeoPoint>>{currentSegment->ULHC};
      continue;
    }

    // General intersection case
    shared_ptr<PathSegment> *const prevSegment = segments[i - 1];
    // We construct four separate planes, and evaluate which one includes all
    // interior points with least overlap
    shared_ptr<SidedPlane> *const candidate1 =
        SidedPlane::constructNormalizedThreePointSidedPlane(
            currentSegment->start, prevSegment->URHC, currentSegment->ULHC,
            currentSegment->LLHC);
    shared_ptr<SidedPlane> *const candidate2 =
        SidedPlane::constructNormalizedThreePointSidedPlane(
            currentSegment->start, currentSegment->ULHC, currentSegment->LLHC,
            prevSegment->LRHC);
    shared_ptr<SidedPlane> *const candidate3 =
        SidedPlane::constructNormalizedThreePointSidedPlane(
            currentSegment->start, currentSegment->LLHC, prevSegment->LRHC,
            prevSegment->URHC);
    shared_ptr<SidedPlane> *const candidate4 =
        SidedPlane::constructNormalizedThreePointSidedPlane(
            currentSegment->start, prevSegment->LRHC, prevSegment->URHC,
            currentSegment->ULHC);

    if (candidate1 == nullptr && candidate2 == nullptr &&
        candidate3 == nullptr && candidate4 == nullptr) {
      // The planes are identical.  We wouldn't need a circle at all except for
      // the possibility of backing up, which is hard to detect here.
      shared_ptr<SegmentEndpoint> *const midEndpoint =
          make_shared<SegmentEndpoint>(
              currentSegment->start, prevSegment->endCutoffPlane,
              currentSegment->startCutoffPlane, currentSegment->ULHC,
              currentSegment->LLHC);
      // don't need a circle at all.  Special constructor...
      endPoints.push_back(midEndpoint);
    } else {
      endPoints.push_back(make_shared<SegmentEndpoint>(
          currentSegment->start, prevSegment->endCutoffPlane,
          currentSegment->startCutoffPlane, prevSegment->URHC,
          prevSegment->LRHC, currentSegment->ULHC, currentSegment->LLHC,
          candidate1, candidate2, candidate3, candidate4));
    }
  }
  // Do final endpoint
  shared_ptr<PathSegment> *const lastSegment = segments[segments.size() - 1];
  endPoints.push_back(make_shared<SegmentEndpoint>(
      lastSegment->end, lastSegment->endCutoffPlane, lastSegment->URHC,
      lastSegment->LRHC));
}

GeoStandardPath::GeoStandardPath(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoStandardPath(
          planetModel, SerializableObject::readDouble(inputStream),
          SerializableObject::readPointArray(planetModel, inputStream))
{
}

void GeoStandardPath::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, cutoffAngle);
  SerializableObject::writePointArray(outputStream, points);
}

double GeoStandardPath::computePathCenterDistance(
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

double
GeoStandardPath::computeNearestDistance(shared_ptr<DistanceStyle> distanceStyle,
                                        double const x, double const y,
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

double GeoStandardPath::distance(shared_ptr<DistanceStyle> distanceStyle,
                                 double const x, double const y, double const z)
{
  // Algorithm:
  // (1) If the point is within any of the segments along the path, return that
  // value. (2) If the point is within any of the segment end circles along the
  // path, return that value. The algorithm loops over the whole path to get the
  // shortest distance
  double bestDistance = numeric_limits<double>::infinity();

  double currentDistance = 0.0;
  for (auto segment : segments) {
    double distance =
        segment->pathDistance(planetModel, distanceStyle, x, y, z);
    if (distance != numeric_limits<double>::infinity()) {
      constexpr double thisDistance = distanceStyle->fromAggregationForm(
          distanceStyle->aggregateDistances(currentDistance, distance));
      if (thisDistance < bestDistance) {
        bestDistance = thisDistance;
      }
    }
    currentDistance = distanceStyle->aggregateDistances(
        currentDistance, segment->fullPathDistance(distanceStyle));
  }

  int segmentIndex = 0;
  currentDistance = 0.0;
  for (auto endpoint : endPoints) {
    double distance = endpoint->pathDistance(distanceStyle, x, y, z);
    if (distance != numeric_limits<double>::infinity()) {
      constexpr double thisDistance = distanceStyle->fromAggregationForm(
          distanceStyle->aggregateDistances(currentDistance, distance));
      if (thisDistance < bestDistance) {
        bestDistance = thisDistance;
      }
    }
    if (segmentIndex < segments.size()) {
      currentDistance = distanceStyle->aggregateDistances(
          currentDistance,
          segments[segmentIndex++]->fullPathDistance(distanceStyle));
    }
  }

  return bestDistance;
}

double GeoStandardPath::deltaDistance(shared_ptr<DistanceStyle> distanceStyle,
                                      double const x, double const y,
                                      double const z)
{
  // Algorithm:
  // (1) If the point is within any of the segments along the path, return that
  // value. (2) If the point is within any of the segment end circles along the
  // path, return that value. Finds best distance
  double bestDistance = numeric_limits<double>::infinity();

  for (auto segment : segments) {
    constexpr double distance =
        segment->pathDeltaDistance(planetModel, distanceStyle, x, y, z);
    if (distance != numeric_limits<double>::infinity()) {
      constexpr double thisDistance =
          distanceStyle->fromAggregationForm(distance);
      if (thisDistance < bestDistance) {
        bestDistance = thisDistance;
      }
    }
  }

  for (auto endpoint : endPoints) {
    constexpr double distance =
        endpoint->pathDeltaDistance(distanceStyle, x, y, z);
    if (distance != numeric_limits<double>::infinity()) {
      constexpr double thisDistance =
          distanceStyle->fromAggregationForm(distance);
      if (thisDistance < bestDistance) {
        bestDistance = thisDistance;
      }
    }
  }

  return bestDistance;
}

void GeoStandardPath::distanceBounds(shared_ptr<Bounds> bounds,
                                     shared_ptr<DistanceStyle> distanceStyle,
                                     double const distanceValue)
{
  // TBD: Compute actual bounds based on distance
  getBounds(bounds);
}

double GeoStandardPath::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
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

bool GeoStandardPath::isWithin(double const x, double const y, double const z)
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

std::deque<std::shared_ptr<GeoPoint>> GeoStandardPath::getEdgePoints()
{
  return edgePoints;
}

bool GeoStandardPath::intersects(
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
  for (auto pathPoint : endPoints) {
    if (pathPoint->intersects(planetModel, plane, notablePoints, bounds)) {
      return true;
    }
  }

  for (auto pathSegment : segments) {
    if (pathSegment->intersects(planetModel, plane, notablePoints, bounds)) {
      return true;
    }
  }

  return false;
}

bool GeoStandardPath::intersects(shared_ptr<GeoShape> geoShape)
{
  for (auto pathPoint : endPoints) {
    if (pathPoint->intersects(geoShape)) {
      return true;
    }
  }

  for (auto pathSegment : segments) {
    if (pathSegment->intersects(geoShape)) {
      return true;
    }
  }

  return false;
}

void GeoStandardPath::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBasePath::getBounds(bounds);
  // For building bounds, order matters.  We want to traverse
  // never more than 180 degrees longitude at a pop or we risk having the
  // bounds object get itself inverted.  So do the edges first.
  for (auto pathSegment : segments) {
    pathSegment->getBounds(planetModel, bounds);
  }
  for (auto pathPoint : endPoints) {
    pathPoint->getBounds(planetModel, bounds);
  }
}

bool GeoStandardPath::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoStandardPath>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoStandardPath> p = any_cast<std::shared_ptr<GeoStandardPath>>(o);
  if (!GeoBasePath::equals(p)) {
    return false;
  }
  if (cutoffAngle != p->cutoffAngle) {
    return false;
  }
  return points.equals(p->points);
}

int GeoStandardPath::hashCode()
{
  int result = GeoBasePath::hashCode();
  int64_t temp = Double::doubleToLongBits(cutoffAngle);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  result = 31 * result + points.hashCode();
  return result;
}

wstring GeoStandardPath::toString()
{
  return L"GeoStandardPath: {planetmodel=" + planetModel + L", width=" +
         to_wstring(cutoffAngle) + L"(" +
         to_wstring(cutoffAngle * 180.0 / M_PI) + L"), points={" + points +
         L"}}";
}

std::deque<std::shared_ptr<GeoPoint>> const
    GeoStandardPath::SegmentEndpoint::circlePoints =
        std::deque<std::shared_ptr<GeoPoint>>(0);
std::deque<std::shared_ptr<Membership>> const
    GeoStandardPath::SegmentEndpoint::NO_MEMBERSHIP =
        std::deque<std::shared_ptr<Membership>>(0);

GeoStandardPath::SegmentEndpoint::SegmentEndpoint(shared_ptr<GeoPoint> point)
    : point(point), circlePlane(this->circlePlane.reset()),
      cutoffPlanes(this->cutoffPlanes.clear()),
      notablePoints(this->notablePoints.clear())
{
}

GeoStandardPath::SegmentEndpoint::SegmentEndpoint(
    shared_ptr<GeoPoint> point, shared_ptr<Plane> normalPlane,
    shared_ptr<GeoPoint> upperPoint, shared_ptr<GeoPoint> lowerPoint)
    : point(point),
      circlePlane(SidedPlane::constructNormalizedPerpendicularSidedPlane(
          point, normalPlane, upperPoint, lowerPoint)),
      cutoffPlanes(NO_MEMBERSHIP), notablePoints(circlePoints)
{
  // Construct a sided plane that goes through the two points and whose normal
  // is in the normalPlane.
}

GeoStandardPath::SegmentEndpoint::SegmentEndpoint(
    shared_ptr<GeoPoint> point, shared_ptr<SidedPlane> cutoffPlane,
    shared_ptr<GeoPoint> topEdgePoint, shared_ptr<GeoPoint> bottomEdgePoint)
    : point(point),
      circlePlane(SidedPlane::constructNormalizedPerpendicularSidedPlane(
          point, cutoffPlane, topEdgePoint, bottomEdgePoint)),
      cutoffPlanes(std::deque<std::shared_ptr<Membership>>{
          make_shared<SidedPlane>(cutoffPlane)}),
      notablePoints(
          std::deque<std::shared_ptr<GeoPoint>>{topEdgePoint, bottomEdgePoint})
{
  // To construct the plane, we now just need D, which is simply the negative of
  // the evaluation of the circle normal deque at one of the points.
}

GeoStandardPath::SegmentEndpoint::SegmentEndpoint(
    shared_ptr<GeoPoint> point, shared_ptr<SidedPlane> cutoffPlane1,
    shared_ptr<SidedPlane> cutoffPlane2, shared_ptr<GeoPoint> topEdgePoint,
    shared_ptr<GeoPoint> bottomEdgePoint)
    : point(point),
      circlePlane(SidedPlane::constructNormalizedPerpendicularSidedPlane(
          point, cutoffPlane1, topEdgePoint, bottomEdgePoint)),
      cutoffPlanes(std::deque<std::shared_ptr<Membership>>{
          make_shared<SidedPlane>(cutoffPlane1),
          make_shared<SidedPlane>(cutoffPlane2)}),
      notablePoints(
          std::deque<std::shared_ptr<GeoPoint>>{topEdgePoint, bottomEdgePoint})
{
  // To construct the plane, we now just need D, which is simply the negative of
  // the evaluation of the circle normal deque at one of the points.
}

GeoStandardPath::SegmentEndpoint::SegmentEndpoint(
    shared_ptr<GeoPoint> point, shared_ptr<SidedPlane> prevCutoffPlane,
    shared_ptr<SidedPlane> nextCutoffPlane, shared_ptr<GeoPoint> notCand2Point,
    shared_ptr<GeoPoint> notCand1Point, shared_ptr<GeoPoint> notCand3Point,
    shared_ptr<GeoPoint> notCand4Point, shared_ptr<SidedPlane> candidate1,
    shared_ptr<SidedPlane> candidate2, shared_ptr<SidedPlane> candidate3,
    shared_ptr<SidedPlane> candidate4)
    : point(point)
{
  // Note: What we really need is a single plane that goes through all four
  // points. Since that's not possible in the ellipsoid case (because three
  // points determine a plane, not four), we need an approximation that at least
  // creates a boundary that has no interruptions. There are three obvious
  // choices for the third point: either (a) one of the two remaining points, or
  // (b) the top or bottom edge intersection point.  (a) has no guarantee of
  // continuity, while (b) is capable of producing something very far from a
  // circle if the angle between segments is acute. The solution is to look for
  // the side (top or bottom) that has an intersection within the shape.  We use
  // the two points from the opposite side to determine the plane, AND we pick
  // the third to be either of the two points on the intersecting side PROVIDED
  // that the other point is within the final circle we come up with.

  // We construct four separate planes, and evaluate which one includes all
  // interior points with least overlap (Constructed beforehand because we need
  // them for degeneracy check)

  constexpr bool cand1IsOtherWithin =
      candidate1 != nullptr ? candidate1->isWithin(notCand1Point) : false;
  constexpr bool cand2IsOtherWithin =
      candidate2 != nullptr ? candidate2->isWithin(notCand2Point) : false;
  constexpr bool cand3IsOtherWithin =
      candidate3 != nullptr ? candidate3->isWithin(notCand3Point) : false;
  constexpr bool cand4IsOtherWithin =
      candidate4 != nullptr ? candidate4->isWithin(notCand4Point) : false;

  if (cand1IsOtherWithin && cand2IsOtherWithin && cand3IsOtherWithin &&
      cand4IsOtherWithin) {
    // The only way we should see both within is if all four points are
    // coplanar.  In that case, we default to the simplest treatment.
    this->circlePlane = candidate1; // doesn't matter which
    this->notablePoints = std::deque<std::shared_ptr<GeoPoint>>{
        notCand2Point, notCand3Point, notCand1Point, notCand4Point};
    this->cutoffPlanes = std::deque<std::shared_ptr<Membership>>{
        make_shared<SidedPlane>(prevCutoffPlane),
        make_shared<SidedPlane>(nextCutoffPlane)};
  } else if (cand1IsOtherWithin) {
    // Use candidate1, and DON'T include prevCutoffPlane in the cutoff planes
    // deque
    this->circlePlane = candidate1;
    this->notablePoints = std::deque<std::shared_ptr<GeoPoint>>{
        notCand2Point, notCand3Point, notCand4Point};
    this->cutoffPlanes = std::deque<std::shared_ptr<Membership>>{
        make_shared<SidedPlane>(nextCutoffPlane)};
  } else if (cand2IsOtherWithin) {
    // Use candidate2
    this->circlePlane = candidate2;
    this->notablePoints = std::deque<std::shared_ptr<GeoPoint>>{
        notCand3Point, notCand4Point, notCand1Point};
    this->cutoffPlanes = std::deque<std::shared_ptr<Membership>>{
        make_shared<SidedPlane>(nextCutoffPlane)};
  } else if (cand3IsOtherWithin) {
    this->circlePlane = candidate3;
    this->notablePoints = std::deque<std::shared_ptr<GeoPoint>>{
        notCand4Point, notCand1Point, notCand2Point};
    this->cutoffPlanes = std::deque<std::shared_ptr<Membership>>{
        make_shared<SidedPlane>(prevCutoffPlane)};
  } else if (cand4IsOtherWithin) {
    this->circlePlane = candidate4;
    this->notablePoints = std::deque<std::shared_ptr<GeoPoint>>{
        notCand1Point, notCand2Point, notCand3Point};
    this->cutoffPlanes = std::deque<std::shared_ptr<Membership>>{
        make_shared<SidedPlane>(prevCutoffPlane)};
  } else {
    // dunno what happened
    throw runtime_error(L"Couldn't come up with a plane through three points "
                        L"that included the fourth");
  }
}

bool GeoStandardPath::SegmentEndpoint::isWithin(shared_ptr<Vector> point)
{
  if (circlePlane == nullptr) {
    return false;
  }
  if (!circlePlane->isWithin(point)) {
    return false;
  }
  for (auto m : cutoffPlanes) {
    if (!m->isWithin(point)) {
      return false;
    }
  }
  return true;
}

bool GeoStandardPath::SegmentEndpoint::isWithin(double const x, double const y,
                                                double const z)
{
  if (circlePlane == nullptr) {
    return false;
  }
  if (!circlePlane->isWithin(x, y, z)) {
    return false;
  }
  for (auto m : cutoffPlanes) {
    if (!m->isWithin(x, y, z)) {
      return false;
    }
  }
  return true;
}

double GeoStandardPath::SegmentEndpoint::pathDeltaDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  if (!isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  constexpr double theDistance = distanceStyle->toAggregationForm(
      distanceStyle->computeDistance(this->point, x, y, z));
  return distanceStyle->aggregateDistances(theDistance, theDistance);
}

double GeoStandardPath::SegmentEndpoint::pathDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  if (!isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  return distanceStyle->toAggregationForm(
      distanceStyle->computeDistance(this->point, x, y, z));
}

double GeoStandardPath::SegmentEndpoint::nearestPathDistance(
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

double GeoStandardPath::SegmentEndpoint::pathCenterDistance(
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

double GeoStandardPath::SegmentEndpoint::outsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  return distanceStyle->computeDistance(this->point, x, y, z);
}

bool GeoStandardPath::SegmentEndpoint::intersects(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> p,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    std::deque<std::shared_ptr<Membership>> &bounds)
{
  // System.err.println("  looking for intersection between plane "+p+" and
  // circle "+circlePlane+" on proper side of "+cutoffPlanes+" within "+bounds);
  if (circlePlane == nullptr) {
    return false;
  }
  return circlePlane->intersects(planetModel, p, notablePoints,
                                 this->notablePoints, bounds,
                                 this->cutoffPlanes);
}

bool GeoStandardPath::SegmentEndpoint::intersects(shared_ptr<GeoShape> geoShape)
{
  // System.err.println("  looking for intersection between plane "+p+" and
  // circle "+circlePlane+" on proper side of "+cutoffPlanes+" within "+bounds);
  if (circlePlane == nullptr) {
    return false;
  }
  return geoShape->intersects(circlePlane, this->notablePoints,
                              this->cutoffPlanes);
}

void GeoStandardPath::SegmentEndpoint::getBounds(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Bounds> bounds)
{
  bounds->addPoint(point);
  if (circlePlane == nullptr) {
    return;
  }
  bounds->addPlane(planetModel, circlePlane);
}

bool GeoStandardPath::SegmentEndpoint::equals(any o)
{
  if (!(std::dynamic_pointer_cast<SegmentEndpoint>(o) != nullptr)) {
    return false;
  }
  shared_ptr<SegmentEndpoint> other =
      any_cast<std::shared_ptr<SegmentEndpoint>>(o);
  return point->equals(other->point);
}

int GeoStandardPath::SegmentEndpoint::hashCode() { return point->hashCode(); }

wstring GeoStandardPath::SegmentEndpoint::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return point->toString();
}

GeoStandardPath::PathSegment::PathSegment(
    shared_ptr<PlanetModel> planetModel, shared_ptr<GeoPoint> start,
    shared_ptr<GeoPoint> end, shared_ptr<Plane> normalizedConnectingPlane,
    double const planeBoundingOffset)
    : start(start), end(end),
      normalizedConnectingPlane(normalizedConnectingPlane),
      upperConnectingPlane(make_shared<SidedPlane>(
          start, normalizedConnectingPlane, -planeBoundingOffset)),
      lowerConnectingPlane(make_shared<SidedPlane>(
          start, normalizedConnectingPlane, planeBoundingOffset)),
      startCutoffPlane(
          make_shared<SidedPlane>(end, normalizedConnectingPlane, start)),
      endCutoffPlane(
          make_shared<SidedPlane>(start, normalizedConnectingPlane, end)),
      URHC(outerInstance->points[0]), LRHC(outerInstance->points[0]),
      ULHC(outerInstance->points[0]), LLHC(outerInstance->points[0]),
      upperConnectingPlanePoints(
          std::deque<std::shared_ptr<GeoPoint>>{ULHC, URHC}),
      lowerConnectingPlanePoints(
          std::deque<std::shared_ptr<GeoPoint>>{LLHC, LRHC}),
      startCutoffPlanePoints(
          std::deque<std::shared_ptr<GeoPoint>>{ULHC, LLHC}),
      endCutoffPlanePoints(std::deque<std::shared_ptr<GeoPoint>>{URHC, LRHC})
{

  // Either start or end should be on the correct side
  // Cutoff planes use opposite endpoints as correct side examples
  std::deque<std::shared_ptr<Membership>> upperSide = {upperConnectingPlane};
  std::deque<std::shared_ptr<Membership>> lowerSide = {lowerConnectingPlane};
  std::deque<std::shared_ptr<Membership>> startSide = {startCutoffPlane};
  std::deque<std::shared_ptr<Membership>> endSide = {endCutoffPlane};
  std::deque<std::shared_ptr<GeoPoint>> points;
  points = upperConnectingPlane->findIntersections(
      planetModel, startCutoffPlane, {lowerSide, endSide});
  if (points.empty()) {
    throw invalid_argument(
        L"Some segment boundary points are off the ellipsoid; path too wide");
  }
  if (points.size() > 1) {
    throw invalid_argument(L"Ambiguous boundary points; path too short");
  }
  points = upperConnectingPlane->findIntersections(planetModel, endCutoffPlane,
                                                   {lowerSide, startSide});
  if (points.empty()) {
    throw invalid_argument(
        L"Some segment boundary points are off the ellipsoid; path too wide");
  }
  if (points.size() > 1) {
    throw invalid_argument(L"Ambiguous boundary points; path too short");
  }
  points = lowerConnectingPlane->findIntersections(
      planetModel, startCutoffPlane, {upperSide, endSide});
  if (points.empty()) {
    throw invalid_argument(
        L"Some segment boundary points are off the ellipsoid; path too wide");
  }
  if (points.size() > 1) {
    throw invalid_argument(L"Ambiguous boundary points; path too short");
  }
  points = lowerConnectingPlane->findIntersections(planetModel, endCutoffPlane,
                                                   {upperSide, startSide});
  if (points.empty()) {
    throw invalid_argument(
        L"Some segment boundary points are off the ellipsoid; path too wide");
  }
  if (points.size() > 1) {
    throw invalid_argument(L"Ambiguous boundary points; path too short");
  }
}

double GeoStandardPath::PathSegment::fullPathDistance(
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

bool GeoStandardPath::PathSegment::isWithin(shared_ptr<Vector> point)
{
  return startCutoffPlane->isWithin(point) && endCutoffPlane->isWithin(point) &&
         upperConnectingPlane->isWithin(point) &&
         lowerConnectingPlane->isWithin(point);
}

bool GeoStandardPath::PathSegment::isWithin(double const x, double const y,
                                            double const z)
{
  return startCutoffPlane->isWithin(x, y, z) &&
         endCutoffPlane->isWithin(x, y, z) &&
         upperConnectingPlane->isWithin(x, y, z) &&
         lowerConnectingPlane->isWithin(x, y, z);
}

double GeoStandardPath::PathSegment::pathCenterDistance(
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

double GeoStandardPath::PathSegment::nearestPathDistance(
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

double GeoStandardPath::PathSegment::pathDeltaDistance(
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
    constexpr double theDistance =
        distanceStyle->computeDistance(start, x, y, z);
    return distanceStyle->aggregateDistances(theDistance, theDistance);
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
  constexpr double theDistance = distanceStyle->toAggregationForm(
      distanceStyle->computeDistance(thePoint, x, y, z));
  return distanceStyle->aggregateDistances(theDistance, theDistance);
}

double GeoStandardPath::PathSegment::pathDistance(
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

double GeoStandardPath::PathSegment::outsideDistance(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  constexpr double upperDistance = distanceStyle->computeDistance(
      planetModel, upperConnectingPlane, x, y, z,
      {lowerConnectingPlane, startCutoffPlane, endCutoffPlane});
  constexpr double lowerDistance = distanceStyle->computeDistance(
      planetModel, lowerConnectingPlane, x, y, z,
      {upperConnectingPlane, startCutoffPlane, endCutoffPlane});
  constexpr double startDistance = distanceStyle->computeDistance(
      planetModel, startCutoffPlane, x, y, z,
      {endCutoffPlane, lowerConnectingPlane, upperConnectingPlane});
  constexpr double endDistance = distanceStyle->computeDistance(
      planetModel, endCutoffPlane, x, y, z,
      {startCutoffPlane, lowerConnectingPlane, upperConnectingPlane});
  constexpr double ULHCDistance = distanceStyle->computeDistance(ULHC, x, y, z);
  constexpr double URHCDistance = distanceStyle->computeDistance(URHC, x, y, z);
  constexpr double LLHCDistance = distanceStyle->computeDistance(LLHC, x, y, z);
  constexpr double LRHCDistance = distanceStyle->computeDistance(LRHC, x, y, z);
  return min(
      min(min(upperDistance, lowerDistance), min(startDistance, endDistance)),
      min(min(ULHCDistance, URHCDistance), min(LLHCDistance, LRHCDistance)));
}

bool GeoStandardPath::PathSegment::intersects(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> p,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    std::deque<std::shared_ptr<Membership>> &bounds)
{
  return upperConnectingPlane->intersects(
             planetModel, p, notablePoints, upperConnectingPlanePoints, bounds,
             {lowerConnectingPlane, startCutoffPlane, endCutoffPlane}) ||
         lowerConnectingPlane->intersects(
             planetModel, p, notablePoints, lowerConnectingPlanePoints, bounds,
             {upperConnectingPlane, startCutoffPlane, endCutoffPlane});
}

bool GeoStandardPath::PathSegment::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(
             upperConnectingPlane, upperConnectingPlanePoints,
             {lowerConnectingPlane, startCutoffPlane, endCutoffPlane}) ||
         geoShape->intersects(
             lowerConnectingPlane, lowerConnectingPlanePoints,
             {upperConnectingPlane, startCutoffPlane, endCutoffPlane});
}

void GeoStandardPath::PathSegment::getBounds(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Bounds> bounds)
{
  // We need to do all bounding planes as well as corner points
  bounds->addPoint(start)
      ->addPoint(end)
      ->addPoint(ULHC)
      ->addPoint(URHC)
      ->addPoint(LRHC)
      ->addPoint(LLHC)
      ->addPlane(planetModel, upperConnectingPlane,
                 {lowerConnectingPlane, startCutoffPlane, endCutoffPlane})
      ->addPlane(planetModel, lowerConnectingPlane,
                 {upperConnectingPlane, startCutoffPlane, endCutoffPlane})
      ->addPlane(planetModel, startCutoffPlane,
                 {endCutoffPlane, upperConnectingPlane, lowerConnectingPlane})
      ->addPlane(planetModel, endCutoffPlane,
                 {startCutoffPlane, upperConnectingPlane, lowerConnectingPlane})
      ->addIntersection(planetModel, upperConnectingPlane, startCutoffPlane,
                        {lowerConnectingPlane, endCutoffPlane})
      ->addIntersection(planetModel, startCutoffPlane, lowerConnectingPlane,
                        {endCutoffPlane, upperConnectingPlane})
      ->addIntersection(planetModel, lowerConnectingPlane, endCutoffPlane,
                        {upperConnectingPlane, startCutoffPlane})
      ->addIntersection(planetModel, endCutoffPlane, upperConnectingPlane,
                        {startCutoffPlane, lowerConnectingPlane});
}
} // namespace org::apache::lucene::spatial3d::geom