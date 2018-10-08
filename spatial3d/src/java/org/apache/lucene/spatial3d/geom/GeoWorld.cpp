using namespace std;

#include "GeoWorld.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const GeoWorld::edgePoints =
    std::deque<std::shared_ptr<GeoPoint>>(0);

GeoWorld::GeoWorld(shared_ptr<PlanetModel> planetModel)
    : GeoBaseBBox(planetModel),
      originPoint(make_shared<GeoPoint>(planetModel->ab, 1.0, 0.0, 0.0))
{
}

GeoWorld::GeoWorld(shared_ptr<PlanetModel> planetModel,
                   shared_ptr<InputStream> inputStream) 
    : GeoWorld(planetModel)
{
}

void GeoWorld::write(shared_ptr<OutputStream> outputStream) 
{
  // Nothing needed
}

shared_ptr<GeoBBox> GeoWorld::expand(double const angle)
{
  return shared_from_this();
}

double GeoWorld::getRadius() { return M_PI; }

shared_ptr<GeoPoint> GeoWorld::getCenter()
{
  // Totally arbitrary
  return originPoint;
}

bool GeoWorld::isWithin(double const x, double const y, double const z)
{
  return true;
}

std::deque<std::shared_ptr<GeoPoint>> GeoWorld::getEdgePoints()
{
  return edgePoints;
}

bool GeoWorld::intersects(shared_ptr<Plane> p,
                          std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                          deque<Membership> &bounds)
{
  return false;
}

bool GeoWorld::intersects(shared_ptr<GeoShape> geoShape) { return false; }

void GeoWorld::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBaseBBox::getBounds(bounds);
  // Unnecessary
  // bounds.noLongitudeBound().noTopLatitudeBound().noBottomLatitudeBound();
}

int GeoWorld::getRelationship(shared_ptr<GeoShape> path)
{
  if (path->getEdgePoints().size() > 0) {
    // Path is always within the world
    return WITHIN;
  }

  return OVERLAPS;
}

double GeoWorld::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                 double const x, double const y, double const z)
{
  return 0.0;
}

bool GeoWorld::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoWorld>(o) != nullptr)) {
    return false;
  }
  return GeoBaseBBox::equals(o);
}

int GeoWorld::hashCode() { return GeoBaseBBox::hashCode(); }

wstring GeoWorld::toString()
{
  return L"GeoWorld: {planetmodel=" + planetModel + L"}";
}
} // namespace org::apache::lucene::spatial3d::geom