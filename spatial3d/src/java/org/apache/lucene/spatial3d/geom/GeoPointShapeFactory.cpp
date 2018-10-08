using namespace std;

#include "GeoPointShapeFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoPointShapeFactory::GeoPointShapeFactory() {}

shared_ptr<GeoPointShape>
GeoPointShapeFactory::makeGeoPointShape(shared_ptr<PlanetModel> planetModel,
                                        double const lat, double const lon)
{
  return make_shared<GeoDegeneratePoint>(planetModel, lat, lon);
}
} // namespace org::apache::lucene::spatial3d::geom