using namespace std;

#include "StandardObjects.h"

namespace org::apache::lucene::spatial3d::geom
{

unordered_map<type_info, int> StandardObjects::classRegsitry =
    unordered_map<type_info, int>();
unordered_map<int, type_info> StandardObjects::codeRegsitry =
    unordered_map<int, type_info>();

StandardObjects::StaticConstructor::StaticConstructor()
{
  classRegsitry.emplace(GeoPoint::typeid, 0);
  classRegsitry.emplace(GeoRectangle::typeid, 1);
  classRegsitry.emplace(GeoStandardCircle::typeid, 2);
  classRegsitry.emplace(GeoStandardPath::typeid, 3);
  classRegsitry.emplace(GeoConvexPolygon::typeid, 4);
  classRegsitry.emplace(GeoConcavePolygon::typeid, 5);
  classRegsitry.emplace(GeoComplexPolygon::typeid, 6);
  classRegsitry.emplace(GeoCompositePolygon::typeid, 7);
  classRegsitry.emplace(GeoCompositeMembershipShape::typeid, 8);
  classRegsitry.emplace(GeoCompositeAreaShape::typeid, 9);
  classRegsitry.emplace(GeoDegeneratePoint::typeid, 10);
  classRegsitry.emplace(GeoDegenerateHorizontalLine::typeid, 11);
  classRegsitry.emplace(GeoDegenerateLatitudeZone::typeid, 12);
  classRegsitry.emplace(GeoDegenerateLongitudeSlice::typeid, 13);
  classRegsitry.emplace(GeoDegenerateVerticalLine::typeid, 14);
  classRegsitry.emplace(GeoLatitudeZone::typeid, 15);
  classRegsitry.emplace(GeoLongitudeSlice::typeid, 16);
  classRegsitry.emplace(GeoNorthLatitudeZone::typeid, 17);
  classRegsitry.emplace(GeoNorthRectangle::typeid, 18);
  classRegsitry.emplace(GeoSouthLatitudeZone::typeid, 19);
  classRegsitry.emplace(GeoSouthRectangle::typeid, 20);
  classRegsitry.emplace(GeoWideDegenerateHorizontalLine::typeid, 21);
  classRegsitry.emplace(GeoWideLongitudeSlice::typeid, 22);
  classRegsitry.emplace(GeoWideNorthRectangle::typeid, 23);
  classRegsitry.emplace(GeoWideRectangle::typeid, 24);
  classRegsitry.emplace(GeoWideSouthRectangle::typeid, 25);
  classRegsitry.emplace(GeoWorld::typeid, 26);
  classRegsitry.emplace(dXdYdZSolid::typeid, 27);
  classRegsitry.emplace(dXdYZSolid::typeid, 28);
  classRegsitry.emplace(dXYdZSolid::typeid, 29);
  classRegsitry.emplace(dXYZSolid::typeid, 30);
  classRegsitry.emplace(XdYdZSolid::typeid, 31);
  classRegsitry.emplace(XdYZSolid::typeid, 32);
  classRegsitry.emplace(XYdZSolid::typeid, 33);
  classRegsitry.emplace(StandardXYZSolid::typeid, 34);
  classRegsitry.emplace(PlanetModel::typeid, 35);
  classRegsitry.emplace(GeoDegeneratePath::typeid, 36);
  classRegsitry.emplace(GeoExactCircle::typeid, 37);
  classRegsitry.emplace(GeoS2Shape::typeid, 38);

  for (auto clazz : classRegsitry) {
    codeRegsitry.emplace(classRegsitry[clazz.first], clazz.first);
  }
}

StandardObjects::StaticConstructor StandardObjects::staticConstructor;
} // namespace org::apache::lucene::spatial3d::geom