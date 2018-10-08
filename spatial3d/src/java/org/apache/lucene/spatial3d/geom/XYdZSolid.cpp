using namespace std;

#include "XYdZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

XYdZSolid::XYdZSolid(shared_ptr<PlanetModel> planetModel, double const minX,
                     double const maxX, double const minY, double const maxY,
                     double const Z)
    : BaseXYZSolid(planetModel), minX(minX), maxX(maxX), minY(minY), maxY(maxY),
      Z(Z),
      minXPlane(make_shared<SidedPlane>(maxX, 0.0, 0.0, xUnitVector, -minX)),
      maxXPlane(make_shared<SidedPlane>(minX, 0.0, 0.0, xUnitVector, -maxX)),
      minYPlane(make_shared<SidedPlane>(0.0, maxY, 0.0, yUnitVector, -minY)),
      maxYPlane(make_shared<SidedPlane>(0.0, minY, 0.0, yUnitVector, -maxY)),
      zPlane(make_shared<Plane>(zUnitVector, -Z)),
      edgePoints(glueTogether({minXZ, maxXZ, minYZ, maxYZ, zEdges})),
      notableZPoints(glueTogether({minXZ, maxXZ, minYZ, maxYZ}))
{
  // Argument checking
  if (maxX - minX < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"X values in wrong order or identical");
  }
  if (maxY - minY < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Y values in wrong order or identical");
  }

  constexpr double worldMinZ = planetModel->getMinimumZValue();
  constexpr double worldMaxZ = planetModel->getMaximumZValue();

  // Construct the planes

  // We need at least one point on the planet surface for each manifestation of
  // the shape. There can be up to 2 (on opposite sides of the world).  But we
  // have to go through 4 combinations of adjacent planes in order to find out
  // if any have 2 intersection solution. Typically, this requires 4 square root
  // operations.
  std::deque<std::shared_ptr<GeoPoint>> minXZ = minXPlane->findIntersections(
      planetModel, zPlane, {maxXPlane, minYPlane, maxYPlane});
  std::deque<std::shared_ptr<GeoPoint>> maxXZ = maxXPlane->findIntersections(
      planetModel, zPlane, {minXPlane, minYPlane, maxYPlane});
  std::deque<std::shared_ptr<GeoPoint>> minYZ = minYPlane->findIntersections(
      planetModel, zPlane, {maxYPlane, minXPlane, maxXPlane});
  std::deque<std::shared_ptr<GeoPoint>> maxYZ = maxYPlane->findIntersections(
      planetModel, zPlane, {minYPlane, minXPlane, maxXPlane});

  // Now, compute the edge points.
  // This is the trickiest part of setting up an XYZSolid.  We've computed
  // intersections already, so we'll start there.  We know that at most there
  // will be two disconnected shapes on the planet surface. But there's also a
  // case where exactly one plane slices through the world, and none of the
  // bounding plane intersections do.  Thus, if we don't find any of the edge
  // intersection cases, we have to look for that last case.

  // If we still haven't encountered anything, we need to look at
  // single-plane/world intersections. We detect these by looking at the world
  // model and noting its x, y, and z bounds. The cases we are looking for are
  // when the four corner points for any given plane are all outside of the
  // world, AND that plane intersects the world. There are four corner points
  // all told; we must evaluate these WRT the planet surface.
  constexpr bool minXminYZ = planetModel->pointOutside(minX, minY, Z);
  constexpr bool minXmaxYZ = planetModel->pointOutside(minX, maxY, Z);
  constexpr bool maxXminYZ = planetModel->pointOutside(maxX, minY, Z);
  constexpr bool maxXmaxYZ = planetModel->pointOutside(maxX, maxY, Z);

  std::deque<std::shared_ptr<GeoPoint>> zEdges;
  if (Z - worldMinZ >= -Vector::MINIMUM_RESOLUTION &&
      Z - worldMaxZ <= Vector::MINIMUM_RESOLUTION && minX < 0.0 && maxX > 0.0 &&
      minY < 0.0 && maxY > 0.0 && minXminYZ && minXmaxYZ && maxXminYZ &&
      maxXmaxYZ) {
    // Find any point on the minZ plane that intersects the world
    // First construct a perpendicular plane that will allow us to find a sample
    // point. This plane is vertical and goes through the points (0,0,0) and
    // (1,0,0) Then use it to compute a sample point.
    shared_ptr<GeoPoint> *const intPoint =
        zPlane->getSampleIntersectionPoint(planetModel, xVerticalPlane);
    if (intPoint != nullptr) {
      zEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
    } else {
      zEdges = EMPTY_POINTS;
    }
  } else {
    zEdges = EMPTY_POINTS;
  }
}

XYdZSolid::XYdZSolid(shared_ptr<PlanetModel> planetModel,
                     shared_ptr<InputStream> inputStream) 
    : XYdZSolid(planetModel, SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream))
{
}

void XYdZSolid::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, minX);
  SerializableObject::writeDouble(outputStream, maxX);
  SerializableObject::writeDouble(outputStream, minY);
  SerializableObject::writeDouble(outputStream, maxY);
  SerializableObject::writeDouble(outputStream, Z);
}

std::deque<std::shared_ptr<GeoPoint>> XYdZSolid::getEdgePoints()
{
  return edgePoints;
}

bool XYdZSolid::isWithin(double const x, double const y, double const z)
{
  return minXPlane->isWithin(x, y, z) && maxXPlane->isWithin(x, y, z) &&
         minYPlane->isWithin(x, y, z) && maxYPlane->isWithin(x, y, z) &&
         zPlane->evaluateIsZero(x, y, z);
}

int XYdZSolid::getRelationship(shared_ptr<GeoShape> path)
{

  // System.err.println(this+" getrelationship with "+path);
  constexpr int insideRectangle = isShapeInsideArea(path);
  if (insideRectangle == SOME_INSIDE) {
    // System.err.println(" some inside");
    return OVERLAPS;
  }

  // Figure out if the entire XYZArea is contained by the shape.
  constexpr int insideShape = isAreaInsideShape(path);
  if (insideShape == SOME_INSIDE) {
    return OVERLAPS;
  }

  if (insideRectangle == ALL_INSIDE && insideShape == ALL_INSIDE) {
    // System.err.println(" inside of each other");
    return OVERLAPS;
  }

  if (path->intersects(zPlane, notableZPoints,
                       {minXPlane, maxXPlane, minYPlane, maxYPlane})) {
    // System.err.println(" edges intersect");
    return OVERLAPS;
  }

  if (insideRectangle == ALL_INSIDE) {
    // System.err.println(" shape inside rectangle");
    return WITHIN;
  }

  if (insideShape == ALL_INSIDE) {
    // System.err.println(" shape contains rectangle");
    return CONTAINS;
  }
  // System.err.println(" disjoint");
  return DISJOINT;
}

bool XYdZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<XYdZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<XYdZSolid> other = any_cast<std::shared_ptr<XYdZSolid>>(o);
  if (!BaseXYZSolid::equals(other)) {
    return false;
  }
  return other->minXPlane->equals(minXPlane) &&
         other->maxXPlane->equals(maxXPlane) &&
         other->minYPlane->equals(minYPlane) &&
         other->maxYPlane->equals(maxYPlane) && other->zPlane->equals(zPlane);
}

int XYdZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  result = 31 * result + minXPlane->hashCode();
  result = 31 * result + maxXPlane->hashCode();
  result = 31 * result + minYPlane->hashCode();
  result = 31 * result + maxYPlane->hashCode();
  result = 31 * result + zPlane->hashCode();
  return result;
}

wstring XYdZSolid::toString()
{
  return L"XYdZSolid: {planetmodel=" + planetModel + L", minXplane=" +
         minXPlane + L", maxXplane=" + maxXPlane + L", minYplane=" + minYPlane +
         L", maxYplane=" + maxYPlane + L", zplane=" + zPlane + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom