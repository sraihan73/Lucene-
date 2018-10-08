using namespace std;

#include "Vector.h"

namespace org::apache::lucene::spatial3d::geom
{

const double Vector::MINIMUM_ANGULAR_RESOLUTION = M_PI * MINIMUM_RESOLUTION;
const double Vector::MINIMUM_RESOLUTION_SQUARED =
    MINIMUM_RESOLUTION * MINIMUM_RESOLUTION;
const double Vector::MINIMUM_RESOLUTION_CUBED =
    MINIMUM_RESOLUTION_SQUARED * MINIMUM_RESOLUTION;
const double Vector::MINIMUM_GRAM_SCHMIDT_ENVELOPE = MINIMUM_RESOLUTION * 0.5;

Vector::Vector(double x, double y, double z) : x(x), y(y), z(z) {}

Vector::Vector(shared_ptr<Vector> A, double const BX, double const BY,
               double const BZ)
    : Vector(A->x, A->y, A->z, BX, BY, BZ)
{
  // We're really looking at two vectors and computing a perpendicular one from
  // that.
}

Vector::Vector(double const AX, double const AY, double const AZ,
               double const BX, double const BY, double const BZ)
    : x(normalizeX), y(normalizeY), z(normalizeZ)
{
  // We're really looking at two vectors and computing a perpendicular one from
  // that. We can think of this as having three points -- the origin, and two
  // points that aren't the origin. Normally, we can compute the perpendicular
  // deque this way: x = u2v3 - u3v2 y = u3v1 - u1v3 z = u1v2 - u2v1 Sometimes
  // that produces a plane that does not contain the original three points,
  // however, due to numerical precision issues.  Then we continue making the
  // answer more precise using the Gram-Schmidt process:
  // https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process

  // Compute the naive perpendicular
  constexpr double thisX = AY * BZ - AZ * BY;
  constexpr double thisY = AZ * BX - AX * BZ;
  constexpr double thisZ = AX * BY - AY * BX;

  constexpr double magnitude = Vector::magnitude(thisX, thisY, thisZ);
  if (magnitude == 0.0) {
    throw invalid_argument(L"Degenerate/parallel deque constructed");
  }
  constexpr double inverseMagnitude = 1.0 / magnitude;

  double normalizeX = thisX * inverseMagnitude;
  double normalizeY = thisY * inverseMagnitude;
  double normalizeZ = thisZ * inverseMagnitude;
  // For a plane to work, the dot product between the normal deque
  // and the points needs to be less than the minimum resolution.
  // This is sometimes not true for points that are very close. Therefore
  // we need to adjust
  int i = 0;
  while (true) {
    constexpr double currentDotProdA =
        AX * normalizeX + AY * normalizeY + AZ * normalizeZ;
    constexpr double currentDotProdB =
        BX * normalizeX + BY * normalizeY + BZ * normalizeZ;
    if (abs(currentDotProdA) < MINIMUM_GRAM_SCHMIDT_ENVELOPE &&
        abs(currentDotProdB) < MINIMUM_GRAM_SCHMIDT_ENVELOPE) {
      break;
    }
    // Converge on the one that has largest dot product
    constexpr double currentVectorX;
    constexpr double currentVectorY;
    constexpr double currentVectorZ;
    constexpr double currentDotProd;
    if (abs(currentDotProdA) > abs(currentDotProdB)) {
      currentVectorX = AX;
      currentVectorY = AY;
      currentVectorZ = AZ;
      currentDotProd = currentDotProdA;
    } else {
      currentVectorX = BX;
      currentVectorY = BY;
      currentVectorZ = BZ;
      currentDotProd = currentDotProdB;
    }

    // Adjust
    normalizeX = normalizeX - currentDotProd * currentVectorX;
    normalizeY = normalizeY - currentDotProd * currentVectorY;
    normalizeZ = normalizeZ - currentDotProd * currentVectorZ;
    // Normalize
    constexpr double correctedMagnitude =
        Vector::magnitude(normalizeX, normalizeY, normalizeZ);
    constexpr double inverseCorrectedMagnitude = 1.0 / correctedMagnitude;
    normalizeX = normalizeX * inverseCorrectedMagnitude;
    normalizeY = normalizeY * inverseCorrectedMagnitude;
    normalizeZ = normalizeZ * inverseCorrectedMagnitude;
    // This is  probably not needed as the method seems to converge
    // quite quickly. But it is safer to have a way out.
    if (i++ > 10) {
      throw invalid_argument(
          L"Plane could not be constructed! Could not find a normal deque.");
    }
  }
}

Vector::Vector(shared_ptr<Vector> A, shared_ptr<Vector> B)
    : Vector(A, B->x, B->y, B->z)
{
}

double Vector::magnitude(double const x, double const y, double const z)
{
  return sqrt(x * x + y * y + z * z);
}

shared_ptr<Vector> Vector::normalize()
{
  double denom = magnitude();
  if (denom < MINIMUM_RESOLUTION) {
    // Degenerate, can't normalize
    return nullptr;
  }
  double normFactor = 1.0 / denom;
  return make_shared<Vector>(x * normFactor, y * normFactor, z * normFactor);
}

bool Vector::crossProductEvaluateIsZero(shared_ptr<Vector> A,
                                        shared_ptr<Vector> B,
                                        shared_ptr<Vector> point)
{
  // Include Gram-Schmidt in-line so we avoid creating objects unnecessarily
  // Compute the naive perpendicular
  constexpr double thisX = A->y * B->z - A->z * B->y;
  constexpr double thisY = A->z * B->x - A->x * B->z;
  constexpr double thisZ = A->x * B->y - A->y * B->x;

  constexpr double magnitude = Vector::magnitude(thisX, thisY, thisZ);
  if (magnitude == 0.0) {
    return true;
  }

  constexpr double inverseMagnitude = 1.0 / magnitude;

  double normalizeX = thisX * inverseMagnitude;
  double normalizeY = thisY * inverseMagnitude;
  double normalizeZ = thisZ * inverseMagnitude;
  // For a plane to work, the dot product between the normal deque
  // and the points needs to be less than the minimum resolution.
  // This is sometimes not true for points that are very close. Therefore
  // we need to adjust
  int i = 0;
  while (true) {
    constexpr double currentDotProdA =
        A->x * normalizeX + A->y * normalizeY + A->z * normalizeZ;
    constexpr double currentDotProdB =
        B->x * normalizeX + B->y * normalizeY + B->z * normalizeZ;
    if (abs(currentDotProdA) < MINIMUM_GRAM_SCHMIDT_ENVELOPE &&
        abs(currentDotProdB) < MINIMUM_GRAM_SCHMIDT_ENVELOPE) {
      break;
    }
    // Converge on the one that has largest dot product
    constexpr double currentVectorX;
    constexpr double currentVectorY;
    constexpr double currentVectorZ;
    constexpr double currentDotProd;
    if (abs(currentDotProdA) > abs(currentDotProdB)) {
      currentVectorX = A->x;
      currentVectorY = A->y;
      currentVectorZ = A->z;
      currentDotProd = currentDotProdA;
    } else {
      currentVectorX = B->x;
      currentVectorY = B->y;
      currentVectorZ = B->z;
      currentDotProd = currentDotProdB;
    }

    // Adjust
    normalizeX = normalizeX - currentDotProd * currentVectorX;
    normalizeY = normalizeY - currentDotProd * currentVectorY;
    normalizeZ = normalizeZ - currentDotProd * currentVectorZ;
    // Normalize
    constexpr double correctedMagnitude =
        Vector::magnitude(normalizeX, normalizeY, normalizeZ);
    constexpr double inverseCorrectedMagnitude = 1.0 / correctedMagnitude;
    normalizeX = normalizeX * inverseCorrectedMagnitude;
    normalizeY = normalizeY * inverseCorrectedMagnitude;
    normalizeZ = normalizeZ * inverseCorrectedMagnitude;
    // This is  probably not needed as the method seems to converge
    // quite quickly. But it is safer to have a way out.
    if (i++ > 10) {
      throw invalid_argument(
          L"Plane could not be constructed! Could not find a normal deque.");
    }
  }
  return abs(normalizeX * point->x + normalizeY * point->y +
             normalizeZ * point->z) < MINIMUM_RESOLUTION;
}

double Vector::dotProduct(shared_ptr<Vector> v)
{
  return this->x * v->x + this->y * v->y + this->z * v->z;
}

double Vector::dotProduct(double const x, double const y, double const z)
{
  return this->x * x + this->y * y + this->z * z;
}

bool Vector::isWithin(std::deque<std::shared_ptr<Membership>> &bounds,
                      deque<Membership> &moreBounds)
{
  // Return true if the point described is within all provided bounds
  // System.err.println("  checking if "+this+" is within bounds");
  for (auto bound : bounds) {
    if (bound != nullptr && !bound->isWithin(shared_from_this())) {
      // System.err.println("    NOT within "+bound);
      return false;
    }
  }
  for (shared_ptr<Membership> bound : moreBounds) {
    if (bound != nullptr && !bound->isWithin(shared_from_this())) {
      // System.err.println("    NOT within "+bound);
      return false;
    }
  }
  // System.err.println("    is within");
  return true;
}

shared_ptr<Vector> Vector::translate(double const xOffset, double const yOffset,
                                     double const zOffset)
{
  return make_shared<Vector>(x - xOffset, y - yOffset, z - zOffset);
}

shared_ptr<Vector> Vector::rotateXY(double const angle)
{
  return rotateXY(sin(angle), cos(angle));
}

shared_ptr<Vector> Vector::rotateXY(double const sinAngle,
                                    double const cosAngle)
{
  return make_shared<Vector>(x * cosAngle - y * sinAngle,
                             x * sinAngle + y * cosAngle, z);
}

shared_ptr<Vector> Vector::rotateXZ(double const angle)
{
  return rotateXZ(sin(angle), cos(angle));
}

shared_ptr<Vector> Vector::rotateXZ(double const sinAngle,
                                    double const cosAngle)
{
  return make_shared<Vector>(x * cosAngle - z * sinAngle, y,
                             x * sinAngle + z * cosAngle);
}

shared_ptr<Vector> Vector::rotateZY(double const angle)
{
  return rotateZY(sin(angle), cos(angle));
}

shared_ptr<Vector> Vector::rotateZY(double const sinAngle,
                                    double const cosAngle)
{
  return make_shared<Vector>(x, z * sinAngle + y * cosAngle,
                             z * cosAngle - y * sinAngle);
}

double Vector::linearDistanceSquared(shared_ptr<Vector> v)
{
  double deltaX = this->x - v->x;
  double deltaY = this->y - v->y;
  double deltaZ = this->z - v->z;
  return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}

double Vector::linearDistanceSquared(double const x, double const y,
                                     double const z)
{
  double deltaX = this->x - x;
  double deltaY = this->y - y;
  double deltaZ = this->z - z;
  return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}

double Vector::linearDistance(shared_ptr<Vector> v)
{
  return sqrt(linearDistanceSquared(v));
}

double Vector::linearDistance(double const x, double const y, double const z)
{
  return sqrt(linearDistanceSquared(x, y, z));
}

double Vector::normalDistanceSquared(shared_ptr<Vector> v)
{
  double t = dotProduct(v);
  double deltaX = this->x * t - v->x;
  double deltaY = this->y * t - v->y;
  double deltaZ = this->z * t - v->z;
  return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}

double Vector::normalDistanceSquared(double const x, double const y,
                                     double const z)
{
  double t = dotProduct(x, y, z);
  double deltaX = this->x * t - x;
  double deltaY = this->y * t - y;
  double deltaZ = this->z * t - z;
  return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}

double Vector::normalDistance(shared_ptr<Vector> v)
{
  return sqrt(normalDistanceSquared(v));
}

double Vector::normalDistance(double const x, double const y, double const z)
{
  return sqrt(normalDistanceSquared(x, y, z));
}

double Vector::magnitude() { return magnitude(x, y, z); }

bool Vector::isNumericallyIdentical(double const otherX, double const otherY,
                                    double const otherZ)
{
  constexpr double deltaX = x - otherX;
  constexpr double deltaY = y - otherY;
  constexpr double deltaZ = z - otherZ;
  return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ <
         MINIMUM_RESOLUTION_SQUARED;
}

bool Vector::isNumericallyIdentical(shared_ptr<Vector> other)
{
  constexpr double deltaX = x - other->x;
  constexpr double deltaY = y - other->y;
  constexpr double deltaZ = z - other->z;
  return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ <
         MINIMUM_RESOLUTION_SQUARED;
}

bool Vector::isParallel(double const otherX, double const otherY,
                        double const otherZ)
{
  constexpr double thisX = y * otherZ - z * otherY;
  constexpr double thisY = z * otherX - x * otherZ;
  constexpr double thisZ = x * otherY - y * otherX;
  return thisX * thisX + thisY * thisY + thisZ * thisZ <
         MINIMUM_RESOLUTION_SQUARED;
}

bool Vector::isParallel(shared_ptr<Vector> other)
{
  constexpr double thisX = y * other->z - z * other->y;
  constexpr double thisY = z * other->x - x * other->z;
  constexpr double thisZ = x * other->y - y * other->x;
  return thisX * thisX + thisY * thisY + thisZ * thisZ <
         MINIMUM_RESOLUTION_SQUARED;
}

double
Vector::computeDesiredEllipsoidMagnitude(shared_ptr<PlanetModel> planetModel,
                                         double const x, double const y,
                                         double const z)
{
  return 1.0 / sqrt(x * x * planetModel->inverseAbSquared +
                    y * y * planetModel->inverseAbSquared +
                    z * z * planetModel->inverseCSquared);
}

double
Vector::computeDesiredEllipsoidMagnitude(shared_ptr<PlanetModel> planetModel,
                                         double const z)
{
  return 1.0 / sqrt((1.0 - z * z) * planetModel->inverseAbSquared +
                    z * z * planetModel->inverseCSquared);
}

bool Vector::equals(any o)
{
  if (!(std::dynamic_pointer_cast<Vector>(o) != nullptr)) {
    return false;
  }
  shared_ptr<Vector> other = any_cast<std::shared_ptr<Vector>>(o);
  return (other->x == x && other->y == y && other->z == z);
}

int Vector::hashCode()
{
  int result;
  int64_t temp;
  temp = Double::doubleToLongBits(x);
  result = static_cast<int>(
      temp ^
      (static_cast<int64_t>(static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(y);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(z);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring Vector::toString()
{
  return L"[X=" + to_wstring(x) + L", Y=" + to_wstring(y) + L", Z=" +
         to_wstring(z) + L"]";
}
} // namespace org::apache::lucene::spatial3d::geom