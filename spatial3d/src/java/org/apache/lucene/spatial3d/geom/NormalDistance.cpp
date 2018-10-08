using namespace std;

#include "NormalDistance.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<NormalDistance> NormalDistance::INSTANCE =
    make_shared<NormalDistance>();

NormalDistance::NormalDistance() {}

double NormalDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                       shared_ptr<GeoPoint> point2)
{
  return point1->normalDistance(point2);
}

double NormalDistance::computeDistance(shared_ptr<GeoPoint> point1,
                                       double const x2, double const y2,
                                       double const z2)
{
  return point1->normalDistance(x2, y2, z2);
}

double NormalDistance::computeDistance(shared_ptr<PlanetModel> planetModel,
                                       shared_ptr<Plane> plane,
                                       shared_ptr<GeoPoint> point,
                                       deque<Membership> &bounds)
{
  return plane->normalDistance(point, {bounds});
}

double NormalDistance::computeDistance(shared_ptr<PlanetModel> planetModel,
                                       shared_ptr<Plane> plane, double const x,
                                       double const y, double const z,
                                       deque<Membership> &bounds)
{
  return plane->normalDistance(x, {y, z, bounds});
}

std::deque<std::shared_ptr<GeoPoint>> NormalDistance::findDistancePoints(
    shared_ptr<PlanetModel> planetModel, double const distanceValue,
    shared_ptr<GeoPoint> startPoint, shared_ptr<Plane> plane,
    deque<Membership> &bounds)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}

double
NormalDistance::findMinimumArcDistance(shared_ptr<PlanetModel> planetModel,
                                       double const distanceValue)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}

double
NormalDistance::findMaximumArcDistance(shared_ptr<PlanetModel> planetModel,
                                       double const distanceValue)
{
  throw make_shared<IllegalStateException>(
      L"Reverse mapping not implemented for this distance metric");
}
} // namespace org::apache::lucene::spatial3d::geom