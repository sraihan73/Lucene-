using namespace std;

#include "GeoBaseDistanceShape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBaseDistanceShape::GeoBaseDistanceShape(shared_ptr<PlanetModel> planetModel)
    : GeoBaseAreaShape(planetModel)
{
}

bool GeoBaseDistanceShape::isWithin(shared_ptr<Vector> point)
{
  return isWithin(point->x, point->y, point->z);
}

double
GeoBaseDistanceShape::computeDistance(shared_ptr<DistanceStyle> distanceStyle,
                                      shared_ptr<GeoPoint> point)
{
  return computeDistance(distanceStyle, point->x, point->y, point->z);
}

double
GeoBaseDistanceShape::computeDistance(shared_ptr<DistanceStyle> distanceStyle,
                                      double const x, double const y,
                                      double const z)
{
  if (!isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  return distance(distanceStyle, x, y, z);
}

double GeoBaseDistanceShape::computeDeltaDistance(
    shared_ptr<DistanceStyle> distanceStyle, shared_ptr<GeoPoint> point)
{
  return computeDeltaDistance(distanceStyle, point->x, point->y, point->z);
}

double GeoBaseDistanceShape::computeDeltaDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  if (!isWithin(x, y, z)) {
    return numeric_limits<double>::infinity();
  }
  return deltaDistance(distanceStyle, x, y, z);
}

double
GeoBaseDistanceShape::deltaDistance(shared_ptr<DistanceStyle> distanceStyle,
                                    double const x, double const y,
                                    double const z)
{
  return distance(distanceStyle, x, y, z) * 2.0;
}

void GeoBaseDistanceShape::getDistanceBounds(
    shared_ptr<Bounds> bounds, shared_ptr<DistanceStyle> distanceStyle,
    double const distanceValue)
{
  if (distanceValue == numeric_limits<double>::infinity()) {
    getBounds(bounds);
    return;
  }
  distanceBounds(bounds, distanceStyle, distanceValue);
}
} // namespace org::apache::lucene::spatial3d::geom