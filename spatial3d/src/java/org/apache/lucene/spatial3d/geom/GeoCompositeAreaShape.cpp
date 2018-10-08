using namespace std;

#include "GeoCompositeAreaShape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoCompositeAreaShape::GeoCompositeAreaShape(
    shared_ptr<PlanetModel> planetModel)
    : GeoBaseCompositeAreaShape<GeoAreaShape>(planetModel)
{
}

GeoCompositeAreaShape::GeoCompositeAreaShape(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoBaseCompositeAreaShape<GeoAreaShape>(planetModel, inputStream,
                                              GeoAreaShape::class)
{
}

bool GeoCompositeAreaShape::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoCompositeAreaShape>(o) != nullptr)) {
    return false;
  }
  return GeoBaseCompositeAreaShape<GeoAreaShape>::equals(o);
}

wstring GeoCompositeAreaShape::toString()
{
  return L"GeoCompositeAreaShape: {" + shapes + L'}';
}
} // namespace org::apache::lucene::spatial3d::geom