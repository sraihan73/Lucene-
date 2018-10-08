using namespace std;

#include "XYZBounds.h"

namespace org::apache::lucene::spatial3d::geom
{

const double XYZBounds::FUDGE_FACTOR = Vector::MINIMUM_RESOLUTION * 1e3;

XYZBounds::XYZBounds() {}

optional<double> XYZBounds::getMinimumX() { return minX; }

optional<double> XYZBounds::getMaximumX() { return maxX; }

optional<double> XYZBounds::getMinimumY() { return minY; }

optional<double> XYZBounds::getMaximumY() { return maxY; }

optional<double> XYZBounds::getMinimumZ() { return minZ; }

optional<double> XYZBounds::getMaximumZ() { return maxZ; }

bool XYZBounds::isSmallestMinX(shared_ptr<PlanetModel> planetModel)
{
  if (!minX) {
    return false;
  }
  return minX - planetModel->getMinimumXValue() < Vector::MINIMUM_RESOLUTION;
}

bool XYZBounds::isLargestMaxX(shared_ptr<PlanetModel> planetModel)
{
  if (!maxX) {
    return false;
  }
  return planetModel->getMaximumXValue() - maxX < Vector::MINIMUM_RESOLUTION;
}

bool XYZBounds::isSmallestMinY(shared_ptr<PlanetModel> planetModel)
{
  if (!minY) {
    return false;
  }
  return minY - planetModel->getMinimumYValue() < Vector::MINIMUM_RESOLUTION;
}

bool XYZBounds::isLargestMaxY(shared_ptr<PlanetModel> planetModel)
{
  if (!maxY) {
    return false;
  }
  return planetModel->getMaximumYValue() - maxY < Vector::MINIMUM_RESOLUTION;
}

bool XYZBounds::isSmallestMinZ(shared_ptr<PlanetModel> planetModel)
{
  if (!minZ) {
    return false;
  }
  return minZ - planetModel->getMinimumZValue() < Vector::MINIMUM_RESOLUTION;
}

bool XYZBounds::isLargestMaxZ(shared_ptr<PlanetModel> planetModel)
{
  if (!maxZ) {
    return false;
  }
  return planetModel->getMaximumZValue() - maxZ < Vector::MINIMUM_RESOLUTION;
}

shared_ptr<Bounds> XYZBounds::addPlane(shared_ptr<PlanetModel> planetModel,
                                       shared_ptr<Plane> plane,
                                       deque<Membership> &bounds)
{
  plane->recordBounds(planetModel, shared_from_this(), bounds);
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::addHorizontalPlane(
    shared_ptr<PlanetModel> planetModel, double const latitude,
    shared_ptr<Plane> horizontalPlane, deque<Membership> &bounds)
{
  return addPlane(planetModel, horizontalPlane, {bounds});
}

shared_ptr<Bounds> XYZBounds::addVerticalPlane(
    shared_ptr<PlanetModel> planetModel, double const longitude,
    shared_ptr<Plane> verticalPlane, deque<Membership> &bounds)
{
  return addPlane(planetModel, verticalPlane, {bounds});
}

shared_ptr<Bounds> XYZBounds::addXValue(shared_ptr<GeoPoint> point)
{
  return addXValue(point->x);
}

shared_ptr<Bounds> XYZBounds::addXValue(double const x)
{
  constexpr double small = x - FUDGE_FACTOR;
  if (!minX || minX > small) {
    minX = optional<double>(small);
  }
  constexpr double large = x + FUDGE_FACTOR;
  if (!maxX || maxX < large) {
    maxX = optional<double>(large);
  }
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::addYValue(shared_ptr<GeoPoint> point)
{
  return addYValue(point->y);
}

shared_ptr<Bounds> XYZBounds::addYValue(double const y)
{
  constexpr double small = y - FUDGE_FACTOR;
  if (!minY || minY > small) {
    minY = optional<double>(small);
  }
  constexpr double large = y + FUDGE_FACTOR;
  if (!maxY || maxY < large) {
    maxY = optional<double>(large);
  }
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::addZValue(shared_ptr<GeoPoint> point)
{
  return addZValue(point->z);
}

shared_ptr<Bounds> XYZBounds::addZValue(double const z)
{
  constexpr double small = z - FUDGE_FACTOR;
  if (!minZ || minZ > small) {
    minZ = optional<double>(small);
  }
  constexpr double large = z + FUDGE_FACTOR;
  if (!maxZ || maxZ < large) {
    maxZ = optional<double>(large);
  }
  return shared_from_this();
}

shared_ptr<Bounds>
XYZBounds::addIntersection(shared_ptr<PlanetModel> planetModel,
                           shared_ptr<Plane> plane1, shared_ptr<Plane> plane2,
                           deque<Membership> &bounds)
{
  plane1->recordBounds(planetModel, shared_from_this(), plane2, {bounds});
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::addPoint(shared_ptr<GeoPoint> point)
{
  return addXValue(point)->addYValue(point)->addZValue(point);
}

shared_ptr<Bounds> XYZBounds::isWide()
{
  // No specific thing we need to do.
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::noLongitudeBound()
{
  // No specific thing we need to do.
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::noTopLatitudeBound()
{
  // No specific thing we need to do.
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::noBottomLatitudeBound()
{
  // No specific thing we need to do.
  return shared_from_this();
}

shared_ptr<Bounds> XYZBounds::noBound(shared_ptr<PlanetModel> planetModel)
{
  minX = planetModel->getMinimumXValue();
  maxX = planetModel->getMaximumXValue();
  minY = planetModel->getMinimumYValue();
  maxY = planetModel->getMaximumYValue();
  minZ = planetModel->getMinimumZValue();
  maxZ = planetModel->getMaximumZValue();
  return shared_from_this();
}

wstring XYZBounds::toString()
{
  return L"XYZBounds: [xmin=" + minX + L" xmax=" + maxX + L" ymin=" + minY +
         L" ymax=" + maxY + L" zmin=" + minZ + L" zmax=" + maxZ + L"]";
}
} // namespace org::apache::lucene::spatial3d::geom