using namespace std;

#include "XdYdZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

XdYdZSolid::XdYdZSolid(shared_ptr<PlanetModel> planetModel, double const minX,
                       double const maxX, double const Y, double const Z)
    : BaseXYZSolid(planetModel), minX(minX), maxX(maxX), Y(Y), Z(Z),
      surfacePoints(
          yPlane::findIntersections(planetModel, zPlane, minXPlane, maxXPlane))
{
  // Argument checking
  if (maxX - minX < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"X values in wrong order or identical");
  }

  // Build the planes and intersect them.
  shared_ptr<Plane> *const yPlane = make_shared<Plane>(yUnitVector, -Y);
  shared_ptr<Plane> *const zPlane = make_shared<Plane>(zUnitVector, -Z);
  shared_ptr<SidedPlane> *const minXPlane =
      make_shared<SidedPlane>(maxX, 0.0, 0.0, xUnitVector, -minX);
  shared_ptr<SidedPlane> *const maxXPlane =
      make_shared<SidedPlane>(minX, 0.0, 0.0, xUnitVector, -maxX);
}

XdYdZSolid::XdYdZSolid(shared_ptr<PlanetModel> planetModel,
                       shared_ptr<InputStream> inputStream) 
    : XdYdZSolid(planetModel, SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream))
{
}

void XdYdZSolid::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, minX);
  SerializableObject::writeDouble(outputStream, maxX);
  SerializableObject::writeDouble(outputStream, Y);
  SerializableObject::writeDouble(outputStream, Z);
}

std::deque<std::shared_ptr<GeoPoint>> XdYdZSolid::getEdgePoints()
{
  return surfacePoints;
}

bool XdYdZSolid::isWithin(double const x, double const y, double const z)
{
  for (auto p : surfacePoints) {
    if (p->isIdentical(x, y, z)) {
      return true;
    }
  }
  return false;
}

int XdYdZSolid::getRelationship(shared_ptr<GeoShape> path)
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

  if (insideRectangle == ALL_INSIDE) {
    return WITHIN;
  }

  if (insideShape == ALL_INSIDE) {
    // System.err.println(" shape contains rectangle");
    return CONTAINS;
  }
  // System.err.println(" disjoint");
  return DISJOINT;
}

bool XdYdZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<XdYdZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<XdYdZSolid> other = any_cast<std::shared_ptr<XdYdZSolid>>(o);
  if (!BaseXYZSolid::equals(other) ||
      surfacePoints.size() != other->surfacePoints.size()) {
    return false;
  }
  for (int i = 0; i < surfacePoints.size(); i++) {
    if (!surfacePoints[i]->equals(other->surfacePoints[i])) {
      return false;
    }
  }
  return true;
}

int XdYdZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  for (auto p : surfacePoints) {
    result = 31 * result + p->hashCode();
  }
  return result;
}

wstring XdYdZSolid::toString()
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  for (auto p : surfacePoints) {
    sb->append(L" ")->append(p)->append(L" ");
  }
  return L"XdYdZSolid: {planetmodel=" + planetModel + L", " + sb->toString() +
         L"}";
}
} // namespace org::apache::lucene::spatial3d::geom