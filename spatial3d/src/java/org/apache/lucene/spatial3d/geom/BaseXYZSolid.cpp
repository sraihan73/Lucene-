using namespace std;

#include "BaseXYZSolid.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<Vector> BaseXYZSolid::xUnitVector =
    make_shared<Vector>(1.0, 0.0, 0.0);
const shared_ptr<Vector> BaseXYZSolid::yUnitVector =
    make_shared<Vector>(0.0, 1.0, 0.0);
const shared_ptr<Vector> BaseXYZSolid::zUnitVector =
    make_shared<Vector>(0.0, 0.0, 1.0);
const shared_ptr<Plane> BaseXYZSolid::xVerticalPlane =
    make_shared<Plane>(0.0, 1.0, 0.0, 0.0);
const shared_ptr<Plane> BaseXYZSolid::yVerticalPlane =
    make_shared<Plane>(1.0, 0.0, 0.0, 0.0);
std::deque<std::shared_ptr<GeoPoint>> const BaseXYZSolid::EMPTY_POINTS =
    std::deque<std::shared_ptr<GeoPoint>>(0);

BaseXYZSolid::BaseXYZSolid(shared_ptr<PlanetModel> planetModel)
    : BasePlanetObject(planetModel)
{
}

std::deque<std::shared_ptr<GeoPoint>>
BaseXYZSolid::glueTogether(deque<GeoPoint> &pointArrays)
{
  int count = 0;
  for (shared_ptr<GeoPoint> pointArray : pointArrays) {
    count += pointArray.size();
  }
  std::deque<std::shared_ptr<GeoPoint>> rval(count);
  count = 0;
  for (shared_ptr<GeoPoint> pointArray : pointArrays) {
    for (auto point : pointArray) {
      rval[count++] = point;
    }
  }
  return rval;
}

bool BaseXYZSolid::isWithin(shared_ptr<Vector> point)
{
  return isWithin(point->x, point->y, point->z);
}

int BaseXYZSolid::isShapeInsideArea(shared_ptr<GeoShape> path)
{
  std::deque<std::shared_ptr<GeoPoint>> pathPoints = path->getEdgePoints();
  if (pathPoints.empty()) {
    return NO_EDGEPOINTS;
  }
  bool foundOutside = false;
  bool foundInside = false;
  for (auto p : pathPoints) {
    if (isWithin(p)) {
      foundInside = true;
    } else {
      foundOutside = true;
    }
    if (foundInside && foundOutside) {
      return SOME_INSIDE;
    }
  }
  if (!foundInside && !foundOutside) {
    return NONE_INSIDE;
  }
  if (foundInside && !foundOutside) {
    return ALL_INSIDE;
  }
  if (foundOutside && !foundInside) {
    return NONE_INSIDE;
  }
  return SOME_INSIDE;
}

int BaseXYZSolid::isAreaInsideShape(shared_ptr<GeoShape> path)
{
  std::deque<std::shared_ptr<GeoPoint>> edgePoints = getEdgePoints();
  if (edgePoints.empty()) {
    return NO_EDGEPOINTS;
  }
  bool foundOutside = false;
  bool foundInside = false;
  for (auto p : edgePoints) {
    if (path->isWithin(p)) {
      foundInside = true;
    } else {
      foundOutside = true;
    }
    if (foundInside && foundOutside) {
      return SOME_INSIDE;
    }
  }
  if (!foundInside && !foundOutside) {
    return NONE_INSIDE;
  }
  if (foundInside && !foundOutside) {
    return ALL_INSIDE;
  }
  if (foundOutside && !foundInside) {
    return NONE_INSIDE;
  }
  return SOME_INSIDE;
}

bool BaseXYZSolid::equals(any o)
{
  if (!(std::dynamic_pointer_cast<BaseXYZSolid>(o) != nullptr)) {
    return false;
  }
  shared_ptr<BaseXYZSolid> other = any_cast<std::shared_ptr<BaseXYZSolid>>(o);
  return BasePlanetObject::equals(other);
}

int BaseXYZSolid::hashCode() { return BasePlanetObject::hashCode(); }
} // namespace org::apache::lucene::spatial3d::geom