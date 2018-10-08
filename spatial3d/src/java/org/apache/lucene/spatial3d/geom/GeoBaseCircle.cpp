using namespace std;

#include "GeoBaseCircle.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBaseCircle::GeoBaseCircle(shared_ptr<PlanetModel> planetModel)
    : GeoBaseDistanceShape(planetModel)
{
}
} // namespace org::apache::lucene::spatial3d::geom