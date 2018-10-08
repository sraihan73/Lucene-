using namespace std;

#include "GeoBaseMembershipShape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBaseMembershipShape::GeoBaseMembershipShape(
    shared_ptr<PlanetModel> planetModel)
    : GeoBaseShape(planetModel)
{
}

bool GeoBaseMembershipShape::isWithin(shared_ptr<Vector> point)
{
  return isWithin(point->x, point->y, point->z);
}

double GeoBaseMembershipShape::computeOutsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, shared_ptr<GeoPoint> point)
{
  return computeOutsideDistance(distanceStyle, point->x, point->y, point->z);
}

double GeoBaseMembershipShape::computeOutsideDistance(
    shared_ptr<DistanceStyle> distanceStyle, double const x, double const y,
    double const z)
{
  if (isWithin(x, y, z)) {
    return 0.0;
  }
  return outsideDistance(distanceStyle, x, y, z);
}
} // namespace org::apache::lucene::spatial3d::geom