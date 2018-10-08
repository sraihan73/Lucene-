using namespace std;

#include "StandardXYZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

StandardXYZSolid::StandardXYZSolid(shared_ptr<PlanetModel> planetModel,
                                   double const minX, double const maxX,
                                   double const minY, double const maxY,
                                   double const minZ, double const maxZ)
    : BaseXYZSolid(planetModel), minX(minX), maxX(maxX), minY(minY), maxY(maxY),
      minZ(minZ), maxZ(maxZ),
      isWholeWorld((minX - worldMinX < -Vector::MINIMUM_RESOLUTION) &&
                   (maxX - worldMaxX > Vector::MINIMUM_RESOLUTION) &&
                   (minY - worldMinY < -Vector::MINIMUM_RESOLUTION) &&
                   (maxY - worldMaxY > Vector::MINIMUM_RESOLUTION) &&
                   (minZ - worldMinZ < -Vector::MINIMUM_RESOLUTION) &&
                   (maxZ - worldMaxZ > Vector::MINIMUM_RESOLUTION))
{
  // Argument checking
  if (maxX - minX < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"X values in wrong order or identical");
  }
  if (maxY - minY < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Y values in wrong order or identical");
  }
  if (maxZ - minZ < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Z values in wrong order or identical");
  }

  constexpr double worldMinX = planetModel->getMinimumXValue();
  constexpr double worldMaxX = planetModel->getMaximumXValue();
  constexpr double worldMinY = planetModel->getMinimumYValue();
  constexpr double worldMaxY = planetModel->getMaximumYValue();
  constexpr double worldMinZ = planetModel->getMinimumZValue();
  constexpr double worldMaxZ = planetModel->getMaximumZValue();

  // We must distinguish between the case where the solid represents the entire
  // world, and when the solid has no overlap with any part of the surface.  In
  // both cases, there will be no edgepoints.

  if (isWholeWorld) {
    minXPlane.reset();
    maxXPlane.reset();
    minYPlane.reset();
    maxYPlane.reset();
    minZPlane.reset();
    maxZPlane.reset();
    minXPlaneIntersects = false;
    maxXPlaneIntersects = false;
    minYPlaneIntersects = false;
    maxYPlaneIntersects = false;
    minZPlaneIntersects = false;
    maxZPlaneIntersects = false;
    notableMinXPoints.clear();
    notableMaxXPoints.clear();
    notableMinYPoints.clear();
    notableMaxYPoints.clear();
    notableMinZPoints.clear();
    notableMaxZPoints.clear();
    edgePoints.clear();
  } else {
    // Construct the planes
    minXPlane = make_shared<SidedPlane>(maxX, 0.0, 0.0, xUnitVector, -minX);
    maxXPlane = make_shared<SidedPlane>(minX, 0.0, 0.0, xUnitVector, -maxX);
    minYPlane = make_shared<SidedPlane>(0.0, maxY, 0.0, yUnitVector, -minY);
    maxYPlane = make_shared<SidedPlane>(0.0, minY, 0.0, yUnitVector, -maxY);
    minZPlane = make_shared<SidedPlane>(0.0, 0.0, maxZ, zUnitVector, -minZ);
    maxZPlane = make_shared<SidedPlane>(0.0, 0.0, minZ, zUnitVector, -maxZ);

    // We need at least one point on the planet surface for each manifestation
    // of the shape. There can be up to 2 (on opposite sides of the world).  But
    // we have to go through 12 combinations of adjacent planes in order to find
    // out if any have 2 intersection solution. Typically, this requires 12
    // square root operations.
    std::deque<std::shared_ptr<GeoPoint>> minXminY =
        minXPlane->findIntersections(
            planetModel, minYPlane,
            {maxXPlane, maxYPlane, minZPlane, maxZPlane});
    std::deque<std::shared_ptr<GeoPoint>> minXmaxY =
        minXPlane->findIntersections(
            planetModel, maxYPlane,
            {maxXPlane, minYPlane, minZPlane, maxZPlane});
    std::deque<std::shared_ptr<GeoPoint>> minXminZ =
        minXPlane->findIntersections(
            planetModel, minZPlane,
            {maxXPlane, maxZPlane, minYPlane, maxYPlane});
    std::deque<std::shared_ptr<GeoPoint>> minXmaxZ =
        minXPlane->findIntersections(
            planetModel, maxZPlane,
            {maxXPlane, minZPlane, minYPlane, maxYPlane});

    std::deque<std::shared_ptr<GeoPoint>> maxXminY =
        maxXPlane->findIntersections(
            planetModel, minYPlane,
            {minXPlane, maxYPlane, minZPlane, maxZPlane});
    std::deque<std::shared_ptr<GeoPoint>> maxXmaxY =
        maxXPlane->findIntersections(
            planetModel, maxYPlane,
            {minXPlane, minYPlane, minZPlane, maxZPlane});
    std::deque<std::shared_ptr<GeoPoint>> maxXminZ =
        maxXPlane->findIntersections(
            planetModel, minZPlane,
            {minXPlane, maxZPlane, minYPlane, maxYPlane});
    std::deque<std::shared_ptr<GeoPoint>> maxXmaxZ =
        maxXPlane->findIntersections(
            planetModel, maxZPlane,
            {minXPlane, minZPlane, minYPlane, maxYPlane});

    std::deque<std::shared_ptr<GeoPoint>> minYminZ =
        minYPlane->findIntersections(
            planetModel, minZPlane,
            {maxYPlane, maxZPlane, minXPlane, maxXPlane});
    std::deque<std::shared_ptr<GeoPoint>> minYmaxZ =
        minYPlane->findIntersections(
            planetModel, maxZPlane,
            {maxYPlane, minZPlane, minXPlane, maxXPlane});
    std::deque<std::shared_ptr<GeoPoint>> maxYminZ =
        maxYPlane->findIntersections(
            planetModel, minZPlane,
            {minYPlane, maxZPlane, minXPlane, maxXPlane});
    std::deque<std::shared_ptr<GeoPoint>> maxYmaxZ =
        maxYPlane->findIntersections(
            planetModel, maxZPlane,
            {minYPlane, minZPlane, minXPlane, maxXPlane});

    notableMinXPoints = glueTogether({minXminY, minXmaxY, minXminZ, minXmaxZ});
    notableMaxXPoints = glueTogether({maxXminY, maxXmaxY, maxXminZ, maxXmaxZ});
    notableMinYPoints = glueTogether({minXminY, maxXminY, minYminZ, minYmaxZ});
    notableMaxYPoints = glueTogether({minXmaxY, maxXmaxY, maxYminZ, maxYmaxZ});
    notableMinZPoints = glueTogether({minXminZ, maxXminZ, minYminZ, maxYminZ});
    notableMaxZPoints = glueTogether({minXmaxZ, maxXmaxZ, minYmaxZ, maxYmaxZ});

    // System.err.println(
    //  " notableMinXPoints="+Arrays.asList(notableMinXPoints)+"
    //  notableMaxXPoints="+Arrays.asList(notableMaxXPoints)+ "
    //  notableMinYPoints="+Arrays.asList(notableMinYPoints)+"
    //  notableMaxYPoints="+Arrays.asList(notableMaxYPoints)+ "
    //  notableMinZPoints="+Arrays.asList(notableMinZPoints)+"
    //  notableMaxZPoints="+Arrays.asList(notableMaxZPoints));

    // Now, compute the edge points.
    // This is the trickiest part of setting up an XYZSolid.  We've computed
    // intersections already, so we'll start there. There can be a number of
    // shapes, each of which needs an edgepoint.  Each side by itself might
    // contribute an edgepoint, for instance, if the plane describing that side
    // intercepts the planet in such a way that the ellipse of interception does
    // not meet any other planes.  Plane intersections can each contribute 0, 1,
    // or 2 edgepoints.
    //
    // All of this makes for a lot of potential edgepoints, but I believe these
    // can be pruned back with careful analysis. I haven't yet done that
    // analysis, however, so I will treat them all as individual edgepoints.

    // The cases we are looking for are when the four corner points for any
    // given plane are all outside of the world, AND that plane intersects the
    // world. There are eight corner points all told; we must evaluate these WRT
    // the planet surface.
    constexpr bool minXminYminZ = planetModel->pointOutside(minX, minY, minZ);
    constexpr bool minXminYmaxZ = planetModel->pointOutside(minX, minY, maxZ);
    constexpr bool minXmaxYminZ = planetModel->pointOutside(minX, maxY, minZ);
    constexpr bool minXmaxYmaxZ = planetModel->pointOutside(minX, maxY, maxZ);
    constexpr bool maxXminYminZ = planetModel->pointOutside(maxX, minY, minZ);
    constexpr bool maxXminYmaxZ = planetModel->pointOutside(maxX, minY, maxZ);
    constexpr bool maxXmaxYminZ = planetModel->pointOutside(maxX, maxY, minZ);
    constexpr bool maxXmaxYmaxZ = planetModel->pointOutside(maxX, maxY, maxZ);

    // System.err.println("Outside world: minXminYminZ="+minXminYminZ+"
    // minXminYmaxZ="+minXminYmaxZ+" minXmaxYminZ="+minXmaxYminZ+
    //  " minXmaxYmaxZ="+minXmaxYmaxZ+" maxXminYminZ="+maxXminYminZ+"
    //  maxXminYmaxZ="+maxXminYmaxZ+" maxXmaxYminZ="+maxXmaxYminZ+ "
    //  maxXmaxYmaxZ="+maxXmaxYmaxZ);

    // Look at single-plane/world intersections.
    // We detect these by looking at the world model and noting its x, y, and z
    // bounds.

    std::deque<std::shared_ptr<GeoPoint>> minXEdges;
    if (minX - worldMinX >= -Vector::MINIMUM_RESOLUTION &&
        minX - worldMaxX <= Vector::MINIMUM_RESOLUTION && minY < 0.0 &&
        maxY > 0.0 && minZ < 0.0 && maxZ > 0.0 && minXminYminZ &&
        minXminYmaxZ && minXmaxYminZ && minXmaxYmaxZ) {
      // Find any point on the minX plane that intersects the world
      // First construct a perpendicular plane that will allow us to find a
      // sample point. This plane is vertical and goes through the points
      // (0,0,0) and (1,0,0) Then use it to compute a sample point.
      shared_ptr<GeoPoint> *const intPoint =
          minXPlane->getSampleIntersectionPoint(planetModel, xVerticalPlane);
      if (intPoint != nullptr) {
        minXEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
      } else {
        // No intersection found?
        minXEdges = EMPTY_POINTS;
      }
    } else {
      minXEdges = EMPTY_POINTS;
    }

    std::deque<std::shared_ptr<GeoPoint>> maxXEdges;
    if (maxX - worldMinX >= -Vector::MINIMUM_RESOLUTION &&
        maxX - worldMaxX <= Vector::MINIMUM_RESOLUTION && minY < 0.0 &&
        maxY > 0.0 && minZ < 0.0 && maxZ > 0.0 && maxXminYminZ &&
        maxXminYmaxZ && maxXmaxYminZ && maxXmaxYmaxZ) {
      // Find any point on the maxX plane that intersects the world
      // First construct a perpendicular plane that will allow us to find a
      // sample point. This plane is vertical and goes through the points
      // (0,0,0) and (1,0,0) Then use it to compute a sample point.
      shared_ptr<GeoPoint> *const intPoint =
          maxXPlane->getSampleIntersectionPoint(planetModel, xVerticalPlane);
      if (intPoint != nullptr) {
        maxXEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
      } else {
        maxXEdges = EMPTY_POINTS;
      }
    } else {
      maxXEdges = EMPTY_POINTS;
    }

    std::deque<std::shared_ptr<GeoPoint>> minYEdges;
    if (minY - worldMinY >= -Vector::MINIMUM_RESOLUTION &&
        minY - worldMaxY <= Vector::MINIMUM_RESOLUTION && minX < 0.0 &&
        maxX > 0.0 && minZ < 0.0 && maxZ > 0.0 && minXminYminZ &&
        minXminYmaxZ && maxXminYminZ && maxXminYmaxZ) {
      // Find any point on the minY plane that intersects the world
      // First construct a perpendicular plane that will allow us to find a
      // sample point. This plane is vertical and goes through the points
      // (0,0,0) and (0,1,0) Then use it to compute a sample point.
      shared_ptr<GeoPoint> *const intPoint =
          minYPlane->getSampleIntersectionPoint(planetModel, yVerticalPlane);
      if (intPoint != nullptr) {
        minYEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
      } else {
        minYEdges = EMPTY_POINTS;
      }
    } else {
      minYEdges = EMPTY_POINTS;
    }

    std::deque<std::shared_ptr<GeoPoint>> maxYEdges;
    if (maxY - worldMinY >= -Vector::MINIMUM_RESOLUTION &&
        maxY - worldMaxY <= Vector::MINIMUM_RESOLUTION && minX < 0.0 &&
        maxX > 0.0 && minZ < 0.0 && maxZ > 0.0 && minXmaxYminZ &&
        minXmaxYmaxZ && maxXmaxYminZ && maxXmaxYmaxZ) {
      // Find any point on the maxY plane that intersects the world
      // First construct a perpendicular plane that will allow us to find a
      // sample point. This plane is vertical and goes through the points
      // (0,0,0) and (0,1,0) Then use it to compute a sample point.
      shared_ptr<GeoPoint> *const intPoint =
          maxYPlane->getSampleIntersectionPoint(planetModel, yVerticalPlane);
      if (intPoint != nullptr) {
        maxYEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
      } else {
        maxYEdges = EMPTY_POINTS;
      }
    } else {
      maxYEdges = EMPTY_POINTS;
    }

    std::deque<std::shared_ptr<GeoPoint>> minZEdges;
    if (minZ - worldMinZ >= -Vector::MINIMUM_RESOLUTION &&
        minZ - worldMaxZ <= Vector::MINIMUM_RESOLUTION && minX < 0.0 &&
        maxX > 0.0 && minY < 0.0 && maxY > 0.0 && minXminYminZ &&
        minXmaxYminZ && maxXminYminZ && maxXmaxYminZ) {
      // Find any point on the minZ plane that intersects the world
      // First construct a perpendicular plane that will allow us to find a
      // sample point. This plane is vertical and goes through the points
      // (0,0,0) and (1,0,0) Then use it to compute a sample point.
      shared_ptr<GeoPoint> *const intPoint =
          minZPlane->getSampleIntersectionPoint(planetModel, xVerticalPlane);
      if (intPoint != nullptr) {
        minZEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
      } else {
        minZEdges = EMPTY_POINTS;
      }
    } else {
      minZEdges = EMPTY_POINTS;
    }

    std::deque<std::shared_ptr<GeoPoint>> maxZEdges;
    if (maxZ - worldMinZ >= -Vector::MINIMUM_RESOLUTION &&
        maxZ - worldMaxZ <= Vector::MINIMUM_RESOLUTION && minX < 0.0 &&
        maxX > 0.0 && minY < 0.0 && maxY > 0.0 && minXminYmaxZ &&
        minXmaxYmaxZ && maxXminYmaxZ && maxXmaxYmaxZ) {
      // Find any point on the maxZ plane that intersects the world
      // First construct a perpendicular plane that will allow us to find a
      // sample point. This plane is vertical and goes through the points
      // (0,0,0) and (1,0,0) (that is, its orientation doesn't matter) Then use
      // it to compute a sample point.
      shared_ptr<GeoPoint> *const intPoint =
          maxZPlane->getSampleIntersectionPoint(planetModel, xVerticalPlane);
      if (intPoint != nullptr) {
        maxZEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
      } else {
        maxZEdges = EMPTY_POINTS;
      }
    } else {
      maxZEdges = EMPTY_POINTS;
    }

    // System.err.println(
    //  " minXEdges="+Arrays.asList(minXEdges)+"
    //  maxXEdges="+Arrays.asList(maxXEdges)+ "
    //  minYEdges="+Arrays.asList(minYEdges)+"
    //  maxYEdges="+Arrays.asList(maxYEdges)+ "
    //  minZEdges="+Arrays.asList(minZEdges)+"
    //  maxZEdges="+Arrays.asList(maxZEdges));

    minXPlaneIntersects = notableMinXPoints.size() + minXEdges.size() > 0;
    maxXPlaneIntersects = notableMaxXPoints.size() + maxXEdges.size() > 0;
    minYPlaneIntersects = notableMinYPoints.size() + minYEdges.size() > 0;
    maxYPlaneIntersects = notableMaxYPoints.size() + maxYEdges.size() > 0;
    minZPlaneIntersects = notableMinZPoints.size() + minZEdges.size() > 0;
    maxZPlaneIntersects = notableMaxZPoints.size() + maxZEdges.size() > 0;

    // Glue everything together.  This is not a minimal set of edgepoints, as of
    // now, but it does completely describe all shapes on the planet.
    this->edgePoints = glueTogether(
        {minXminY, minXmaxY, minXminZ, minXmaxZ, maxXminY, maxXmaxY, maxXminZ,
         maxXmaxZ, minYminZ, minYmaxZ, maxYminZ, maxYmaxZ, minXEdges, maxXEdges,
         minYEdges, maxYEdges, minZEdges, maxZEdges});
  }
}

StandardXYZSolid::StandardXYZSolid(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : StandardXYZSolid(planetModel, SerializableObject::readDouble(inputStream),
                       SerializableObject::readDouble(inputStream),
                       SerializableObject::readDouble(inputStream),
                       SerializableObject::readDouble(inputStream),
                       SerializableObject::readDouble(inputStream),
                       SerializableObject::readDouble(inputStream))
{
}

void StandardXYZSolid::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, minX);
  SerializableObject::writeDouble(outputStream, maxX);
  SerializableObject::writeDouble(outputStream, minY);
  SerializableObject::writeDouble(outputStream, maxY);
  SerializableObject::writeDouble(outputStream, minZ);
  SerializableObject::writeDouble(outputStream, maxZ);
}

std::deque<std::shared_ptr<GeoPoint>> StandardXYZSolid::getEdgePoints()
{
  return edgePoints;
}

bool StandardXYZSolid::isWithin(double const x, double const y, double const z)
{
  if (isWholeWorld) {
    return true;
  }
  return minXPlane->isWithin(x, y, z) && maxXPlane->isWithin(x, y, z) &&
         minYPlane->isWithin(x, y, z) && maxYPlane->isWithin(x, y, z) &&
         minZPlane->isWithin(x, y, z) && maxZPlane->isWithin(x, y, z);
}

int StandardXYZSolid::getRelationship(shared_ptr<GeoShape> path)
{
  if (isWholeWorld) {
    if (path->getEdgePoints().size() > 0) {
      return WITHIN;
    }
    return OVERLAPS;
  }

  /*
  for (GeoPoint p : getEdgePoints()) {
    System.err.println(" Edge point "+p+" path.isWithin()? "+path.isWithin(p));
  }

  for (GeoPoint p : path.getEdgePoints()) {
    System.err.println(" path edge point "+p+" isWithin()? "+isWithin(p)+"
  minx="+minXPlane.evaluate(p)+" maxx="+maxXPlane.evaluate(p)+"
  miny="+minYPlane.evaluate(p)+" maxy="+maxYPlane.evaluate(p)+"
  minz="+minZPlane.evaluate(p)+" maxz="+maxZPlane.evaluate(p));
  }
  */

  // System.err.println(this+" getrelationship with "+path);
  constexpr int insideRectangle = isShapeInsideArea(path);
  if (insideRectangle == SOME_INSIDE) {
    // System.err.println(" some shape points inside area");
    return OVERLAPS;
  }

  // Figure out if the entire XYZArea is contained by the shape.
  constexpr int insideShape = isAreaInsideShape(path);
  if (insideShape == SOME_INSIDE) {
    // System.err.println(" some area points inside shape");
    return OVERLAPS;
  }

  if (insideRectangle == ALL_INSIDE && insideShape == ALL_INSIDE) {
    // System.err.println(" inside of each other");
    return OVERLAPS;
  }

  if ((minXPlaneIntersects && path->intersects(minXPlane, notableMinXPoints,
                                               {maxXPlane, minYPlane, maxYPlane,
                                                minZPlane, maxZPlane})) ||
      (maxXPlaneIntersects && path->intersects(maxXPlane, notableMaxXPoints,
                                               {minXPlane, minYPlane, maxYPlane,
                                                minZPlane, maxZPlane})) ||
      (minYPlaneIntersects && path->intersects(minYPlane, notableMinYPoints,
                                               {maxYPlane, minXPlane, maxXPlane,
                                                minZPlane, maxZPlane})) ||
      (maxYPlaneIntersects && path->intersects(maxYPlane, notableMaxYPoints,
                                               {minYPlane, minXPlane, maxXPlane,
                                                minZPlane, maxZPlane})) ||
      (minZPlaneIntersects && path->intersects(minZPlane, notableMinZPoints,
                                               {maxZPlane, minXPlane, maxXPlane,
                                                minYPlane, maxYPlane})) ||
      (maxZPlaneIntersects && path->intersects(maxZPlane, notableMaxZPoints,
                                               {minZPlane, minXPlane, maxXPlane,
                                                minYPlane, maxYPlane}))) {
    // System.err.println(" edges intersect");
    return OVERLAPS;
  }

  if (insideRectangle == ALL_INSIDE) {
    // System.err.println(" all shape points inside area");
    return WITHIN;
  }

  if (insideShape == ALL_INSIDE) {
    // System.err.println(" all area points inside shape");
    return CONTAINS;
  }
  // System.err.println(" disjoint");
  return DISJOINT;
}

bool StandardXYZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<StandardXYZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<StandardXYZSolid> other =
      any_cast<std::shared_ptr<StandardXYZSolid>>(o);
  if (!BaseXYZSolid::equals(other) || other->isWholeWorld != isWholeWorld) {
    return false;
  }
  if (!isWholeWorld) {
    return other->minXPlane->equals(minXPlane) &&
           other->maxXPlane->equals(maxXPlane) &&
           other->minYPlane->equals(minYPlane) &&
           other->maxYPlane->equals(maxYPlane) &&
           other->minZPlane->equals(minZPlane) &&
           other->maxZPlane->equals(maxZPlane);
  }
  return true;
}

int StandardXYZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  result = 31 * result + (isWholeWorld ? 1 : 0);
  if (!isWholeWorld) {
    result = 31 * result + minXPlane->hashCode();
    result = 31 * result + maxXPlane->hashCode();
    result = 31 * result + minYPlane->hashCode();
    result = 31 * result + maxYPlane->hashCode();
    result = 31 * result + minZPlane->hashCode();
    result = 31 * result + maxZPlane->hashCode();
  }
  return result;
}

wstring StandardXYZSolid::toString()
{
  return L"StandardXYZSolid: {planetmodel=" + planetModel + L", isWholeWorld=" +
         StringHelper::toString(isWholeWorld) + L", minXplane=" + minXPlane +
         L", maxXplane=" + maxXPlane + L", minYplane=" + minYPlane +
         L", maxYplane=" + maxYPlane + L", minZplane=" + minZPlane +
         L", maxZplane=" + maxZPlane + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom