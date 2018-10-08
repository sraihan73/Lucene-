using namespace std;

#include "dXdYdZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const dXdYdZSolid::nullPoints =
    std::deque<std::shared_ptr<GeoPoint>>(0);

dXdYdZSolid::dXdYdZSolid(shared_ptr<PlanetModel> planetModel, double const X,
                         double const Y, double const Z)
    : BaseXYZSolid(planetModel), X(X), Y(Y), Z(Z),
      isOnSurface(planetModel->pointOnSurface(X, Y, Z))
{

  if (isOnSurface) {
    thePoint = make_shared<GeoPoint>(X, Y, Z);
    edgePoints = std::deque<std::shared_ptr<GeoPoint>>{thePoint};
  } else {
    thePoint.reset();
    edgePoints = nullPoints;
  }
}

dXdYdZSolid::dXdYdZSolid(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<InputStream> inputStream) 
    : dXdYdZSolid(planetModel, SerializableObject::readDouble(inputStream),
                  SerializableObject::readDouble(inputStream),
                  SerializableObject::readDouble(inputStream))
{
}

void dXdYdZSolid::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, X);
  SerializableObject::writeDouble(outputStream, Y);
  SerializableObject::writeDouble(outputStream, Z);
}

std::deque<std::shared_ptr<GeoPoint>> dXdYdZSolid::getEdgePoints()
{
  return edgePoints;
}

bool dXdYdZSolid::isWithin(double const x, double const y, double const z)
{
  if (!isOnSurface) {
    return false;
  }
  return thePoint->isIdentical(x, y, z);
}

int dXdYdZSolid::getRelationship(shared_ptr<GeoShape> path)
{
  if (!isOnSurface) {
    return DISJOINT;
  }

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

  if (insideRectangle == ALL_INSIDE) {
    // System.err.println(" shape inside area entirely");
    return WITHIN;
  }

  if (insideShape == ALL_INSIDE) {
    // System.err.println(" shape contains area entirely");
    return CONTAINS;
  }
  // System.err.println(" disjoint");
  return DISJOINT;
}

bool dXdYdZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<dXdYdZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<dXdYdZSolid> other = any_cast<std::shared_ptr<dXdYdZSolid>>(o);
  if (!BaseXYZSolid::equals(other) || other->isOnSurface != isOnSurface) {
    return false;
  }
  if (isOnSurface) {
    return other->thePoint->equals(thePoint);
  }
  return true;
}

int dXdYdZSolid::hashCode()
{
  int result = BaseXYZSolid::hashCode();
  result = 31 * result + (isOnSurface ? 1 : 0);
  if (isOnSurface) {
    result = 31 * result + thePoint->hashCode();
  }
  return result;
}

wstring dXdYdZSolid::toString()
{
  return L"dXdYdZSolid: {planetmodel=" + planetModel + L", isOnSurface=" +
         StringHelper::toString(isOnSurface) + L", thePoint=" + thePoint + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom