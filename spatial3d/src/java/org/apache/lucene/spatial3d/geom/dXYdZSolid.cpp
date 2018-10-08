using namespace std;

#include "dXYdZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

dXYdZSolid::dXYdZSolid(shared_ptr<PlanetModel> planetModel, double const X,
                       double const minY, double const maxY, double const Z)
    : BaseXYZSolid(planetModel), X(X), minY(minY), maxY(maxY), Z(Z),
      surfacePoints(
          xPlane::findIntersections(planetModel, zPlane, minYPlane, maxYPlane))
{
  // Argument checking
  if (maxY - minY < Vector::MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Y values in wrong order or identical");
  }

  // Build the planes and intersect them.
  shared_ptr<Plane> *const xPlane = make_shared<Plane>(xUnitVector, -X);
  shared_ptr<Plane> *const zPlane = make_shared<Plane>(zUnitVector, -Z);
  shared_ptr<SidedPlane> *const minYPlane =
      make_shared<SidedPlane>(0.0, maxY, 0.0, yUnitVector, -minY);
  shared_ptr<SidedPlane> *const maxYPlane =
      make_shared<SidedPlane>(0.0, minY, 0.0, yUnitVector, -maxY);
}

dXYdZSolid::dXYdZSolid(shared_ptr<PlanetModel> planetModel,
                       shared_ptr<InputStream> inputStream) 
    : dXYdZSolid(planetModel, SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream),
                 SerializableObject::readDouble(inputStream))
{
}

void dXYdZSolid::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, X);
  SerializableObject::writeDouble(outputStream, minY);
  SerializableObject::writeDouble(outputStream, maxY);
  SerializableObject::writeDouble(outputStream, Z);
}

std::deque<std::shared_ptr<GeoPoint>> dXYdZSolid::getEdgePoints()
{
  return surfacePoints;
}

bool dXYdZSolid::isWithin(double const x, double const y, double const z)
{
  for (auto p : surfacePoints) {
    if (p->isIdentical(x, y, z)) {
      return true;
    }
  }
  return false;
}

int dXYdZSolid::getRelationship(shared_ptr<GeoShape> path)
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

bool dXYdZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<dXYdZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<dXYdZSolid> other = any_cast<std::shared_ptr<dXYdZSolid>>(o);
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

int dXYdZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  for (auto p : surfacePoints) {
    result = 31 * result + p->hashCode();
  }
  return result;
}

wstring dXYdZSolid::toString()
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  for (auto p : surfacePoints) {
    sb->append(L" ")->append(p)->append(L" ");
  }
  return L"dXYdZSolid: {planetmodel=" + planetModel + L", " + sb->toString() +
         L"}";
}
} // namespace org::apache::lucene::spatial3d::geom