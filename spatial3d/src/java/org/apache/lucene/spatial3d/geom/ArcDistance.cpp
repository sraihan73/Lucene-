using namespace std;

#include "ArcDistance.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<ArcDistance> ArcDistance::INSTANCE =
    make_shared<ArcDistance>();

ArcDistance::ArcDistance() {}

double ArcDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                    shared_ptr<GeoPoint> point2)
{
  return point1->arcDistance(point2);
}

double ArcDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                    double const x2, double const y2,
                                    double const z2)
{
  return point1->arcDistance(x2, y2, z2);
}

double ArcDistance::computeDistance(shared_ptr<PlanetModel> planetModel,
                                    shared_ptr<Plane> plane,
                                    shared_ptr<GeoPoint> point,
                                    deque<Membership> &bounds)
{
  return plane->arcDistance(planetModel, point, {bounds});
}

double ArcDistance::computeDistance(shared_ptr<PlanetModel> planetModel,
                                    shared_ptr<Plane> plane, double const x,
                                    double const y, double const z,
                                    deque<Membership> &bounds)
{
  return plane->arcDistance(planetModel, x, {y, z, bounds});
}

std::deque<std::shared_ptr<GeoPoint>> ArcDistance::findDistancePoints(
    shared_ptr<PlanetModel> planetModel, double const distanceValue,
    shared_ptr<GeoPoint> startPoint, shared_ptr<Plane> plane,
    deque<Membership> &bounds)
{
  return plane->findArcDistancePoints(planetModel, distanceValue, startPoint,
                                      {bounds});
}

double ArcDistance::findMinimumArcDistance(shared_ptr<PlanetModel> planetModel,
                                           double const distanceValue)
{
  return distanceValue;
}

double ArcDistance::findMaximumArcDistance(shared_ptr<PlanetModel> planetModel,
                                           double const distanceValue)
{
  return distanceValue;
}
} // namespace org::apache::lucene::spatial3d::geom