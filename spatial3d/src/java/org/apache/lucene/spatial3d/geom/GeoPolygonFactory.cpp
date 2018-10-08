using namespace std;

#include "GeoPolygonFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoPolygonFactory::GeoPolygonFactory() {}

shared_ptr<GeoPolygon> GeoPolygonFactory::makeGeoConcavePolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<GeoPoint>> &pointList)
{
  return make_shared<GeoConcavePolygon>(planetModel, pointList);
}

shared_ptr<GeoPolygon> GeoPolygonFactory::makeGeoConvexPolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<GeoPoint>> &pointList)
{
  return make_shared<GeoConvexPolygon>(planetModel, pointList);
}

shared_ptr<GeoPolygon> GeoPolygonFactory::makeGeoConcavePolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<GeoPoint>> &pointList,
    deque<std::shared_ptr<GeoPolygon>> &holes)
{
  return make_shared<GeoConcavePolygon>(planetModel, pointList, holes);
}

shared_ptr<GeoPolygon> GeoPolygonFactory::makeGeoConvexPolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<GeoPoint>> &pointList,
    deque<std::shared_ptr<GeoPolygon>> &holes)
{
  return make_shared<GeoConvexPolygon>(planetModel, pointList, holes);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public PolygonDescription(final java.util.List<?
// extends GeoPoint> points)
GeoPolygonFactory::PolygonDescription::PolygonDescription(
    deque<T1> const points)
    : PolygonDescription(points, new ArrayList<>())
{
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public PolygonDescription(final java.util.List<?
// extends GeoPoint> points, final java.util.List<? extends PolygonDescription>
// holes)
GeoPolygonFactory::PolygonDescription::PolygonDescription(
    deque<T1> const points, deque<T2> const holes)
    : points(points), holes(holes)
{
}

shared_ptr<GeoPolygon>
GeoPolygonFactory::makeGeoPolygon(shared_ptr<PlanetModel> planetModel,
                                  shared_ptr<PolygonDescription> description)
{
  return makeGeoPolygon(planetModel, description, 0.0);
}

shared_ptr<GeoPolygon>
GeoPolygonFactory::makeGeoPolygon(shared_ptr<PlanetModel> planetModel,
                                  shared_ptr<PolygonDescription> description,
                                  double const leniencyValue)
{

  // First, convert the holes to polygons in their own right.
  const deque<std::shared_ptr<GeoPolygon>> holes;
  if (description->holes.size() > 0 && description->holes.size() > 0) {
    holes = deque<>(description->holes.size());
    for (auto holeDescription : description->holes) {
      shared_ptr<GeoPolygon> *const gp =
          makeGeoPolygon(planetModel, holeDescription, leniencyValue);
      if (gp == nullptr) {
        return nullptr;
      }
      holes.push_back(gp);
    }
  } else {
    holes.clear();
  }

  if (description->points.size() <= SMALL_POLYGON_CUTOFF_EDGES) {
    // First, exercise a sanity filter on the provided pointList, and remove
    // identical points, linear points, and backtracks
    // System.err.println(" filtering "+pointList.size()+" points...");
    // final long startTime = System.currentTimeMillis();
    const deque<std::shared_ptr<GeoPoint>> firstFilteredPointList =
        filterPoints(description->points);
    if (firstFilteredPointList.empty()) {
      return nullptr;
    }
    const deque<std::shared_ptr<GeoPoint>> filteredPointList =
        filterEdges(firstFilteredPointList, leniencyValue);
    // System.err.println("  ...done in
    // "+(System.currentTimeMillis()-startTime)+"ms
    // ("+((filteredPointList==null)?"degenerate":(filteredPointList.size()+"
    // points"))+")");
    if (filteredPointList.empty()) {
      return nullptr;
    }

    try {
      // First approximation to find a point
      shared_ptr<GeoPoint> *const centerOfMass =
          getCenterOfMass(planetModel, filteredPointList);
      const optional<bool> isCenterOfMassInside =
          isInsidePolygon(centerOfMass, filteredPointList);
      if (isCenterOfMassInside) {
        return generateGeoPolygon(planetModel, filteredPointList, holes,
                                  centerOfMass, isCenterOfMassInside);
      }

      // System.err.println("points="+pointList);
      // Create a random number generator.  Effectively this furnishes us with a
      // repeatable sequence of points to use for poles.
      shared_ptr<Random> *const generator = make_shared<Random>(1234);
      for (int counter = 0; counter < 1000000; counter++) {
        // counter++;
        // Pick the next random pole
        shared_ptr<GeoPoint> *const pole =
            pickPole(generator, planetModel, filteredPointList);
        // Is it inside or outside?
        const optional<bool> isPoleInside =
            isInsidePolygon(pole, filteredPointList);
        if (isPoleInside) {
          // Legal pole
          // System.out.println("Took "+counter+" iterations to find pole");
          // System.out.println("Pole = "+pole+"; isInside="+isPoleInside+";
          // pointList = "+pointList);
          return generateGeoPolygon(planetModel, filteredPointList, holes, pole,
                                    isPoleInside);
        }
        // If pole choice was illegal, try another one
      }
      throw invalid_argument(
          L"cannot find a point that is inside the polygon " +
          filteredPointList);
    } catch (const TileException &e) {
      // Couldn't tile the polygon; use GeoComplexPolygon instead, if we can.
    }
  }
  // Fallback: create large geo polygon, using complex polygon logic.
  const deque<std::shared_ptr<PolygonDescription>> pd =
      deque<std::shared_ptr<PolygonDescription>>(1);
  pd.push_back(description);
  return makeLargeGeoPolygon(planetModel, pd);
}

shared_ptr<GeoPolygon>
GeoPolygonFactory::makeGeoPolygon(shared_ptr<PlanetModel> planetModel,
                                  deque<std::shared_ptr<GeoPoint>> &pointList)
{
  return makeGeoPolygon(planetModel, pointList, nullptr);
}

shared_ptr<GeoPolygon>
GeoPolygonFactory::makeGeoPolygon(shared_ptr<PlanetModel> planetModel,
                                  deque<std::shared_ptr<GeoPoint>> &pointList,
                                  deque<std::shared_ptr<GeoPolygon>> &holes)
{
  return makeGeoPolygon(planetModel, pointList, holes, 0.0);
}

shared_ptr<GeoPolygon>
GeoPolygonFactory::makeGeoPolygon(shared_ptr<PlanetModel> planetModel,
                                  deque<std::shared_ptr<GeoPoint>> &pointList,
                                  deque<std::shared_ptr<GeoPolygon>> &holes,
                                  double const leniencyValue)
{
  // First, exercise a sanity filter on the provided pointList, and remove
  // identical points, linear points, and backtracks
  // System.err.println(" filtering "+pointList.size()+" points...");
  // final long startTime = System.currentTimeMillis();
  const deque<std::shared_ptr<GeoPoint>> firstFilteredPointList =
      filterPoints(pointList);
  if (firstFilteredPointList.empty()) {
    return nullptr;
  }
  const deque<std::shared_ptr<GeoPoint>> filteredPointList =
      filterEdges(firstFilteredPointList, leniencyValue);
  // System.err.println("  ...done in
  // "+(System.currentTimeMillis()-startTime)+"ms
  // ("+((filteredPointList==null)?"degenerate":(filteredPointList.size()+"
  // points"))+")");
  if (filteredPointList.empty()) {
    return nullptr;
  }

  try {
    // First approximation to find a point
    shared_ptr<GeoPoint> *const centerOfMass =
        getCenterOfMass(planetModel, filteredPointList);
    const optional<bool> isCenterOfMassInside =
        isInsidePolygon(centerOfMass, filteredPointList);
    if (isCenterOfMassInside) {
      return generateGeoPolygon(planetModel, filteredPointList, holes,
                                centerOfMass, isCenterOfMassInside);
    }

    // System.err.println("points="+pointList);
    // Create a random number generator.  Effectively this furnishes us with a
    // repeatable sequence of points to use for poles.
    shared_ptr<Random> *const generator = make_shared<Random>(1234);
    for (int counter = 0; counter < 1000000; counter++) {
      // counter++;
      // Pick the next random pole
      shared_ptr<GeoPoint> *const pole =
          pickPole(generator, planetModel, filteredPointList);
      // Is it inside or outside?
      const optional<bool> isPoleInside =
          isInsidePolygon(pole, filteredPointList);
      if (isPoleInside) {
        // Legal pole
        // System.out.println("Took "+counter+" iterations to find pole");
        // System.out.println("Pole = "+pole+"; isInside="+isPoleInside+";
        // pointList = "+pointList);
        return generateGeoPolygon(planetModel, filteredPointList, holes, pole,
                                  isPoleInside);
      }
      // If pole choice was illegal, try another one
    }
    throw invalid_argument(L"cannot find a point that is inside the polygon " +
                           filteredPointList);
  } catch (const TileException &e) {
    // Couldn't tile the polygon; use GeoComplexPolygon instead, if we can.
    if (holes.size() > 0 && holes.size() > 0) {
      // We currently cannot get the deque of points that went into making a hole
      // back out, so don't allow this case. In order to support it, we really
      // need to change the API contract, which is a bigger deal.
      throw invalid_argument(e->what());
    }
    const deque<std::shared_ptr<PolygonDescription>> description =
        deque<std::shared_ptr<PolygonDescription>>(1);
    description.push_back(make_shared<PolygonDescription>(pointList));
    return makeLargeGeoPolygon(planetModel, description);
  }
}

shared_ptr<GeoPoint>
GeoPolygonFactory::getCenterOfMass(shared_ptr<PlanetModel> planetModel,
                                   deque<std::shared_ptr<GeoPoint>> &points)
{
  double x = 0;
  double y = 0;
  double z = 0;
  // get center of mass
  for (auto point : points) {
    x += point->x;
    y += point->y;
    z += point->z;
  }
  // Normalization is not needed because createSurfacePoint does the scaling
  // anyway.
  return planetModel->createSurfacePoint(x, y, z);
}

shared_ptr<GeoPolygon> GeoPolygonFactory::makeLargeGeoPolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<PolygonDescription>> &shapesList)
{

  // We're going to be building a single-level deque of shapes in the end, with a
  // single point that we know to be inside/outside, which is not on an edge.

  const deque<deque<std::shared_ptr<GeoPoint>>> pointsList =
      deque<deque<std::shared_ptr<GeoPoint>>>();

  shared_ptr<BestShape> testPointShape = nullptr;
  for (auto shape : shapesList) {
    // Convert this shape and its holes to a general deque of shapes.  We also
    // need to identify exactly one legal, non-degenerate shape with no children
    // that we can use to find a test point.  We also optimize to choose as
    // small as possible a polygon for determining the in-set-ness of the test
    // point.
    testPointShape = convertPolygon(pointsList, shape, testPointShape, true);
  }

  // If there's no polygon we can use to determine a test point, we throw up.
  if (testPointShape == nullptr) {
    throw invalid_argument(
        L"couldn't find a non-degenerate polygon for in-set determination");
  }

  shared_ptr<GeoPoint> *const centerOfMass =
      getCenterOfMass(planetModel, testPointShape->points);
  shared_ptr<GeoComplexPolygon> *const comRval =
      testPointShape->createGeoComplexPolygon(planetModel, pointsList,
                                              centerOfMass);
  if (comRval != nullptr) {
    return comRval;
  }

  // Center of mass didn't work.
  // Create a random number generator.  Effectively this furnishes us with a
  // repeatable sequence of points to use for poles.
  shared_ptr<Random> *const generator = make_shared<Random>(1234);
  for (int counter = 0; counter < 1000000; counter++) {
    // Pick the next random pole
    shared_ptr<GeoPoint> *const pole =
        pickPole(generator, planetModel, testPointShape->points);
    shared_ptr<GeoComplexPolygon> *const rval =
        testPointShape->createGeoComplexPolygon(planetModel, pointsList, pole);
    if (rval != nullptr) {
      return rval;
    }
    // If pole choice was illegal, try another one
  }
  throw invalid_argument(L"cannot find a point that is inside the polygon " +
                         testPointShape);
}

shared_ptr<BestShape> GeoPolygonFactory::convertPolygon(
    deque<deque<std::shared_ptr<GeoPoint>>> &pointsList,
    shared_ptr<PolygonDescription> shape, shared_ptr<BestShape> testPointShape,
    bool const mustBeInside)
{
  // First, remove duplicate points.  If degenerate, just ignore the shape.
  const deque<std::shared_ptr<GeoPoint>> filteredPoints =
      filterPoints(shape->points);
  if (filteredPoints.empty()) {
    return testPointShape;
  }

  // Non-degenerate.  Check if this is a candidate for in-set determination.
  if (shape->holes.empty()) {
    // This shape is a candidate for a test point.
    if (testPointShape == nullptr ||
        testPointShape->points.size() > filteredPoints.size()) {
      testPointShape = make_shared<BestShape>(filteredPoints, mustBeInside);
    }
  }

  pointsList.push_back(filteredPoints);

  // Now, do all holes too
  for (auto hole : shape->holes) {
    testPointShape =
        convertPolygon(pointsList, hole, testPointShape, !mustBeInside);
  }

  // Done; return the updated test point shape.
  return testPointShape;
}

GeoPolygonFactory::BestShape::BestShape(
    deque<std::shared_ptr<GeoPoint>> &points, bool const poleMustBeInside)
    : points(points)
{
  this->poleMustBeInside = poleMustBeInside;
}

shared_ptr<GeoComplexPolygon>
GeoPolygonFactory::BestShape::createGeoComplexPolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<deque<std::shared_ptr<GeoPoint>>> &pointsList,
    shared_ptr<GeoPoint> testPoint)
{
  // Is it inside or outside?
  const optional<bool> isTestPointInside = isInsidePolygon(testPoint, points);
  if (isTestPointInside) {
    // Legal pole
    if (isTestPointInside == poleMustBeInside) {
      return make_shared<GeoComplexPolygon>(planetModel, pointsList, testPoint,
                                            isTestPointInside);
    } else {
      return make_shared<GeoComplexPolygon>(
          planetModel, pointsList,
          make_shared<GeoPoint>(-testPoint->x, -testPoint->y, -testPoint->z),
          !isTestPointInside);
    }
  }
  // If pole choice was illegal, try another one
  return nullptr;
}

shared_ptr<GeoPolygon> GeoPolygonFactory::generateGeoPolygon(
    shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<GeoPoint>> &filteredPointList,
    deque<std::shared_ptr<GeoPolygon>> &holes, shared_ptr<GeoPoint> testPoint,
    bool const testPointInside) 
{
  // We will be trying twice to find the right GeoPolygon, using alternate
  // siding choices for the first polygon side.  While this looks like it might
  // be 2x as expensive as it could be, there's really no other choice I can
  // find.
  shared_ptr<SidedPlane> *const initialPlane = make_shared<SidedPlane>(
      testPoint, filteredPointList[0], filteredPointList[1]);
  // We don't know if this is the correct siding choice.  We will only know as
  // we build the complex polygon. So we need to be prepared to try both
  // possibilities.
  shared_ptr<GeoCompositePolygon> rval =
      make_shared<GeoCompositePolygon>(planetModel);
  shared_ptr<MutableBoolean> seenConcave = make_shared<MutableBoolean>();
  if (buildPolygonShape(rval, seenConcave, planetModel, filteredPointList,
                        make_shared<BitSet>(), 0, 1, initialPlane, holes,
                        testPoint) == false) {
    // The testPoint was within the shape.  Was that intended?
    if (testPointInside) {
      // Yes: build it for real
      rval = make_shared<GeoCompositePolygon>(planetModel);
      seenConcave = make_shared<MutableBoolean>();
      buildPolygonShape(rval, seenConcave, planetModel, filteredPointList,
                        make_shared<BitSet>(), 0, 1, initialPlane, holes,
                        nullptr);
      return rval;
    }
    // No: do the complement and return that.
    rval = make_shared<GeoCompositePolygon>(planetModel);
    seenConcave = make_shared<MutableBoolean>();
    buildPolygonShape(rval, seenConcave, planetModel, filteredPointList,
                      make_shared<BitSet>(), 0, 1,
                      make_shared<SidedPlane>(initialPlane), holes, nullptr);
    return rval;
  } else {
    // The testPoint was outside the shape.  Was that intended?
    if (!testPointInside) {
      // Yes: return what we just built
      return rval;
    }
    // No: return the complement
    rval = make_shared<GeoCompositePolygon>(planetModel);
    seenConcave = make_shared<MutableBoolean>();
    buildPolygonShape(rval, seenConcave, planetModel, filteredPointList,
                      make_shared<BitSet>(), 0, 1,
                      make_shared<SidedPlane>(initialPlane), holes, nullptr);
    return rval;
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: static java.util.List<GeoPoint> filterPoints(final
// java.util.List<? extends GeoPoint> input)
deque<std::shared_ptr<GeoPoint>>
GeoPolygonFactory::filterPoints(deque<T1> const input)
{

  const deque<std::shared_ptr<GeoPoint>> noIdenticalPoints =
      deque<std::shared_ptr<GeoPoint>>(input.size());

  // Backtrack to find something different from the first point
  int startIndex = -1;
  shared_ptr<GeoPoint> *const comparePoint = input[0];
  for (int i = 0; i < input.size() - 1; i++) {
    shared_ptr<GeoPoint> *const thePoint =
        input[getLegalIndex(-i - 1, input.size())];
    if (!thePoint->isNumericallyIdentical(comparePoint)) {
      startIndex = getLegalIndex(-i, input.size());
      break;
    }
  }
  if (startIndex == -1) {
    return nullptr;
  }

  // Now we can start the process of walking around, removing duplicate points.
  int currentIndex = startIndex;
  while (true) {
    shared_ptr<GeoPoint> *const currentPoint = input[currentIndex];
    noIdenticalPoints.push_back(currentPoint);
    while (true) {
      currentIndex = getLegalIndex(currentIndex + 1, input.size());
      if (currentIndex == startIndex) {
        break;
      }
      shared_ptr<GeoPoint> *const nextNonIdenticalPoint = input[currentIndex];
      if (!nextNonIdenticalPoint->isNumericallyIdentical(currentPoint)) {
        break;
      }
    }
    if (currentIndex == startIndex) {
      break;
    }
  }

  if (noIdenticalPoints.size() < 3) {
    return nullptr;
  }

  return noIdenticalPoints;
}

deque<std::shared_ptr<GeoPoint>> GeoPolygonFactory::filterEdges(
    deque<std::shared_ptr<GeoPoint>> &noIdenticalPoints,
    double const leniencyValue)
{

  // Now, do the search needed to find a path that has no coplanarities in it.
  // It is important to check coplanarities using the points that are further
  // away so the plane is more precise.

  for (int i = 0; i < noIdenticalPoints.size(); i++) {
    // Search starting for current index.
    shared_ptr<SafePath> *const resultPath =
        findSafePath(noIdenticalPoints, i, leniencyValue);
    if (resultPath != nullptr && resultPath->previous != nullptr) {
      // Read out result, maintaining ordering
      const deque<std::shared_ptr<GeoPoint>> rval =
          deque<std::shared_ptr<GeoPoint>>(noIdenticalPoints.size());
      resultPath->fillInList(rval);
      return rval;
    }
  }
  // No path found.  This means that everything was coplanar.
  return nullptr;
}

shared_ptr<SafePath>
GeoPolygonFactory::findSafePath(deque<std::shared_ptr<GeoPoint>> &points,
                                int const startIndex,
                                double const leniencyValue)
{
  shared_ptr<SafePath> safePath = nullptr;
  for (int i = startIndex; i < startIndex + points.size(); i++) {
    // get start point, always the same for an iteration
    constexpr int startPointIndex = getLegalIndex(i - 1, points.size());
    shared_ptr<GeoPoint> *const startPoint = points[startPointIndex];
    // get end point, can be coplanar and therefore change
    int endPointIndex = getLegalIndex(i, points.size());
    shared_ptr<GeoPoint> endPoint = points[endPointIndex];

    if (startPoint->isNumericallyIdentical(endPoint)) {
      // go to next if identical
      continue;
    }
    // Check if nextPoints are co-planar, if so advance to next point.
    // if we go over the start index then we have no succeed.
    while (true) {
      int nextPointIndex = getLegalIndex(endPointIndex + 1, points.size());
      shared_ptr<GeoPoint> *const nextPoint = points[nextPointIndex];
      if (startPoint->isNumericallyIdentical(nextPoint)) {
        // all coplanar
        return nullptr;
      }
      if (!Plane::arePointsCoplanar(startPoint, endPoint, nextPoint)) {
        // no coplanar.
        break;
      }
      if (endPointIndex == startIndex) {
        // we are over the path, we fail.
        return nullptr;
      }
      // advance
      endPointIndex = nextPointIndex;
      endPoint = nextPoint;
      i++;
    }

    if (safePath != nullptr && endPointIndex == startIndex) {
      // We are already at the start, current point is coplanar with
      // start point, no need to add this node.
      break;
    }
    // Create node and move to next one
    shared_ptr<Plane> currentPlane = make_shared<Plane>(startPoint, endPoint);
    safePath =
        make_shared<SafePath>(safePath, endPoint, endPointIndex, currentPlane);
  }
  return safePath;
}

shared_ptr<GeoPoint>
GeoPolygonFactory::pickPole(shared_ptr<Random> generator,
                            shared_ptr<PlanetModel> planetModel,
                            deque<std::shared_ptr<GeoPoint>> &points)
{
  constexpr int pointIndex = generator->nextInt(points.size());
  shared_ptr<GeoPoint> *const closePoint = points[pointIndex];
  // We pick a random angle and random arc distance, then generate a point based
  // on closePoint
  constexpr double angle = generator->nextDouble() * M_PI * 2.0 - M_PI;
  double maxArcDistance = points[0]->arcDistance(points[1]);
  double trialArcDistance = points[0]->arcDistance(points[2]);
  if (trialArcDistance > maxArcDistance) {
    maxArcDistance = trialArcDistance;
  }
  constexpr double arcDistance =
      maxArcDistance - generator->nextDouble() * maxArcDistance;
  // We come up with a unit circle (x,y,z) coordinate given the random angle and
  // arc distance.  The point is centered around the positive x axis.
  constexpr double x = cos(arcDistance);
  constexpr double sinArcDistance = sin(arcDistance);
  constexpr double y = cos(angle) * sinArcDistance;
  constexpr double z = sin(angle) * sinArcDistance;
  // Now, use closePoint for a rotation pole
  constexpr double sinLatitude = sin(closePoint->getLatitude());
  constexpr double cosLatitude = cos(closePoint->getLatitude());
  constexpr double sinLongitude = sin(closePoint->getLongitude());
  constexpr double cosLongitude = cos(closePoint->getLongitude());
  // This transformation should take the point (1,0,0) and transform it to the
  // closepoint's actual (x,y,z) coordinates. Coordinate rotation formula: x1 =
  // x0 cos T - y0 sin T y1 = x0 sin T + y0 cos T We're in essence undoing the
  // following transformation (from GeoPolygonFactory): x1 = x0 cos az + y0 sin
  // az y1 = - x0 sin az + y0 cos az z1 = z0 x2 = x1 cos al + z1 sin al y2 = y1
  // z2 = - x1 sin al + z1 cos al
  // So, we reverse the order of the transformations, AND we transform
  // backwards. Transforming backwards means using these identities: sin(-angle)
  // = -sin(angle), cos(-angle) = cos(angle) So: x1 = x0 cos al - z0 sin al y1 =
  // y0 z1 = x0 sin al + z0 cos al x2 = x1 cos az - y1 sin az y2 = x1 sin az +
  // y1 cos az z2 = z1
  constexpr double x1 = x * cosLatitude - z * sinLatitude;
  constexpr double y1 = y;
  constexpr double z1 = x * sinLatitude + z * cosLatitude;
  constexpr double x2 = x1 * cosLongitude - y1 * sinLongitude;
  constexpr double y2 = x1 * sinLongitude + y1 * cosLongitude;
  constexpr double z2 = z1;
  // Finally, scale to put the point on the surface
  return planetModel->createSurfacePoint(x2, y2, z2);
}

optional<bool> GeoPolygonFactory::isInsidePolygon(
    shared_ptr<GeoPoint> point, deque<std::shared_ptr<GeoPoint>> &polyPoints)
{
  // First, compute sine and cosine of pole point latitude and longitude
  constexpr double latitude = point->getLatitude();
  constexpr double longitude = point->getLongitude();
  constexpr double sinLatitude = sin(latitude);
  constexpr double cosLatitude = cos(latitude);
  constexpr double sinLongitude = sin(longitude);
  constexpr double cosLongitude = cos(longitude);

  // Now, compute the incremental arc distance around the points of the polygon
  double arcDistance = 0.0;
  optional<double> prevAngle = nullopt;
  // System.out.println("Computing angles:");
  for (auto polyPoint : polyPoints) {
    const optional<double> angle = computeAngle(
        polyPoint, sinLatitude, cosLatitude, sinLongitude, cosLongitude);
    if (!angle) {
      return nullopt;
    }
    // System.out.println("Computed angle: "+angle);
    if (prevAngle) {
      // Figure out delta between prevAngle and current angle, and add it to
      // arcDistance
      double angleDelta = angle - prevAngle;
      if (angleDelta < -M_PI) {
        angleDelta += M_PI * 2.0;
      }
      if (angleDelta > M_PI) {
        angleDelta -= M_PI * 2.0;
      }
      if (abs(angleDelta - M_PI) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
        return nullopt;
      }
      // System.out.println(" angle delta = "+angleDelta);
      arcDistance += angleDelta;
      // System.out.println(" For point "+polyPoint+" angle is "+angle+"; delta
      // is "+angleDelta+"; arcDistance is "+arcDistance);
    }
    prevAngle = angle;
  }
  if (prevAngle) {
    const optional<double> lastAngle = computeAngle(
        polyPoints[0], sinLatitude, cosLatitude, sinLongitude, cosLongitude);
    if (!lastAngle) {
      return nullopt;
    }
    // System.out.println("Computed last angle: "+lastAngle);
    // Figure out delta and add it
    double angleDelta = lastAngle - prevAngle;
    if (angleDelta < -M_PI) {
      angleDelta += M_PI * 2.0;
    }
    if (angleDelta > M_PI) {
      angleDelta -= M_PI * 2.0;
    }
    if (abs(angleDelta - M_PI) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return nullopt;
    }
    // System.out.println(" angle delta = "+angleDelta);
    arcDistance += angleDelta;
    // System.out.println(" For point "+polyPoints.get(0)+" angle is
    // "+lastAngle+"; delta is "+angleDelta+"; arcDistance is "+arcDistance);
  }

  // Clockwise == inside == negative
  // System.out.println("Arcdistance = "+arcDistance);
  if (abs(arcDistance) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    // No idea what direction, so try another pole.
    return nullopt;
  }
  return arcDistance > 0.0;
}

optional<double> GeoPolygonFactory::computeAngle(shared_ptr<GeoPoint> point,
                                                 double const sinLatitude,
                                                 double const cosLatitude,
                                                 double const sinLongitude,
                                                 double const cosLongitude)
{
  // Coordinate rotation formula:
  // x1 = x0 cos T - y0 sin T
  // y1 = x0 sin T + y0 cos T
  // We need to rotate the point in question into the coordinate frame specified
  // by the lat and lon trig functions. To do this we need to do two rotations
  // on it.  First rotation is in x/y.  Second rotation is in x/z. And we rotate
  // in the negative direction. So: x1 = x0 cos az + y0 sin az y1 = - x0 sin az
  // + y0 cos az z1 = z0 x2 = x1 cos al + z1 sin al y2 = y1 z2 = - x1 sin al +
  // z1 cos al

  constexpr double x1 = point->x * cosLongitude + point->y * sinLongitude;
  constexpr double y1 = -point->x * sinLongitude + point->y * cosLongitude;
  constexpr double z1 = point->z;

  // final double x2 = x1 * cosLatitude + z1 * sinLatitude;
  constexpr double y2 = y1;
  constexpr double z2 = -x1 * sinLatitude + z1 * cosLatitude;

  // Now we should be looking down the X axis; the original point has rotated
  // coordinates (N, 0, 0). So we can just compute the angle using y2 and z2.
  // (If Math.sqrt(y2*y2 + z2 * z2) is 0.0, then the point is on the pole and we
  // need another one).
  if (sqrt(y2 * y2 + z2 * z2) < Vector::MINIMUM_RESOLUTION) {
    return nullopt;
  }

  return atan2(z2, y2);
}

bool GeoPolygonFactory::buildPolygonShape(
    shared_ptr<GeoCompositePolygon> rval,
    shared_ptr<MutableBoolean> seenConcave, shared_ptr<PlanetModel> planetModel,
    deque<std::shared_ptr<GeoPoint>> &pointsList,
    shared_ptr<BitSet> internalEdges, int const startPointIndex,
    int const endPointIndex, shared_ptr<SidedPlane> startingEdge,
    deque<std::shared_ptr<GeoPolygon>> &holes,
    shared_ptr<GeoPoint> testPoint) 
{

  // It could be the case that we need a concave polygon.  So we need to try and
  // look for that case as part of the general code for constructing complex
  // polygons.

  // Note that there can be only one concave polygon.  This code will enforce
  // that condition and will return false if it is violated.

  // The code here must keep track of two lists of sided planes.  The first deque
  // contains the planes consistent with a concave polygon.  This deque will grow
  // and shrink.  The second deque is built starting at the current edge that was
  // last consistent with the concave polygon, and contains all edges consistent
  // with a convex polygon. When that sequence of edges is done, then an
  // internal edge is created and the identified points are converted to a
  // convex polygon.  That internal edge is used to extend the deque of edges in
  // the concave polygon edge deque.

  // The edge buffer.
  shared_ptr<EdgeBuffer> *const edgeBuffer = make_shared<EdgeBuffer>(
      pointsList, internalEdges, startPointIndex, endPointIndex, startingEdge);

  /*
  // Verify that the polygon does not self-intersect
  // Now, look for non-adjacent edges that cross.
  System.err.println("Looking for intersections...");
  System.err.println("Starting edge is: "+startingEdge);
  final Iterator<Edge> edgeIterator = edgeBuffer.iterator();
  while (edgeIterator.hasNext()) {
    final Edge edge = edgeIterator.next();
    final Set<Edge> excludedEdges = new HashSet<>();
    excludedEdges.add(edge);
    Edge oneBoundary = edgeBuffer.getPrevious(edge);
    while (oneBoundary.plane.isNumericallyIdentical(edge.plane)) {
      excludedEdges.add(oneBoundary);
      oneBoundary = edgeBuffer.getPrevious(oneBoundary);
    }
    excludedEdges.add(oneBoundary);
    Edge otherBoundary = edgeBuffer.getNext(edge);
    while (otherBoundary.plane.isNumericallyIdentical(edge.plane)) {
      excludedEdges.add(otherBoundary);
      otherBoundary = edgeBuffer.getNext(otherBoundary);
    }
    excludedEdges.add(otherBoundary);

    // Now go through all other edges and rule out any intersections
    final Iterator<Edge> compareIterator = edgeBuffer.iterator();
    while (compareIterator.hasNext()) {
      final Edge compareEdge = compareIterator.next();
      if (!excludedEdges.contains(compareEdge)) {
        // Found an edge we can compare with!
        //System.err.println("Found a compare edge...");
        bool nonOverlapping = true;
        // We need the other boundaries though.
        Edge oneCompareBoundary = edgeBuffer.getPrevious(compareEdge);
        while
  (oneCompareBoundary.plane.isNumericallyIdentical(compareEdge.plane)) { if
  (excludedEdges.contains(oneCompareBoundary)) {
            //System.err.println(" excluded because oneCompareBoundary found to
  be in set"); nonOverlapping = false; break;
          }
          oneCompareBoundary = edgeBuffer.getPrevious(oneCompareBoundary);
        }
        Edge otherCompareBoundary = edgeBuffer.getNext(compareEdge);
        while
  (otherCompareBoundary.plane.isNumericallyIdentical(compareEdge.plane)) { if
  (excludedEdges.contains(otherCompareBoundary)) {
            //System.err.println(" excluded because otherCompareBoundary found
  to be in set"); nonOverlapping = false; break;
          }
          otherCompareBoundary = edgeBuffer.getNext(otherCompareBoundary);
        }
        if (nonOverlapping) {
          //System.err.println("Preparing to call findIntersections...");
          // Finally do an intersection test
          if (edge.plane.findIntersections(planetModel, compareEdge.plane,
  oneBoundary.plane, otherBoundary.plane, oneCompareBoundary.plane,
  otherCompareBoundary.plane).length > 0) { throw new
  IllegalArgumentException("polygon has intersecting edges");
          }
        }
      }
    }
  }
  */

  // Starting state:
  // The stopping point
  shared_ptr<Edge> stoppingPoint = edgeBuffer->pickOne();
  // The current edge
  shared_ptr<Edge> currentEdge = stoppingPoint;

  // Progressively look for convex sections.  If we find one, we emit it and
  // replace it. Keep going until we have been around once and nothing needed to
  // change, and then do the concave polygon, if necessary.
  while (true) {

    if (currentEdge == nullptr) {
      // We're done!
      break;
    }

    // Find convexity around the current edge, if any
    const optional<bool> foundIt = findConvexPolygon(
        planetModel, currentEdge, rval, edgeBuffer, holes, testPoint);
    if (!foundIt) {
      return false;
    }

    if (foundIt) {
      // New start point
      stoppingPoint = edgeBuffer->pickOne();
      currentEdge = stoppingPoint;
      // back around
      continue;
    }

    // Otherwise, go on to the next
    currentEdge = edgeBuffer->getNext(currentEdge);
    if (currentEdge == stoppingPoint) {
      break;
    }
  }

  // Look for any reason that the concave polygon cannot be created.
  // This test is really the converse of the one for a convex polygon.
  // Points on the edge of a convex polygon MUST be inside all the other
  // edges.  For a concave polygon, this check is still the same, except we have
  // to look at the reverse sided planes, not the forward ones.

  // If we find a point that is outside of the complementary edges, it means
  // that the point is in fact able to form a convex polygon with the edge it is
  // offending.

  // If what is left has any plane/point pair that is on the wrong side, we have
  // to split using one of the plane endpoints and the point in question.  This
  // is best structured as a recursion, if detected.

  // Note: Any edge that fails means (I think!!) that there's another edge that
  // will also fail. This is because each point is included in two edges. So,
  // when we look for a non-conforming edge, and we can find one (but can't use
  // it), we also can find another edge that we might be able to use instead. If
  // this is true, it means we should continue when we find a bad edge we can't
  // use -- but we need to keep track of this, and fail hard if we don't find a
  // place to split.
  bool foundBadEdge = false;
  constexpr org::apache::lucene::spatial3d::geom::GeoPolygonFactory::
      EdgeBuffer::const_iterator checkIterator = edgeBuffer->begin();
  while (checkIterator != edgeBuffer->end()) {
    shared_ptr<Edge> *const checkEdge = *checkIterator;
    shared_ptr<SidedPlane> *const flippedPlane =
        make_shared<SidedPlane>(checkEdge->plane);
    // Now walk around again looking for points that fail.
    constexpr org::apache::lucene::spatial3d::geom::GeoPolygonFactory::
        EdgeBuffer::const_iterator confirmIterator = edgeBuffer->begin();
    while (confirmIterator != edgeBuffer->end()) {
      shared_ptr<Edge> *const confirmEdge = *confirmIterator;
      if (confirmEdge == checkEdge) {
        continue;
      }
      // Look for a point that is on the wrong side of the check edge.  This
      // means that we can't build the polygon.
      shared_ptr<GeoPoint> *const thePoint;
      if (checkEdge->startPoint != confirmEdge->startPoint &&
          checkEdge->endPoint != confirmEdge->startPoint &&
          !flippedPlane->isWithin(confirmEdge->startPoint)) {
        thePoint = confirmEdge->startPoint;
      } else if (checkEdge->startPoint != confirmEdge->endPoint &&
                 checkEdge->endPoint != confirmEdge->endPoint &&
                 !flippedPlane->isWithin(confirmEdge->endPoint)) {
        thePoint = confirmEdge->endPoint;
      } else {
        thePoint.reset();
      }
      if (thePoint != nullptr) {
        // Note that we found a problem.
        foundBadEdge = true;
        // thePoint is on the wrong side of the complementary plane.  That means
        // we cannot build a concave polygon, because the complement would not
        // be a legal convex polygon.
        // But we can take advantage of the fact that the distance between the
        // edge and thePoint is less than 180 degrees, and so we can split the
        // would-be concave polygon into three segments.  The first segment
        // includes the edge and thePoint, and uses the sense of the edge to
        // determine the sense of the polygon.

        // This should be the only problematic part of the polygon.
        // We know that thePoint is on the "wrong" side of the edge -- that is,
        // it's on the side that the edge is pointing at.

        // The proposed tiling generates two new edges -- one from thePoint to
        // the start point of the edge we found, and the other from thePoint to
        // the end point of the edge.  We generate that as a triangle convex
        // polygon, and tile the two remaining pieces.
        if (Plane::arePointsCoplanar(checkEdge->startPoint, checkEdge->endPoint,
                                     thePoint)) {
          // Can't build this particular tile because of colinearity, so advance
          // to another that maybe we can build.
          continue;
        }
        const deque<std::shared_ptr<GeoPoint>> thirdPartPoints =
            deque<std::shared_ptr<GeoPoint>>(3);
        shared_ptr<BitSet> *const thirdPartInternal = make_shared<BitSet>();
        thirdPartPoints.push_back(checkEdge->startPoint);
        thirdPartInternal->set(0, checkEdge->isInternal);
        thirdPartPoints.push_back(checkEdge->endPoint);
        thirdPartInternal->set(1, true);
        thirdPartPoints.push_back(thePoint);
        assert((checkEdge->plane->isWithin(thePoint),
                L"Point was on wrong side of complementary plane, so must be "
                L"on the right side of the non-complementary plane!"));
        // Check for illegal argument using try/catch rather than pre-emptive
        // check, since it cuts down on building objects for a rare case
        shared_ptr<GeoPolygon> *const convexPart =
            make_shared<GeoConvexPolygon>(planetModel, thirdPartPoints, holes,
                                          thirdPartInternal, true);
        // System.out.println("convex part = "+convexPart);
        rval->addShape(convexPart);

        // The part preceding the bad edge, back to thePoint, needs to be
        // recursively processed.  So, assemble what we need, which is basically
        // a deque of edges.
        shared_ptr<Edge> loopEdge = edgeBuffer->getPrevious(checkEdge);
        const deque<std::shared_ptr<GeoPoint>> firstPartPoints =
            deque<std::shared_ptr<GeoPoint>>();
        shared_ptr<BitSet> *const firstPartInternal = make_shared<BitSet>();
        int i = 0;
        while (true) {
          firstPartPoints.push_back(loopEdge->endPoint);
          if (loopEdge->endPoint == thePoint) {
            break;
          }
          firstPartInternal->set(i++, loopEdge->isInternal);
          loopEdge = edgeBuffer->getPrevious(loopEdge);
        }
        firstPartInternal->set(i, true);
        // System.out.println("Doing first part...");
        if (buildPolygonShape(
                rval, seenConcave, planetModel, firstPartPoints,
                firstPartInternal, firstPartPoints.size() - 1, 0,
                make_shared<SidedPlane>(checkEdge->endPoint, false,
                                        checkEdge->startPoint, thePoint),
                holes, testPoint) == false) {
          return false;
        }
        // System.out.println("...done first part.");

        const deque<std::shared_ptr<GeoPoint>> secondPartPoints =
            deque<std::shared_ptr<GeoPoint>>();
        shared_ptr<BitSet> *const secondPartInternal = make_shared<BitSet>();
        loopEdge = edgeBuffer->getNext(checkEdge);
        i = 0;
        while (true) {
          secondPartPoints.push_back(loopEdge->startPoint);
          if (loopEdge->startPoint == thePoint) {
            break;
          }
          secondPartInternal->set(i++, loopEdge->isInternal);
          loopEdge = edgeBuffer->getNext(loopEdge);
        }
        secondPartInternal->set(i, true);
        // System.out.println("Doing second part...");
        if (buildPolygonShape(
                rval, seenConcave, planetModel, secondPartPoints,
                secondPartInternal, secondPartPoints.size() - 1, 0,
                make_shared<SidedPlane>(checkEdge->startPoint, false,
                                        checkEdge->endPoint, thePoint),
                holes, testPoint) == false) {
          return false;
        }
        // System.out.println("... done second part");

        return true;
      }
      confirmIterator++;
    }
    checkIterator++;
  }

  if (foundBadEdge) {
    // Unaddressed bad edge
    throw make_shared<TileException>(
        L"Could not tile polygon; found a pathological coplanarity that "
        L"couldn't be addressed");
  }

  // No violations found: we know it's a legal concave polygon.

  // If there's anything left in the edge buffer, convert to concave polygon.
  // System.out.println("adding concave part");
  if (makeConcavePolygon(planetModel, rval, seenConcave, edgeBuffer, holes,
                         testPoint) == false) {
    return false;
  }
  return true;
}

bool GeoPolygonFactory::makeConcavePolygon(
    shared_ptr<PlanetModel> planetModel, shared_ptr<GeoCompositePolygon> rval,
    shared_ptr<MutableBoolean> seenConcave, shared_ptr<EdgeBuffer> edgeBuffer,
    deque<std::shared_ptr<GeoPolygon>> &holes,
    shared_ptr<GeoPoint> testPoint) 
{

  if (edgeBuffer->size() == 0) {
    return true;
  }

  if (seenConcave->value) {
    throw invalid_argument(
        L"Illegal polygon; polygon edges intersect each other");
  }

  seenConcave->value = true;

  // If there are less than three edges, something got messed up somehow.  Don't
  // know how this can happen but check.
  if (edgeBuffer->size() < 3) {
    // Linear...
    // Here we can emit GeoWorld, but probably this means we had a broken poly
    // to start with.
    throw invalid_argument(
        L"Illegal polygon; polygon edges intersect each other");
  }

  // Create the deque of points
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>(edgeBuffer->size());
  shared_ptr<BitSet> *const internalEdges =
      make_shared<BitSet>(edgeBuffer->size() - 1);

  // System.out.println("Concave polygon points:");
  shared_ptr<Edge> edge = edgeBuffer->pickOne();
  bool isInternal = false;
  for (int i = 0; i < edgeBuffer->size(); i++) {
    // System.out.println(" "+edge.plane+":
    // "+edge.startPoint+"->"+edge.endPoint+"; previous?
    // "+(edge.plane.isWithin(edgeBuffer.getPrevious(edge).startPoint)?"in":"out")+"
    // next?
    // "+(edge.plane.isWithin(edgeBuffer.getNext(edge).endPoint)?"in":"out"));
    points.push_back(edge->startPoint);
    if (i < edgeBuffer->size() - 1) {
      internalEdges->set(i, edge->isInternal);
    } else {
      isInternal = edge->isInternal;
    }
    edge = edgeBuffer->getNext(edge);
  }

  try {
    if (testPoint != nullptr && holes.size() > 0 && holes.size() > 0) {
      // No holes, for test
      shared_ptr<GeoPolygon> *const testPolygon =
          make_shared<GeoConcavePolygon>(planetModel, points, nullptr,
                                         internalEdges, isInternal);
      if (testPolygon->isWithin(testPoint)) {
        return false;
      }
    }

    shared_ptr<GeoPolygon> *const realPolygon = make_shared<GeoConcavePolygon>(
        planetModel, points, holes, internalEdges, isInternal);
    if (testPoint != nullptr && (holes.empty() || holes.empty())) {
      if (realPolygon->isWithin(testPoint)) {
        return false;
      }
    }

    rval->addShape(realPolygon);
    return true;
  } catch (const invalid_argument &e) {
    throw make_shared<TileException>(e.what());
  }
}

optional<bool> GeoPolygonFactory::findConvexPolygon(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Edge> currentEdge,
    shared_ptr<GeoCompositePolygon> rval, shared_ptr<EdgeBuffer> edgeBuffer,
    deque<std::shared_ptr<GeoPolygon>> &holes,
    shared_ptr<GeoPoint> testPoint) 
{

  // System.out.println("Looking at edge "+currentEdge+" with startpoint
  // "+currentEdge.startPoint+" endpoint "+currentEdge.endPoint);

  // Initialize the structure.
  // We don't keep track of order here; we just care about membership.
  // The only exception is the head and tail pointers.
  shared_ptr<Set<std::shared_ptr<Edge>>> *const includedEdges =
      unordered_set<std::shared_ptr<Edge>>();
  includedEdges->add(currentEdge);
  shared_ptr<Edge> firstEdge = currentEdge;
  shared_ptr<Edge> lastEdge = currentEdge;

  // First, walk towards the end until we need to stop
  while (true) {
    if (firstEdge->startPoint == lastEdge->endPoint) {
      break;
    }
    shared_ptr<Edge> *const newLastEdge = edgeBuffer->getNext(lastEdge);
    if (Plane::arePointsCoplanar(lastEdge->startPoint, lastEdge->endPoint,
                                 newLastEdge->endPoint)) {
      break;
    }
    // Planes that are almost identical cannot be properly handled by the
    // standard polygon logic.  Detect this case and, if found, give up on the
    // tiling -- we'll need to create a large poly instead.
    if (lastEdge->plane->isFunctionallyIdentical(newLastEdge->plane)) {
      throw make_shared<TileException>(
          L"Two adjacent edge planes are effectively parallel despite "
          L"filtering; give up on tiling");
    }
    if (isWithin(newLastEdge->endPoint, includedEdges)) {
      // System.out.println(" maybe can extend to next edge");
      // Found a candidate for extension.  But do some other checks first.
      // Basically, we need to know if we construct a polygon here will overlap
      // with other remaining points?
      shared_ptr<SidedPlane> *const returnBoundary;
      if (firstEdge->startPoint != newLastEdge->endPoint) {
        if (Plane::arePointsCoplanar(firstEdge->endPoint, firstEdge->startPoint,
                                     newLastEdge->endPoint) ||
            Plane::arePointsCoplanar(firstEdge->startPoint,
                                     newLastEdge->endPoint,
                                     newLastEdge->startPoint)) {
          break;
        }
        returnBoundary = make_shared<SidedPlane>(
            firstEdge->endPoint, firstEdge->startPoint, newLastEdge->endPoint);
      } else {
        returnBoundary.reset();
      }
      // The complete set of sided planes for the tentative new polygon include
      // the ones in includedEdges, plus the one from newLastEdge, plus the new
      // tentative return boundary.  We have to make sure there are no points
      // from elsewhere within the tentative convex polygon.
      bool foundPointInside = false;
      constexpr org::apache::lucene::spatial3d::geom::GeoPolygonFactory::
          EdgeBuffer::const_iterator edgeIterator = edgeBuffer->begin();
      while (edgeIterator != edgeBuffer->end()) {
        shared_ptr<Edge> *const edge = *edgeIterator;
        if (!includedEdges->contains(edge) && edge != newLastEdge) {
          // This edge has a point to check
          if (edge->startPoint != newLastEdge->endPoint) {
            // look at edge.startPoint
            if (isWithin(edge->startPoint, includedEdges, newLastEdge,
                         returnBoundary)) {
              // System.out.println("  nope; point within found:
              // "+edge.startPoint);
              foundPointInside = true;
              break;
            }
          }
          if (edge->endPoint != firstEdge->startPoint) {
            // look at edge.endPoint
            if (isWithin(edge->endPoint, includedEdges, newLastEdge,
                         returnBoundary)) {
              // System.out.println("  nope; point within found:
              // "+edge.endPoint);
              foundPointInside = true;
              break;
            }
          }
        }
        edgeIterator++;
      }

      if (!foundPointInside) {
        // System.out.println("  extending!");
        // Extend the polygon by the new last edge
        includedEdges->add(newLastEdge);
        lastEdge = newLastEdge;
        // continue extending in this direction
        continue;
      }
    }
    // We can't extend any more in this direction, so break from the loop.
    break;
  }

  // Now, walk towards the beginning until we need to stop
  while (true) {
    if (firstEdge->startPoint == lastEdge->endPoint) {
      break;
    }
    shared_ptr<Edge> *const newFirstEdge = edgeBuffer->getPrevious(firstEdge);
    if (Plane::arePointsCoplanar(newFirstEdge->startPoint,
                                 newFirstEdge->endPoint, firstEdge->endPoint)) {
      break;
    }
    // Planes that are almost identical cannot be properly handled by the
    // standard polygon logic.  Detect this case and, if found, give up on the
    // tiling -- we'll need to create a large poly instead.
    if (firstEdge->plane->isFunctionallyIdentical(newFirstEdge->plane)) {
      throw make_shared<TileException>(
          L"Two adjacent edge planes are effectively parallel despite "
          L"filtering; give up on tiling");
    }
    if (isWithin(newFirstEdge->startPoint, includedEdges)) {
      // System.out.println(" maybe can extend to previous edge");
      // Found a candidate for extension.  But do some other checks first.
      // Basically, we need to know if we construct a polygon here will overlap
      // with other remaining points?
      shared_ptr<SidedPlane> *const returnBoundary;
      if (newFirstEdge->startPoint != lastEdge->endPoint) {
        if (Plane::arePointsCoplanar(lastEdge->startPoint, lastEdge->endPoint,
                                     newFirstEdge->startPoint) ||
            Plane::arePointsCoplanar(lastEdge->endPoint,
                                     newFirstEdge->startPoint,
                                     newFirstEdge->endPoint)) {
          break;
        }
        returnBoundary = make_shared<SidedPlane>(
            lastEdge->startPoint, lastEdge->endPoint, newFirstEdge->startPoint);
      } else {
        returnBoundary.reset();
      }
      // The complete set of sided planes for the tentative new polygon include
      // the ones in includedEdges, plus the one from newLastEdge, plus the new
      // tentative return boundary.  We have to make sure there are no points
      // from elsewhere within the tentative convex polygon.
      bool foundPointInside = false;
      constexpr org::apache::lucene::spatial3d::geom::GeoPolygonFactory::
          EdgeBuffer::const_iterator edgeIterator = edgeBuffer->begin();
      while (edgeIterator != edgeBuffer->end()) {
        shared_ptr<Edge> *const edge = *edgeIterator;
        if (!includedEdges->contains(edge) && edge != newFirstEdge) {
          // This edge has a point to check
          if (edge->startPoint != lastEdge->endPoint) {
            // look at edge.startPoint
            if (isWithin(edge->startPoint, includedEdges, newFirstEdge,
                         returnBoundary)) {
              // System.out.println("  nope; point within found:
              // "+edge.startPoint);
              foundPointInside = true;
              break;
            }
          }
          if (edge->endPoint != newFirstEdge->startPoint) {
            // look at edge.endPoint
            if (isWithin(edge->endPoint, includedEdges, newFirstEdge,
                         returnBoundary)) {
              // System.out.println("  nope; point within found:
              // "+edge.endPoint);
              foundPointInside = true;
              break;
            }
          }
        }
        edgeIterator++;
      }

      if (!foundPointInside) {
        // System.out.println("  extending!");
        // Extend the polygon by the new last edge
        includedEdges->add(newFirstEdge);
        firstEdge = newFirstEdge;
        // continue extending in this direction
        continue;
      }
    }
    // We can't extend any more in this direction, so break from the loop.
    break;
  }

  // Ok, figure out what we've accumulated.  If it is enough for a polygon,
  // build it.

  if (includedEdges->size() < 2) {
    // System.out.println("Done edge "+currentEdge+": no poly found");
    return false;
  }

  // It's enough to build a convex polygon
  // System.out.println("Edge "+currentEdge+": Found complex poly");

  // Create the point deque and edge deque, starting with the first edge and going
  // to the last.  The return edge will be between the start point of the first
  // edge and the end point of the last edge.  If the first edge start point is
  // the same as the last edge end point, it's a degenerate case and we want to
  // just clean out the edge buffer entirely.

  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>(includedEdges->size() + 1);
  shared_ptr<BitSet> *const internalEdges =
      make_shared<BitSet>(includedEdges->size());
  constexpr bool returnIsInternal;

  if (firstEdge->startPoint == lastEdge->endPoint) {
    // Degenerate case!!  There is no return edge -- or rather, we already have
    // it.
    if (includedEdges->size() < 3) {
      // This means we found a degenerate cycle of edges.  If we emit a polygon
      // at this point it has no contents, so we generate no polygon.
      return false;
    }

    if (firstEdge->plane->isFunctionallyIdentical(lastEdge->plane)) {
      throw make_shared<TileException>(
          L"Two adjacent edge planes are effectively parallel despite "
          L"filtering; give up on tiling");
    }

    // Now look for completely planar points.  This too is a degeneracy
    // condition that we should return "false" for.
    shared_ptr<Edge> edge = firstEdge;
    points.push_back(edge->startPoint);
    int k = 0;
    while (true) {
      if (edge == lastEdge) {
        break;
      }
      points.push_back(edge->endPoint);
      internalEdges->set(k++, edge->isInternal);
      edge = edgeBuffer->getNext(edge);
    }
    returnIsInternal = lastEdge->isInternal;
    edgeBuffer->clear();
  } else {
    // Build the return edge (internal, of course)
    shared_ptr<SidedPlane> *const returnSidedPlane = make_shared<SidedPlane>(
        firstEdge->endPoint, false, firstEdge->startPoint, lastEdge->endPoint);
    shared_ptr<Edge> *const returnEdge = make_shared<Edge>(
        firstEdge->startPoint, lastEdge->endPoint, returnSidedPlane, true);
    if (returnEdge->plane->isFunctionallyIdentical(lastEdge->plane) ||
        returnEdge->plane->isFunctionallyIdentical(firstEdge->plane)) {
      throw make_shared<TileException>(
          L"Two adjacent edge planes are effectively parallel despite "
          L"filtering; give up on tiling");
    }
    // Build point deque and edge deque
    const deque<std::shared_ptr<Edge>> edges =
        deque<std::shared_ptr<Edge>>(includedEdges->size());
    returnIsInternal = true;

    // Now look for completely planar points.  This too is a degeneracy
    // condition that we should return "false" for.
    shared_ptr<Edge> edge = firstEdge;
    points.push_back(edge->startPoint);
    int k = 0;
    while (true) {
      points.push_back(edge->endPoint);
      internalEdges->set(k++, edge->isInternal);
      edges.push_back(edge);
      if (edge == lastEdge) {
        break;
      }
      edge = edgeBuffer->getNext(edge);
    }
    // Modify the edge buffer
    edgeBuffer->replace(edges, returnEdge);
  }

  // Now, construct the polygon
  if (testPoint != nullptr && holes.size() > 0 && holes.size() > 0) {
    // No holes, for test
    shared_ptr<GeoPolygon> *const testPolygon = make_shared<GeoConvexPolygon>(
        planetModel, points, nullptr, internalEdges, returnIsInternal);
    if (testPolygon->isWithin(testPoint)) {
      return nullopt;
    }
  }

  shared_ptr<GeoPolygon> *const realPolygon = make_shared<GeoConvexPolygon>(
      planetModel, points, holes, internalEdges, returnIsInternal);
  if (testPoint != nullptr && (holes.empty() || holes.empty())) {
    if (realPolygon->isWithin(testPoint)) {
      return nullopt;
    }
  }

  rval->addShape(realPolygon);
  return true;
}

bool GeoPolygonFactory::isWithin(shared_ptr<GeoPoint> point,
                                 shared_ptr<Set<std::shared_ptr<Edge>>> edgeSet,
                                 shared_ptr<Edge> extension,
                                 shared_ptr<SidedPlane> returnBoundary)
{
  if (!extension->plane->isWithin(point)) {
    return false;
  }
  if (returnBoundary != nullptr && !returnBoundary->isWithin(point)) {
    return false;
  }
  return isWithin(point, edgeSet);
}

bool GeoPolygonFactory::isWithin(shared_ptr<GeoPoint> point,
                                 shared_ptr<Set<std::shared_ptr<Edge>>> edgeSet)
{
  for (auto edge : edgeSet) {
    if (!edge->plane->isWithin(point)) {
      return false;
    }
  }
  return true;
}

int GeoPolygonFactory::getLegalIndex(int index, int size)
{
  while (index < 0) {
    index += size;
  }
  while (index >= size) {
    index -= size;
  }
  return index;
}

GeoPolygonFactory::Edge::Edge(shared_ptr<GeoPoint> startPoint,
                              shared_ptr<GeoPoint> endPoint,
                              shared_ptr<SidedPlane> plane,
                              bool const isInternal)
    : plane(plane), startPoint(startPoint), endPoint(endPoint),
      isInternal(isInternal)
{
}

int GeoPolygonFactory::Edge::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool GeoPolygonFactory::Edge::equals(any const o)
{
  return o == shared_from_this();
}

GeoPolygonFactory::EdgeBufferIterator::EdgeBufferIterator(
    shared_ptr<EdgeBuffer> edgeBuffer)
    : edgeBuffer(edgeBuffer), firstEdge(currentEdge)
{
  this->currentEdge = edgeBuffer->pickOne();
}

bool GeoPolygonFactory::EdgeBufferIterator::hasNext()
{
  return currentEdge != nullptr;
}

shared_ptr<Edge> GeoPolygonFactory::EdgeBufferIterator::next()
{
  shared_ptr<Edge> *const rval = currentEdge;
  if (currentEdge != nullptr) {
    currentEdge = edgeBuffer->getNext(currentEdge);
    if (currentEdge == firstEdge) {
      currentEdge.reset();
    }
  }
  return rval;
}

void GeoPolygonFactory::EdgeBufferIterator::remove()
{
  throw runtime_error(L"Unsupported operation");
}

GeoPolygonFactory::EdgeBuffer::EdgeBuffer(
    deque<std::shared_ptr<GeoPoint>> &pointList,
    shared_ptr<BitSet> internalEdges, int const startPlaneStartIndex,
    int const startPlaneEndIndex, shared_ptr<SidedPlane> startPlane)
{
  /*
  System.out.println("Start plane index: "+startPlaneStartIndex+" End plane
  index: "+startPlaneEndIndex+" Initial points:"); for (final GeoPoint p :
  pointList) { System.out.println(" "+p);
  }
  */

  shared_ptr<Edge> *const startEdge = make_shared<Edge>(
      pointList[startPlaneStartIndex], pointList[startPlaneEndIndex],
      startPlane, internalEdges->get(startPlaneStartIndex));
  // Fill in the EdgeBuffer by walking around creating more stuff
  shared_ptr<Edge> currentEdge = startEdge;
  int startIndex = startPlaneStartIndex;
  int endIndex = startPlaneEndIndex;
  while (true) {
    /*
    System.out.println("For plane "+currentEdge.plane+", the following points
    are in/out:"); for (final GeoPoint p: pointList) { System.out.println("
    "+p+" is: "+(currentEdge.plane.isWithin(p)?"in":"out"));
    }
    */

    // Check termination condition
    if (currentEdge->endPoint == startEdge->startPoint) {
      // We finish here.  Link the current edge to the start edge, and exit
      previousEdges.emplace(startEdge, currentEdge);
      nextEdges.emplace(currentEdge, startEdge);
      edges->add(startEdge);
      break;
    }

    // Compute the next edge
    startIndex = endIndex;
    endIndex++;
    if (endIndex >= pointList.size()) {
      endIndex -= pointList.size();
    }
    // Get the next point
    shared_ptr<GeoPoint> *const newPoint = pointList[endIndex];
    // Build the new edge
    // We need to know the sidedness of the new plane.  The point we're going to
    // be presenting to it has a certain relationship with the sided plane we
    // already have for the current edge.  If the current edge is colinear with
    // the new edge, then we want to maintain the same relationship.  If the new
    // edge is not colinear, then we can use the new point's relationship with
    // the current edge as our guide.

    constexpr bool isNewPointWithin = currentEdge->plane->isWithin(newPoint);
    shared_ptr<GeoPoint> *const pointToPresent = currentEdge->startPoint;

    shared_ptr<SidedPlane> *const newPlane = make_shared<SidedPlane>(
        pointToPresent, isNewPointWithin, pointList[startIndex], newPoint);
    shared_ptr<Edge> *const newEdge =
        make_shared<Edge>(pointList[startIndex], pointList[endIndex], newPlane,
                          internalEdges->get(startIndex));

    // Link it in
    previousEdges.emplace(newEdge, currentEdge);
    nextEdges.emplace(currentEdge, newEdge);
    edges->add(newEdge);
    currentEdge = newEdge;
  }

  oneEdge = startEdge;

  // Verify the structure.
  // verify();
}

shared_ptr<Edge>
GeoPolygonFactory::EdgeBuffer::getPrevious(shared_ptr<Edge> currentEdge)
{
  return previousEdges[currentEdge];
}

shared_ptr<Edge>
GeoPolygonFactory::EdgeBuffer::getNext(shared_ptr<Edge> currentEdge)
{
  return nextEdges[currentEdge];
}

void GeoPolygonFactory::EdgeBuffer::replace(
    deque<std::shared_ptr<Edge>> &removeList, shared_ptr<Edge> newEdge)
{
  /*
  System.out.println("Replacing: ");
  for (final Edge e : removeList) {
    System.out.println(" "+e.startPoint+"-->"+e.endPoint);
  }
  System.out.println("...with: "+newEdge.startPoint+"-->"+newEdge.endPoint);
  */
  shared_ptr<Edge> *const previous = previousEdges[removeList[0]];
  shared_ptr<Edge> *const next = nextEdges[removeList[removeList.size() - 1]];
  edges->add(newEdge);
  previousEdges.emplace(newEdge, previous);
  nextEdges.emplace(previous, newEdge);
  previousEdges.emplace(next, newEdge);
  nextEdges.emplace(newEdge, next);
  for (auto edge : removeList) {
    if (edge == oneEdge) {
      oneEdge = newEdge;
    }
    edges->remove(edge);
    previousEdges.erase(edge);
    nextEdges.erase(edge);
  }
  // verify();
}

void GeoPolygonFactory::EdgeBuffer::clear()
{
  edges->clear();
  previousEdges.clear();
  nextEdges.clear();
  oneEdge.reset();
}

int GeoPolygonFactory::EdgeBuffer::size() { return edges->size(); }

shared_ptr<Iterator<std::shared_ptr<Edge>>>
GeoPolygonFactory::EdgeBuffer::iterator()
{
  return make_shared<EdgeBufferIterator>(shared_from_this());
}

shared_ptr<Edge> GeoPolygonFactory::EdgeBuffer::pickOne() { return oneEdge; }

GeoPolygonFactory::SafePath::SafePath(shared_ptr<SafePath> previous,
                                      shared_ptr<GeoPoint> lastPoint,
                                      int const lastPointIndex,
                                      shared_ptr<Plane> lastPlane)
    : lastPoint(lastPoint), lastPointIndex(lastPointIndex),
      lastPlane(lastPlane), previous(previous)
{
}

void GeoPolygonFactory::SafePath::fillInList(
    deque<std::shared_ptr<GeoPoint>> &pointList)
{
  // we don't use recursion because it can be problematic
  // for polygons with many points.
  shared_ptr<SafePath> safePath = shared_from_this();
  while (safePath->previous != nullptr) {
    pointList.push_back(safePath->lastPoint);
    safePath = safePath->previous;
  }
  pointList.push_back(safePath->lastPoint);
  reverse(pointList.begin(), pointList.end());
}

GeoPolygonFactory::TileException::TileException(const wstring &msg)
    : Exception(msg)
{
}
} // namespace org::apache::lucene::spatial3d::geom