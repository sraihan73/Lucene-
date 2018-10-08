using namespace std;

#include "GeoComplexPolygon.h"

namespace org::apache::lucene::spatial3d::geom
{

const double GeoComplexPolygon::NEAR_EDGE_CUTOFF =
    -Vector::MINIMUM_RESOLUTION * 10000.0;

GeoComplexPolygon::GeoComplexPolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<deque<std::shared_ptr<GeoPoint>>> &pointsList,
    shared_ptr<GeoPoint> testPoint, bool const testPointInSet)
    : GeoBasePolygon(planetModel), xTree(make_shared<XTree>(allEdges)),
      yTree(make_shared<YTree>(allEdges)), zTree(make_shared<ZTree>(allEdges)),
      pointsList(pointsList) / *For serialization * /,
      testPoint1InSet(testPointInSet), testPoint1(testPoint),
      testPoint2InSet(
          isInSet(testPoint2->x, testPoint2->y, testPoint2->z, testPoint1,
                  testPoint1InSet, testPoint1FixedXPlane,
                  testPoint1FixedXAbovePlane, testPoint1FixedXBelowPlane,
                  testPoint1FixedYPlane, testPoint1FixedYAbovePlane,
                  testPoint1FixedYBelowPlane, testPoint1FixedZPlane,
                  testPoint1FixedZAbovePlane, testPoint1FixedZBelowPlane)),
      testPoint2(
          make_shared<GeoPoint>(-testPoint->x, -testPoint->y, -testPoint->z)),
      testPoint1FixedYPlane(make_shared<Plane>(0.0, 1.0, 0.0, -testPoint1->y)),
      testPoint1FixedXPlane(make_shared<Plane>(1.0, 0.0, 0.0, -testPoint1->x)),
      testPoint1FixedZPlane(make_shared<Plane>(0.0, 0.0, 1.0, -testPoint1->z)),
      testPoint2FixedYPlane(make_shared<Plane>(0.0, 1.0, 0.0, -testPoint2->y)),
      testPoint2FixedXPlane(make_shared<Plane>(1.0, 0.0, 0.0, -testPoint2->x)),
      testPoint2FixedZPlane(make_shared<Plane>(0.0, 0.0, 1.0, -testPoint2->z)),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>(pointsList.size())),
      shapeStartEdges(std::deque<std::shared_ptr<Edge>>(pointsList.size()))
{

  assert((planetModel->pointOnSurface(testPoint->x, testPoint->y, testPoint->z),
          L"Test point is not on the ellipsoid surface"));

  // Construct and index edges
  const deque<std::shared_ptr<Edge>> allEdges =
      deque<std::shared_ptr<Edge>>();
  int edgePointIndex = 0;
  for (auto shapePoints : pointsList) {
    allEdges.reserve(allEdges.size() + shapePoints.size());
    shared_ptr<GeoPoint> lastGeoPoint = shapePoints[shapePoints.size() - 1];
    edgePoints[edgePointIndex] = lastGeoPoint;
    shared_ptr<Edge> lastEdge = nullptr;
    shared_ptr<Edge> firstEdge = nullptr;
    for (auto thisGeoPoint : shapePoints) {
      assert((planetModel->pointOnSurface(thisGeoPoint),
              L"Polygon edge point must be on surface; " + thisGeoPoint +
                  L" is not"));
      shared_ptr<Edge> *const edge =
          make_shared<Edge>(planetModel, lastGeoPoint, thisGeoPoint);
      allEdges.push_back(edge);
      // Now, link
      if (firstEdge == nullptr) {
        firstEdge = edge;
      }
      if (lastEdge != nullptr) {
        lastEdge->next = edge;
        edge->previous = lastEdge;
      }
      lastEdge = edge;
      lastGeoPoint = thisGeoPoint;
    }
    firstEdge->previous = lastEdge;
    lastEdge->next = firstEdge;
    shapeStartEdges[edgePointIndex] = firstEdge;
    edgePointIndex++;
  }

  // Record testPoint1 as-is
  // Pick the antipodes for testPoint2

  assert(
      (planetModel->pointOnSurface(testPoint2->x, testPoint2->y, testPoint2->z),
       L"Test point 2 is off of ellipsoid"));

  // Construct fixed planes for testPoint1

  shared_ptr<Plane> testPoint1FixedYAbovePlane =
      make_shared<Plane>(testPoint1FixedYPlane, true);

  // We compare the plane's Y value (etc), which is -D, with the planet's
  // maximum and minimum Y poles.

  if (-testPoint1FixedYAbovePlane->D - planetModel->getMaximumYValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumYValue() + testPoint1FixedYAbovePlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint1FixedYAbovePlane.reset();
  }
  this->testPoint1FixedYAbovePlane = testPoint1FixedYAbovePlane;

  shared_ptr<Plane> testPoint1FixedYBelowPlane =
      make_shared<Plane>(testPoint1FixedYPlane, false);
  if (-testPoint1FixedYBelowPlane->D - planetModel->getMaximumYValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumYValue() + testPoint1FixedYBelowPlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint1FixedYBelowPlane.reset();
  }
  this->testPoint1FixedYBelowPlane = testPoint1FixedYBelowPlane;

  shared_ptr<Plane> testPoint1FixedXAbovePlane =
      make_shared<Plane>(testPoint1FixedXPlane, true);
  if (-testPoint1FixedXAbovePlane->D - planetModel->getMaximumXValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumXValue() + testPoint1FixedXAbovePlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint1FixedXAbovePlane.reset();
  }
  this->testPoint1FixedXAbovePlane = testPoint1FixedXAbovePlane;

  shared_ptr<Plane> testPoint1FixedXBelowPlane =
      make_shared<Plane>(testPoint1FixedXPlane, false);
  if (-testPoint1FixedXBelowPlane->D - planetModel->getMaximumXValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumXValue() + testPoint1FixedXBelowPlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint1FixedXBelowPlane.reset();
  }
  this->testPoint1FixedXBelowPlane = testPoint1FixedXBelowPlane;

  shared_ptr<Plane> testPoint1FixedZAbovePlane =
      make_shared<Plane>(testPoint1FixedZPlane, true);
  if (-testPoint1FixedZAbovePlane->D - planetModel->getMaximumZValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumZValue() + testPoint1FixedZAbovePlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint1FixedZAbovePlane.reset();
  }
  this->testPoint1FixedZAbovePlane = testPoint1FixedZAbovePlane;

  shared_ptr<Plane> testPoint1FixedZBelowPlane =
      make_shared<Plane>(testPoint1FixedZPlane, false);
  if (-testPoint1FixedZBelowPlane->D - planetModel->getMaximumZValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumZValue() + testPoint1FixedZBelowPlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint1FixedZBelowPlane.reset();
  }
  this->testPoint1FixedZBelowPlane = testPoint1FixedZBelowPlane;

  // Construct fixed planes for testPoint2

  shared_ptr<Plane> testPoint2FixedYAbovePlane =
      make_shared<Plane>(testPoint2FixedYPlane, true);
  if (-testPoint2FixedYAbovePlane->D - planetModel->getMaximumYValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumYValue() + testPoint2FixedYAbovePlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint2FixedYAbovePlane.reset();
  }
  this->testPoint2FixedYAbovePlane = testPoint2FixedYAbovePlane;

  shared_ptr<Plane> testPoint2FixedYBelowPlane =
      make_shared<Plane>(testPoint2FixedYPlane, false);
  if (-testPoint2FixedYBelowPlane->D - planetModel->getMaximumYValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumYValue() + testPoint2FixedYBelowPlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint2FixedYBelowPlane.reset();
  }
  this->testPoint2FixedYBelowPlane = testPoint2FixedYBelowPlane;

  shared_ptr<Plane> testPoint2FixedXAbovePlane =
      make_shared<Plane>(testPoint2FixedXPlane, true);
  if (-testPoint2FixedXAbovePlane->D - planetModel->getMaximumXValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumXValue() + testPoint2FixedXAbovePlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint2FixedXAbovePlane.reset();
  }
  this->testPoint2FixedXAbovePlane = testPoint2FixedXAbovePlane;

  shared_ptr<Plane> testPoint2FixedXBelowPlane =
      make_shared<Plane>(testPoint2FixedXPlane, false);
  if (-testPoint2FixedXBelowPlane->D - planetModel->getMaximumXValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumXValue() + testPoint2FixedXBelowPlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint2FixedXBelowPlane.reset();
  }
  this->testPoint2FixedXBelowPlane = testPoint2FixedXBelowPlane;

  shared_ptr<Plane> testPoint2FixedZAbovePlane =
      make_shared<Plane>(testPoint2FixedZPlane, true);
  if (-testPoint2FixedZAbovePlane->D - planetModel->getMaximumZValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumZValue() + testPoint2FixedZAbovePlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint2FixedZAbovePlane.reset();
  }
  this->testPoint2FixedZAbovePlane = testPoint2FixedZAbovePlane;

  shared_ptr<Plane> testPoint2FixedZBelowPlane =
      make_shared<Plane>(testPoint2FixedZPlane, false);
  if (-testPoint2FixedZBelowPlane->D - planetModel->getMaximumZValue() >
          NEAR_EDGE_CUTOFF ||
      planetModel->getMinimumZValue() + testPoint2FixedZBelowPlane->D >
          NEAR_EDGE_CUTOFF) {
    testPoint2FixedZBelowPlane.reset();
  }
  this->testPoint2FixedZBelowPlane = testPoint2FixedZBelowPlane;

  // We know inset/out-of-set for testPoint1 only right now

  // System.out.println("Determining in-set-ness of test point2
  // ("+testPoint2+"):");
  // We must compute the crossings from testPoint1 to testPoint2 in order to
  // figure out whether testPoint2 is in-set or out

  // System.out.println("\n... done.  Checking against test point1
  // ("+testPoint1+"):");

  assert((isInSet(testPoint1->x, testPoint1->y, testPoint1->z, testPoint2,
                  testPoint2InSet, testPoint2FixedXPlane,
                  testPoint2FixedXAbovePlane, testPoint2FixedXBelowPlane,
                  testPoint2FixedYPlane, testPoint2FixedYAbovePlane,
                  testPoint2FixedYBelowPlane, testPoint2FixedZPlane,
                  testPoint2FixedZAbovePlane,
                  testPoint2FixedZBelowPlane) == testPoint1InSet,
          L"Test point1 not correctly in/out of set according to test point2"));

  // System.out.println("\n... done");
}

GeoComplexPolygon::GeoComplexPolygon(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoComplexPolygon(planetModel, readPointsList(planetModel, inputStream),
                        new GeoPoint(planetModel, inputStream),
                        SerializableObject::readBoolean(inputStream))
{
}

deque<deque<std::shared_ptr<GeoPoint>>> GeoComplexPolygon::readPointsList(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
{
  constexpr int count = SerializableObject::readInt(inputStream);
  const deque<deque<std::shared_ptr<GeoPoint>>> array_ =
      deque<deque<std::shared_ptr<GeoPoint>>>(count);
  for (int i = 0; i < count; i++) {
    array_.push_back(Arrays::asList(
        SerializableObject::readPointArray(planetModel, inputStream)));
  }
  return array_;
}

void GeoComplexPolygon::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  writePointsList(outputStream, pointsList);
  testPoint1->write(outputStream);
  SerializableObject::writeBoolean(outputStream, testPoint1InSet);
}

void GeoComplexPolygon::writePointsList(
    shared_ptr<OutputStream> outputStream,
    deque<deque<std::shared_ptr<GeoPoint>>> &pointsList) 
{
  SerializableObject::writeInt(outputStream, pointsList.size());
  for (auto points : pointsList) {
    SerializableObject::writePointArray(outputStream, points);
  }
}

bool GeoComplexPolygon::isWithin(double const x, double const y, double const z)
{
  // System.out.println("IsWithin() for ["+x+","+y+","+z+"]");
  try {
    // Try with the primary test point
    // if (true) throw new IllegalArgumentException("use second point as
    // exercise"); System.out.println(" Trying testPoint1...");
    return isInSet(x, y, z, testPoint1, testPoint1InSet, testPoint1FixedXPlane,
                   testPoint1FixedXAbovePlane, testPoint1FixedXBelowPlane,
                   testPoint1FixedYPlane, testPoint1FixedYAbovePlane,
                   testPoint1FixedYBelowPlane, testPoint1FixedZPlane,
                   testPoint1FixedZAbovePlane, testPoint1FixedZBelowPlane);
  } catch (const invalid_argument &e) {
    // Try with an alternate test point
    // e.printStackTrace(System.out);
    // System.out.println(" Trying testPoint2...");
    return isInSet(x, y, z, testPoint2, testPoint2InSet, testPoint2FixedXPlane,
                   testPoint2FixedXAbovePlane, testPoint2FixedXBelowPlane,
                   testPoint2FixedYPlane, testPoint2FixedYAbovePlane,
                   testPoint2FixedYBelowPlane, testPoint2FixedZPlane,
                   testPoint2FixedZAbovePlane, testPoint2FixedZBelowPlane);
  }
}

bool GeoComplexPolygon::isInSet(double const x, double const y, double const z,
                                shared_ptr<GeoPoint> testPoint,
                                bool const testPointInSet,
                                shared_ptr<Plane> testPointFixedXPlane,
                                shared_ptr<Plane> testPointFixedXAbovePlane,
                                shared_ptr<Plane> testPointFixedXBelowPlane,
                                shared_ptr<Plane> testPointFixedYPlane,
                                shared_ptr<Plane> testPointFixedYAbovePlane,
                                shared_ptr<Plane> testPointFixedYBelowPlane,
                                shared_ptr<Plane> testPointFixedZPlane,
                                shared_ptr<Plane> testPointFixedZAbovePlane,
                                shared_ptr<Plane> testPointFixedZBelowPlane)
{

  // System.out.println("\nIsInSet called for ["+x+","+y+","+z+"],
  // testPoint="+testPoint+"; is in set? "+testPointInSet);
  // If we're right on top of the point, we know the answer.
  if (testPoint->isNumericallyIdentical(x, y, z)) {
    return testPointInSet;
  }

  // If we're right on top of any of the test planes, we navigate solely on that
  // plane.
  if (testPointFixedYAbovePlane != nullptr &&
      testPointFixedYBelowPlane != nullptr &&
      testPointFixedYPlane->evaluateIsZero(x, y, z)) {
    // Use the XZ plane exclusively.
    // System.out.println(" Using XZ plane alone");
    shared_ptr<CountingEdgeIterator> *const crossingEdgeIterator =
        createLinearCrossingEdgeIterator(testPoint, testPointFixedYPlane,
                                         testPointFixedYAbovePlane,
                                         testPointFixedYBelowPlane, x, y, z);
    // Traverse our way from the test point to the check point.  Use the y tree
    // because that's fixed.
    yTree->traverse(crossingEdgeIterator, testPoint->y);
    return crossingEdgeIterator->isOnEdge() ||
           (((crossingEdgeIterator->getCrossingCount() & 1) == 0)
                ? testPointInSet
                : !testPointInSet);
  } else if (testPointFixedXAbovePlane != nullptr &&
             testPointFixedXBelowPlane != nullptr &&
             testPointFixedXPlane->evaluateIsZero(x, y, z)) {
    // Use the YZ plane exclusively.
    // System.out.println(" Using YZ plane alone");
    shared_ptr<CountingEdgeIterator> *const crossingEdgeIterator =
        createLinearCrossingEdgeIterator(testPoint, testPointFixedXPlane,
                                         testPointFixedXAbovePlane,
                                         testPointFixedXBelowPlane, x, y, z);
    // Traverse our way from the test point to the check point.  Use the x tree
    // because that's fixed.
    xTree->traverse(crossingEdgeIterator, testPoint->x);
    return crossingEdgeIterator->isOnEdge() ||
           (((crossingEdgeIterator->getCrossingCount() & 1) == 0)
                ? testPointInSet
                : !testPointInSet);
  } else if (testPointFixedZAbovePlane != nullptr &&
             testPointFixedZBelowPlane != nullptr &&
             testPointFixedZPlane->evaluateIsZero(x, y, z)) {
    // System.out.println(" Using XY plane alone");
    shared_ptr<CountingEdgeIterator> *const crossingEdgeIterator =
        createLinearCrossingEdgeIterator(testPoint, testPointFixedZPlane,
                                         testPointFixedZAbovePlane,
                                         testPointFixedZBelowPlane, x, y, z);
    // Traverse our way from the test point to the check point.  Use the z tree
    // because that's fixed.
    zTree->traverse(crossingEdgeIterator, testPoint->z);
    return crossingEdgeIterator->isOnEdge() ||
           (((crossingEdgeIterator->getCrossingCount() & 1) == 0)
                ? testPointInSet
                : !testPointInSet);
  } else {
    // System.out.println(" Using two planes");
    // This is the expensive part!!
    // Changing the code below has an enormous impact on the queries per second
    // we see with the benchmark.

    // We need to use two planes to get there.  We don't know which two planes
    // will do it but we can figure it out.
    shared_ptr<Plane> *const travelPlaneFixedX =
        make_shared<Plane>(1.0, 0.0, 0.0, -x);
    shared_ptr<Plane> *const travelPlaneFixedY =
        make_shared<Plane>(0.0, 1.0, 0.0, -y);
    shared_ptr<Plane> *const travelPlaneFixedZ =
        make_shared<Plane>(0.0, 0.0, 1.0, -z);

    shared_ptr<Plane> fixedYAbovePlane =
        make_shared<Plane>(travelPlaneFixedY, true);
    if (-fixedYAbovePlane->D - planetModel->getMaximumYValue() >
            NEAR_EDGE_CUTOFF ||
        planetModel->getMinimumYValue() + fixedYAbovePlane->D >
            NEAR_EDGE_CUTOFF) {
      fixedYAbovePlane.reset();
    }

    shared_ptr<Plane> fixedYBelowPlane =
        make_shared<Plane>(travelPlaneFixedY, false);
    if (-fixedYBelowPlane->D - planetModel->getMaximumYValue() >
            NEAR_EDGE_CUTOFF ||
        planetModel->getMinimumYValue() + fixedYBelowPlane->D >
            NEAR_EDGE_CUTOFF) {
      fixedYBelowPlane.reset();
    }

    shared_ptr<Plane> fixedXAbovePlane =
        make_shared<Plane>(travelPlaneFixedX, true);
    if (-fixedXAbovePlane->D - planetModel->getMaximumXValue() >
            NEAR_EDGE_CUTOFF ||
        planetModel->getMinimumXValue() + fixedXAbovePlane->D >
            NEAR_EDGE_CUTOFF) {
      fixedXAbovePlane.reset();
    }

    shared_ptr<Plane> fixedXBelowPlane =
        make_shared<Plane>(travelPlaneFixedX, false);
    if (-fixedXBelowPlane->D - planetModel->getMaximumXValue() >
            NEAR_EDGE_CUTOFF ||
        planetModel->getMinimumXValue() + fixedXBelowPlane->D >
            NEAR_EDGE_CUTOFF) {
      fixedXBelowPlane.reset();
    }

    shared_ptr<Plane> fixedZAbovePlane =
        make_shared<Plane>(travelPlaneFixedZ, true);
    if (-fixedZAbovePlane->D - planetModel->getMaximumZValue() >
            NEAR_EDGE_CUTOFF ||
        planetModel->getMinimumZValue() + fixedZAbovePlane->D >
            NEAR_EDGE_CUTOFF) {
      fixedZAbovePlane.reset();
    }

    shared_ptr<Plane> fixedZBelowPlane =
        make_shared<Plane>(travelPlaneFixedZ, false);
    if (-fixedZBelowPlane->D - planetModel->getMaximumZValue() >
            NEAR_EDGE_CUTOFF ||
        planetModel->getMinimumZValue() + fixedZBelowPlane->D >
            NEAR_EDGE_CUTOFF) {
      fixedZBelowPlane.reset();
    }

    // Find the intersection points for each one of these and the complementary
    // test point planes.

    const deque<std::shared_ptr<TraversalStrategy>> traversalStrategies =
        deque<std::shared_ptr<TraversalStrategy>>(12);

    if (testPointFixedYAbovePlane != nullptr &&
        testPointFixedYBelowPlane != nullptr && fixedXAbovePlane != nullptr &&
        fixedXBelowPlane != nullptr) {
      // check if planes intersects  inside world
      constexpr double checkAbove =
          4.0 * (fixedXAbovePlane->D * fixedXAbovePlane->D *
                     planetModel->inverseAbSquared +
                 testPointFixedYAbovePlane->D * testPointFixedYAbovePlane->D *
                     planetModel->inverseAbSquared -
                 1.0);
      constexpr double checkBelow =
          4.0 * (fixedXBelowPlane->D * fixedXBelowPlane->D *
                     planetModel->inverseAbSquared +
                 testPointFixedYBelowPlane->D * testPointFixedYBelowPlane->D *
                     planetModel->inverseAbSquared -
                 1.0);
      if (checkAbove < Vector::MINIMUM_RESOLUTION_SQUARED &&
          checkBelow < Vector::MINIMUM_RESOLUTION_SQUARED) {
        // System.out.println("  Looking for intersections between travel and
        // test point planes...");
        std::deque<std::shared_ptr<GeoPoint>> XIntersectionsY =
            travelPlaneFixedX->findIntersections(planetModel,
                                                 testPointFixedYPlane);
        for (auto p : XIntersectionsY) {
          // Travel would be in YZ plane (fixed x) then in XZ (fixed y)
          // We compute distance we need to travel as a placeholder for the
          // number of intersections we might encounter.
          // final double newDistance = p.arcDistance(testPoint) +
          // p.arcDistance(thePoint);
          constexpr double tpDelta1 = testPoint->x - p->x;
          constexpr double tpDelta2 = testPoint->z - p->z;
          constexpr double cpDelta1 = y - p->y;
          constexpr double cpDelta2 = z - p->z;
          constexpr double newDistance =
              tpDelta1 * tpDelta1 + tpDelta2 * tpDelta2 + cpDelta1 * cpDelta1 +
              cpDelta2 * cpDelta2;
          // final double newDistance = (testPoint.x - p.x) * (testPoint.x -
          // p.x) + (testPoint.z - p.z) * (testPoint.z - p.z)  + (thePoint.y -
          // p.y) * (thePoint.y - p.y) + (thePoint.z - p.z) * (thePoint.z - p.z);
          // final double newDistance = Math.abs(testPoint.x - p.x) +
          // Math.abs(thePoint.y - p.y);
          traversalStrategies.push_back(make_shared<TraversalStrategy>(
              shared_from_this(), newDistance, testPoint->y, x,
              testPointFixedYPlane, testPointFixedYAbovePlane,
              testPointFixedYBelowPlane, travelPlaneFixedX, fixedXAbovePlane,
              fixedXBelowPlane, yTree, xTree, p));
        }
      }
    }
    if (testPointFixedZAbovePlane != nullptr &&
        testPointFixedZBelowPlane != nullptr && fixedXAbovePlane != nullptr &&
        fixedXBelowPlane != nullptr) {
      // check if planes intersects  inside world
      constexpr double checkAbove =
          4.0 * (fixedXAbovePlane->D * fixedXAbovePlane->D *
                     planetModel->inverseAbSquared +
                 testPointFixedZAbovePlane->D * testPointFixedZAbovePlane->D *
                     planetModel->inverseCSquared -
                 1.0);
      constexpr double checkBelow =
          4.0 * (fixedXBelowPlane->D * fixedXBelowPlane->D *
                     planetModel->inverseAbSquared +
                 testPointFixedZBelowPlane->D * testPointFixedZBelowPlane->D *
                     planetModel->inverseCSquared -
                 1.0);
      if (checkAbove < Vector::MINIMUM_RESOLUTION_SQUARED &&
          checkBelow < Vector::MINIMUM_RESOLUTION_SQUARED) {
        // System.out.println("  Looking for intersections between travel and
        // test point planes...");
        std::deque<std::shared_ptr<GeoPoint>> XIntersectionsZ =
            travelPlaneFixedX->findIntersections(planetModel,
                                                 testPointFixedZPlane);
        for (auto p : XIntersectionsZ) {
          // Travel would be in YZ plane (fixed x) then in XY (fixed z)
          // final double newDistance = p.arcDistance(testPoint) +
          // p.arcDistance(thePoint);
          constexpr double tpDelta1 = testPoint->x - p->x;
          constexpr double tpDelta2 = testPoint->y - p->y;
          constexpr double cpDelta1 = y - p->y;
          constexpr double cpDelta2 = z - p->z;
          constexpr double newDistance =
              tpDelta1 * tpDelta1 + tpDelta2 * tpDelta2 + cpDelta1 * cpDelta1 +
              cpDelta2 * cpDelta2;
          // final double newDistance = (testPoint.x - p.x) * (testPoint.x -
          // p.x) + (testPoint.y - p.y) * (testPoint.y - p.y)  + (thePoint.y -
          // p.y) * (thePoint.y - p.y) + (thePoint.z - p.z) * (thePoint.z - p.z);
          // final double newDistance = Math.abs(testPoint.x - p.x) +
          // Math.abs(thePoint.z - p.z);
          traversalStrategies.push_back(make_shared<TraversalStrategy>(
              shared_from_this(), newDistance, testPoint->z, x,
              testPointFixedZPlane, testPointFixedZAbovePlane,
              testPointFixedZBelowPlane, travelPlaneFixedX, fixedXAbovePlane,
              fixedXBelowPlane, zTree, xTree, p));
        }
      }
    }
    if (testPointFixedXAbovePlane != nullptr &&
        testPointFixedXBelowPlane != nullptr && fixedYAbovePlane != nullptr &&
        fixedYBelowPlane != nullptr) {
      // check if planes intersects inside world
      constexpr double checkAbove =
          4.0 * (testPointFixedXAbovePlane->D * testPointFixedXAbovePlane->D *
                     planetModel->inverseAbSquared +
                 fixedYAbovePlane->D * fixedYAbovePlane->D *
                     planetModel->inverseAbSquared -
                 1.0);
      constexpr double checkBelow =
          4.0 * (testPointFixedXBelowPlane->D * testPointFixedXBelowPlane->D *
                     planetModel->inverseAbSquared +
                 fixedYBelowPlane->D * fixedYBelowPlane->D *
                     planetModel->inverseAbSquared -
                 1.0);
      if (checkAbove < Vector::MINIMUM_RESOLUTION_SQUARED &&
          checkBelow < Vector::MINIMUM_RESOLUTION_SQUARED) {
        // System.out.println("  Looking for intersections between travel and
        // test point planes...");
        std::deque<std::shared_ptr<GeoPoint>> YIntersectionsX =
            travelPlaneFixedY->findIntersections(planetModel,
                                                 testPointFixedXPlane);
        for (auto p : YIntersectionsX) {
          // Travel would be in XZ plane (fixed y) then in YZ (fixed x)
          // final double newDistance = p.arcDistance(testPoint) +
          // p.arcDistance(thePoint);
          constexpr double tpDelta1 = testPoint->y - p->y;
          constexpr double tpDelta2 = testPoint->z - p->z;
          constexpr double cpDelta1 = x - p->x;
          constexpr double cpDelta2 = z - p->z;
          constexpr double newDistance =
              tpDelta1 * tpDelta1 + tpDelta2 * tpDelta2 + cpDelta1 * cpDelta1 +
              cpDelta2 * cpDelta2;
          // final double newDistance = (testPoint.y - p.y) * (testPoint.y -
          // p.y) + (testPoint.z - p.z) * (testPoint.z - p.z)  + (thePoint.x -
          // p.x) * (thePoint.x - p.x) + (thePoint.z - p.z) * (thePoint.z - p.z);
          // final double newDistance = Math.abs(testPoint.y - p.y) +
          // Math.abs(thePoint.x - p.x);
          traversalStrategies.push_back(make_shared<TraversalStrategy>(
              shared_from_this(), newDistance, testPoint->x, y,
              testPointFixedXPlane, testPointFixedXAbovePlane,
              testPointFixedXBelowPlane, travelPlaneFixedY, fixedYAbovePlane,
              fixedYBelowPlane, xTree, yTree, p));
        }
      }
    }
    if (testPointFixedZAbovePlane != nullptr &&
        testPointFixedZBelowPlane != nullptr && fixedYAbovePlane != nullptr &&
        fixedYBelowPlane != nullptr) {
      // check if planes intersects inside world
      constexpr double checkAbove =
          4.0 * (testPointFixedZAbovePlane->D * testPointFixedZAbovePlane->D *
                     planetModel->inverseCSquared +
                 fixedYAbovePlane->D * fixedYAbovePlane->D *
                     planetModel->inverseAbSquared -
                 1.0);
      constexpr double checkBelow =
          4.0 * (testPointFixedZBelowPlane->D * testPointFixedZBelowPlane->D *
                     planetModel->inverseCSquared +
                 fixedYBelowPlane->D * fixedYBelowPlane->D *
                     planetModel->inverseAbSquared -
                 1.0);
      if (checkAbove < Vector::MINIMUM_RESOLUTION_SQUARED &&
          checkBelow < Vector::MINIMUM_RESOLUTION_SQUARED) {
        // System.out.println("  Looking for intersections between travel and
        // test point planes...");
        std::deque<std::shared_ptr<GeoPoint>> YIntersectionsZ =
            travelPlaneFixedY->findIntersections(planetModel,
                                                 testPointFixedZPlane);
        for (auto p : YIntersectionsZ) {
          // Travel would be in XZ plane (fixed y) then in XY (fixed z)
          // final double newDistance = p.arcDistance(testPoint) +
          // p.arcDistance(thePoint);
          constexpr double tpDelta1 = testPoint->x - p->x;
          constexpr double tpDelta2 = testPoint->y - p->y;
          constexpr double cpDelta1 = x - p->x;
          constexpr double cpDelta2 = z - p->z;
          constexpr double newDistance =
              tpDelta1 * tpDelta1 + tpDelta2 * tpDelta2 + cpDelta1 * cpDelta1 +
              cpDelta2 * cpDelta2;
          // final double newDistance = (testPoint.x - p.x) * (testPoint.x -
          // p.x) + (testPoint.y - p.y) * (testPoint.y - p.y)  + (thePoint.x -
          // p.x) * (thePoint.x - p.x) + (thePoint.z - p.z) * (thePoint.z - p.z);
          // final double newDistance = Math.abs(testPoint.y - p.y) +
          // Math.abs(thePoint.z - p.z);
          traversalStrategies.push_back(make_shared<TraversalStrategy>(
              shared_from_this(), newDistance, testPoint->z, y,
              testPointFixedZPlane, testPointFixedZAbovePlane,
              testPointFixedZBelowPlane, travelPlaneFixedY, fixedYAbovePlane,
              fixedYBelowPlane, zTree, yTree, p));
        }
      }
    }
    if (testPointFixedXAbovePlane != nullptr &&
        testPointFixedXBelowPlane != nullptr && fixedZAbovePlane != nullptr &&
        fixedZBelowPlane != nullptr) {
      // check if planes intersects inside world
      constexpr double checkAbove =
          4.0 * (testPointFixedXAbovePlane->D * testPointFixedXAbovePlane->D *
                     planetModel->inverseAbSquared +
                 fixedZAbovePlane->D * fixedZAbovePlane->D *
                     planetModel->inverseCSquared -
                 1.0);
      constexpr double checkBelow =
          4.0 * (testPointFixedXBelowPlane->D * testPointFixedXBelowPlane->D *
                     planetModel->inverseAbSquared +
                 fixedZBelowPlane->D * fixedZBelowPlane->D *
                     planetModel->inverseCSquared -
                 1.0);
      if (checkAbove < Vector::MINIMUM_RESOLUTION_SQUARED &&
          checkBelow < Vector::MINIMUM_RESOLUTION_SQUARED) {
        // System.out.println("  Looking for intersections between travel and
        // test point planes...");
        std::deque<std::shared_ptr<GeoPoint>> ZIntersectionsX =
            travelPlaneFixedZ->findIntersections(planetModel,
                                                 testPointFixedXPlane);
        for (auto p : ZIntersectionsX) {
          // Travel would be in XY plane (fixed z) then in YZ (fixed x)
          // final double newDistance = p.arcDistance(testPoint) +
          // p.arcDistance(thePoint);
          constexpr double tpDelta1 = testPoint->y - p->y;
          constexpr double tpDelta2 = testPoint->z - p->z;
          constexpr double cpDelta1 = y - p->y;
          constexpr double cpDelta2 = x - p->x;
          constexpr double newDistance =
              tpDelta1 * tpDelta1 + tpDelta2 * tpDelta2 + cpDelta1 * cpDelta1 +
              cpDelta2 * cpDelta2;
          // final double newDistance = (testPoint.y - p.y) * (testPoint.y -
          // p.y) + (testPoint.z - p.z) * (testPoint.z - p.z)  + (thePoint.y -
          // p.y) * (thePoint.y - p.y) + (thePoint.x - p.x) * (thePoint.x - p.x);
          // final double newDistance = Math.abs(testPoint.z - p.z) +
          // Math.abs(thePoint.x - p.x);
          traversalStrategies.push_back(make_shared<TraversalStrategy>(
              shared_from_this(), newDistance, testPoint->x, z,
              testPointFixedXPlane, testPointFixedXAbovePlane,
              testPointFixedXBelowPlane, travelPlaneFixedZ, fixedZAbovePlane,
              fixedZBelowPlane, xTree, zTree, p));
        }
      }
    }
    if (testPointFixedYAbovePlane != nullptr &&
        testPointFixedYBelowPlane != nullptr && fixedZAbovePlane != nullptr &&
        fixedZBelowPlane != nullptr) {
      // check if planes intersects inside world
      constexpr double checkAbove =
          4.0 * (testPointFixedYAbovePlane->D * testPointFixedYAbovePlane->D *
                     planetModel->inverseAbSquared +
                 fixedZAbovePlane->D * fixedZAbovePlane->D *
                     planetModel->inverseCSquared -
                 1.0);
      constexpr double checkBelow =
          4.0 * (testPointFixedYBelowPlane->D * testPointFixedYBelowPlane->D *
                     planetModel->inverseAbSquared +
                 fixedZBelowPlane->D * fixedZBelowPlane->D *
                     planetModel->inverseCSquared -
                 1.0);
      if (checkAbove < Vector::MINIMUM_RESOLUTION_SQUARED &&
          checkBelow < Vector::MINIMUM_RESOLUTION_SQUARED) {
        // System.out.println("  Looking for intersections between travel and
        // test point planes...");
        std::deque<std::shared_ptr<GeoPoint>> ZIntersectionsY =
            travelPlaneFixedZ->findIntersections(planetModel,
                                                 testPointFixedYPlane);
        for (auto p : ZIntersectionsY) {
          // Travel would be in XY plane (fixed z) then in XZ (fixed y)
          // final double newDistance = p.arcDistance(testPoint) +
          // p.arcDistance(thePoint);
          constexpr double tpDelta1 = testPoint->x - p->x;
          constexpr double tpDelta2 = testPoint->z - p->z;
          constexpr double cpDelta1 = y - p->y;
          constexpr double cpDelta2 = x - p->x;
          constexpr double newDistance =
              tpDelta1 * tpDelta1 + tpDelta2 * tpDelta2 + cpDelta1 * cpDelta1 +
              cpDelta2 * cpDelta2;
          // final double newDistance = (testPoint.x - p.x) * (testPoint.x -
          // p.x) + (testPoint.z - p.z) * (testPoint.z - p.z)  + (thePoint.y -
          // p.y) * (thePoint.y - p.y) + (thePoint.x - p.x) * (thePoint.x - p.x);
          // final double newDistance = Math.abs(testPoint.z - p.z) +
          // Math.abs(thePoint.y - p.y);
          traversalStrategies.push_back(make_shared<TraversalStrategy>(
              shared_from_this(), newDistance, testPoint->y, z,
              testPointFixedYPlane, testPointFixedYAbovePlane,
              testPointFixedYBelowPlane, travelPlaneFixedZ, fixedZAbovePlane,
              fixedZBelowPlane, yTree, zTree, p));
        }
      }
    }

    sort(traversalStrategies.begin(), traversalStrategies.end());

    if (traversalStrategies.empty()) {
      throw invalid_argument(L"No dual-plane travel strategies were found");
    }

    // Loop through travel strategies, in order, until we find one that works.
    for (auto ts : traversalStrategies) {
      try {
        return ts->apply(testPoint, testPointInSet, x, y, z);
      } catch (const invalid_argument &e) {
        // Continue
      }
    }

    throw invalid_argument(L"Exhausted all traversal strategies");
  }
}

std::deque<std::shared_ptr<GeoPoint>> GeoComplexPolygon::getEdgePoints()
{
  return edgePoints;
}

bool GeoComplexPolygon::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  // Create the intersector
  shared_ptr<EdgeIterator> *const intersector =
      make_shared<IntersectorEdgeIterator>(shared_from_this(), p, notablePoints,
                                           bounds);
  // First, compute the bounds for the the plane
  shared_ptr<XYZBounds> *const xyzBounds = make_shared<XYZBounds>();
  p->recordBounds(planetModel, xyzBounds, bounds);
  for (auto point : notablePoints) {
    xyzBounds->addPoint(point);
  }
  // If we have no bounds at all then the answer is "false"
  if (!xyzBounds->getMaximumX() || !xyzBounds->getMinimumX() ||
      !xyzBounds->getMaximumY() || !xyzBounds->getMinimumY() ||
      !xyzBounds->getMaximumZ() || !xyzBounds->getMinimumZ()) {
    return false;
  }
  // Figure out which tree likely works best
  constexpr double xDelta = xyzBounds->getMaximumX() - xyzBounds->getMinimumX();
  constexpr double yDelta = xyzBounds->getMaximumY() - xyzBounds->getMinimumY();
  constexpr double zDelta = xyzBounds->getMaximumZ() - xyzBounds->getMinimumZ();
  // Select the smallest range
  if (xDelta <= yDelta && xDelta <= zDelta) {
    // Drill down in x
    return !xTree->traverse(intersector, xyzBounds->getMinimumX(),
                            xyzBounds->getMaximumX());
  } else if (yDelta <= xDelta && yDelta <= zDelta) {
    // Drill down in y
    return !yTree->traverse(intersector, xyzBounds->getMinimumY(),
                            xyzBounds->getMaximumY());
  } else if (zDelta <= xDelta && zDelta <= yDelta) {
    // Drill down in z
    return !zTree->traverse(intersector, xyzBounds->getMinimumZ(),
                            xyzBounds->getMaximumZ());
  }
  return true;
}

bool GeoComplexPolygon::intersects(shared_ptr<GeoShape> geoShape)
{
  // Create the intersector
  shared_ptr<EdgeIterator> *const intersector =
      make_shared<IntersectorShapeIterator>(shared_from_this(), geoShape);
  // First, compute the bounds for the the plane
  shared_ptr<XYZBounds> *const xyzBounds = make_shared<XYZBounds>();
  geoShape->getBounds(xyzBounds);

  // Figure out which tree likely works best
  constexpr double xDelta = xyzBounds->getMaximumX() - xyzBounds->getMinimumX();
  constexpr double yDelta = xyzBounds->getMaximumY() - xyzBounds->getMinimumY();
  constexpr double zDelta = xyzBounds->getMaximumZ() - xyzBounds->getMinimumZ();
  // Select the smallest range
  // Select the smallest range
  if (xDelta <= yDelta && xDelta <= zDelta) {
    // Drill down in x
    return !xTree->traverse(intersector, xyzBounds->getMinimumX(),
                            xyzBounds->getMaximumX());
  } else if (yDelta <= xDelta && yDelta <= zDelta) {
    // Drill down in y
    return !yTree->traverse(intersector, xyzBounds->getMinimumY(),
                            xyzBounds->getMaximumY());
  } else if (zDelta <= xDelta && zDelta <= yDelta) {
    // Drill down in z
    return !zTree->traverse(intersector, xyzBounds->getMinimumZ(),
                            xyzBounds->getMaximumZ());
  }
  return true;
}

void GeoComplexPolygon::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBasePolygon::getBounds(bounds);
  for (auto startEdge : shapeStartEdges) {
    shared_ptr<Edge> currentEdge = startEdge;
    while (true) {
      bounds->addPoint(currentEdge->startPoint);
      bounds->addPlane(this->planetModel, currentEdge->plane,
                       {currentEdge->startPlane, currentEdge->endPlane});
      currentEdge = currentEdge->next;
      if (currentEdge == startEdge) {
        break;
      }
    }
  }
}

double
GeoComplexPolygon::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z)
{
  double minimumDistance = numeric_limits<double>::infinity();
  for (auto shapeStartEdge : shapeStartEdges) {
    shared_ptr<Edge> shapeEdge = shapeStartEdge;
    while (true) {
      constexpr double newDist =
          distanceStyle->computeDistance(shapeEdge->startPoint, x, y, z);
      if (newDist < minimumDistance) {
        minimumDistance = newDist;
      }
      constexpr double newPlaneDist = distanceStyle->computeDistance(
          planetModel, shapeEdge->plane, x, y, z,
          {shapeEdge->startPlane, shapeEdge->endPlane});
      if (newPlaneDist < minimumDistance) {
        minimumDistance = newPlaneDist;
      }
      shapeEdge = shapeEdge->next;
      if (shapeEdge == shapeStartEdge) {
        break;
      }
    }
  }
  return minimumDistance;
}

shared_ptr<CountingEdgeIterator>
GeoComplexPolygon::createLinearCrossingEdgeIterator(
    shared_ptr<GeoPoint> testPoint, shared_ptr<Plane> plane,
    shared_ptr<Plane> abovePlane, shared_ptr<Plane> belowPlane,
    double const thePointX, double const thePointY, double const thePointZ)
{
  // If thePoint and testPoint are parallel, we won't be able to determine
  // sidedness of the bounding planes.  So detect that case, and build the
  // iterator differently if we find it. This didn't work; not sure why not:
  // if (testPoint.isParallel(thePointX, thePointY, thePointZ)) {
  //  return new FullLinearCrossingEdgeIterator(plane, abovePlane, belowPlane,
  //  thePointX, thePointY, thePointZ);
  //}
  // return new SectorLinearCrossingEdgeIterator(plane, abovePlane, belowPlane,
  // thePointX, thePointY, thePointZ);
  //
  try {
    // System.out.println(" creating sector linear crossing edge iterator");
    return make_shared<SectorLinearCrossingEdgeIterator>(
        shared_from_this(), testPoint, plane, abovePlane, belowPlane, thePointX,
        thePointY, thePointZ);
  } catch (const invalid_argument &e) {
    // Assume we failed because we could not construct bounding planes, so do it
    // another way.
    // System.out.println(" create full linear crossing edge iterator");
    return make_shared<FullLinearCrossingEdgeIterator>(
        shared_from_this(), testPoint, plane, abovePlane, belowPlane, thePointX,
        thePointY, thePointZ);
  }
}

std::deque<double> const GeoComplexPolygon::halfProportions =
    std::deque<double>{0.5};

GeoComplexPolygon::Edge::Edge(shared_ptr<PlanetModel> pm,
                              shared_ptr<GeoPoint> startPoint,
                              shared_ptr<GeoPoint> endPoint)
    : startPoint(startPoint), endPoint(endPoint),
      notablePoints(
          std::deque<std::shared_ptr<GeoPoint>>{startPoint, endPoint}),
      startPlane(make_shared<SidedPlane>(endPoint, plane, startPoint)),
      endPlane(make_shared<SidedPlane>(startPoint, plane, endPoint)),
      backingPlane(
          make_shared<SidedPlane>(interpolationPoint, interpolationPoint, 0.0)),
      plane(make_shared<Plane>(startPoint, endPoint)),
      planeBounds(make_shared<XYZBounds>())
{
  shared_ptr<GeoPoint> *const interpolationPoint =
      plane->interpolate(startPoint, endPoint, halfProportions)[0];
  this->planeBounds->addPoint(startPoint);
  this->planeBounds->addPoint(endPoint);
  this->planeBounds->addPlane(
      pm, this->plane, {this->startPlane, this->endPlane, this->backingPlane});
  // System.out.println("Recording edge ["+startPoint+" --> "+endPoint+"];
  // bounds = "+planeBounds);
}

bool GeoComplexPolygon::Edge::isWithin(double const thePointX,
                                       double const thePointY,
                                       double const thePointZ)
{
  return plane->evaluateIsZero(thePointX, thePointY, thePointZ) &&
         startPlane->isWithin(thePointX, thePointY, thePointZ) &&
         endPlane->isWithin(thePointX, thePointY, thePointZ) &&
         backingPlane->isWithin(thePointX, thePointY, thePointZ);
}

GeoComplexPolygon::TraversalStrategy::TraversalStrategy(
    shared_ptr<GeoComplexPolygon> outerInstance, double const traversalDistance,
    double const firstLegValue, double const secondLegValue,
    shared_ptr<Plane> firstLegPlane, shared_ptr<Plane> firstLegAbovePlane,
    shared_ptr<Plane> firstLegBelowPlane, shared_ptr<Plane> secondLegPlane,
    shared_ptr<Plane> secondLegAbovePlane,
    shared_ptr<Plane> secondLegBelowPlane, shared_ptr<Tree> firstLegTree,
    shared_ptr<Tree> secondLegTree, shared_ptr<GeoPoint> intersectionPoint)
    : traversalDistance(traversalDistance), firstLegValue(firstLegValue),
      secondLegValue(secondLegValue), firstLegPlane(firstLegPlane),
      firstLegAbovePlane(firstLegAbovePlane),
      firstLegBelowPlane(firstLegBelowPlane), secondLegPlane(secondLegPlane),
      secondLegAbovePlane(secondLegAbovePlane),
      secondLegBelowPlane(secondLegBelowPlane), firstLegTree(firstLegTree),
      secondLegTree(secondLegTree), intersectionPoint(intersectionPoint),
      outerInstance(outerInstance)
{
}

bool GeoComplexPolygon::TraversalStrategy::apply(shared_ptr<GeoPoint> testPoint,
                                                 bool const testPointInSet,
                                                 double const x, double const y,
                                                 double const z)
{
  // First, try with two individual legs.  If that doesn't work, try the
  // DualCrossingIterator.
  try {
    // First, we'll determine if the intersection point is in set or not
    // System.out.println(" Finding whether "+intersectionPoint+" is in-set,
    // based on travel from "+testPoint+" along "+firstLegPlane+"
    // (value="+firstLegValue+")");
    shared_ptr<CountingEdgeIterator> *const testPointEdgeIterator =
        outerInstance->createLinearCrossingEdgeIterator(
            testPoint, firstLegPlane, firstLegAbovePlane, firstLegBelowPlane,
            intersectionPoint->x, intersectionPoint->y, intersectionPoint->z);
    // Traverse our way from the test point to the check point.  Use the z tree
    // because that's fixed.
    firstLegTree->traverse(testPointEdgeIterator, firstLegValue);
    constexpr bool intersectionPointOnEdge = testPointEdgeIterator->isOnEdge();
    // If the intersection point is on the edge, we cannot use this combination
    // of legs, since it's not logically possible to compute in-set or
    // out-of-set with such a starting point.
    if (intersectionPointOnEdge) {
      throw invalid_argument(
          L"Intersection point landed on an edge -- illegal path");
    }
    constexpr bool intersectionPointInSet =
        intersectionPointOnEdge ||
        (((testPointEdgeIterator->getCrossingCount() & 1) == 0)
             ? testPointInSet
             : !testPointInSet);

    // System.out.println("  Intersection point in-set?
    // "+intersectionPointInSet+" On edge? "+intersectionPointOnEdge);

    // Now do the final leg
    // System.out.println(" Finding whether ["+x+","+y+","+z+"] is in-set, based
    // on travel from "+intersectionPoint+" along "+secondLegPlane+"
    // (value="+secondLegValue+")");
    shared_ptr<CountingEdgeIterator> *const travelEdgeIterator =
        outerInstance->createLinearCrossingEdgeIterator(
            intersectionPoint, secondLegPlane, secondLegAbovePlane,
            secondLegBelowPlane, x, y, z);
    // Traverse our way from the test point to the check point.
    secondLegTree->traverse(travelEdgeIterator, secondLegValue);
    constexpr bool rval = travelEdgeIterator->isOnEdge() ||
                          (((travelEdgeIterator->getCrossingCount() & 1) == 0)
                               ? intersectionPointInSet
                               : !intersectionPointInSet);

    // System.out.println(" Check point in set? "+rval);
    return rval;
  } catch (const invalid_argument &e) {
    // Intersection point apparently was on edge, so try another strategy
    // System.out.println(" Trying dual crossing edge iterator");
    shared_ptr<CountingEdgeIterator> *const edgeIterator =
        make_shared<DualCrossingEdgeIterator>(
            outerInstance, testPoint, firstLegPlane, firstLegAbovePlane,
            firstLegBelowPlane, secondLegPlane, secondLegAbovePlane,
            secondLegBelowPlane, x, y, z, intersectionPoint);
    firstLegTree->traverse(edgeIterator, firstLegValue);
    if (edgeIterator->isOnEdge()) {
      return true;
    }
    secondLegTree->traverse(edgeIterator, secondLegValue);
    return edgeIterator->isOnEdge() ||
           (((edgeIterator->getCrossingCount() & 1) == 0) ? testPointInSet
                                                          : !testPointInSet);
  }
}

int GeoComplexPolygon::TraversalStrategy::compareTo(
    shared_ptr<TraversalStrategy> other)
{
  if (traversalDistance < other->traversalDistance) {
    return -1;
  } else if (traversalDistance > other->traversalDistance) {
    return 1;
  }
  return 0;
}

GeoComplexPolygon::Node::Node(shared_ptr<Edge> edge, double const minimumValue,
                              double const maximumValue)
    : edge(edge), low(minimumValue), high(maximumValue)
{
  this->max = maximumValue;
}

bool GeoComplexPolygon::Node::traverse(shared_ptr<EdgeIterator> edgeIterator,
                                       double const minValue,
                                       double const maxValue)
{
  if (minValue <= max) {

    // Does this node overlap?
    if (minValue <= high && maxValue >= low) {
      if (edgeIterator->matches(edge) == false) {
        return false;
      }
    }

    if (left != nullptr &&
        left->traverse(edgeIterator, minValue, maxValue) == false) {
      return false;
    }
    if (right != nullptr && maxValue >= low &&
        right->traverse(edgeIterator, minValue, maxValue) == false) {
      return false;
    }
  }
  return true;
}

std::deque<std::shared_ptr<Edge>> const GeoComplexPolygon::Tree::EMPTY_ARRAY =
    std::deque<std::shared_ptr<Edge>>(0);

GeoComplexPolygon::Tree::Tree(deque<std::shared_ptr<Edge>> &allEdges)
    : rootNode(createTree(edges, 0, edges->length - 1))
{
  // Dump edges into an array and then sort it
  std::deque<std::shared_ptr<Node>> edges(allEdges.size());
  int i = 0;
  for (auto edge : allEdges) {
    edges[i++] = make_shared<Node>(edge, getMinimum(edge), getMaximum(edge));
  }
  Arrays::sort(edges, [&](left, right) {
    int ret = Double::compare(left::low, right::low);
    if (ret == 0) {
      ret = Double::compare(left::max, right::max);
    }
    return ret;
  });
}

shared_ptr<Node>
GeoComplexPolygon::Tree::createTree(std::deque<std::shared_ptr<Node>> &edges,
                                    int const low, int const high)
{
  if (low > high) {
    return nullptr;
  }
  // add midpoint
  int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
  shared_ptr<Node> *const newNode = edges[mid];
  // add children
  newNode->left = createTree(edges, low, mid - 1);
  newNode->right = createTree(edges, mid + 1, high);
  // pull up max values to this node
  if (newNode->left != nullptr) {
    newNode->max = max(newNode->max, newNode->left->max);
  }
  if (newNode->right != nullptr) {
    newNode->max = max(newNode->max, newNode->right->max);
  }
  return newNode;
}

bool GeoComplexPolygon::Tree::traverse(shared_ptr<EdgeIterator> edgeIterator,
                                       double const value)
{
  return traverse(edgeIterator, value, value);
}

bool GeoComplexPolygon::Tree::traverse(shared_ptr<EdgeIterator> edgeIterator,
                                       double const minValue,
                                       double const maxValue)
{
  if (rootNode == nullptr) {
    return true;
  }
  return rootNode->traverse(edgeIterator, minValue, maxValue);
}

GeoComplexPolygon::ZTree::ZTree(deque<std::shared_ptr<Edge>> &allEdges)
    : Tree(allEdges)
{
}

double GeoComplexPolygon::ZTree::getMinimum(shared_ptr<Edge> edge)
{
  return edge->planeBounds->getMinimumZ();
}

double GeoComplexPolygon::ZTree::getMaximum(shared_ptr<Edge> edge)
{
  return edge->planeBounds->getMaximumZ();
}

GeoComplexPolygon::YTree::YTree(deque<std::shared_ptr<Edge>> &allEdges)
    : Tree(allEdges)
{
}

double GeoComplexPolygon::YTree::getMinimum(shared_ptr<Edge> edge)
{
  return edge->planeBounds->getMinimumY();
}

double GeoComplexPolygon::YTree::getMaximum(shared_ptr<Edge> edge)
{
  return edge->planeBounds->getMaximumY();
}

GeoComplexPolygon::XTree::XTree(deque<std::shared_ptr<Edge>> &allEdges)
    : Tree(allEdges)
{
}

double GeoComplexPolygon::XTree::getMinimum(shared_ptr<Edge> edge)
{
  return edge->planeBounds->getMinimumX();
}

double GeoComplexPolygon::XTree::getMaximum(shared_ptr<Edge> edge)
{
  return edge->planeBounds->getMaximumX();
}

GeoComplexPolygon::IntersectorEdgeIterator::IntersectorEdgeIterator(
    shared_ptr<GeoComplexPolygon> outerInstance, shared_ptr<Plane> plane,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
    : plane(plane), notablePoints(notablePoints), bounds(bounds),
      outerInstance(outerInstance)
{
}

bool GeoComplexPolygon::IntersectorEdgeIterator::matches(shared_ptr<Edge> edge)
{
  return !plane->intersects(outerInstance->planetModel, edge->plane,
                            notablePoints, edge->notablePoints, bounds,
                            {edge->startPlane, edge->endPlane});
}

GeoComplexPolygon::IntersectorShapeIterator::IntersectorShapeIterator(
    shared_ptr<GeoComplexPolygon> outerInstance, shared_ptr<GeoShape> shape)
    : shape(shape), outerInstance(outerInstance)
{
}

bool GeoComplexPolygon::IntersectorShapeIterator::matches(shared_ptr<Edge> edge)
{
  return !shape->intersects(edge->plane, edge->notablePoints,
                            {edge->startPlane, edge->endPlane});
}

GeoComplexPolygon::FullLinearCrossingEdgeIterator::
    FullLinearCrossingEdgeIterator(
        shared_ptr<GeoComplexPolygon> outerInstance,
        shared_ptr<GeoPoint> testPoint, shared_ptr<Plane> plane,
        shared_ptr<Plane> abovePlane, shared_ptr<Plane> belowPlane,
        double const thePointX, double const thePointY, double const thePointZ)
    : testPoint(testPoint), plane(plane), abovePlane(abovePlane),
      belowPlane(belowPlane), bound(make_shared<SidedPlane>(plane, testPoint)),
      thePointX(thePointX), thePointY(thePointY), thePointZ(thePointZ),
      outerInstance(outerInstance)
{
  assert((plane->evaluateIsZero(thePointX, thePointY, thePointZ),
          L"Check point is not on travel plane"));
  assert(
      (plane->evaluateIsZero(testPoint), L"Test point is not on travel plane"));
  if (plane->isNumericallyIdentical(testPoint)) {
    throw invalid_argument(L"Plane deque identical to testpoint deque");
  }
  // It doesn't matter which 1/2 of the world we choose, but we must choose only
  // one.
  // System.out.println(" Constructing full linear crossing edge iterator");
  // debugIntersectAllEdges(plane, bound);
}

int GeoComplexPolygon::FullLinearCrossingEdgeIterator::getCrossingCount()
{
  return min(aboveCrossingCount, belowCrossingCount);
}

bool GeoComplexPolygon::FullLinearCrossingEdgeIterator::isOnEdge()
{
  return onEdge;
}

bool GeoComplexPolygon::FullLinearCrossingEdgeIterator::matches(
    shared_ptr<Edge> edge)
{
  // System.out.println(" Edge ["+edge.startPoint+" --> "+edge.endPoint+"]
  // potentially crosses travel plane "+plane);
  // Early exit if the point is on the edge.
  if (edge->isWithin(thePointX, thePointY, thePointZ)) {
    // System.out.println("  Point is on the edge; in-set");
    onEdge = true;
    return false;
  }

  // This should precisely mirror what is in DualCrossingIterator, but without
  // the dual crossings. Some edges are going to be given to us even when
  // there's no real intersection, so do that as a sanity check, first.
  std::deque<std::shared_ptr<GeoPoint>> planeCrossings =
      plane->findIntersections(outerInstance->planetModel, edge->plane,
                               {bound, edge->startPlane, edge->endPlane});
  if (planeCrossings.size() > 0 && planeCrossings.empty()) {
    // Sometimes on the hairy edge an intersection will be missed.  This check
    // finds those.
    if (!plane->evaluateIsZero(edge->startPoint) &&
        !plane->evaluateIsZero(edge->endPoint)) {
      return true;
    }
  }

  // System.out.println("  Edge intersects travel plane "+plane);

  // Determine crossings of this edge against all inside/outside planes. There's
  // no further need to look at the actual travel plane itself.
  constexpr int aboveCrossings = countCrossings(edge, abovePlane, bound);
  aboveCrossingCount += aboveCrossings;
  constexpr int belowCrossings = countCrossings(edge, belowPlane, bound);
  belowCrossingCount += belowCrossings;
  // System.out.println("  Above crossings = "+aboveCrossings+"; below crossings
  // = "+belowCrossings);

  return true;
}

int GeoComplexPolygon::FullLinearCrossingEdgeIterator::countCrossings(
    shared_ptr<Edge> edge, shared_ptr<Plane> envelopePlane,
    shared_ptr<Membership> envelopeBound)
{
  std::deque<std::shared_ptr<GeoPoint>> intersections =
      edge->plane->findIntersections(outerInstance->planetModel, envelopePlane,
                                     {envelopeBound});
  int crossings = 0;
  if (intersections.size() > 0) {
    for (auto intersection : intersections) {
      if (edge->startPlane->strictlyWithin(intersection) &&
          edge->endPlane->strictlyWithin(intersection)) {
        // It's unique, so assess it
        crossings +=
            edgeCrossesEnvelope(edge->plane, intersection, envelopePlane) ? 1
                                                                          : 0;
      }
    }
  }
  return crossings;
}

bool GeoComplexPolygon::FullLinearCrossingEdgeIterator::edgeCrossesEnvelope(
    shared_ptr<Plane> edgePlane, shared_ptr<GeoPoint> intersectionPoint,
    shared_ptr<Plane> envelopePlane)
{
  std::deque<std::shared_ptr<GeoPoint>> adjoiningPoints =
      outerInstance->findAdjoiningPoints(edgePlane, intersectionPoint,
                                         envelopePlane);
  if (adjoiningPoints.empty()) {
    return true;
  }
  int withinCount = 0;
  for (auto adjoining : adjoiningPoints) {
    if (plane->evaluateIsZero(adjoining) && bound->isWithin(adjoining)) {
      withinCount++;
    }
  }
  return (withinCount & 1) != 0;
}

GeoComplexPolygon::SectorLinearCrossingEdgeIterator::
    SectorLinearCrossingEdgeIterator(
        shared_ptr<GeoComplexPolygon> outerInstance,
        shared_ptr<GeoPoint> testPoint, shared_ptr<Plane> plane,
        shared_ptr<Plane> abovePlane, shared_ptr<Plane> belowPlane,
        double const thePointX, double const thePointY, double const thePointZ)
    : testPoint(testPoint), plane(plane), abovePlane(abovePlane),
      belowPlane(belowPlane), bound1(bound1Plane), bound2(bound2Plane),
      thePointX(thePointX), thePointY(thePointY), thePointZ(thePointZ),
      outerInstance(outerInstance)
{
  assert((plane->evaluateIsZero(thePointX, thePointY, thePointZ),
          L"Check point is not on travel plane"));
  assert(
      (plane->evaluateIsZero(testPoint), L"Test point is not on travel plane"));
  // We have to be sure we don't accidently create two bounds that would exclude
  // all points. Not sure this can happen but...
  shared_ptr<SidedPlane> *const bound1Plane = make_shared<SidedPlane>(
      thePointX, thePointY, thePointZ, plane, testPoint);
  shared_ptr<SidedPlane> *const bound2Plane = make_shared<SidedPlane>(
      testPoint, plane, thePointX, thePointY, thePointZ);
  if (bound1Plane->isNumericallyIdentical(bound2Plane)) {
    throw invalid_argument(L"Sector iterator unreliable when bounds planes are "
                           L"numerically identical");
  }
  // System.out.println(" Constructing sector linear crossing edge iterator");
  // debugIntersectAllEdges(plane, bound1, bound2);
}

int GeoComplexPolygon::SectorLinearCrossingEdgeIterator::getCrossingCount()
{
  return min(aboveCrossingCount, belowCrossingCount);
}

bool GeoComplexPolygon::SectorLinearCrossingEdgeIterator::isOnEdge()
{
  return onEdge;
}

bool GeoComplexPolygon::SectorLinearCrossingEdgeIterator::matches(
    shared_ptr<Edge> edge)
{
  // System.out.println(" Edge ["+edge.startPoint+" --> "+edge.endPoint+"]
  // potentially crosses travel plane "+plane);
  // Early exit if the point is on the edge.
  if (edge->isWithin(thePointX, thePointY, thePointZ)) {
    // The point is on the edge.  This means it's "in-set" by definition, so
    // abort.
    // System.out.println("  Point is on the edge; in-set");
    onEdge = true;
    return false;
  }

  // System.out.println("  Finding intersections between edge plane and travel
  // plane...");

  // This should precisely mirror what is in DualCrossingIterator, but without
  // the dual crossings. Some edges are going to be given to us even when
  // there's no real intersection, so do that as a sanity check, first.
  std::deque<std::shared_ptr<GeoPoint>> planeCrossings =
      plane->findIntersections(
          outerInstance->planetModel, edge->plane,
          {bound1, bound2, edge->startPlane, edge->endPlane});
  if (planeCrossings.empty()) {
    // System.out.println("  Planes were identical");
  } else if (planeCrossings.empty()) {
    // System.out.println("  There are no intersection points within bounds.");
    /*
    // For debugging purposes, let's repeat the intersection check without
    bounds, and figure out which bound(s) rejected it final GeoPoint[]
    unboundedCrossings = plane.findIntersections(planetModel, edge.plane); for
    (final GeoPoint crossing : unboundedCrossings) { if
    (!bound1.isWithin(crossing)) { System.out.println("   Crossing point
    "+crossing+" rejected by bound1
    ("+((SidedPlane)bound1).evaluate(crossing)+")");
      }
      if (!bound2.isWithin(crossing)) {
        System.out.println("   Crossing point "+crossing+" rejected by bound2
    ("+((SidedPlane)bound2).evaluate(crossing)+")");
      }
      if (!edge.startPlane.isWithin(crossing)) {
        System.out.println("   Crossing point "+crossing+" rejected by
    edge.startPlane ("+((SidedPlane)edge.startPlane).evaluate(crossing)+")");
      }
      if (!edge.endPlane.isWithin(crossing)) {
        System.out.println("   Crossing point "+crossing+" rejected by
    edge.endPlane ("+((SidedPlane)edge.endPlane).evaluate(crossing)+")");
      }
    }
    */
    // Sometimes on the hairy edge an intersection will be missed.  This check
    // finds those.
    if (!plane->evaluateIsZero(edge->startPoint) &&
        !plane->evaluateIsZero(edge->endPoint)) {
      // System.out.println("   Endpoint(s) of edge are not on travel plane;
      // distances: "+plane.evaluate(edge.startPoint)+" and
      // "+plane.evaluate(edge.endPoint));
      // Edge doesn't actually intersect the travel plane.
      return true;
    } else {
      // System.out.println("   Endpoint(s) of edge are on travel plane!");
    }
  } else {
    // System.out.println("  There were intersection points!");
  }

  // System.out.println("  Edge intersects travel plane");

  // Determine crossings of this edge against all inside/outside planes. There's
  // no further need to look at the actual travel plane itself.
  // System.out.println("  Getting above crossings...");
  constexpr int aboveCrossings =
      countCrossings(edge, abovePlane, bound1, bound2);
  aboveCrossingCount += aboveCrossings;
  // System.out.println("  Getting below crossings...");
  constexpr int belowCrossings =
      countCrossings(edge, belowPlane, bound1, bound2);
  belowCrossingCount += belowCrossings;
  // System.out.println("  Above crossings = "+aboveCrossings+"; below crossings
  // = "+belowCrossings);

  return true;
}

int GeoComplexPolygon::SectorLinearCrossingEdgeIterator::countCrossings(
    shared_ptr<Edge> edge, shared_ptr<Plane> envelopePlane,
    shared_ptr<Membership> envelopeBound1,
    shared_ptr<Membership> envelopeBound2)
{
  std::deque<std::shared_ptr<GeoPoint>> intersections =
      edge->plane->findIntersections(outerInstance->planetModel, envelopePlane,
                                     {envelopeBound1, envelopeBound2});
  int crossings = 0;
  if (intersections.size() > 0) {
    for (auto intersection : intersections) {
      if (edge->startPlane->strictlyWithin(intersection) &&
          edge->endPlane->strictlyWithin(intersection)) {
        // System.out.println("   Envelope intersection point = "+intersection);
        // It's unique, so assess it
        constexpr int counter =
            edgeCrossesEnvelope(edge->plane, intersection, envelopePlane) ? 1
                                                                          : 0;
        // System.out.println("   Edge crosses envelope "+counter+" times");
        crossings += counter;
      }
    }
  } else {
    // System.out.println("   Intersections = null");
  }
  return crossings;
}

bool GeoComplexPolygon::SectorLinearCrossingEdgeIterator::edgeCrossesEnvelope(
    shared_ptr<Plane> edgePlane, shared_ptr<GeoPoint> intersectionPoint,
    shared_ptr<Plane> envelopePlane)
{
  std::deque<std::shared_ptr<GeoPoint>> adjoiningPoints =
      outerInstance->findAdjoiningPoints(edgePlane, intersectionPoint,
                                         envelopePlane);
  if (adjoiningPoints.empty()) {
    // System.out.println("    No adjoining points");
    return true;
  }
  int withinCount = 0;
  for (auto adjoining : adjoiningPoints) {
    // System.out.println("    Adjoining point "+adjoining);
    if (plane->evaluateIsZero(adjoining) && bound1->isWithin(adjoining) &&
        bound2->isWithin(adjoining)) {
      // System.out.println("     within!!");
      withinCount++;
    } else {
      // System.out.println("     evaluateIsZero?
      // "+plane.evaluateIsZero(adjoining)+" bound1.isWithin?
      // "+bound1.isWithin(adjoining)+" bound2.isWithin?
      // "+bound2.isWithin(adjoining));
    }
  }
  return (withinCount & 1) != 0;
}

GeoComplexPolygon::DualCrossingEdgeIterator::DualCrossingEdgeIterator(
    shared_ptr<GeoComplexPolygon> outerInstance, shared_ptr<GeoPoint> testPoint,
    shared_ptr<Plane> testPointPlane, shared_ptr<Plane> testPointAbovePlane,
    shared_ptr<Plane> testPointBelowPlane, shared_ptr<Plane> travelPlane,
    shared_ptr<Plane> travelAbovePlane, shared_ptr<Plane> travelBelowPlane,
    double const thePointX, double const thePointY, double const thePointZ,
    shared_ptr<GeoPoint> intersectionPoint)
    : testPoint(testPoint), testPointPlane(testPointPlane),
      testPointAbovePlane(testPointAbovePlane),
      testPointBelowPlane(testPointBelowPlane), travelPlane(travelPlane),
      travelAbovePlane(travelAbovePlane), travelBelowPlane(travelBelowPlane),
      thePointX(thePointX), thePointY(thePointY), thePointZ(thePointZ),
      intersectionPoint(intersectionPoint),
      testPointCutoffPlane(testPointBound1),
      checkPointCutoffPlane(checkPointBound1),
      testPointOtherCutoffPlane(testPointBound2),
      checkPointOtherCutoffPlane(checkPointBound2), outerInstance(outerInstance)
{

  // System.out.println("Intersection point = "+intersectionPoint);
  // System.out.println("TestPoint plane: "+testPoint+" -> "+intersectionPoint);
  // System.out.println("Travel plane:
  // ["+thePointX+","+thePointY+","+thePointZ+"] -> "+intersectionPoint);

  assert((travelPlane->evaluateIsZero(intersectionPoint),
          L"intersection point must be on travel plane"));
  assert((testPointPlane->evaluateIsZero(intersectionPoint),
          L"intersection point must be on test point plane"));

  // System.out.println("Test point distance to intersection point:
  // "+intersectionPoint.linearDistance(testPoint)); System.out.println("Check
  // point distance to intersection point:
  // "+intersectionPoint.linearDistance(thePointX, thePointY, thePointZ));

  assert((!testPoint->isNumericallyIdentical(intersectionPoint),
          L"test point is the same as intersection point"));
  assert((!intersectionPoint->isNumericallyIdentical(thePointX, thePointY,
                                                     thePointZ),
          L"check point is same as intersection point"));

  /*
  final SidedPlane bound1Plane = new SidedPlane(thePointX, thePointY, thePointZ,
  plane, testPoint); final SidedPlane bound2Plane = new SidedPlane(testPoint,
  plane, thePointX, thePointY, thePointZ); if
  (bound1Plane.isNumericallyIdentical(bound2Plane)) { throw new
  IllegalArgumentException("Sector iterator unreliable when bounds planes are
  numerically identical");
  }
  */

  shared_ptr<SidedPlane> *const testPointBound1 =
      make_shared<SidedPlane>(intersectionPoint, testPointPlane, testPoint);
  shared_ptr<SidedPlane> *const testPointBound2 =
      make_shared<SidedPlane>(testPoint, testPointPlane, intersectionPoint);
  if (testPointBound1->isNumericallyIdentical(testPointBound2)) {
    throw invalid_argument(L"Dual iterator unreliable when bounds planes are "
                           L"numerically identical");
  }

  shared_ptr<SidedPlane> *const checkPointBound1 = make_shared<SidedPlane>(
      intersectionPoint, travelPlane, thePointX, thePointY, thePointZ);
  shared_ptr<SidedPlane> *const checkPointBound2 = make_shared<SidedPlane>(
      thePointX, thePointY, thePointZ, travelPlane, intersectionPoint);
  if (checkPointBound1->isNumericallyIdentical(checkPointBound2)) {
    throw invalid_argument(L"Dual iterator unreliable when bounds planes are "
                           L"numerically identical");
  }

  // Sanity check
  assert((testPointCutoffPlane->isWithin(intersectionPoint),
          L"intersection must be within testPointCutoffPlane"));
  assert((testPointOtherCutoffPlane->isWithin(intersectionPoint),
          L"intersection must be within testPointOtherCutoffPlane"));
  assert((checkPointCutoffPlane->isWithin(intersectionPoint),
          L"intersection must be within checkPointCutoffPlane"));
  assert((checkPointOtherCutoffPlane->isWithin(intersectionPoint),
          L"intersection must be within checkPointOtherCutoffPlane"));
}

void GeoComplexPolygon::DualCrossingEdgeIterator::computeInsideOutside()
{
  if (!computedInsideOutside) {
    // Convert travel plane to a sided plane
    shared_ptr<Membership> *const intersectionBound1 =
        make_shared<SidedPlane>(testPoint, travelPlane, travelPlane->D);
    // Convert testPoint plane to a sided plane
    shared_ptr<Membership> *const intersectionBound2 = make_shared<SidedPlane>(
        thePointX, thePointY, thePointZ, testPointPlane, testPointPlane->D);

    assert((intersectionBound1->isWithin(intersectionPoint),
            L"intersection must be within intersectionBound1"));
    assert((intersectionBound2->isWithin(intersectionPoint),
            L"intersection must be within intersectionBound2"));

    // Figure out which of the above/below planes are inside vs. outside.  To do
    // this, we look for the point that is within the bounds of the
    // testPointPlane and travelPlane.  The two sides that intersected there are
    // the inside borders. Each of these can generate two solutions.  We need to
    // refine them to generate only one somehow -- the one in the same area of
    // the world as intersectionPoint. Since the travel/testpoint planes have
    // one fixed coordinate, and that is represented by the plane's D value, it
    // should be possible to choose based on the point's coordinates.
    std::deque<std::shared_ptr<GeoPoint>> aboveAbove =
        travelAbovePlane->findIntersections(
            outerInstance->planetModel, testPointAbovePlane,
            {intersectionBound1, intersectionBound2});
    assert((aboveAbove.size() > 0, L"Above + above should not be coplanar"));
    std::deque<std::shared_ptr<GeoPoint>> aboveBelow =
        travelAbovePlane->findIntersections(
            outerInstance->planetModel, testPointBelowPlane,
            {intersectionBound1, intersectionBound2});
    assert((aboveBelow.size() > 0, L"Above + below should not be coplanar"));
    std::deque<std::shared_ptr<GeoPoint>> belowBelow =
        travelBelowPlane->findIntersections(
            outerInstance->planetModel, testPointBelowPlane,
            {intersectionBound1, intersectionBound2});
    assert((belowBelow.size() > 0, L"Below + below should not be coplanar"));
    std::deque<std::shared_ptr<GeoPoint>> belowAbove =
        travelBelowPlane->findIntersections(
            outerInstance->planetModel, testPointAbovePlane,
            {intersectionBound1, intersectionBound2});
    assert((belowAbove.size() > 0, L"Below + above should not be coplanar"));

    assert((aboveAbove.size() > 0) ? 1 : 0) +
            ((aboveBelow.size() > 0) ? 1 : 0) +
            ((belowBelow.size() > 0) ? 1 : 0) +
            ((belowAbove.size() > 0) ? 1 : 0) ==
        1 : L"Can be exactly one inside point, instead was: aa=" +
            aboveAbove.size() + L" ab=" + aboveBelow.size() + L" bb=" +
            belowBelow.size() + L" ba=" + belowAbove.size();

    std::deque<std::shared_ptr<GeoPoint>> insideInsidePoints;
    if (aboveAbove.size() > 0) {
      travelInsidePlane = travelAbovePlane;
      testPointInsidePlane = testPointAbovePlane;
      travelOutsidePlane = travelBelowPlane;
      testPointOutsidePlane = testPointBelowPlane;
      insideInsidePoints = aboveAbove;
    } else if (aboveBelow.size() > 0) {
      travelInsidePlane = travelAbovePlane;
      testPointInsidePlane = testPointBelowPlane;
      travelOutsidePlane = travelBelowPlane;
      testPointOutsidePlane = testPointAbovePlane;
      insideInsidePoints = aboveBelow;
    } else if (belowBelow.size() > 0) {
      travelInsidePlane = travelBelowPlane;
      testPointInsidePlane = testPointBelowPlane;
      travelOutsidePlane = travelAbovePlane;
      testPointOutsidePlane = testPointAbovePlane;
      insideInsidePoints = belowBelow;
    } else if (belowAbove.size() > 0) {
      travelInsidePlane = travelBelowPlane;
      testPointInsidePlane = testPointAbovePlane;
      travelOutsidePlane = travelAbovePlane;
      testPointOutsidePlane = testPointBelowPlane;
      insideInsidePoints = belowAbove;
    } else {
      throw make_shared<IllegalStateException>(
          L"Can't find traversal intersection among: " + travelAbovePlane +
          L", " + testPointAbovePlane + L", " + travelBelowPlane + L", " +
          testPointBelowPlane);
    }

    // Get the inside-inside intersection point
    // Picking which point, out of two, that corresponds to the already-selected
    // intersectionPoint, is tricky, but it must be done. We expect the choice
    // to be within a small delta of the intersection point in 2 of the
    // dimensions, but not the third
    shared_ptr<GeoPoint> *const insideInsidePoint =
        pickProximate(insideInsidePoints);

    // Get the outside-outside intersection point
    // System.out.println("Computing outside-outside intersection");
    std::deque<std::shared_ptr<GeoPoint>> outsideOutsidePoints =
        testPointOutsidePlane->findIntersections(
            outerInstance->planetModel,
            travelOutsidePlane); // these don't add anything: ,
                                 // checkPointCutoffPlane, testPointCutoffPlane);
    shared_ptr<GeoPoint> *const outsideOutsidePoint =
        pickProximate(outsideOutsidePoints);

    insideTravelCutoffPlane = make_shared<SidedPlane>(
        thePointX, thePointY, thePointZ, travelInsidePlane, insideInsidePoint);
    outsideTravelCutoffPlane =
        make_shared<SidedPlane>(thePointX, thePointY, thePointZ,
                                travelInsidePlane, outsideOutsidePoint);
    insideTestPointCutoffPlane = make_shared<SidedPlane>(
        testPoint, testPointInsidePlane, insideInsidePoint);
    outsideTestPointCutoffPlane = make_shared<SidedPlane>(
        testPoint, testPointOutsidePlane, outsideOutsidePoint);

    /*
    System.out.println("insideTravelCutoffPlane = "+insideTravelCutoffPlane);
    System.out.println("outsideTravelCutoffPlane = "+outsideTravelCutoffPlane);
    System.out.println("insideTestPointCutoffPlane =
    "+insideTestPointCutoffPlane);
    System.out.println("outsideTestPointCutoffPlane =
    "+outsideTestPointCutoffPlane);
    */

    computedInsideOutside = true;
  }
}

shared_ptr<GeoPoint> GeoComplexPolygon::DualCrossingEdgeIterator::pickProximate(
    std::deque<std::shared_ptr<GeoPoint>> &points)
{
  if (points.empty()) {
    throw invalid_argument(L"No off-plane intersection points were found; "
                           L"can't compute traversal");
  } else if (points.size() == 1) {
    return points[0];
  } else {
    constexpr double p1dist =
        computeSquaredDistance(points[0], intersectionPoint);
    constexpr double p2dist =
        computeSquaredDistance(points[1], intersectionPoint);
    if (p1dist < p2dist) {
      return points[0];
    } else if (p2dist < p1dist) {
      return points[1];
    } else {
      throw invalid_argument(L"Neither off-plane intersection point matched "
                             L"intersection point; intersection = " +
                             intersectionPoint + L"; offplane choice 0: " +
                             points[0] + L"; offplane choice 1: " + points[1]);
    }
  }
}

int GeoComplexPolygon::DualCrossingEdgeIterator::getCrossingCount()
{
  // Doesn't return the actual crossing count -- just gets the even/odd part
  // right
  return min(innerCrossingCount, outerCrossingCount);
}

bool GeoComplexPolygon::DualCrossingEdgeIterator::isOnEdge() { return onEdge; }

bool GeoComplexPolygon::DualCrossingEdgeIterator::matches(shared_ptr<Edge> edge)
{
  // Early exit if the point is on the edge, in which case we accidentally
  // discovered the answer.
  if (edge->isWithin(thePointX, thePointY, thePointZ)) {
    onEdge = true;
    return false;
  }

  // All edges that touch the travel planes get assessed the same.  So, for each
  // intersecting edge on both legs: (1) If the edge contains the intersection
  // point, we analyze it on only one leg.  For the other leg, we do nothing.
  // (2) We compute the crossings of the edge with ALL FOUR inner and outer
  // bounding planes. (3) We add the numbers of each kind of crossing to the
  // total for that class of crossing (innerTotal and outerTotal). (4) When done
  // all edges tallied in this way, we take min(innerTotal, outerTotal) and
  // assume that is the number of crossings.
  //
  // Q: What if we see the same edge in both traversals?
  // A: We should really evaluate it only in one.  Keep a hash of the edges
  // we've looked at already and don't process edges twice.

  // Every edge should be looked at only once.
  if (seenEdges != nullptr && seenEdges->contains(edge)) {
    return true;
  }
  if (seenEdges == nullptr) {
    seenEdges = unordered_set<>();
  }
  seenEdges->add(edge);

  // We've never seen this edge before.  Evaluate it in the context of inner and
  // outer planes.
  computeInsideOutside();

  /*
  System.out.println("\nThe following edges should intersect the
  travel/testpoint planes:"); Edge thisEdge = edge; while (true) { final
  GeoPoint[] travelCrossings = travelPlane.findIntersections(planetModel,
  thisEdge.plane, checkPointCutoffPlane, checkPointOtherCutoffPlane,
  thisEdge.startPlane, thisEdge.endPlane); if (travelCrossings == null ||
  travelCrossings.length > 0) { System.out.println("Travel plane:
  "+thisEdge.startPoint+" -> "+thisEdge.endPoint);
    }
    final GeoPoint[] testPointCrossings =
  testPointPlane.findIntersections(planetModel, thisEdge.plane,
  testPointCutoffPlane, testPointOtherCutoffPlane, thisEdge.startPlane,
  thisEdge.endPlane); if (testPointCrossings == null ||
  testPointCrossings.length > 0) { System.out.println("Test point plane:
  "+thisEdge.startPoint+" -> "+thisEdge.endPoint);
    }
    thisEdge = thisEdge.next;
    if (thisEdge == edge) {
      break;
    }
  }
  */

  // System.out.println("");
  // System.out.println("Considering edge "+(edge.startPoint)+" ->
  // "+(edge.endPoint));

  // Some edges are going to be given to us even when there's no real
  // intersection, so do that as a sanity check, first.
  std::deque<std::shared_ptr<GeoPoint>> travelCrossings =
      travelPlane->findIntersections(outerInstance->planetModel, edge->plane,
                                     {checkPointCutoffPlane,
                                      checkPointOtherCutoffPlane,
                                      edge->startPlane, edge->endPlane});
  if (travelCrossings.size() > 0 && travelCrossings.empty()) {
    // System.out.println(" No intersections with travel plane...");
    std::deque<std::shared_ptr<GeoPoint>> testPointCrossings =
        testPointPlane->findIntersections(
            outerInstance->planetModel, edge->plane,
            {testPointCutoffPlane, testPointOtherCutoffPlane, edge->startPlane,
             edge->endPlane});
    if (testPointCrossings.size() > 0 && testPointCrossings.empty()) {
      // As a last resort, see if the edge endpoints are on either plane.  This
      // is sometimes necessary because the intersection computation logic might
      // not detect near-miss edges otherwise.
      // System.out.println(" No intersections with testpoint plane...");
      if (!travelPlane->evaluateIsZero(edge->startPoint) &&
          !travelPlane->evaluateIsZero(edge->endPoint) &&
          !testPointPlane->evaluateIsZero(edge->startPoint) &&
          !testPointPlane->evaluateIsZero(edge->endPoint)) {
        return true;
      } else {
        // System.out.println("
        // Startpoint/travelPlane="+travelPlane.evaluate(edge.startPoint)+"
        // Startpoint/testPointPlane="+testPointPlane.evaluate(edge.startPoint));
        // System.out.println("
        // Endpoint/travelPlane="+travelPlane.evaluate(edge.endPoint)+"
        // Endpoint/testPointPlane="+testPointPlane.evaluate(edge.endPoint));
      }
    } else {
      // System.out.println(" Intersection found with testPoint plane...");
    }
  } else {
    // System.out.println(" Intersection found with travel plane...");
  }

  // System.out.println(" Edge intersects travel or testPoint plane");
  /*
  System.out.println(
    " start point travel dist="+travelPlane.evaluate(edge.startPoint)+"; end
  point travel dist="+travelPlane.evaluate(edge.endPoint)); System.out.println(
    " start point travel above
  dist="+travelAbovePlane.evaluate(edge.startPoint)+"; end point travel above
  dist="+travelAbovePlane.evaluate(edge.endPoint)); System.out.println( " start
  point travel below dist="+travelBelowPlane.evaluate(edge.startPoint)+"; end
  point travel below dist="+travelBelowPlane.evaluate(edge.endPoint));
  System.out.println(
    " start point testpoint dist="+testPointPlane.evaluate(edge.startPoint)+";
  end point testpoint dist="+testPointPlane.evaluate(edge.endPoint));
  System.out.println(
    " start point testpoint above
  dist="+testPointAbovePlane.evaluate(edge.startPoint)+"; end point testpoint
  above dist="+testPointAbovePlane.evaluate(edge.endPoint)); System.out.println(
    " start point testpoint below
  dist="+testPointBelowPlane.evaluate(edge.startPoint)+"; end point testpoint
  below dist="+testPointBelowPlane.evaluate(edge.endPoint));
  */

  // Determine crossings of this edge against all inside/outside planes. There's
  // no further need to look at the actual travel plane itself.
  // System.out.println(" Assessing inner crossings...");
  innerCrossingCount += countCrossings(
      edge, travelInsidePlane, checkPointCutoffPlane, insideTravelCutoffPlane,
      testPointInsidePlane, testPointCutoffPlane, insideTestPointCutoffPlane);
  // System.out.println(" Assessing outer crossings...");
  outerCrossingCount += countCrossings(
      edge, travelOutsidePlane, checkPointCutoffPlane, outsideTravelCutoffPlane,
      testPointOutsidePlane, testPointCutoffPlane, outsideTestPointCutoffPlane);
  /*
  final GeoPoint[] travelInnerCrossings = computeCrossings(travelInsidePlane,
  edge, checkPointCutoffPlane, insideTravelCutoffPlane); final GeoPoint[]
  travelOuterCrossings = computeCrossings(travelOutsidePlane, edge,
  checkPointCutoffPlane, outsideTravelCutoffPlane); final GeoPoint[]
  testPointInnerCrossings = computeCrossings(testPointInsidePlane, edge,
  testPointCutoffPlane, insideTestPointCutoffPlane); final GeoPoint[]
  testPointOuterCrossings = computeCrossings(testPointOutsidePlane, edge,
  testPointCutoffPlane, outsideTestPointCutoffPlane);
  */

  return true;
}

int GeoComplexPolygon::DualCrossingEdgeIterator::countCrossings(
    shared_ptr<Edge> edge, shared_ptr<Plane> travelEnvelopePlane,
    shared_ptr<Membership> travelEnvelopeBound1,
    shared_ptr<Membership> travelEnvelopeBound2,
    shared_ptr<Plane> testPointEnvelopePlane,
    shared_ptr<Membership> testPointEnvelopeBound1,
    shared_ptr<Membership> testPointEnvelopeBound2)
{
  std::deque<std::shared_ptr<GeoPoint>> travelIntersections =
      edge->plane->findIntersections(
          outerInstance->planetModel, travelEnvelopePlane,
          {travelEnvelopeBound1, travelEnvelopeBound2});
  std::deque<std::shared_ptr<GeoPoint>> testPointIntersections =
      edge->plane->findIntersections(
          outerInstance->planetModel, testPointEnvelopePlane,
          {testPointEnvelopeBound1, testPointEnvelopeBound2});
  int crossings = 0;
  if (travelIntersections.size() > 0) {
    for (auto intersection : travelIntersections) {
      if (edge->startPlane->strictlyWithin(intersection) &&
          edge->endPlane->strictlyWithin(intersection)) {
        // Make sure it's not a dup
        bool notDup = true;
        if (testPointIntersections.size() > 0) {
          for (auto otherIntersection : testPointIntersections) {
            if (edge->startPlane->strictlyWithin(otherIntersection) &&
                edge->endPlane->strictlyWithin(otherIntersection) &&
                intersection->isNumericallyIdentical(otherIntersection)) {
              // System.out.println("  Points "+intersection+" and
              // "+otherIntersection+" are duplicates");
              notDup = false;
              break;
            }
          }
        }
        if (!notDup) {
          continue;
        }
        // It's unique, so assess it
        // System.out.println("  Assessing travel envelope intersection point
        // "+intersection+", travelPlane
        // distance="+travelPlane.evaluate(intersection)+"...");
        crossings +=
            edgeCrossesEnvelope(edge->plane, intersection, travelEnvelopePlane)
                ? 1
                : 0;
      }
    }
  }
  if (testPointIntersections.size() > 0) {
    for (auto intersection : testPointIntersections) {
      if (edge->startPlane->strictlyWithin(intersection) &&
          edge->endPlane->strictlyWithin(intersection)) {
        // It's unique, so assess it
        // System.out.println("  Assessing testpoint envelope intersection point
        // "+intersection+", testPointPlane
        // distance="+testPointPlane.evaluate(intersection)+"...");
        crossings += edgeCrossesEnvelope(edge->plane, intersection,
                                         testPointEnvelopePlane)
                         ? 1
                         : 0;
      }
    }
  }
  return crossings;
}

bool GeoComplexPolygon::DualCrossingEdgeIterator::edgeCrossesEnvelope(
    shared_ptr<Plane> edgePlane, shared_ptr<GeoPoint> intersectionPoint,
    shared_ptr<Plane> envelopePlane)
{
  std::deque<std::shared_ptr<GeoPoint>> adjoiningPoints =
      outerInstance->findAdjoiningPoints(edgePlane, intersectionPoint,
                                         envelopePlane);
  if (adjoiningPoints.empty()) {
    // Couldn't find good adjoining points, so just assume there is a crossing.
    return true;
  }
  int withinCount = 0;
  for (auto adjoining : adjoiningPoints) {
    if ((travelPlane->evaluateIsZero(adjoining) &&
         checkPointCutoffPlane->isWithin(adjoining) &&
         checkPointOtherCutoffPlane->isWithin(adjoining)) ||
        (testPointPlane->evaluateIsZero(adjoining) &&
         testPointCutoffPlane->isWithin(adjoining) &&
         testPointOtherCutoffPlane->isWithin(adjoining))) {
      // System.out.println("   Adjoining point "+adjoining+" (intersection dist
      // = "+intersectionPoint.linearDistance(adjoining)+") is within");
      withinCount++;
    } else {
      // System.out.println("   Adjoining point "+adjoining+" (intersection dist
      // = "+intersectionPoint.linearDistance(adjoining)+"; travelPlane
      // dist="+travelPlane.evaluate(adjoining)+"; testPointPlane
      // dist="+testPointPlane.evaluate(adjoining)+") is not within");
    }
  }
  return (withinCount & 1) != 0;
}

const double GeoComplexPolygon::OFF_PLANE_AMOUNT =
    Vector::MINIMUM_RESOLUTION * 0.1;

std::deque<std::shared_ptr<GeoPoint>>
GeoComplexPolygon::findAdjoiningPoints(shared_ptr<Plane> plane,
                                       shared_ptr<GeoPoint> pointOnPlane,
                                       shared_ptr<Plane> envelopePlane)
{
  // Compute a normalized perpendicular deque
  shared_ptr<Vector> *const perpendicular =
      make_shared<Vector>(plane, pointOnPlane);
  double distanceFactor = 0.0;
  for (int i = 0; i < MAX_ITERATIONS; i++) {
    distanceFactor += DELTA_DISTANCE;
    // Compute two new points along this deque from the original
    shared_ptr<GeoPoint> *const pointA = planetModel->createSurfacePoint(
        pointOnPlane->x + perpendicular->x * distanceFactor,
        pointOnPlane->y + perpendicular->y * distanceFactor,
        pointOnPlane->z + perpendicular->z * distanceFactor);
    shared_ptr<GeoPoint> *const pointB = planetModel->createSurfacePoint(
        pointOnPlane->x - perpendicular->x * distanceFactor,
        pointOnPlane->y - perpendicular->y * distanceFactor,
        pointOnPlane->z - perpendicular->z * distanceFactor);
    if (abs(envelopePlane->evaluate(pointA)) > OFF_PLANE_AMOUNT &&
        abs(envelopePlane->evaluate(pointB)) > OFF_PLANE_AMOUNT) {
      // System.out.println("Distance: "+computeSquaredDistance(rval[0],
      // pointOnPlane)+" and "+computeSquaredDistance(rval[1], pointOnPlane));
      return std::deque<std::shared_ptr<GeoPoint>>{pointA, pointB};
    }
    // Loop back around and use a bigger delta
  }
  // Had to abort, so return null.
  // System.out.println("     Adjoining points not found.  Are planes parallel?
  // edge = "+plane+"; envelope = "+envelopePlane+"; perpendicular =
  // "+perpendicular);
  return nullptr;
}

double GeoComplexPolygon::computeSquaredDistance(
    shared_ptr<GeoPoint> checkPoint, shared_ptr<GeoPoint> intersectionPoint)
{
  constexpr double distanceX = checkPoint->x - intersectionPoint->x;
  constexpr double distanceY = checkPoint->y - intersectionPoint->y;
  constexpr double distanceZ = checkPoint->z - intersectionPoint->z;
  return distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ;
}

bool GeoComplexPolygon::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoComplexPolygon>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoComplexPolygon> *const other =
      any_cast<std::shared_ptr<GeoComplexPolygon>>(o);
  return GeoBasePolygon::equals(other) &&
         testPoint1InSet == other->testPoint1InSet &&
         testPoint1->equals(testPoint1) && pointsList.equals(other->pointsList);
}

int GeoComplexPolygon::hashCode()
{
  int result = GeoBasePolygon::hashCode();
  result = 31 * result + Boolean::hashCode(testPoint1InSet);
  result = 31 * result + testPoint1->hashCode();
  result = 31 * result + pointsList.hashCode();
  return result;
}

wstring GeoComplexPolygon::toString()
{
  shared_ptr<StringBuilder> *const edgeDescription =
      make_shared<StringBuilder>();
  for (auto shapeStartEdge : shapeStartEdges) {
    fillInEdgeDescription(edgeDescription, shapeStartEdge);
  }
  return L"GeoComplexPolygon: {planetmodel=" + planetModel +
         L", number of shapes=" + shapeStartEdges.size() + L", address=" +
         Integer::toHexString(hashCode()) + L", testPoint=" + testPoint1 +
         L", testPointInSet=" + StringHelper::toString(testPoint1InSet) +
         L", shapes={" + edgeDescription + L"}}";
}

void GeoComplexPolygon::fillInEdgeDescription(
    shared_ptr<StringBuilder> description, shared_ptr<Edge> startEdge)
{
  description->append(L" {");
  shared_ptr<Edge> currentEdge = startEdge;
  int edgeCounter = 0;
  while (true) {
    if (edgeCounter > 0) {
      description->append(L", ");
    }
    if (edgeCounter >= 20) {
      description->append(L"...");
      break;
    }
    description->append(currentEdge->startPoint);
    currentEdge = currentEdge->next;
    if (currentEdge == startEdge) {
      break;
    }
    edgeCounter++;
  }
}
} // namespace org::apache::lucene::spatial3d::geom