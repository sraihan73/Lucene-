using namespace std;

#include "GeoBasePolygon.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBasePolygon::GeoBasePolygon(shared_ptr<PlanetModel> planetModel)
    : GeoBaseAreaShape(planetModel)
{
}
} // namespace org::apache::lucene::spatial3d::geom