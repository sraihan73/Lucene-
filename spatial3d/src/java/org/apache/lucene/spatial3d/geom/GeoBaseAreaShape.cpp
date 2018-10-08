using namespace std;

#include "GeoBaseAreaShape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBaseAreaShape::GeoBaseAreaShape(shared_ptr<PlanetModel> planetModel)
    : GeoBaseMembershipShape(planetModel)
{
}

int GeoBaseAreaShape::isShapeInsideGeoAreaShape(shared_ptr<GeoShape> geoShape)
{
  bool foundOutside = false;
  bool foundInside = false;
  for (auto p : geoShape->getEdgePoints()) {
    if (isWithin(p)) {
      foundInside = true;
    } else {
      foundOutside = true;
    }
    if (foundInside && foundOutside) {
      return SOME_INSIDE;
    }
  }
  if (!foundInside && !foundOutside) {
    return NONE_INSIDE;
  }
  if (foundInside && !foundOutside) {
    return ALL_INSIDE;
  }
  if (foundOutside && !foundInside) {
    return NONE_INSIDE;
  }
  return SOME_INSIDE;
}

int GeoBaseAreaShape::isGeoAreaShapeInsideShape(shared_ptr<GeoShape> geoshape)
{
  bool foundOutside = false;
  bool foundInside = false;
  for (shared_ptr<GeoPoint> p : getEdgePoints()) {
    if (geoshape->isWithin(p)) {
      foundInside = true;
    } else {
      foundOutside = true;
    }
    if (foundInside && foundOutside) {
      return SOME_INSIDE;
    }
  }
  if (!foundInside && !foundOutside) {
    return NONE_INSIDE;
  }
  if (foundInside && !foundOutside) {
    return ALL_INSIDE;
  }
  if (foundOutside && !foundInside) {
    return NONE_INSIDE;
  }
  return SOME_INSIDE;
}

int GeoBaseAreaShape::getRelationship(shared_ptr<GeoShape> geoShape)
{
  if (!geoShape->getPlanetModel()->equals(planetModel)) {
    throw invalid_argument(
        L"Cannot relate shapes with different planet models.");
  }
  constexpr int insideGeoAreaShape = isShapeInsideGeoAreaShape(geoShape);
  if (insideGeoAreaShape == SOME_INSIDE) {
    return GeoArea::OVERLAPS;
  }

  constexpr int insideShape = isGeoAreaShapeInsideShape(geoShape);
  if (insideShape == SOME_INSIDE) {
    return GeoArea::OVERLAPS;
  }

  if (insideGeoAreaShape == ALL_INSIDE && insideShape == ALL_INSIDE) {
    return GeoArea::OVERLAPS;
  }

  if (intersects(geoShape)) {
    return GeoArea::OVERLAPS;
  }

  if (insideGeoAreaShape == ALL_INSIDE) {
    return GeoArea::WITHIN;
  }

  if (insideShape == ALL_INSIDE) {
    return GeoArea::CONTAINS;
  }

  return GeoArea::DISJOINT;
}
} // namespace org::apache::lucene::spatial3d::geom