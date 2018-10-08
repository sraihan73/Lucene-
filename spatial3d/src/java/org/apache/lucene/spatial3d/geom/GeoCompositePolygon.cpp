using namespace std;

#include "GeoCompositePolygon.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoCompositePolygon::GeoCompositePolygon(shared_ptr<PlanetModel> planetModel)
    : GeoBaseCompositeAreaShape<GeoPolygon>(planetModel)
{
}

GeoCompositePolygon::GeoCompositePolygon(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoBaseCompositeAreaShape<GeoPolygon>(planetModel, inputStream,
                                            GeoPolygon::class)
{
}

bool GeoCompositePolygon::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoCompositePolygon>(o) != nullptr)) {
    return false;
  }
  return GeoBaseCompositeAreaShape<GeoPolygon>::equals(o);
}

wstring GeoCompositePolygon::toString()
{
  return L"GeoCompositePolygon: {" + shapes + L'}';
}
} // namespace org::apache::lucene::spatial3d::geom