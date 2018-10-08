using namespace std;

#include "dXYZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

dXYZSolid::dXYZSolid(shared_ptr<PlanetModel> planetModel, double const X,
                     double const minY, double const maxY, double const minZ,
                     double const maxZ)
    : BaseXYZSolid(planetModel), X(X), minY(minY), maxY(maxY), minZ(minZ),
      maxZ(maxZ), xPlane(make_shared<Plane>(xUnitVector, -X)),
      minYPlane(make_shared<SidedPlane>(0.0, maxY, 0.0, yUnitVector, -minY)),
      maxYPlane(make_shared<SidedPlane>(0.0, minY, 0.0, yUnitVector, -maxY)),
      minZPlane(make_shared<SidedPlane>(0.0, 0.0, maxZ, zUnitVector, -minZ)),
      maxZPlane(make_shared<SidedPlane>(0.0, 0.0, minZ, zUnitVector, -maxZ)),
      edgePoints(glueTogether({XminY, XmaxY, XminZ, XmaxZ, xEdges})),
      notableXPoints(glueTogether({XminY, XmaxY, XminZ, XmaxZ}))
{
  // Argument checking
  if (maxY - minY < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Y values in wrong order or identical");
  }
  if (maxZ - minZ < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Z values in wrong order or identical");
  }

  constexpr double worldMinX = planetModel->getMinimumXValue();
  constexpr double worldMaxX = planetModel->getMaximumXValue();

  // Construct the planes

  // We need at least one point on the planet surface for each manifestation of
  // the shape. There can be up to 2 (on opposite sides of the world).  But we
  // have to go through 4 combinations of adjacent planes in order to find out
  // if any have 2 intersection solution. Typically, this requires 4 square root
  // operations.
  std::deque<std::shared_ptr<GeoPoint>> XminY = xPlane->findIntersections(
      planetModel, minYPlane, {maxYPlane, minZPlane, maxZPlane});
  std::deque<std::shared_ptr<GeoPoint>> XmaxY = xPlane->findIntersections(
      planetModel, maxYPlane, {minYPlane, minZPlane, maxZPlane});
  std::deque<std::shared_ptr<GeoPoint>> XminZ = xPlane->findIntersections(
      planetModel, minZPlane, {maxZPlane, minYPlane, maxYPlane});
  std::deque<std::shared_ptr<GeoPoint>> XmaxZ = xPlane->findIntersections(
      planetModel, maxZPlane, {minZPlane, minYPlane, maxYPlane});

  // Now, compute the edge points.
  // This is the trickiest part of setting up an XYZSolid.  We've computed
  // intersections already, so we'll start there.  We know that at most there
  // will be two disconnected shapes on the planet surface. But there's also a
  // case where exactly one plane slices through the world, and none of the
  // bounding plane intersections do.  Thus, if we don't find any of the edge
  // intersection cases, we have to look for that last case.

  // We need to look at single-plane/world intersections.
  // We detect these by looking at the world model and noting its x, y, and z
  // bounds. For the single-dimension degenerate case, there's really only one
  // plane that can possibly intersect the world. The cases we are looking for
  // are when the four corner points for any given plane are all outside of the
  // world, AND that plane intersects the world. There are four corner points
  // all told; we must evaluate these WRT the planet surface.
  constexpr bool XminYminZ = planetModel->pointOutside(X, minY, minZ);
  constexpr bool XminYmaxZ = planetModel->pointOutside(X, minY, maxZ);
  constexpr bool XmaxYminZ = planetModel->pointOutside(X, maxY, minZ);
  constexpr bool XmaxYmaxZ = planetModel->pointOutside(X, maxY, maxZ);

  std::deque<std::shared_ptr<GeoPoint>> xEdges;
  if (X - worldMinX >= -Vector::MINIMUM_RESOLUTION &&
      X - worldMaxX <= Vector::MINIMUM_RESOLUTION && minY < 0.0 && maxY > 0.0 &&
      minZ < 0.0 && maxZ > 0.0 && XminYminZ && XminYmaxZ && XmaxYminZ &&
      XmaxYmaxZ) {
    // Find any point on the X plane that intersects the world
    // First construct a perpendicular plane that will allow us to find a sample
    // point. This plane is vertical and goes through the points (0,0,0) and
    // (1,0,0) Then use it to compute a sample point.
    shared_ptr<GeoPoint> *const intPoint =
        xPlane->getSampleIntersectionPoint(planetModel, xVerticalPlane);
    if (intPoint != nullptr) {
      xEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
    } else {
      xEdges = EMPTY_POINTS;
    }
  } else {
    xEdges = EMPTY_POINTS;
  }
}

dXYZSolid::dXYZSolid(shared_ptr<PlanetModel> planetModel,
                     shared_ptr<InputStream> inputStream) 
    : dXYZSolid(planetModel, SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream))
{
}

void dXYZSolid::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, X);
  SerializableObject::writeDouble(outputStream, minY);
  SerializableObject::writeDouble(outputStream, maxY);
  SerializableObject::writeDouble(outputStream, minZ);
  SerializableObject::writeDouble(outputStream, maxZ);
}

std::deque<std::shared_ptr<GeoPoint>> dXYZSolid::getEdgePoints()
{
  return edgePoints;
}

bool dXYZSolid::isWithin(double const x, double const y, double const z)
{
  return xPlane->evaluateIsZero(x, y, z) && minYPlane->isWithin(x, y, z) &&
         maxYPlane->isWithin(x, y, z) && minZPlane->isWithin(x, y, z) &&
         maxZPlane->isWithin(x, y, z);
}

int dXYZSolid::getRelationship(shared_ptr<GeoShape> path)
{
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

  // The entire locus of points in this shape is on a single plane, so we only
  // need ot look for an intersection with that plane.
  // System.err.println("xPlane = "+xPlane);
  if (path->intersects(xPlane, notableXPoints,
                       {minYPlane, maxYPlane, minZPlane, maxZPlane})) {
    // System.err.println(" edges intersect");
    return OVERLAPS;
  }

  if (insideRectangle == ALL_INSIDE) {
    // System.err.println(" shape points inside area");
    return WITHIN;
  }

  if (insideShape == ALL_INSIDE) {
    // System.err.println(" shape contains all area");
    return CONTAINS;
  }
  // System.err.println(" disjoint");
  return DISJOINT;
}

bool dXYZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<dXYZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<dXYZSolid> other = any_cast<std::shared_ptr<dXYZSolid>>(o);
  if (!BaseXYZSolid::equals(other)) {
    return false;
  }
  return other->xPlane->equals(xPlane) && other->minYPlane->equals(minYPlane) &&
         other->maxYPlane->equals(maxYPlane) &&
         other->minZPlane->equals(minZPlane) &&
         other->maxZPlane->equals(maxZPlane);
}

int dXYZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  result = 31 * result + xPlane->hashCode();
  result = 31 * result + minYPlane->hashCode();
  result = 31 * result + maxYPlane->hashCode();
  result = 31 * result + minZPlane->hashCode();
  result = 31 * result + maxZPlane->hashCode();
  return result;
}

wstring dXYZSolid::toString()
{
  return L"dXYZSolid: {planetmodel=" + planetModel + L", xplane=" + xPlane +
         L", minYplane=" + minYPlane + L", maxYplane=" + maxYPlane +
         L", minZplane=" + minZPlane + L", maxZplane=" + maxZPlane + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom