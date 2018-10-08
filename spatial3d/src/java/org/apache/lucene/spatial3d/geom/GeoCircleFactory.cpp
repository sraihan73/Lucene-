using namespace std;

#include "GeoCircleFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoCircleFactory::GeoCircleFactory() {}

shared_ptr<GeoCircle>
GeoCircleFactory::makeGeoCircle(shared_ptr<PlanetModel> planetModel,
                                double const latitude, double const longitude,
                                double const cutoffAngle)
{
  if (cutoffAngle < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    return make_shared<GeoDegeneratePoint>(planetModel, latitude, longitude);
  }
  return make_shared<GeoStandardCircle>(planetModel, latitude, longitude,
                                        cutoffAngle);
}

shared_ptr<GeoCircle> GeoCircleFactory::makeExactGeoCircle(
    shared_ptr<PlanetModel> planetModel, double const latitude,
    double const longitude, double const radius, double const accuracy)
{
  if (radius < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    return make_shared<GeoDegeneratePoint>(planetModel, latitude, longitude);
  }
  return make_shared<GeoExactCircle>(planetModel, latitude, longitude, radius,
                                     accuracy);
}
} // namespace org::apache::lucene::spatial3d::geom