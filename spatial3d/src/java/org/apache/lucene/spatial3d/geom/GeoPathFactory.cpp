using namespace std;

#include "GeoPathFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoPathFactory::GeoPathFactory() {}

shared_ptr<GeoPath>
GeoPathFactory::makeGeoPath(shared_ptr<PlanetModel> planetModel,
                            double const maxCutoffAngle,
                            std::deque<std::shared_ptr<GeoPoint>> &pathPoints)
{
  if (maxCutoffAngle < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    return make_shared<GeoDegeneratePath>(planetModel,
                                          filterPoints(pathPoints));
  }
  return make_shared<GeoStandardPath>(planetModel, maxCutoffAngle,
                                      filterPoints(pathPoints));
}

std::deque<std::shared_ptr<GeoPoint>>
GeoPathFactory::filterPoints(std::deque<std::shared_ptr<GeoPoint>> &pathPoints)
{
  const deque<std::shared_ptr<GeoPoint>> noIdenticalPoints =
      deque<std::shared_ptr<GeoPoint>>(pathPoints.size());
  for (int i = 0; i < pathPoints.size() - 1; i++) {
    if (!pathPoints[i]->isNumericallyIdentical(pathPoints[i + 1])) {
      noIdenticalPoints.push_back(pathPoints[i]);
    }
  }
  noIdenticalPoints.push_back(pathPoints[pathPoints.size() - 1]);
  return noIdenticalPoints.toArray(
      std::deque<std::shared_ptr<GeoPoint>>(noIdenticalPoints.size()));
}
} // namespace org::apache::lucene::spatial3d::geom