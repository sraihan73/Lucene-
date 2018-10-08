using namespace std;

#include "GeoBaseBBox.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBaseBBox::GeoBaseBBox(shared_ptr<PlanetModel> planetModel)
    : GeoBaseAreaShape(planetModel)
{
}
} // namespace org::apache::lucene::spatial3d::geom