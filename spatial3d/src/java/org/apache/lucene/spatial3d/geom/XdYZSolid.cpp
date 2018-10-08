using namespace std;

#include "XdYZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

XdYZSolid::XdYZSolid(shared_ptr<PlanetModel> planetModel, double const minX,
                     double const maxX, double const Y, double const minZ,
                     double const maxZ)
    : BaseXYZSolid(planetModel), minX(minX), maxX(maxX), Y(Y), minZ(minZ),
      maxZ(maxZ),
      minXPlane(make_shared<SidedPlane>(maxX, 0.0, 0.0, xUnitVector, -minX)),
      maxXPlane(make_shared<SidedPlane>(minX, 0.0, 0.0, xUnitVector, -maxX)),
      yPlane(make_shared<Plane>(yUnitVector, -Y)),
      minZPlane(make_shared<SidedPlane>(0.0, 0.0, maxZ, zUnitVector, -minZ)),
      maxZPlane(make_shared<SidedPlane>(0.0, 0.0, minZ, zUnitVector, -maxZ)),
      edgePoints(glueTogether({minXY, maxXY, YminZ, YmaxZ, yEdges})),
      notableYPoints(glueTogether({minXY, maxXY, YminZ, YmaxZ}))
{
  // Argument checking
  if (maxX - minX < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"X values in wrong order or identical");
  }
  if (maxZ - minZ < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Z values in wrong order or identical");
  }

  constexpr double worldMinY = planetModel->getMinimumYValue();
  constexpr double worldMaxY = planetModel->getMaximumYValue();

  // Construct the planes

  // We need at least one point on the planet surface for each manifestation of
  // the shape. There can be up to 2 (on opposite sides of the world).  But we
  // have to go through 4 combinations of adjacent planes in order to find out
  // if any have 2 intersection solution. Typically, this requires 4 square root
  // operations.
  std::deque<std::shared_ptr<GeoPoint>> minXY = minXPlane->findIntersections(
      planetModel, yPlane, {maxXPlane, minZPlane, maxZPlane});
  std::deque<std::shared_ptr<GeoPoint>> maxXY = maxXPlane->findIntersections(
      planetModel, yPlane, {minXPlane, minZPlane, maxZPlane});
  std::deque<std::shared_ptr<GeoPoint>> YminZ = yPlane->findIntersections(
      planetModel, minZPlane, {maxZPlane, minXPlane, maxXPlane});
  std::deque<std::shared_ptr<GeoPoint>> YmaxZ = yPlane->findIntersections(
      planetModel, maxZPlane, {minZPlane, minXPlane, maxXPlane});

  // Now, compute the edge points.
  // This is the trickiest part of setting up an XYZSolid.  We've computed
  // intersections already, so we'll start there.  We know that at most there
  // will be two disconnected shapes on the planet surface. But there's also a
  // case where exactly one plane slices through the world, and none of the
  // bounding plane intersections do.  Thus, if we don't find any of the edge
  // intersection cases, we have to look for that last case.

  // We need to look at single-plane/world intersections.
  // We detect these by looking at the world model and noting its x, y, and z
  // bounds. The cases we are looking for are when the four corner points for
  // any given plane are all outside of the world, AND that plane intersects the
  // world. There are four corner points all told; we must evaluate these WRT
  // the planet surface.
  constexpr bool minXYminZ = planetModel->pointOutside(minX, Y, minZ);
  constexpr bool minXYmaxZ = planetModel->pointOutside(minX, Y, maxZ);
  constexpr bool maxXYminZ = planetModel->pointOutside(maxX, Y, minZ);
  constexpr bool maxXYmaxZ = planetModel->pointOutside(maxX, Y, maxZ);

  std::deque<std::shared_ptr<GeoPoint>> yEdges;
  if (Y - worldMinY >= -Vector::MINIMUM_RESOLUTION &&
      Y - worldMaxY <= Vector::MINIMUM_RESOLUTION && minX < 0.0 && maxX > 0.0 &&
      minZ < 0.0 && maxZ > 0.0 && minXYminZ && minXYmaxZ && maxXYminZ &&
      maxXYmaxZ) {
    // Find any point on the minY plane that intersects the world
    // First construct a perpendicular plane that will allow us to find a sample
    // point. This plane is vertical and goes through the points (0,0,0) and
    // (0,1,0) Then use it to compute a sample point.
    shared_ptr<GeoPoint> *const intPoint =
        yPlane->getSampleIntersectionPoint(planetModel, yVerticalPlane);
    if (intPoint != nullptr) {
      yEdges = std::deque<std::shared_ptr<GeoPoint>>{intPoint};
    } else {
      yEdges = EMPTY_POINTS;
    }
  } else {
    yEdges = EMPTY_POINTS;
  }
}

XdYZSolid::XdYZSolid(shared_ptr<PlanetModel> planetModel,
                     shared_ptr<InputStream> inputStream) 
    : XdYZSolid(planetModel, SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream),
                SerializableObject::readDouble(inputStream))
{
}

void XdYZSolid::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, minX);
  SerializableObject::writeDouble(outputStream, maxX);
  SerializableObject::writeDouble(outputStream, Y);
  SerializableObject::writeDouble(outputStream, minZ);
  SerializableObject::writeDouble(outputStream, maxZ);
}

std::deque<std::shared_ptr<GeoPoint>> XdYZSolid::getEdgePoints()
{
  return edgePoints;
}

bool XdYZSolid::isWithin(double const x, double const y, double const z)
{
  return minXPlane->isWithin(x, y, z) && maxXPlane->isWithin(x, y, z) &&
         yPlane->evaluateIsZero(x, y, z) && minZPlane->isWithin(x, y, z) &&
         maxZPlane->isWithin(x, y, z);
}

int XdYZSolid::getRelationship(shared_ptr<GeoShape> path)
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

  if (path->intersects(yPlane, notableYPoints,
                       {minXPlane, maxXPlane, minZPlane, maxZPlane})) {
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

bool XdYZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<XdYZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<XdYZSolid> other = any_cast<std::shared_ptr<XdYZSolid>>(o);
  if (!BaseXYZSolid::equals(other)) {
    return false;
  }
  return other->minXPlane->equals(minXPlane) &&
         other->maxXPlane->equals(maxXPlane) && other->yPlane->equals(yPlane) &&
         other->minZPlane->equals(minZPlane) &&
         other->maxZPlane->equals(maxZPlane);
}

int XdYZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  result = 31 * result + minXPlane->hashCode();
  result = 31 * result + maxXPlane->hashCode();
  result = 31 * result + yPlane->hashCode();
  result = 31 * result + minZPlane->hashCode();
  result = 31 * result + maxZPlane->hashCode();
  return result;
}

wstring XdYZSolid::toString()
{
  return L"XdYZSolid: {planetmodel=" + planetModel + L", minXplane=" +
         minXPlane + L", maxXplane=" + maxXPlane + L", yplane=" + yPlane +
         L", minZplane=" + minZPlane + L", maxZplane=" + maxZPlane + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom