using namespace std;

#include "GeoCompositeMembershipShape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoCompositeMembershipShape::GeoCompositeMembershipShape(
    shared_ptr<PlanetModel> planetModel)
    : GeoBaseCompositeMembershipShape<GeoMembershipShape>(planetModel)
{
}

GeoCompositeMembershipShape::GeoCompositeMembershipShape(
    shared_ptr<PlanetModel> planetModel,
    shared_ptr<InputStream> inputStream) 
    : GeoBaseCompositeMembershipShape<GeoMembershipShape>(
          planetModel, inputStream, GeoMembershipShape::class)
{
}

bool GeoCompositeMembershipShape::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoCompositeMembershipShape>(o) != nullptr)) {
    return false;
  }
  return GeoBaseCompositeMembershipShape<GeoMembershipShape>::equals(o);
}

wstring GeoCompositeMembershipShape::toString()
{
  return L"GeoCompositeMembershipShape: {" + shapes + L'}';
}
} // namespace org::apache::lucene::spatial3d::geom