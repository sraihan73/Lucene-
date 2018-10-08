using namespace std;

#include "GeoPoint.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoPoint::GeoPoint(shared_ptr<PlanetModel> planetModel, double const sinLat,
                   double const sinLon, double const cosLat,
                   double const cosLon, double const lat, double const lon)
    : GeoPoint(computeDesiredEllipsoidMagnitude(planetModel, cosLat * cosLon,
                                                cosLat * sinLon, sinLat),
               cosLat * cosLon, cosLat * sinLon, sinLat, lat, lon)
{
}

GeoPoint::GeoPoint(shared_ptr<PlanetModel> planetModel, double const sinLat,
                   double const sinLon, double const cosLat,
                   double const cosLon)
    : GeoPoint(computeDesiredEllipsoidMagnitude(planetModel, cosLat * cosLon,
                                                cosLat * sinLon, sinLat),
               cosLat * cosLon, cosLat * sinLon, sinLat)
{
}

GeoPoint::GeoPoint(shared_ptr<PlanetModel> planetModel, double const lat,
                   double const lon)
    : GeoPoint(planetModel, Math::sin(lat), Math::sin(lon), Math::cos(lat),
               Math::cos(lon), lat, lon)
{
}

GeoPoint::GeoPoint(shared_ptr<PlanetModel> planetModel,
                   shared_ptr<InputStream> inputStream) 
    : GeoPoint(planetModel, SerializableObject::readDouble(inputStream),
               SerializableObject::readDouble(inputStream))
{
  // Note: this relies on left-right parameter execution order!!  Much code
  // depends on that though and it is apparently in a java spec:
  // https://stackoverflow.com/questions/2201688/order-of-execution-of-parameters-guarantees-in-java
}

GeoPoint::GeoPoint(double const magnitude, double const x, double const y,
                   double const z, double lat, double lon)
    : Vector(x * magnitude, y * magnitude, z * magnitude)
{
  this->magnitude_ = magnitude;
  if (lat > M_PI * 0.5 || lat < -M_PI * 0.5) {
    throw invalid_argument(
        L"Latitude " + to_wstring(lat) +
        L" is out of range: must range from -Math.PI/2 to Math.PI/2");
  }
  if (lon < -M_PI || lon > M_PI) {
    throw invalid_argument(
        L"Longitude " + to_wstring(lon) +
        L" is out of range: must range from -Math.PI to Math.PI");
  }
  this->latitude = lat;
  this->longitude = lon;
}

GeoPoint::GeoPoint(double const magnitude, double const x, double const y,
                   double const z)
    : Vector(x * magnitude, y * magnitude, z * magnitude)
{
  this->magnitude_ = magnitude;
}

GeoPoint::GeoPoint(double const x, double const y, double const z)
    : Vector(x, y, z)
{
}

void GeoPoint::write(shared_ptr<OutputStream> outputStream) 
{
  SerializableObject::writeDouble(outputStream, getLatitude());
  SerializableObject::writeDouble(outputStream, getLongitude());
}

double GeoPoint::arcDistance(shared_ptr<Vector> v)
{
  return Tools::safeAcos(dotProduct(v) / (magnitude() * v->magnitude()));
}

double GeoPoint::arcDistance(double const x, double const y, double const z)
{
  return Tools::safeAcos(dotProduct(x, y, z) /
                         (magnitude() * Vector::magnitude(x, y, z)));
}

double GeoPoint::getLatitude()
{
  double lat = this->latitude; // volatile-read once
  if (lat == -numeric_limits<double>::infinity()) {
    this->latitude = lat = asin(z / magnitude());
  }
  return lat;
}

double GeoPoint::getLongitude()
{
  double lon = this->longitude; // volatile-read once
  if (lon == -numeric_limits<double>::infinity()) {
    if (abs(x) < MINIMUM_RESOLUTION && abs(y) < MINIMUM_RESOLUTION) {
      this->longitude = lon = 0.0;
    } else {
      this->longitude = lon = atan2(y, x);
    }
  }
  return lon;
}

double GeoPoint::magnitude()
{
  double mag = this->magnitude_; // volatile-read once
  if (mag == -numeric_limits<double>::infinity()) {
    this->magnitude_ = mag = Vector::magnitude();
  }
  return mag;
}

bool GeoPoint::isIdentical(shared_ptr<GeoPoint> p)
{
  return isIdentical(p->x, p->y, p->z);
}

bool GeoPoint::isIdentical(double const x, double const y, double const z)
{
  return abs(this->x - x) < MINIMUM_RESOLUTION &&
         abs(this->y - y) < MINIMUM_RESOLUTION &&
         abs(this->z - z) < MINIMUM_RESOLUTION;
}

wstring GeoPoint::toString()
{
  if (this->longitude == -numeric_limits<double>::infinity()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Vector::toString();
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"[lat=" + to_wstring(getLatitude()) + L", lon=" +
         to_wstring(getLongitude()) + L"(" + Vector::toString() + L")]";
}
} // namespace org::apache::lucene::spatial3d::geom