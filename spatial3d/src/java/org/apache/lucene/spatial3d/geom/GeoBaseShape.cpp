using namespace std;

#include "GeoBaseShape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBaseShape::GeoBaseShape(shared_ptr<PlanetModel> planetModel)
    : BasePlanetObject(planetModel)
{
}

void GeoBaseShape::getBounds(shared_ptr<Bounds> bounds)
{
  if (isWithin(planetModel->NORTH_POLE)) {
    bounds->noTopLatitudeBound()->noLongitudeBound()->addPoint(
        planetModel->NORTH_POLE);
  }
  if (isWithin(planetModel->SOUTH_POLE)) {
    bounds->noBottomLatitudeBound()->noLongitudeBound()->addPoint(
        planetModel->SOUTH_POLE);
  }
  if (isWithin(planetModel->MIN_X_POLE)) {
    bounds->addPoint(planetModel->MIN_X_POLE);
  }
  if (isWithin(planetModel->MAX_X_POLE)) {
    bounds->addPoint(planetModel->MAX_X_POLE);
  }
  if (isWithin(planetModel->MIN_Y_POLE)) {
    bounds->addPoint(planetModel->MIN_Y_POLE);
  }
  if (isWithin(planetModel->MAX_Y_POLE)) {
    bounds->addPoint(planetModel->MAX_Y_POLE);
  }
}
} // namespace org::apache::lucene::spatial3d::geom