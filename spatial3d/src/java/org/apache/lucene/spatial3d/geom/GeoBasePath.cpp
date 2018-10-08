using namespace std;

#include "GeoBasePath.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBasePath::GeoBasePath(shared_ptr<PlanetModel> planetModel)
    : GeoBaseDistanceShape(planetModel)
{
}
} // namespace org::apache::lucene::spatial3d::geom