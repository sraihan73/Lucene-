using namespace std;

#include "NormalSquaredDistance.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<NormalSquaredDistance> NormalSquaredDistance::INSTANCE =
    make_shared<NormalSquaredDistance>();

NormalSquaredDistance::NormalSquaredDistance() {}

double NormalSquaredDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                              shared_ptr<GeoPoint> point2)
{
  return point1->normalDistanceSquared(point2);
}

double NormalSquaredDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                              double const x2, double const y2,
                                              double const z2)
{
  return point1->normalDistanceSquared(x2, y2, z2);
}

double NormalSquaredDistance::computeDistance(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> plane,
    shared_ptr<GeoPoint> point, deque<Membership> &bounds)
{
  return plane->normalDistanceSquared(point, {bounds});
}

double NormalSquaredDistance::computeDistance(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> plane,
    double const x, double const y, double const z, deque<Membership> &bounds)
{
  return plane->normalDistanceSquared(x, {y, z, bounds});
}

double NormalSquaredDistance::toAggregationForm(double const distance)
{
  return sqrt(distance);
}

double
NormalSquaredDistance::fromAggregationForm(double const aggregateDistance)
{
  return aggregateDistance * aggregateDistance;
}

std::deque<std::shared_ptr<GeoPoint>>
NormalSquaredDistance::findDistancePoints(shared_ptr<PlanetModel> planetModel,
                                          double const distanceValue,
                                          shared_ptr<GeoPoint> startPoint,
                                          shared_ptr<Plane> plane,
                                          deque<Membership> &bounds)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}

double NormalSquaredDistance::findMinimumArcDistance(
    shared_ptr<PlanetModel> planetModel, double const distanceValue)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}

double NormalSquaredDistance::findMaximumArcDistance(
    shared_ptr<PlanetModel> planetModel, double const distanceValue)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}
} // namespace org::apache::lucene::spatial3d::geom