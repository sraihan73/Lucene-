using namespace std;

#include "GeoS2Shape.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoS2Shape::GeoS2Shape(shared_ptr<PlanetModel> planetModel,
                       shared_ptr<GeoPoint> point1, shared_ptr<GeoPoint> point2,
                       shared_ptr<GeoPoint> point3, shared_ptr<GeoPoint> point4)
    : GeoBasePolygon(planetModel), point1(point1), point2(point2),
      point3(point3), point4(point4),
      plane1(make_shared<SidedPlane>(point4, point1, point2)),
      plane2(make_shared<SidedPlane>(point1, point2, point3)),
      plane3(make_shared<SidedPlane>(point2, point3, point4)),
      plane4(make_shared<SidedPlane>(point3, point4, point1)),
      plane1Points(std::deque<std::shared_ptr<GeoPoint>>{point1, point2}),
      plane2Points(std::deque<std::shared_ptr<GeoPoint>>{point2, point3}),
      plane3Points(std::deque<std::shared_ptr<GeoPoint>>{point3, point4}),
      plane4Points(std::deque<std::shared_ptr<GeoPoint>>{point4, point1}),
      edgePoints(std::deque<std::shared_ptr<GeoPoint>>{point1})
{

  // Now build the four planes

  // collect the notable points for the planes
}

GeoS2Shape::GeoS2Shape(shared_ptr<PlanetModel> planetModel,
                       shared_ptr<InputStream> inputStream) 
    : GeoS2Shape(planetModel,
                 (GeoPoint)SerializableObject::readObject(inputStream),
                 (GeoPoint)SerializableObject::readObject(inputStream),
                 (GeoPoint)SerializableObject::readObject(inputStream),
                 (GeoPoint)SerializableObject::readObject(inputStream))
{
}

void GeoS2Shape::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeObject(outputStream, point1);
  SerializableObject::writeObject(outputStream, point2);
  SerializableObject::writeObject(outputStream, point3);
  SerializableObject::writeObject(outputStream, point4);
}

bool GeoS2Shape::isWithin(double const x, double const y, double const z)
{
  return plane1->isWithin(x, y, z) && plane2->isWithin(x, y, z) &&
         plane3->isWithin(x, y, z) && plane4->isWithin(x, y, z);
}

std::deque<std::shared_ptr<GeoPoint>> GeoS2Shape::getEdgePoints()
{
  return edgePoints;
}

bool GeoS2Shape::intersects(
    shared_ptr<Plane> p, std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    deque<Membership> &bounds)
{
  return p->intersects(planetModel, plane1, notablePoints, plane1Points, bounds,
                       {plane2, plane4}) ||
         p->intersects(planetModel, plane2, notablePoints, plane2Points, bounds,
                       {plane3, plane1}) ||
         p->intersects(planetModel, plane3, notablePoints, plane3Points, bounds,
                       {plane4, plane2}) ||
         p->intersects(planetModel, plane4, notablePoints, plane4Points, bounds,
                       {plane1, plane3});
}

bool GeoS2Shape::intersects(shared_ptr<GeoShape> geoShape)
{
  return geoShape->intersects(plane1, plane1Points, {plane2, plane4}) ||
         geoShape->intersects(plane2, plane2Points, {plane3, plane1}) ||
         geoShape->intersects(plane3, plane3Points, {plane4, plane2}) ||
         geoShape->intersects(plane4, plane4Points, {plane1, plane3});
}

void GeoS2Shape::getBounds(shared_ptr<Bounds> bounds)
{
  GeoBasePolygon::getBounds(bounds);
  bounds->addPlane(planetModel, plane1, {plane2, plane4})
      ->addPlane(planetModel, plane2, {plane3, plane1})
      ->addPlane(planetModel, plane3, {plane4, plane2})
      ->addPlane(planetModel, plane4, {plane1, plane3})
      ->addPoint(point1)
      ->addPoint(point2)
      ->addPoint(point3)
      ->addPoint(point4);
}

double GeoS2Shape::outsideDistance(shared_ptr<DistanceStyle> distanceStyle,
                                   double x, double y, double z)
{
  constexpr double planeDistance1 = distanceStyle->computeDistance(
      planetModel, plane1, x, y, z, {plane2, plane4});
  constexpr double planeDistance2 = distanceStyle->computeDistance(
      planetModel, plane2, x, y, z, {plane3, plane1});
  constexpr double planeDistance3 = distanceStyle->computeDistance(
      planetModel, plane3, x, y, z, {plane4, plane2});
  constexpr double planeDistance4 = distanceStyle->computeDistance(
      planetModel, plane4, x, y, z, {plane1, plane3});

  constexpr double pointDistance1 =
      distanceStyle->computeDistance(point1, x, y, z);
  constexpr double pointDistance2 =
      distanceStyle->computeDistance(point2, x, y, z);
  constexpr double pointDistance3 =
      distanceStyle->computeDistance(point3, x, y, z);
  constexpr double pointDistance4 =
      distanceStyle->computeDistance(point4, x, y, z);

  return min(min(min(planeDistance1, planeDistance2),
                 min(planeDistance3, planeDistance4)),
             min(min(pointDistance1, pointDistance2),
                 min(pointDistance3, pointDistance4)));
}

bool GeoS2Shape::equals(any o)
{
  if (!(std::dynamic_pointer_cast<GeoS2Shape>(o) != nullptr)) {
    return false;
  }
  shared_ptr<GeoS2Shape> other = any_cast<std::shared_ptr<GeoS2Shape>>(o);
  return GeoBasePolygon::equals(other) && other->point1->equals(point1) &&
         other->point2->equals(point2) && other->point3->equals(point3) &&
         other->point4->equals(point4);
}

int GeoS2Shape::hashCode()
{
  int result = GeoBasePolygon::hashCode();
  result = 31 * result + point1->hashCode();
  result = 31 * result + point2->hashCode();
  result = 31 * result + point3->hashCode();
  result = 31 * result + point4->hashCode();
  return result;
}

wstring GeoS2Shape::toString()
{
  return L"GeoS2Shape: {planetmodel=" + planetModel + L", point1=" + point1 +
         L", point2=" + point2 + L", point3=" + point3 + L", point4=" + point4 +
         L"}";
}
} // namespace org::apache::lucene::spatial3d::geom