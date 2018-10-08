using namespace std;

#include "LinearSquaredDistance.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<LinearSquaredDistance> LinearSquaredDistance::INSTANCE =
    make_shared<LinearSquaredDistance>();

LinearSquaredDistance::LinearSquaredDistance() {}

double LinearSquaredDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                              shared_ptr<GeoPoint> point2)
{
  return point1->linearDistanceSquared(point2);
}

double LinearSquaredDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                              double const x2, double const y2,
                                              double const z2)
{
  return point1->linearDistanceSquared(x2, y2, z2);
}

double LinearSquaredDistance::computeDistance(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> plane,
    shared_ptr<GeoPoint> point, deque<Membership> &bounds)
{
  return plane->linearDistanceSquared(planetModel, point, {bounds});
}

double LinearSquaredDistance::computeDistance(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> plane,
    double const x, double const y, double const z, deque<Membership> &bounds)
{
  return plane->linearDistanceSquared(planetModel, x, {y, z, bounds});
}

double LinearSquaredDistance::toAggregationForm(double const distance)
{
  return sqrt(distance);
}

double
LinearSquaredDistance::fromAggregationForm(double const aggregateDistance)
{
  return aggregateDistance * aggregateDistance;
}

std::deque<std::shared_ptr<GeoPoint>>
LinearSquaredDistance::findDistancePoints(shared_ptr<PlanetModel> planetModel,
                                          double const distanceValue,
                                          shared_ptr<GeoPoint> startPoint,
                                          shared_ptr<Plane> plane,
                                          deque<Membership> &bounds)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}

double LinearSquaredDistance::findMinimumArcDistance(
    shared_ptr<PlanetModel> planetModel, double const distanceValue)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}

double LinearSquaredDistance::findMaximumArcDistance(
    shared_ptr<PlanetModel> planetModel, double const distanceValue)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}
} // namespace org::apache::lucene::spatial3d::geom