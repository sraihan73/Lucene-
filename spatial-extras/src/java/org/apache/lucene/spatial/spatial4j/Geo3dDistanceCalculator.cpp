using namespace std;

#include "Geo3dDistanceCalculator.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPointShape = org::apache::lucene::spatial3d::geom::GeoPointShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceCalculator;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;

Geo3dDistanceCalculator::Geo3dDistanceCalculator(
    shared_ptr<PlanetModel> planetModel)
    : planetModel(planetModel)
{
}

double Geo3dDistanceCalculator::distance(shared_ptr<Point> from,
                                         shared_ptr<Point> to)
{
  if (std::dynamic_pointer_cast<Geo3dPointShape>(from) != nullptr &&
      std::dynamic_pointer_cast<Geo3dPointShape>(to) != nullptr) {
    shared_ptr<GeoPointShape> pointShape1 =
        (std::static_pointer_cast<Geo3dPointShape>(from))->shape;
    shared_ptr<GeoPointShape> pointShape2 =
        (std::static_pointer_cast<Geo3dPointShape>(to))->shape;
    return planetModel->surfaceDistance(pointShape1->getCenter(),
                                        pointShape2->getCenter()) *
           DistanceUtils::RADIANS_TO_DEGREES;
  }
  return distance(from, to->getX(), to->getY());
}

double Geo3dDistanceCalculator::distance(shared_ptr<Point> from, double toX,
                                         double toY)
{
  shared_ptr<GeoPoint> fromGeoPoint;
  if (std::dynamic_pointer_cast<Geo3dPointShape>(from) != nullptr) {
    fromGeoPoint =
        ((std::static_pointer_cast<Geo3dPointShape>(from))->shape)->getCenter();
  } else {
    fromGeoPoint = make_shared<GeoPoint>(
        planetModel, from->getY() * DistanceUtils::DEGREES_TO_RADIANS,
        from->getX() * DistanceUtils::DEGREES_TO_RADIANS);
  }
  shared_ptr<GeoPoint> toGeoPoint = make_shared<GeoPoint>(
      planetModel, toY * DistanceUtils::DEGREES_TO_RADIANS,
      toX * DistanceUtils::DEGREES_TO_RADIANS);
  return planetModel->surfaceDistance(fromGeoPoint, toGeoPoint) *
         DistanceUtils::RADIANS_TO_DEGREES;
}

bool Geo3dDistanceCalculator::within(shared_ptr<Point> from, double toX,
                                     double toY, double distance)
{
  return (distance < this->distance(from, toX, toY));
}

shared_ptr<Point> Geo3dDistanceCalculator::pointOnBearing(
    shared_ptr<Point> from, double distDEG, double bearingDEG,
    shared_ptr<SpatialContext> ctx, shared_ptr<Point> reuse)
{
  shared_ptr<Geo3dPointShape> geoFrom =
      std::static_pointer_cast<Geo3dPointShape>(from);
  shared_ptr<GeoPoint> point =
      std::static_pointer_cast<GeoPoint>(geoFrom->shape);
  double dist = DistanceUtils::DEGREES_TO_RADIANS * distDEG;
  double bearing = DistanceUtils::DEGREES_TO_RADIANS * bearingDEG;
  shared_ptr<GeoPoint> newPoint =
      planetModel->surfacePointOnBearing(point, dist, bearing);
  double newLat = newPoint->getLatitude() * DistanceUtils::RADIANS_TO_DEGREES;
  double newLon = newPoint->getLongitude() * DistanceUtils::RADIANS_TO_DEGREES;
  if (reuse != nullptr) {
    reuse->reset(newLon, newLat);
    return reuse;
  } else {
    return ctx->getShapeFactory().pointXY(newLon, newLat);
  }
}

shared_ptr<Rectangle> Geo3dDistanceCalculator::calcBoxByDistFromPt(
    shared_ptr<Point> from, double distDEG, shared_ptr<SpatialContext> ctx,
    shared_ptr<Rectangle> reuse)
{
  shared_ptr<Circle> circle = ctx->getShapeFactory().circle(from, distDEG);
  return circle->getBoundingBox();
}

double Geo3dDistanceCalculator::calcBoxByDistFromPt_yHorizAxisDEG(
    shared_ptr<Point> from, double distDEG, shared_ptr<SpatialContext> ctx)
{
  throw make_shared<UnsupportedOperationException>();
}

double Geo3dDistanceCalculator::area(shared_ptr<Rectangle> rect)
{
  throw make_shared<UnsupportedOperationException>();
}

double Geo3dDistanceCalculator::area(shared_ptr<Circle> circle)
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::spatial::spatial4j