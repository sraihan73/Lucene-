using namespace std;

#include "GeoAreaFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoAreaFactory::GeoAreaFactory() {}

shared_ptr<GeoArea>
GeoAreaFactory::makeGeoArea(shared_ptr<PlanetModel> planetModel,
                            double const topLat, double const bottomLat,
                            double const leftLon, double const rightLon)
{
  return GeoBBoxFactory::makeGeoBBox(planetModel, topLat, bottomLat, leftLon,
                                     rightLon);
}

shared_ptr<GeoArea> GeoAreaFactory::makeGeoArea(
    shared_ptr<PlanetModel> planetModel, double const minX, double const maxX,
    double const minY, double const maxY, double const minZ, double const maxZ)
{
  return XYZSolidFactory::makeXYZSolid(planetModel, minX, maxX, minY, maxY,
                                       minZ, maxZ);
}
} // namespace org::apache::lucene::spatial3d::geom