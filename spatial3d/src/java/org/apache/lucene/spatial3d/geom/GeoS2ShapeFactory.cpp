using namespace std;

#include "GeoS2ShapeFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoS2ShapeFactory::GeoS2ShapeFactory() {}

shared_ptr<GeoPolygon> GeoS2ShapeFactory::makeGeoS2Shape(
    shared_ptr<PlanetModel> planetModel, shared_ptr<GeoPoint> point1,
    shared_ptr<GeoPoint> point2, shared_ptr<GeoPoint> point3,
    shared_ptr<GeoPoint> point4)
{
  return make_shared<GeoS2Shape>(planetModel, point1, point2, point3, point4);
}
} // namespace org::apache::lucene::spatial3d::geom