using namespace std;

#include "dXdYZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

dXdYZSolid::dXdYZSolid(shared_ptr<PlanetModel> planetModel, double const X,
                       double const Y, double const minZ, double const maxZ)
    : BaseXYZSolid(planetModel), X(X), Y(Y), minZ(minZ), maxZ(maxZ),
      surfacePoints(
          xPlane::findIntersections(planetModel, yPlane, minZPlane, maxZPlane))
{
  // Argument checking
  if (maxZ - minZ < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Z values in wrong order or identical");
  }

  // Build the planes and intersect them.
  shared_ptr<Plane> *const xPlane = make_shared<Plane>(xUnitVector, -X);
  shared_ptr<Plane> *const yPlane = make_shared<Plane>(yUnitVector, -Y);
  shared_ptr<SidedPlane> *const minZPlane =
      make_shared<SidedPlane>(0.0, 0.0, maxZ, zUnitVector, -minZ);
  shared_ptr<SidedPlane> *const maxZPlane =
      make_shared<SidedPlane>(0.0, 0.0, minZ, zUnitVector, -maxZ);
}

dXdYZSolid::dXdYZSolid(shared_ptr<PlanetModel> planetModel,
                       shared_ptr<InputStream> inputStream) 
    : dXdYZSolid(planetModel, SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream))
{
}

void dXdYZSolid::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, X);
  SerializableObject::writeDouble(outputStream, Y);
  SerializableObject::writeDouble(outputStream, minZ);
  SerializableObject::writeDouble(outputStream, maxZ);
}

std::deque<std::shared_ptr<GeoPoint>> dXdYZSolid::getEdgePoints()
{
  return surfacePoints;
}

bool dXdYZSolid::isWithin(double const x, double const y, double const z)
{
  for (auto p : surfacePoints) {
    if (p->isIdentical(x, y, z)) {
      return true;
    }
  }
  return false;
}

int dXdYZSolid::getRelationship(shared_ptr<GeoShape> path)
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

bool dXdYZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<dXdYZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<dXdYZSolid> other = any_cast<std::shared_ptr<dXdYZSolid>>(o);
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

int dXdYZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  for (auto p : surfacePoints) {
    result = 31 * result + p->hashCode();
  }
  return result;
}

wstring dXdYZSolid::toString()
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  for (auto p : surfacePoints) {
    sb->append(L" ")->append(p)->append(L" ");
  }
  return L"dXdYZSolid: {planetmodel=" + planetModel + L", " + sb->toString() +
         L"}";
}
} // namespace org::apache::lucene::spatial3d::geom