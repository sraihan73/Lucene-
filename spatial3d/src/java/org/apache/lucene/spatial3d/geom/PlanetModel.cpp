using namespace std;

#include "PlanetModel.h"

namespace org::apache::lucene::spatial3d::geom
{

const shared_ptr<PlanetModel> PlanetModel::SPHERE =
    make_shared<PlanetModel>(1.0, 1.0);
const shared_ptr<PlanetModel> PlanetModel::WGS84 = make_shared<PlanetModel>(
    WGS84_EQUATORIAL / WGS84_MEAN, WGS84_POLAR / WGS84_MEAN);

PlanetModel::PlanetModel(double const ab, double const c)
    : ab(ab), c(c), inverseAb(1.0 / ab), inverseC(1.0 / c),
      inverseAbSquared(inverseAb * inverseAb),
      inverseCSquared(inverseC * inverseC), flattening((ab - c) * inverseAb),
      squareRatio((ab * ab - c * c) / (c * c)), scale((2.0 * ab + c) / 3.0),
      inverseScale(1.0 / scale),
      NORTH_POLE(make_shared<GeoPoint>(c, 0.0, 0.0, 1.0, M_PI * 0.5, 0.0)),
      SOUTH_POLE(make_shared<GeoPoint>(c, 0.0, 0.0, -1.0, -M_PI * 0.5, 0.0)),
      MIN_X_POLE(make_shared<GeoPoint>(ab, -1.0, 0.0, 0.0, 0.0, -M_PI)),
      MAX_X_POLE(make_shared<GeoPoint>(ab, 1.0, 0.0, 0.0, 0.0, 0.0)),
      MIN_Y_POLE(make_shared<GeoPoint>(ab, 0.0, -1.0, 0.0, 0.0, -M_PI * 0.5)),
      MAX_Y_POLE(make_shared<GeoPoint>(ab, 0.0, 1.0, 0.0, 0.0, M_PI * 0.5)),
      minimumPoleDistance(min(surfaceDistance(NORTH_POLE, SOUTH_POLE),
                              surfaceDistance(MIN_X_POLE, MAX_X_POLE)))
{
}

PlanetModel::PlanetModel(shared_ptr<InputStream> inputStream) 
    : PlanetModel(SerializableObject::readDouble(inputStream),
                  SerializableObject::readDouble(inputStream))
{
}

void PlanetModel::write(shared_ptr<OutputStream> outputStream) throw(
    IOException)
{
  SerializableObject::writeDouble(outputStream, ab);
  SerializableObject::writeDouble(outputStream, c);
}

bool PlanetModel::isSphere() { return this->ab == this->c; }

double PlanetModel::getMinimumMagnitude() { return min(this->ab, this->c); }

double PlanetModel::getMaximumMagnitude() { return max(this->ab, this->c); }

double PlanetModel::getMinimumXValue() { return -this->ab; }

double PlanetModel::getMaximumXValue() { return this->ab; }

double PlanetModel::getMinimumYValue() { return -this->ab; }

double PlanetModel::getMaximumYValue() { return this->ab; }

double PlanetModel::getMinimumZValue() { return -this->c; }

double PlanetModel::getMaximumZValue() { return this->c; }

bool PlanetModel::pointOnSurface(shared_ptr<Vector> v)
{
  return pointOnSurface(v->x, v->y, v->z);
}

bool PlanetModel::pointOnSurface(double const x, double const y, double const z)
{
  // Equation of planet surface is:
  // x^2 / a^2 + y^2 / b^2 + z^2 / c^2 - 1 = 0
  return abs(x * x * inverseAb * inverseAb + y * y * inverseAb * inverseAb +
             z * z * inverseC * inverseC - 1.0) < Vector::MINIMUM_RESOLUTION;
}

bool PlanetModel::pointOutside(shared_ptr<Vector> v)
{
  return pointOutside(v->x, v->y, v->z);
}

bool PlanetModel::pointOutside(double const x, double const y, double const z)
{
  // Equation of planet surface is:
  // x^2 / a^2 + y^2 / b^2 + z^2 / c^2 - 1 = 0
  return (x * x + y * y) * inverseAb * inverseAb + z * z * inverseC * inverseC -
             1.0 >
         Vector::MINIMUM_RESOLUTION;
}

shared_ptr<GeoPoint> PlanetModel::createSurfacePoint(shared_ptr<Vector> deque)
{
  return createSurfacePoint(deque->x, deque->y, deque->z);
}

shared_ptr<GeoPoint>
PlanetModel::createSurfacePoint(double const x, double const y, double const z)
{
  // The equation of the surface is:
  // (x^2 / a^2 + y^2 / b^2 + z^2 / c^2) = 1
  // We will need to scale the passed-in x, y, z values:
  // ((tx)^2 / a^2 + (ty)^2 / b^2 + (tz)^2 / c^2) = 1
  // t^2 * (x^2 / a^2 + y^2 / b^2 + z^2 / c^2)  = 1
  // t = sqrt ( 1 / (x^2 / a^2 + y^2 / b^2 + z^2 / c^2))
  constexpr double t =
      sqrt(1.0 / (x * x * inverseAbSquared + y * y * inverseAbSquared +
                  z * z * inverseCSquared));
  return make_shared<GeoPoint>(t * x, t * y, t * z);
}

shared_ptr<GeoPoint> PlanetModel::bisection(shared_ptr<GeoPoint> pt1,
                                            shared_ptr<GeoPoint> pt2)
{
  constexpr double A0 = (pt1->x + pt2->x) * 0.5;
  constexpr double B0 = (pt1->y + pt2->y) * 0.5;
  constexpr double C0 = (pt1->z + pt2->z) * 0.5;

  constexpr double denom = inverseAbSquared * A0 * A0 +
                           inverseAbSquared * B0 * B0 +
                           inverseCSquared * C0 * C0;

  if (denom < Vector::MINIMUM_RESOLUTION) {
    // Bisection is undefined
    return nullptr;
  }

  constexpr double t = sqrt(1.0 / denom);

  return make_shared<GeoPoint>(t * A0, t * B0, t * C0);
}

double PlanetModel::surfaceDistance(shared_ptr<GeoPoint> pt1,
                                    shared_ptr<GeoPoint> pt2)
{
  constexpr double L = pt2->getLongitude() - pt1->getLongitude();
  constexpr double U1 = atan((1.0 - flattening) * tan(pt1->getLatitude()));
  constexpr double U2 = atan((1.0 - flattening) * tan(pt2->getLatitude()));

  constexpr double sinU1 = sin(U1);
  constexpr double cosU1 = cos(U1);
  constexpr double sinU2 = sin(U2);
  constexpr double cosU2 = cos(U2);

  constexpr double dCosU1CosU2 = cosU1 * cosU2;
  constexpr double dCosU1SinU2 = cosU1 * sinU2;

  constexpr double dSinU1SinU2 = sinU1 * sinU2;
  constexpr double dSinU1CosU2 = sinU1 * cosU2;

  double lambda = L;
  double lambdaP = M_PI * 2.0;
  int iterLimit = 0;
  double cosSqAlpha;
  double sinSigma;
  double cos2SigmaM;
  double cosSigma;
  double sigma;
  double sinAlpha;
  double C;
  double sinLambda, cosLambda;

  do {
    sinLambda = sin(lambda);
    cosLambda = cos(lambda);
    sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) +
                    (dCosU1SinU2 - dSinU1CosU2 * cosLambda) *
                        (dCosU1SinU2 - dSinU1CosU2 * cosLambda));

    if (sinSigma == 0.0) {
      return 0.0;
    }
    cosSigma = dSinU1SinU2 + dCosU1CosU2 * cosLambda;
    sigma = atan2(sinSigma, cosSigma);
    sinAlpha = dCosU1CosU2 * sinLambda / sinSigma;
    cosSqAlpha = 1.0 - sinAlpha * sinAlpha;
    cos2SigmaM = cosSigma - 2.0 * dSinU1SinU2 / cosSqAlpha;

    if (isnan(cos2SigmaM)) {
      cos2SigmaM = 0.0; // equatorial line: cosSqAlpha=0
    }
    C = flattening / 16.0 * cosSqAlpha *
        (4.0 + flattening * (4.0 - 3.0 * cosSqAlpha));
    lambdaP = lambda;
    lambda = L + (1.0 - C) * flattening * sinAlpha *
                     (sigma + C * sinSigma *
                                  (cos2SigmaM +
                                   C * cosSigma *
                                       (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));
  } while (abs(lambda - lambdaP) >= Vector::MINIMUM_RESOLUTION &&
           ++iterLimit < 100);
  constexpr double uSq = cosSqAlpha * this->squareRatio;
  constexpr double A =
      1.0 +
      uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
  constexpr double B =
      uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));
  constexpr double deltaSigma =
      B * sinSigma *
      (cos2SigmaM +
       B / 4.0 *
           (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
            B / 6.0 * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) *
                (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));

  return c * inverseScale * A * (sigma - deltaSigma);
}

shared_ptr<GeoPoint>
PlanetModel::surfacePointOnBearing(shared_ptr<GeoPoint> from, double const dist,
                                   double const bearing)
{
  // Algorithm using Vincenty's formulae
  // (https://en.wikipedia.org/wiki/Vincenty%27s_formulae) which takes into
  // account that planets may not be spherical.
  // Code adaptation from
  // http://www.movable-type.co.uk/scripts/latlong-vincenty.html

  double lat = from->getLatitude();
  double lon = from->getLongitude();
  double sinα1 = sin(bearing);
  double cosα1 = cos(bearing);

  double tanU1 = (1.0 - flattening) * tan(lat);
  double cosU1 = 1.0 / sqrt((1.0 + tanU1 * tanU1));
  double sinU1 = tanU1 * cosU1;

  double σ1 = atan2(tanU1, cosα1);
  double sinα = cosU1 * sinα1;
  double cosSqα = 1.0 - sinα * sinα;
  double uSq = cosSqα * squareRatio;
  double A = 1.0 + uSq / 16384.0 *
                       (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
  double B =
      uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));

  double cos2σM;
  double sinσ;
  double cosσ;
  double Δσ;

  double σ = dist / (c * inverseScale * A);
  double σʹ;
  double iterations = 0;
  do {
    cos2σM = cos(2.0 * σ1 + σ);
    sinσ = sin(σ);
    cosσ = cos(σ);
    Δσ = B * sinσ *
         (cos2σM + B / 4.0 *
                       (cosσ * (-1.0 + 2.0 * cos2σM * cos2σM) -
                        B / 6.0 * cos2σM * (-3.0 + 4.0 * sinσ * sinσ) *
                            (-3.0 + 4.0 * cos2σM * cos2σM)));
    σʹ = σ;
    σ = dist / (c * inverseScale * A) + Δσ;
  } while (abs(σ - σʹ) >= Vector::MINIMUM_RESOLUTION && ++iterations < 100);
  double x = sinU1 * sinσ - cosU1 * cosσ * cosα1;
  double φ2 = atan2(sinU1 * cosσ + cosU1 * sinσ * cosα1,
                    (1.0 - flattening) * sqrt(sinα * sinα + x * x));
  double λ = atan2(sinσ * sinα1, cosU1 * cosσ - sinU1 * sinσ * cosα1);
  double C =
      flattening / 16.0 * cosSqα * (4.0 + flattening * (4.0 - 3.0 * cosSqα));
  double L =
      λ -
      (1.0 - C) * flattening * sinα *
          (σ + C * sinσ * (cos2σM + C * cosσ * (-1.0 + 2.0 * cos2σM * cos2σM)));
  double λ2 =
      (lon + L + 3.0 * M_PI) % (2.0 * M_PI) - M_PI; // normalise to -180..+180

  return make_shared<GeoPoint>(shared_from_this(), φ2, λ2);
}

bool PlanetModel::equals(any const o)
{
  if (!(std::dynamic_pointer_cast<PlanetModel>(o) != nullptr)) {
    return false;
  }
  shared_ptr<PlanetModel> *const other =
      any_cast<std::shared_ptr<PlanetModel>>(o);
  return ab == other->ab && c == other->c;
}

int PlanetModel::hashCode()
{
  return Double::hashCode(ab) + Double::hashCode(c);
}

wstring PlanetModel::toString()
{
  if (this->equals(SPHERE)) {
    return L"PlanetModel.SPHERE";
  } else if (this->equals(WGS84)) {
    return L"PlanetModel.WGS84";
  } else {
    return L"PlanetModel(ab=" + to_wstring(ab) + L" c=" + to_wstring(c) + L")";
  }
}
} // namespace org::apache::lucene::spatial3d::geom