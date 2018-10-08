using namespace std;

#include "GeoConvexPolygon.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoConvexPolygon::GeoConvexPolygon(shared_ptr<PlanetModel> planetModel,
                                   deque<std::shared_ptr<GeoPoint>> &pointList)
    : GeoConvexPolygon(planetModel, pointList, nullptr)
{
}

GeoConvexPolygon::GeoConvexPolygon(shared_ptr<PlanetModel> planetModel,
                                   deque<std::shared_ptr<GeoPoint>> &pointList,
                                   deque<std::shared_ptr<GeoPolygon>> &holes)
    : GeoBasePolygon(planetModel), points(pointList),
      isInternalEdges(make_shared<BitSet>())
{
  if (holes.size() > 0 && holes.empty()) {
    this->holes.clear();
  } else {
    this->holes = holes;
  }
  done(false);
}

GeoConvexPolygon::GeoConvexPolygon(shared_ptr<PlanetModel> planetModel,
                                   deque<std::shared_ptr<GeoPoint>> &pointList,
                                   shared_ptr<BitSet> internalEdgeFlags,
                                   bool const returnEdgeInternal)
    : GeoConvexPolygon(planetModel, pointList, nullptr, internalEdgeFlags,
                       returnEdgeInternal)
{
}

GeoConvexPolygon::GeoConvexPolygon(shared_ptr<PlanetModel> planetModel,
                                   deque<std::shared_ptr<GeoPoint>> &pointList,
                                   deque<std::shared_ptr<GeoPolygon>> &holes,
                                   shared_ptr<BitSet> internalEdgeFlags,
                                   bool const returnEdgeInternal)
    : GeoBasePolygon(planetModel), points(pointList),
      isInternalEdges(internalEdgeFlags)
{
  if (holes.size() > 0 && holes.empty()) {
    this->holes.clear();
  } else {
    this->holes = holes;
  }
  done(returnEdgeInternal);
}

GeoConvexPolygon::GeoConvexPolygon(shared_ptr<PlanetModel> planetModel,
                                   double const startLatitude,
                                   double const startLongitude)
    : GeoConvexPolygon(planetModel, startLatitude, startLongitude, nullptr)
{
}

GeoConvexPolygon::GeoConvexPolygon(shared_ptr<PlanetModel> planetModel,
                                   double const startLatitude,
                                   double const startLongitude,
                                   deque<std::shared_ptr<GeoPolygon>> &holes)
    : GeoBasePolygon(planetModel), points(deque<>()),
      isInternalEdges(make_shared<BitSet>())
{
  if (holes.size() > 0 && holes.empty()) {
    this->holes.clear();
  } else {
    this->holes = holes;
  }
  points.push_back(
      make_shared<GeoPoint>(planetModel, startLatitude, startLongitude));
}

void GeoConvexPolygon::addPoint(double const latitude, double const longitude,
                                bool const isInternalEdge)
{
  if (isDone) {
    throw make_shared<IllegalStateException>(
        L"Can't call addPoint() if done() already called");
  }
  if (isInternalEdge) {
    isInternalEdges->set(points.size() - 1);
  }
  points.push_back(make_shared<GeoPoint>(planetModel, latitude, longitude));
}

void GeoConvexPolygon::done(bool const isInternalReturnEdge)
{
  if (isDone) {
    throw make_shared<IllegalStateException>(
        L"Can't call done() more than once");
  }
  // If fewer than 3 points, can't do it.
  if (points.size() < 3) {
    throw invalid_argument(L"Polygon needs at least three points.");
  }

  if (isInternalReturnEdge) {
    isInternalEdges->set(points.size() - 1);
  }

  isDone = true;

  // Time to construct the planes.  If the polygon is truly convex, then any
  // adjacent point to a segment can provide an interior measurement.
  edges = std::deque<std::shared_ptr<SidedPlane>>(points.size());
  notableEdgePoints =
      std::deque<std::deque<std::shared_ptr<GeoPoint>>>(points.size());

  for (int i = 0; i < points.size(); i++) {
    shared_ptr<GeoPoint> *const start = points[i];
    shared_ptr<GeoPoint> *const end = points[legalIndex(i + 1)];
    // We have to find the next point that is not on the plane between start and
    // end. If there is no such point, it's an error.
    shared_ptr<Plane> *const planeToFind = make_shared<Plane>(start, end);
    int endPointIndex = -1;
    for (int j = 0; j < points.size(); j++) {
      constexpr int index = legalIndex(j + i + 2);
      if (!planeToFind->evaluateIsZero(points[index])) {
        endPointIndex = index;
        break;
      }
    }
    if (endPointIndex == -1) {
      throw invalid_argument(L"Polygon points are all coplanar: " + points);
    }
    shared_ptr<GeoPoint> *const check = points[endPointIndex];
    shared_ptr<SidedPlane> *const sp =
        make_shared<SidedPlane>(check, start, end);
    edges[i] = sp;
    notableEdgePoints[i] = std::deque<std::shared_ptr<GeoPoint>>{start, end};
  }

  // For each edge, create a bounds object.
  eitherBounds = unordered_map<>(edges.size());
  prevBrotherMap = unordered_map<>(edges.size());
  nextBrotherMap = unordered_map<>(edges.size());
  for (int edgeIndex = 0; edgeIndex < edges.size(); edgeIndex++) {
    shared_ptr<SidedPlane> *const edge = edges[edgeIndex];
    int bound1Index = legalIndex(edgeIndex + 1);
    while (edges[bound1Index]->isNumericallyIdentical(edge)) {
      if (bound1Index == edgeIndex) {
        throw invalid_argument(L"Constructed planes are all coplanar: " +
                               points);
      }
      bound1Index = legalIndex(bound1Index + 1);
    }
    int bound2Index = legalIndex(edgeIndex - 1);
    // Look for bound2
    while (edges[bound2Index]->isNumericallyIdentical(edge)) {
      if (bound2Index == edgeIndex) {
        throw invalid_argument(L"Constructed planes are all coplanar: " +
                               points);
      }
      bound2Index = legalIndex(bound2Index - 1);
    }
    // Also confirm that all interior points are within the bounds
    int startingIndex = bound2Index;
    while (true) {
      startingIndex = legalIndex(startingIndex + 1);
      if (startingIndex == bound1Index) {
        break;
      }
      shared_ptr<GeoPoint> *const interiorPoint = points[startingIndex];
      if (!edges[bound1Index]->isWithin(interiorPoint) ||
          !edges[bound2Index]->isWithin(interiorPoint)) {
        throw invalid_argument(
            L"Convex polygon has a side that is more than 180 degrees");
      }
    }
    eitherBounds.emplace(
        edge, make_shared<EitherBound>(edges[bound1Index], edges[bound2Index]));
    // When we are done with this cycle, we'll need to build the intersection
    // bound for each edge and its brother. For now, keep track of the
    // relationships.
    nextBrotherMap.emplace(edge, edges[bound1Index]);
    prevBrotherMap.emplace(edge, edges[bound2Index]);
  }

  // Pick an edge point arbitrarily from the outer polygon.  Glom this together
  // with all edge points from inner polygons.
  int edgePointCount = 1;
  if (holes.size() > 0) {
    for (auto hole : holes) {
      edgePointCount += hole->getEdgePoints()->length;
    }
  }
  edgePoints = std::deque<std::shared_ptr<GeoPoint>>(edgePointCount);
  edgePointCount = 0;
  edgePoints[edgePointCount++] = points[0];
  if (holes.size() > 0) {
    for (auto hole : holes) {
      std::deque<std::shared_ptr<GeoPoint>> holeEdgePoints =
          hole->getEdgePoints();
      for (auto p : holeEdgePoints) {
        edgePoints[edgePointCount++] = p;
      }
    }
  }

  if (isWithinHoles(points[0])) {
    throw invalid_argument(
        L"Polygon edge intersects a polygon hole; not allowed");
  }
}

bool GeoConvexPolygon::isWithinHoles(shared_ptr<GeoPoint> point)
{
  if (holes.size() > 0) {
    for (auto hole : holes) {
      if (!hole->isWithin(point)) {
        return true;
      }
    }
  }
  return false;
}

int GeoConvexPolygon::legalIndex(int index)
{
  while (index >= points.size()) {
    index -= points.size();
  }
  while (index < 0) {
    index += points.size();
  }
  return index;
}

GeoConvexPolygon::GeoConvexPolygon(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoBasePolygon(planetModel),
      points(deque<GeoPoint>{
          SerializableObject::readPointArray(planetModel, inputStream)}),
      isInternalEdges(SerializableObject::readBitSet(inputStream))
{
  const deque<std::shared_ptr<GeoPolygon>> holes = deque<GeoPolygon>{
      SerializableObject::readPolygonArray(planetModel, inputStream)};
  if (holes.size() > 0 && holes.empty()) {
    this->holes.clear();
  } else {
    this->holes = holes;
  }
  done(this->isInternalEdges->get(points.size() - 1));
}

void GeoConvexPolygon::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writePointArray(outputStream, points);
  SerializableObject::writePolygonArray(outputStream, holes);
  SerializableObject::writeBitSet(outputStream, isInternalEdges);
}

bool GeoConvexPolygon::isWithin(double const x, double const y, double const z)
{
  if (!localIsWithin(x, y, z)) {
    return false;
  }
  if (holes.size() > 0) {
    for (auto polygon : holes) {
      if (!polygon->isWithin(x, y, z)) {
        return false;
      }
    }
  }
  return true;
}

bool GeoConvexPolygon::localIsWithin(shared_ptr<Vector> v)
{
  return localIsWithin(v->x, v->y, v->z);
}

bool GeoConvexPolygon::localIsWithin(double const x, double const y,
                                     double const z)
{
  for (auto edge : edges) {
    if (!edge->isWithin(x, y, z)) {
      return false;
    }
  }
  return true;
}

std::deque<std::shared_ptr<GeoPoint>> GeoConvexPolygon::getEdgePoints()
{
  return edgePoints;
}

bool GeoConvexPolygon::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // System.err.println("Checking for polygon intersection with plane
  // "+p+"...");
  for (int edgeIndex = 0; edgeIndex < edges.size(); edgeIndex++) {
    shared_ptr<SidedPlane> *const edge = edges[edgeIndex];
    std::deque<std::shared_ptr<GeoPoint>> points =
        this->notableEdgePoints[edgeIndex];
    if (!isInternalEdges->get(edgeIndex)) {
      // System.err.println("Checking convex edge "+edge+" for intersection
      // against plane "+p);
      if (edge->intersects(planetModel, p, notablePoints, points, bounds,
                           {eitherBounds[edge]})) {
        // System.err.println(" intersects!");
        return true;
      }
    }
  }
  if (holes.size() > 0) {
    // Each hole needs to be looked at for intersection too, since a shape can
    // be entirely within the hole
    for (auto hole : holes) {
      if (hole->intersects(p, notablePoints, bounds)) {
        return true;
      }
    }
  }
  // System.err.println(" no intersection");
  return false;
}

bool GeoConvexPolygon::intersects(shared_ptr<GeoShape> shape)
{
  for (int edgeIndex = 0; edgeIndex < edges.size(); edgeIndex++) {
    shared_ptr<SidedPlane> *const edge = edges[edgeIndex];
    std::deque<std::shared_ptr<GeoPoint>> points =
        this->notableEdgePoints[edgeIndex];
    if (!isInternalEdges->get(edgeIndex)) {
      if (shape->intersects(edge, points, {eitherBounds[edge]})) {
        return true;
      }
    }
  }
  if (holes.size() > 0) {
    for (auto hole : holes) {
      if (hole->intersects(shape)) {
        return true;
      }
    }
  }
  return false;
}

GeoConvexPolygon::EitherBound::EitherBound(shared_ptr<SidedPlane> sideBound1,
                                           shared_ptr<SidedPlane> sideBound2)
    : sideBound1(sideBound1), sideBound2(sideBound2)
{
}

bool GeoConvexPolygon::EitherBound::isWithin(shared_ptr<Vector> v)
{
  return sideBound1->isWithin(v) && sideBound2->isWithin(v);
}

bool GeoConvexPolygon::EitherBound::isWithin(double const x, double const y,
                                             double const z)
{
  return sideBound1->isWithin(x, y, z) && sideBound2->isWithin(x, y, z);
}

wstring GeoConvexPolygon::EitherBound::toString()
{
  return L"(" + sideBound1 + L"," + sideBound2 + L")";
}

void GeoConvexPolygon::getBounds(shared_ptr<Bounds> bounds)
{
  // Because of holes, we don't want to use superclass method
  if (localIsWithin(planetModel->NORTH_POLE)) {
    bounds->noTopLatitudeBound()->noLongitudeBound()->addPoint(
        planetModel->NORTH_POLE);
  }
  if (localIsWithin(planetModel->SOUTH_POLE)) {
    bounds->noBottomLatitudeBound()->noLongitudeBound()->addPoint(
        planetModel->SOUTH_POLE);
  }
  if (localIsWithin(planetModel->MIN_X_POLE)) {
    bounds->addPoint(planetModel->MIN_X_POLE);
  }
  if (localIsWithin(planetModel->MAX_X_POLE)) {
    bounds->addPoint(planetModel->MAX_X_POLE);
  }
  if (localIsWithin(planetModel->MIN_Y_POLE)) {
    bounds->addPoint(planetModel->MIN_Y_POLE);
  }
  if (localIsWithin(planetModel->MAX_Y_POLE)) {
    bounds->addPoint(planetModel->MAX_Y_POLE);
  }

  // Add all the points and the intersections
  for (auto point : points) {
    bounds->addPoint(point);
  }

  // Add planes with membership.
  for (auto edge : edges) {
    bounds->addPlane(planetModel, edge, {eitherBounds[edge]});
    shared_ptr<SidedPlane> *const nextEdge = nextBrotherMap[edge];
    bounds->addIntersection(planetModel, edge, nextEdge,
                            {prevBrotherMap[edge], nextBrotherMap[nextEdge]});
  }
}

double
GeoConvexPolygon::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                  double const x, double const y,
                                  double const z)
{
  double minimumDistance = numeric_limits<double>::infinity();
  for (auto edgePoint : points) {
    constexpr double newDist =
        distanceStyle->computeDistance(edgePoint, x, y, z);
    if (newDist < minimumDistance) {
      minimumDistance = newDist;
    }
  }
  for (auto edgePlane : edges) {
    constexpr double newDist = distanceStyle->computeDistance(
        planetModel, edgePlane, x, y, z, {eitherBounds[edgePlane]});
    if (newDist < minimumDistance) {
      minimumDistance = newDist;
    }
  }
  if (holes.size() > 0) {
    for (auto hole : holes) {
      double holeDistance =
          hole->computeOutsideDistance(distanceStyle, x, y, z);
      if (holeDistance != 0.0 && holeDistance < minimumDistance) {
        minimumDistance = holeDistance;
      }
    }
  }
  return minimumDistance;
}

bool GeoConvexPolygon::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoConvexPolygon>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoConvexPolygon> *const other =
      any_cast<std::shared_ptr<GeoConvexPolygon>>(o);
  if (!GeoBasePolygon::equals(other)) {
    return false;
  }
  if (!other->isInternalEdges->equals(isInternalEdges)) {
    return false;
  }
  if (other->holes.size() > 0 || holes.size() > 0) {
    if (other->holes.empty() || holes.empty()) {
      return false;
    }
    if (!other->holes.equals(holes)) {
      return false;
    }
  }
  return (other->points.equals(points));
}

int GeoConvexPolygon::hashCode()
{
  int result = GeoBasePolygon::hashCode();
  result = 31 * result + points.hashCode();
  if (holes.size() > 0) {
    result = 31 * result + holes.hashCode();
  }
  return result;
}

wstring GeoConvexPolygon::toString()
{
  return L"GeoConvexPolygon: {planetmodel=" + planetModel + L", points=" +
         points + L", internalEdges=" + isInternalEdges +
         ((holes.empty()) ? L"" : L", holes=" + holes) + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom