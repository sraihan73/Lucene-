using namespace std;

#include "Plane.h"

namespace org::apache::lucene::spatial3d::geom
{

std::deque<std::shared_ptr<GeoPoint>> const Plane::NO_POINTS =
    std::deque<std::shared_ptr<GeoPoint>>(0);
std::deque<std::shared_ptr<Membership>> const Plane::NO_BOUNDS =
    std::deque<std::shared_ptr<Membership>>(0);
const shared_ptr<Plane> Plane::normalYPlane =
    make_shared<Plane>(0.0, 1.0, 0.0, 0.0);
const shared_ptr<Plane> Plane::normalXPlane =
    make_shared<Plane>(1.0, 0.0, 0.0, 0.0);
const shared_ptr<Plane> Plane::normalZPlane =
    make_shared<Plane>(0.0, 0.0, 1.0, 0.0);

Plane::Plane(double const A, double const B, double const C, double const D)
    : Vector(A, B, C), D(D)
{
}

Plane::Plane(shared_ptr<Vector> A, double const BX, double const BY,
             double const BZ)
    : Vector(A, BX, BY, BZ), D(0.0)
{
}

Plane::Plane(shared_ptr<Vector> A, shared_ptr<Vector> B) : Vector(A, B), D(0.0)
{
}

Plane::Plane(shared_ptr<PlanetModel> planetModel, double const sinLat)
    : Vector(0.0, 0.0, 1.0),
      D(-sinLat * computeDesiredEllipsoidMagnitude(planetModel, sinLat))
{
}

Plane::Plane(double const x, double const y) : Vector(y, -x, 0.0), D(0.0) {}

Plane::Plane(shared_ptr<Vector> v, double const D)
    : Vector(v->x, v->y, v->z), D(D)
{
}

Plane::Plane(shared_ptr<Plane> basePlane, bool const above)
    : Plane(basePlane->x, basePlane->y, basePlane->z,
            above ? Math::nextUp(basePlane->D + MINIMUM_RESOLUTION)
                  : Math::nextDown(basePlane->D - MINIMUM_RESOLUTION))
{
}

shared_ptr<Plane> Plane::constructNormalizedZPlane(deque<Vector> &planePoints)
{
  // Pick the best one (with the greatest x-y distance)
  double bestDistance = 0.0;
  shared_ptr<Vector> bestPoint = nullptr;
  for (shared_ptr<Vector> point : planePoints) {
    constexpr double pointDist = point->x * point->x + point->y * point->y;
    if (pointDist > bestDistance) {
      bestDistance = pointDist;
      bestPoint = point;
    }
  }
  return constructNormalizedZPlane({bestPoint->x, bestPoint->y});
}

shared_ptr<Plane> Plane::constructNormalizedYPlane(deque<Vector> &planePoints)
{
  // Pick the best one (with the greatest x-z distance)
  double bestDistance = 0.0;
  shared_ptr<Vector> bestPoint = nullptr;
  for (shared_ptr<Vector> point : planePoints) {
    constexpr double pointDist = point->x * point->x + point->z * point->z;
    if (pointDist > bestDistance) {
      bestDistance = pointDist;
      bestPoint = point;
    }
  }
  return constructNormalizedYPlane({bestPoint->x, bestPoint->z, 0.0});
}

shared_ptr<Plane> Plane::constructNormalizedXPlane(deque<Vector> &planePoints)
{
  // Pick the best one (with the greatest y-z distance)
  double bestDistance = 0.0;
  shared_ptr<Vector> bestPoint = nullptr;
  for (shared_ptr<Vector> point : planePoints) {
    constexpr double pointDist = point->y * point->y + point->z * point->z;
    if (pointDist > bestDistance) {
      bestDistance = pointDist;
      bestPoint = point;
    }
  }
  return constructNormalizedXPlane({bestPoint->y, bestPoint->z, 0.0});
}

shared_ptr<Plane> Plane::constructNormalizedZPlane(double const x,
                                                   double const y)
{
  if (abs(x) < MINIMUM_RESOLUTION && abs(y) < MINIMUM_RESOLUTION) {
    return nullptr;
  }
  constexpr double denom = 1.0 / sqrt(x * x + y * y);
  return make_shared<Plane>(y * denom, -x * denom, 0.0, 0.0);
}

shared_ptr<Plane> Plane::constructNormalizedYPlane(double const x,
                                                   double const z,
                                                   double const DValue)
{
  if (abs(x) < MINIMUM_RESOLUTION && abs(z) < MINIMUM_RESOLUTION) {
    return nullptr;
  }
  constexpr double denom = 1.0 / sqrt(x * x + z * z);
  return make_shared<Plane>(z * denom, 0.0, -x * denom, DValue);
}

shared_ptr<Plane> Plane::constructNormalizedXPlane(double const y,
                                                   double const z,
                                                   double const DValue)
{
  if (abs(y) < MINIMUM_RESOLUTION && abs(z) < MINIMUM_RESOLUTION) {
    return nullptr;
  }
  constexpr double denom = 1.0 / sqrt(y * y + z * z);
  return make_shared<Plane>(0.0, z * denom, -y * denom, DValue);
}

double Plane::evaluate(shared_ptr<Vector> v) { return dotProduct(v) + D; }

double Plane::evaluate(double const x, double const y, double const z)
{
  return dotProduct(x, y, z) + D;
}

bool Plane::evaluateIsZero(shared_ptr<Vector> v)
{
  return abs(evaluate(v)) < MINIMUM_RESOLUTION;
}

bool Plane::evaluateIsZero(double const x, double const y, double const z)
{
  return abs(evaluate(x, y, z)) < MINIMUM_RESOLUTION;
}

shared_ptr<Plane> Plane::normalize()
{
  shared_ptr<Vector> normVect = Vector::normalize();
  if (normVect == nullptr) {
    return nullptr;
  }
  return make_shared<Plane>(normVect, this->D);
}

double Plane::arcDistance(shared_ptr<PlanetModel> planetModel,
                          shared_ptr<GeoPoint> v, deque<Membership> &bounds)
{
  return arcDistance(planetModel, v->x, {v->y, v->z, bounds});
}

double Plane::arcDistance(shared_ptr<PlanetModel> planetModel, double const x,
                          double const y, double const z,
                          deque<Membership> &bounds)
{

  if (evaluateIsZero(x, y, z)) {
    if (meetsAllBounds(x, y, z, bounds)) {
      return 0.0;
    }
    return numeric_limits<double>::infinity();
  }

  // First, compute the perpendicular plane.
  shared_ptr<Plane> *const perpPlane =
      make_shared<Plane>(this->y * z - this->z * y, this->z * x - this->x * z,
                         this->x * y - this->y * x, 0.0);

  // We need to compute the intersection of two planes on the geo surface: this
  // one, and its perpendicular. Then, we need to choose which of the two points
  // we want to compute the distance to.  We pick the shorter distance always.

  std::deque<std::shared_ptr<GeoPoint>> intersectionPoints =
      findIntersections(planetModel, perpPlane);

  // For each point, compute a linear distance, and take the minimum of them
  double minDistance = numeric_limits<double>::infinity();

  for (auto intersectionPoint : intersectionPoints) {
    if (meetsAllBounds(intersectionPoint, bounds)) {
      constexpr double theDistance = intersectionPoint->arcDistance(x, y, z);
      if (theDistance < minDistance) {
        minDistance = theDistance;
      }
    }
  }
  return minDistance;
}

double Plane::normalDistance(shared_ptr<Vector> v, deque<Membership> &bounds)
{
  return normalDistance(v->x, {v->y, v->z, bounds});
}

double Plane::normalDistance(double const x, double const y, double const z,
                             deque<Membership> &bounds)
{

  constexpr double dist = evaluate(x, y, z);
  constexpr double perpX = x - dist * this->x;
  constexpr double perpY = y - dist * this->y;
  constexpr double perpZ = z - dist * this->z;

  if (!meetsAllBounds(perpX, perpY, perpZ, bounds)) {
    return numeric_limits<double>::infinity();
  }

  return abs(dist);
}

double Plane::normalDistanceSquared(shared_ptr<Vector> v,
                                    deque<Membership> &bounds)
{
  return normalDistanceSquared(v->x, {v->y, v->z, bounds});
}

double Plane::normalDistanceSquared(double const x, double const y,
                                    double const z, deque<Membership> &bounds)
{
  constexpr double normal = normalDistance(x, {y, z, bounds});
  if (normal == numeric_limits<double>::infinity()) {
    return normal;
  }
  return normal * normal;
}

double Plane::linearDistance(shared_ptr<PlanetModel> planetModel,
                             shared_ptr<GeoPoint> v, deque<Membership> &bounds)
{
  return linearDistance(planetModel, v->x, {v->y, v->z, bounds});
}

double Plane::linearDistance(shared_ptr<PlanetModel> planetModel,
                             double const x, double const y, double const z,
                             deque<Membership> &bounds)
{
  if (evaluateIsZero(x, y, z)) {
    if (meetsAllBounds(x, y, z, bounds)) {
      return 0.0;
    }
    return numeric_limits<double>::infinity();
  }

  // First, compute the perpendicular plane.
  shared_ptr<Plane> *const perpPlane =
      make_shared<Plane>(this->y * z - this->z * y, this->z * x - this->x * z,
                         this->x * y - this->y * x, 0.0);

  // We need to compute the intersection of two planes on the geo surface: this
  // one, and its perpendicular. Then, we need to choose which of the two points
  // we want to compute the distance to.  We pick the shorter distance always.

  std::deque<std::shared_ptr<GeoPoint>> intersectionPoints =
      findIntersections(planetModel, perpPlane);

  // For each point, compute a linear distance, and take the minimum of them
  double minDistance = numeric_limits<double>::infinity();

  for (auto intersectionPoint : intersectionPoints) {
    if (meetsAllBounds(intersectionPoint, bounds)) {
      constexpr double theDistance = intersectionPoint->linearDistance(x, y, z);
      if (theDistance < minDistance) {
        minDistance = theDistance;
      }
    }
  }
  return minDistance;
}

double Plane::linearDistanceSquared(shared_ptr<PlanetModel> planetModel,
                                    shared_ptr<GeoPoint> v,
                                    deque<Membership> &bounds)
{
  return linearDistanceSquared(planetModel, v->x, {v->y, v->z, bounds});
}

double Plane::linearDistanceSquared(shared_ptr<PlanetModel> planetModel,
                                    double const x, double const y,
                                    double const z, deque<Membership> &bounds)
{
  constexpr double linearDistance =
      this->linearDistance(planetModel, x, {y, z, bounds});
  return linearDistance * linearDistance;
}

std::deque<std::shared_ptr<GeoPoint>>
Plane::interpolate(shared_ptr<GeoPoint> start, shared_ptr<GeoPoint> end,
                   std::deque<double> &proportions)
{
  // Steps:
  // (1) Translate (x0,y0,z0) of endpoints into origin-centered place:
  // x1 = x0 + D*A
  // y1 = y0 + D*B
  // z1 = z0 + D*C
  // (2) Rotate counterclockwise in x-y:
  // ra = -atan2(B,A)
  // x2 = x1 cos ra - y1 sin ra
  // y2 = x1 sin ra + y1 cos ra
  // z2 = z1
  // Faster:
  // cos ra = A/sqrt(A^2+B^2+C^2)
  // sin ra = -B/sqrt(A^2+B^2+C^2)
  // cos (-ra) = A/sqrt(A^2+B^2+C^2)
  // sin (-ra) = B/sqrt(A^2+B^2+C^2)
  // (3) Rotate clockwise in x-z:
  // ha = pi/2 - asin(C/sqrt(A^2+B^2+C^2))
  // x3 = x2 cos ha - z2 sin ha
  // y3 = y2
  // z3 = x2 sin ha + z2 cos ha
  // At this point, z3 should be zero.
  // Faster:
  // sin(ha) = cos(asin(C/sqrt(A^2+B^2+C^2))) = sqrt(1 - C^2/(A^2+B^2+C^2)) =
  // sqrt(A^2+B^2)/sqrt(A^2+B^2+C^2) cos(ha) = sin(asin(C/sqrt(A^2+B^2+C^2))) =
  // C/sqrt(A^2+B^2+C^2) (4) Compute interpolations by getting longitudes of
  // original points la = atan2(y3,x3) (5) Rotate new points (xN0, yN0, zN0)
  // counter-clockwise in x-z: ha = -(pi - asin(C/sqrt(A^2+B^2+C^2))) xN1 = xN0
  // cos ha - zN0 sin ha yN1 = yN0 zN1 = xN0 sin ha + zN0 cos ha (6) Rotate new
  // points clockwise in x-y: ra = atan2(B,A) xN2 = xN1 cos ra - yN1 sin ra yN2
  // = xN1 sin ra + yN1 cos ra zN2 = zN1 (7) Translate new points: xN3 = xN2 -
  // D*A yN3 = yN2 - D*B zN3 = zN2 - D*C

  // First, calculate the angles and their sin/cos values
  double A = x;
  double B = y;
  double C = z;

  // Translation amounts
  constexpr double transX = -D * A;
  constexpr double transY = -D * B;
  constexpr double transZ = -D * C;

  double cosRA;
  double sinRA;
  double cosHA;
  double sinHA;

  double magnitude = this->magnitude();
  if (magnitude >= MINIMUM_RESOLUTION) {
    constexpr double denom = 1.0 / magnitude;
    A *= denom;
    B *= denom;
    C *= denom;

    // cos ra = A/sqrt(A^2+B^2+C^2)
    // sin ra = -B/sqrt(A^2+B^2+C^2)
    // cos (-ra) = A/sqrt(A^2+B^2+C^2)
    // sin (-ra) = B/sqrt(A^2+B^2+C^2)
    constexpr double xyMagnitude = sqrt(A * A + B * B);
    if (xyMagnitude >= MINIMUM_RESOLUTION) {
      constexpr double xyDenom = 1.0 / xyMagnitude;
      cosRA = A * xyDenom;
      sinRA = -B * xyDenom;
    } else {
      cosRA = 1.0;
      sinRA = 0.0;
    }

    // sin(ha) = cos(asin(C/sqrt(A^2+B^2+C^2))) = sqrt(1 - C^2/(A^2+B^2+C^2)) =
    // sqrt(A^2+B^2)/sqrt(A^2+B^2+C^2) cos(ha) = sin(asin(C/sqrt(A^2+B^2+C^2)))
    // = C/sqrt(A^2+B^2+C^2)
    sinHA = xyMagnitude;
    cosHA = C;
  } else {
    cosRA = 1.0;
    sinRA = 0.0;
    cosHA = 1.0;
    sinHA = 0.0;
  }

  // Forward-translate the start and end points
  shared_ptr<Vector> *const modifiedStart =
      modify(start, transX, transY, transZ, sinRA, cosRA, sinHA, cosHA);
  shared_ptr<Vector> *const modifiedEnd =
      modify(end, transX, transY, transZ, sinRA, cosRA, sinHA, cosHA);
  if (abs(modifiedStart->z) >= MINIMUM_RESOLUTION) {
    throw invalid_argument(L"Start point was not on plane: " +
                           to_wstring(modifiedStart->z));
  }
  if (abs(modifiedEnd->z) >= MINIMUM_RESOLUTION) {
    throw invalid_argument(L"End point was not on plane: " +
                           to_wstring(modifiedEnd->z));
  }

  // Compute the angular distance between start and end point
  constexpr double startAngle = atan2(modifiedStart->y, modifiedStart->x);
  constexpr double endAngle = atan2(modifiedEnd->y, modifiedEnd->x);

  constexpr double startMagnitude = sqrt(modifiedStart->x * modifiedStart->x +
                                         modifiedStart->y * modifiedStart->y);
  double delta;

  double newEndAngle = endAngle;
  while (newEndAngle < startAngle) {
    newEndAngle += M_PI * 2.0;
  }

  if (newEndAngle - startAngle <= M_PI) {
    delta = newEndAngle - startAngle;
  } else {
    double newStartAngle = startAngle;
    while (newStartAngle < endAngle) {
      newStartAngle += M_PI * 2.0;
    }
    delta = newStartAngle - endAngle;
  }

  std::deque<std::shared_ptr<GeoPoint>> returnValues(proportions.size());
  for (int i = 0; i < returnValues.size(); i++) {
    constexpr double newAngle = startAngle + proportions[i] * delta;
    constexpr double sinNewAngle = sin(newAngle);
    constexpr double cosNewAngle = cos(newAngle);
    shared_ptr<Vector> *const newVector = make_shared<Vector>(
        cosNewAngle * startMagnitude, sinNewAngle * startMagnitude, 0.0);
    returnValues[i] = reverseModify(newVector, transX, transY, transZ, sinRA,
                                    cosRA, sinHA, cosHA);
  }

  return returnValues;
}

shared_ptr<Vector> Plane::modify(shared_ptr<GeoPoint> start,
                                 double const transX, double const transY,
                                 double const transZ, double const sinRA,
                                 double const cosRA, double const sinHA,
                                 double const cosHA)
{
  return start->translate(transX, transY, transZ)
      ->rotateXY(sinRA, cosRA)
      ->rotateXZ(sinHA, cosHA);
}

shared_ptr<GeoPoint>
Plane::reverseModify(shared_ptr<Vector> point, double const transX,
                     double const transY, double const transZ,
                     double const sinRA, double const cosRA, double const sinHA,
                     double const cosHA)
{
  shared_ptr<Vector> *const result = point->rotateXZ(-sinHA, cosHA)
                                         ->rotateXY(-sinRA, cosRA)
                                         ->translate(-transX, -transY, -transZ);
  return make_shared<GeoPoint>(result->x, result->y, result->z);
}

std::deque<std::shared_ptr<GeoPoint>>
Plane::findIntersections(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<Plane> q, deque<Membership> &bounds)
{
  if (isNumericallyIdentical(q)) {
    return nullptr;
  }
  return findIntersections(planetModel, q, {bounds, NO_BOUNDS});
}

std::deque<std::shared_ptr<GeoPoint>>
Plane::findCrossings(shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> q,
                     deque<Membership> &bounds)
{
  if (isNumericallyIdentical(q)) {
    return nullptr;
  }
  return findCrossings(planetModel, q, {bounds, NO_BOUNDS});
}

bool Plane::arePointsCoplanar(shared_ptr<GeoPoint> A, shared_ptr<GeoPoint> B,
                              shared_ptr<GeoPoint> C)
{
  return Vector::crossProductEvaluateIsZero(A, B, C) ||
         Vector::crossProductEvaluateIsZero(A, C, B) ||
         Vector::crossProductEvaluateIsZero(B, C, A);
}

std::deque<std::shared_ptr<GeoPoint>>
Plane::findIntersections(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<Plane> q,
                         std::deque<std::shared_ptr<Membership>> &bounds,
                         std::deque<std::shared_ptr<Membership>> &moreBounds)
{
  // System.err.println("Looking for intersection between plane "+this+" and
  // plane "+q+" within bounds");
  // Unnormalized, unchecked...
  constexpr double lineVectorX = y * q->z - z * q->y;
  constexpr double lineVectorY = z * q->x - x * q->z;
  constexpr double lineVectorZ = x * q->y - y * q->x;
  if (abs(lineVectorX) < MINIMUM_RESOLUTION &&
      abs(lineVectorY) < MINIMUM_RESOLUTION &&
      abs(lineVectorZ) < MINIMUM_RESOLUTION) {
    // Degenerate case: parallel planes
    // System.err.println(" planes are parallel - no intersection");
    return NO_POINTS;
  }

  // The line will have the equation: A t + A0 = x, B t + B0 = y, C t + C0 = z.
  // We have A, B, and C.  In order to come up with A0, B0, and C0, we need to
  // find a point that is on both planes. To do this, we find the largest deque
  // value (either x, y, or z), and look for a point that solves both plane
  // equations simultaneous.  For example, let's say that the deque is
  // (0.5,0.5,1), and the two plane equations are: 0.7 x + 0.3 y + 0.1 z + 0.0 =
  // 0 and 0.9 x - 0.1 y + 0.2 z + 4.0 = 0 Then we'd pick z = 0, so the
  // equations to solve for x and y would be: 0.7 x + 0.3y = 0.0 0.9 x - 0.1y =
  // -4.0
  // ... which can readily be solved using standard linear algebra.  Generally:
  // Q0 x + R0 y = S0
  // Q1 x + R1 y = S1
  // ... can be solved by Cramer's rule:
  // x = det(S0 R0 / S1 R1) / det(Q0 R0 / Q1 R1)
  // y = det(Q0 S0 / Q1 S1) / det(Q0 R0 / Q1 R1)
  // ... where det( a b / c d ) = ad - bc, so:
  // x = (S0 * R1 - R0 * S1) / (Q0 * R1 - R0 * Q1)
  // y = (Q0 * S1 - S0 * Q1) / (Q0 * R1 - R0 * Q1)
  double x0;
  double y0;
  double z0;
  // We try to maximize the determinant in the denominator
  constexpr double denomYZ = this->y * q->z - this->z * q->y;
  constexpr double denomXZ = this->x * q->z - this->z * q->x;
  constexpr double denomXY = this->x * q->y - this->y * q->x;
  if (abs(denomYZ) >= abs(denomXZ) && abs(denomYZ) >= abs(denomXY)) {
    // X is the biggest, so our point will have x0 = 0.0
    if (abs(denomYZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return NO_POINTS;
    }
    constexpr double denom = 1.0 / denomYZ;
    x0 = 0.0;
    y0 = (-this->D * q->z - this->z * -q->D) * denom;
    z0 = (this->y * -q->D + this->D * q->y) * denom;
  } else if (abs(denomXZ) >= abs(denomXY) && abs(denomXZ) >= abs(denomYZ)) {
    // Y is the biggest, so y0 = 0.0
    if (abs(denomXZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return NO_POINTS;
    }
    constexpr double denom = 1.0 / denomXZ;
    x0 = (-this->D * q->z - this->z * -q->D) * denom;
    y0 = 0.0;
    z0 = (this->x * -q->D + this->D * q->x) * denom;
  } else {
    // Z is the biggest, so Z0 = 0.0
    if (abs(denomXY) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return NO_POINTS;
    }
    constexpr double denom = 1.0 / denomXY;
    x0 = (-this->D * q->y - this->y * -q->D) * denom;
    y0 = (this->x * -q->D + this->D * q->x) * denom;
    z0 = 0.0;
  }

  // Once an intersecting line is determined, the next step is to intersect that
  // line with the ellipsoid, which will yield zero, one, or two points. The
  // ellipsoid equation: 1,0 = x^2/a^2 + y^2/b^2 + z^2/c^2 1.0 = (At+A0)^2/a^2 +
  // (Bt+B0)^2/b^2 + (Ct+C0)^2/c^2 A^2 t^2 / a^2 + 2AA0t / a^2 + A0^2 / a^2 +
  // B^2 t^2 / b^2 + 2BB0t / b^2 + B0^2 / b^2 + C^2 t^2 / c^2 + 2CC0t / c^2 +
  // C0^2 / c^2  - 1,0 = 0.0 [A^2 / a^2 + B^2 / b^2 + C^2 / c^2] t^2 + [2AA0 /
  // a^2 + 2BB0 / b^2 + 2CC0 / c^2] t + [A0^2 / a^2 + B0^2 / b^2 + C0^2 / c^2 -
  // 1,0] = 0.0 Use the quadratic formula to determine t values and candidate
  // point(s)
  constexpr double A =
      lineVectorX * lineVectorX * planetModel->inverseAbSquared +
      lineVectorY * lineVectorY * planetModel->inverseAbSquared +
      lineVectorZ * lineVectorZ * planetModel->inverseCSquared;
  constexpr double B = 2.0 * (lineVectorX * x0 * planetModel->inverseAbSquared +
                              lineVectorY * y0 * planetModel->inverseAbSquared +
                              lineVectorZ * z0 * planetModel->inverseCSquared);
  constexpr double C = x0 * x0 * planetModel->inverseAbSquared +
                       y0 * y0 * planetModel->inverseAbSquared +
                       z0 * z0 * planetModel->inverseCSquared - 1.0;

  constexpr double BsquaredMinus = B * B - 4.0 * A * C;
  if (abs(BsquaredMinus) < MINIMUM_RESOLUTION_SQUARED) {
    // System.err.println(" One point of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // One solution only
    constexpr double t = -B * inverse2A;
    // Maybe we can save ourselves the cost of construction of a point?
    constexpr double pointX = lineVectorX * t + x0;
    constexpr double pointY = lineVectorY * t + y0;
    constexpr double pointZ = lineVectorZ * t + z0;
    for (auto bound : bounds) {
      if (!bound->isWithin(pointX, pointY, pointZ)) {
        return NO_POINTS;
      }
    }
    for (auto bound : moreBounds) {
      if (!bound->isWithin(pointX, pointY, pointZ)) {
        return NO_POINTS;
      }
    }
    return std::deque<std::shared_ptr<GeoPoint>>{
        make_shared<GeoPoint>(pointX, pointY, pointZ)};
  } else if (BsquaredMinus > 0.0) {
    // System.err.println(" Two points of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // Two solutions
    constexpr double sqrtTerm = sqrt(BsquaredMinus);
    constexpr double t1 = (-B + sqrtTerm) * inverse2A;
    constexpr double t2 = (-B - sqrtTerm) * inverse2A;
    // Up to two points being returned.  Do what we can to save on object
    // creation though.
    constexpr double point1X = lineVectorX * t1 + x0;
    constexpr double point1Y = lineVectorY * t1 + y0;
    constexpr double point1Z = lineVectorZ * t1 + z0;
    constexpr double point2X = lineVectorX * t2 + x0;
    constexpr double point2Y = lineVectorY * t2 + y0;
    constexpr double point2Z = lineVectorZ * t2 + z0;
    bool point1Valid = true;
    bool point2Valid = true;
    for (auto bound : bounds) {
      if (!bound->isWithin(point1X, point1Y, point1Z)) {
        point1Valid = false;
        break;
      }
    }
    if (point1Valid) {
      for (auto bound : moreBounds) {
        if (!bound->isWithin(point1X, point1Y, point1Z)) {
          point1Valid = false;
          break;
        }
      }
    }
    for (auto bound : bounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        point2Valid = false;
        break;
      }
    }
    if (point2Valid) {
      for (auto bound : moreBounds) {
        if (!bound->isWithin(point2X, point2Y, point2Z)) {
          point2Valid = false;
          break;
        }
      }
    }

    if (point1Valid && point2Valid) {
      return std::deque<std::shared_ptr<GeoPoint>>{
          make_shared<GeoPoint>(point1X, point1Y, point1Z),
          make_shared<GeoPoint>(point2X, point2Y, point2Z)};
    }
    if (point1Valid) {
      return std::deque<std::shared_ptr<GeoPoint>>{
          make_shared<GeoPoint>(point1X, point1Y, point1Z)};
    }
    if (point2Valid) {
      return std::deque<std::shared_ptr<GeoPoint>>{
          make_shared<GeoPoint>(point2X, point2Y, point2Z)};
    }
    return NO_POINTS;
  } else {
    // System.err.println(" no solutions - no intersection");
    return NO_POINTS;
  }
}

std::deque<std::shared_ptr<GeoPoint>>
Plane::findCrossings(shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> q,
                     std::deque<std::shared_ptr<Membership>> &bounds,
                     std::deque<std::shared_ptr<Membership>> &moreBounds)
{
  // This code in this method is very similar to findIntersections(), but
  // eliminates the cases where crossings are detected. Unnormalized,
  // unchecked...
  constexpr double lineVectorX = y * q->z - z * q->y;
  constexpr double lineVectorY = z * q->x - x * q->z;
  constexpr double lineVectorZ = x * q->y - y * q->x;
  if (abs(lineVectorX) < MINIMUM_RESOLUTION &&
      abs(lineVectorY) < MINIMUM_RESOLUTION &&
      abs(lineVectorZ) < MINIMUM_RESOLUTION) {
    // Degenerate case: parallel planes
    return NO_POINTS;
  }

  // The line will have the equation: A t + A0 = x, B t + B0 = y, C t + C0 = z.
  // We have A, B, and C.  In order to come up with A0, B0, and C0, we need to
  // find a point that is on both planes. To do this, we find the largest deque
  // value (either x, y, or z), and look for a point that solves both plane
  // equations simultaneous.  For example, let's say that the deque is
  // (0.5,0.5,1), and the two plane equations are: 0.7 x + 0.3 y + 0.1 z + 0.0 =
  // 0 and 0.9 x - 0.1 y + 0.2 z + 4.0 = 0 Then we'd pick z = 0, so the
  // equations to solve for x and y would be: 0.7 x + 0.3y = 0.0 0.9 x - 0.1y =
  // -4.0
  // ... which can readily be solved using standard linear algebra.  Generally:
  // Q0 x + R0 y = S0
  // Q1 x + R1 y = S1
  // ... can be solved by Cramer's rule:
  // x = det(S0 R0 / S1 R1) / det(Q0 R0 / Q1 R1)
  // y = det(Q0 S0 / Q1 S1) / det(Q0 R0 / Q1 R1)
  // ... where det( a b / c d ) = ad - bc, so:
  // x = (S0 * R1 - R0 * S1) / (Q0 * R1 - R0 * Q1)
  // y = (Q0 * S1 - S0 * Q1) / (Q0 * R1 - R0 * Q1)
  double x0;
  double y0;
  double z0;
  // We try to maximize the determinant in the denominator
  constexpr double denomYZ = this->y * q->z - this->z * q->y;
  constexpr double denomXZ = this->x * q->z - this->z * q->x;
  constexpr double denomXY = this->x * q->y - this->y * q->x;
  if (abs(denomYZ) >= abs(denomXZ) && abs(denomYZ) >= abs(denomXY)) {
    // X is the biggest, so our point will have x0 = 0.0
    if (abs(denomYZ) < MINIMUM_RESOLUTION_SQUARED) {
      return NO_POINTS;
    }
    constexpr double denom = 1.0 / denomYZ;
    x0 = 0.0;
    y0 = (-this->D * q->z - this->z * -q->D) * denom;
    z0 = (this->y * -q->D + this->D * q->y) * denom;
  } else if (abs(denomXZ) >= abs(denomXY) && abs(denomXZ) >= abs(denomYZ)) {
    // Y is the biggest, so y0 = 0.0
    if (abs(denomXZ) < MINIMUM_RESOLUTION_SQUARED) {
      return NO_POINTS;
    }
    constexpr double denom = 1.0 / denomXZ;
    x0 = (-this->D * q->z - this->z * -q->D) * denom;
    y0 = 0.0;
    z0 = (this->x * -q->D + this->D * q->x) * denom;
  } else {
    // Z is the biggest, so Z0 = 0.0
    if (abs(denomXY) < MINIMUM_RESOLUTION_SQUARED) {
      return NO_POINTS;
    }
    constexpr double denom = 1.0 / denomXY;
    x0 = (-this->D * q->y - this->y * -q->D) * denom;
    y0 = (this->x * -q->D + this->D * q->x) * denom;
    z0 = 0.0;
  }

  // Once an intersecting line is determined, the next step is to intersect that
  // line with the ellipsoid, which will yield zero, one, or two points. The
  // ellipsoid equation: 1,0 = x^2/a^2 + y^2/b^2 + z^2/c^2 1.0 = (At+A0)^2/a^2 +
  // (Bt+B0)^2/b^2 + (Ct+C0)^2/c^2 A^2 t^2 / a^2 + 2AA0t / a^2 + A0^2 / a^2 +
  // B^2 t^2 / b^2 + 2BB0t / b^2 + B0^2 / b^2 + C^2 t^2 / c^2 + 2CC0t / c^2 +
  // C0^2 / c^2  - 1,0 = 0.0 [A^2 / a^2 + B^2 / b^2 + C^2 / c^2] t^2 + [2AA0 /
  // a^2 + 2BB0 / b^2 + 2CC0 / c^2] t + [A0^2 / a^2 + B0^2 / b^2 + C0^2 / c^2 -
  // 1,0] = 0.0 Use the quadratic formula to determine t values and candidate
  // point(s)
  constexpr double A =
      lineVectorX * lineVectorX * planetModel->inverseAbSquared +
      lineVectorY * lineVectorY * planetModel->inverseAbSquared +
      lineVectorZ * lineVectorZ * planetModel->inverseCSquared;
  constexpr double B = 2.0 * (lineVectorX * x0 * planetModel->inverseAbSquared +
                              lineVectorY * y0 * planetModel->inverseAbSquared +
                              lineVectorZ * z0 * planetModel->inverseCSquared);
  constexpr double C = x0 * x0 * planetModel->inverseAbSquared +
                       y0 * y0 * planetModel->inverseAbSquared +
                       z0 * z0 * planetModel->inverseCSquared - 1.0;

  constexpr double BsquaredMinus = B * B - 4.0 * A * C;
  if (abs(BsquaredMinus) < MINIMUM_RESOLUTION_SQUARED) {
    // One point of intersection: cannot be a crossing.
    return NO_POINTS;
  } else if (BsquaredMinus > 0.0) {
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // Two solutions
    constexpr double sqrtTerm = sqrt(BsquaredMinus);
    constexpr double t1 = (-B + sqrtTerm) * inverse2A;
    constexpr double t2 = (-B - sqrtTerm) * inverse2A;
    // Up to two points being returned.  Do what we can to save on object
    // creation though.
    constexpr double point1X = lineVectorX * t1 + x0;
    constexpr double point1Y = lineVectorY * t1 + y0;
    constexpr double point1Z = lineVectorZ * t1 + z0;
    constexpr double point2X = lineVectorX * t2 + x0;
    constexpr double point2Y = lineVectorY * t2 + y0;
    constexpr double point2Z = lineVectorZ * t2 + z0;
    bool point1Valid = true;
    bool point2Valid = true;
    for (auto bound : bounds) {
      if (!bound->isWithin(point1X, point1Y, point1Z)) {
        point1Valid = false;
        break;
      }
    }
    if (point1Valid) {
      for (auto bound : moreBounds) {
        if (!bound->isWithin(point1X, point1Y, point1Z)) {
          point1Valid = false;
          break;
        }
      }
    }
    for (auto bound : bounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        point2Valid = false;
        break;
      }
    }
    if (point2Valid) {
      for (auto bound : moreBounds) {
        if (!bound->isWithin(point2X, point2Y, point2Z)) {
          point2Valid = false;
          break;
        }
      }
    }

    if (point1Valid && point2Valid) {
      return std::deque<std::shared_ptr<GeoPoint>>{
          make_shared<GeoPoint>(point1X, point1Y, point1Z),
          make_shared<GeoPoint>(point2X, point2Y, point2Z)};
    }
    if (point1Valid) {
      return std::deque<std::shared_ptr<GeoPoint>>{
          make_shared<GeoPoint>(point1X, point1Y, point1Z)};
    }
    if (point2Valid) {
      return std::deque<std::shared_ptr<GeoPoint>>{
          make_shared<GeoPoint>(point2X, point2Y, point2Z)};
    }
    return NO_POINTS;
  } else {
    // No solutions.
    return NO_POINTS;
  }
}

void Plane::findIntersectionBounds(shared_ptr<PlanetModel> planetModel,
                                   shared_ptr<Bounds> boundsInfo,
                                   shared_ptr<Plane> q,
                                   deque<Membership> &bounds)
{
  // System.out.println("Finding intersection bounds");
  // Unnormalized, unchecked...
  constexpr double lineVectorX = y * q->z - z * q->y;
  constexpr double lineVectorY = z * q->x - x * q->z;
  constexpr double lineVectorZ = x * q->y - y * q->x;
  if (abs(lineVectorX) < MINIMUM_RESOLUTION &&
      abs(lineVectorY) < MINIMUM_RESOLUTION &&
      abs(lineVectorZ) < MINIMUM_RESOLUTION) {
    // Degenerate case: parallel planes
    // System.out.println(" planes are parallel - no intersection");
    return;
  }

  // The line will have the equation: A t + A0 = x, B t + B0 = y, C t + C0 = z.
  // We have A, B, and C.  In order to come up with A0, B0, and C0, we need to
  // find a point that is on both planes. To do this, we find the largest deque
  // value (either x, y, or z), and look for a point that solves both plane
  // equations simultaneous.  For example, let's say that the deque is
  // (0.5,0.5,1), and the two plane equations are: 0.7 x + 0.3 y + 0.1 z + 0.0 =
  // 0 and 0.9 x - 0.1 y + 0.2 z + 4.0 = 0 Then we'd pick z = 0, so the
  // equations to solve for x and y would be: 0.7 x + 0.3y = 0.0 0.9 x - 0.1y =
  // -4.0
  // ... which can readily be solved using standard linear algebra.  Generally:
  // Q0 x + R0 y = S0
  // Q1 x + R1 y = S1
  // ... can be solved by Cramer's rule:
  // x = det(S0 R0 / S1 R1) / det(Q0 R0 / Q1 R1)
  // y = det(Q0 S0 / Q1 S1) / det(Q0 R0 / Q1 R1)
  // ... where det( a b / c d ) = ad - bc, so:
  // x = (S0 * R1 - R0 * S1) / (Q0 * R1 - R0 * Q1)
  // y = (Q0 * S1 - S0 * Q1) / (Q0 * R1 - R0 * Q1)
  // We try to maximize the determinant in the denominator
  constexpr double denomYZ = this->y * q->z - this->z * q->y;
  constexpr double denomXZ = this->x * q->z - this->z * q->x;
  constexpr double denomXY = this->x * q->y - this->y * q->x;
  if (abs(denomYZ) >= abs(denomXZ) && abs(denomYZ) >= abs(denomXY)) {
    // System.out.println("X biggest");
    // X is the biggest, so our point will have x0 = 0.0
    if (abs(denomYZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.out.println(" Denominator is zero: no intersection");
      return;
    }
    constexpr double denom = 1.0 / denomYZ;
    // Each value of D really is two values of D.  That makes 4 combinations.
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ, 0.0,
                     (-(this->D + MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D + MINIMUM_RESOLUTION)) *
                         denom,
                     (this->y * -(q->D + MINIMUM_RESOLUTION) +
                      (this->D + MINIMUM_RESOLUTION) * q->y) *
                         denom,
                     {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ, 0.0,
                     (-(this->D - MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D + MINIMUM_RESOLUTION)) *
                         denom,
                     (this->y * -(q->D + MINIMUM_RESOLUTION) +
                      (this->D - MINIMUM_RESOLUTION) * q->y) *
                         denom,
                     {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ, 0.0,
                     (-(this->D + MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D - MINIMUM_RESOLUTION)) *
                         denom,
                     (this->y * -(q->D - MINIMUM_RESOLUTION) +
                      (this->D + MINIMUM_RESOLUTION) * q->y) *
                         denom,
                     {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ, 0.0,
                     (-(this->D - MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D - MINIMUM_RESOLUTION)) *
                         denom,
                     (this->y * -(q->D - MINIMUM_RESOLUTION) +
                      (this->D - MINIMUM_RESOLUTION) * q->y) *
                         denom,
                     {bounds});
  } else if (abs(denomXZ) >= abs(denomXY) && abs(denomXZ) >= abs(denomYZ)) {
    // System.out.println("Y biggest");
    // Y is the biggest, so y0 = 0.0
    if (abs(denomXZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.out.println(" Denominator is zero: no intersection");
      return;
    }
    constexpr double denom = 1.0 / denomXZ;
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D + MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D + MINIMUM_RESOLUTION)) *
                         denom,
                     0.0,
                     (this->x * -(q->D + MINIMUM_RESOLUTION) +
                      (this->D + MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D - MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D + MINIMUM_RESOLUTION)) *
                         denom,
                     0.0,
                     (this->x * -(q->D + MINIMUM_RESOLUTION) +
                      (this->D - MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D + MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D - MINIMUM_RESOLUTION)) *
                         denom,
                     0.0,
                     (this->x * -(q->D - MINIMUM_RESOLUTION) +
                      (this->D + MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D - MINIMUM_RESOLUTION) * q->z -
                      this->z * -(q->D - MINIMUM_RESOLUTION)) *
                         denom,
                     0.0,
                     (this->x * -(q->D - MINIMUM_RESOLUTION) +
                      (this->D - MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     {bounds});
  } else {
    // System.out.println("Z biggest");
    // Z is the biggest, so Z0 = 0.0
    if (abs(denomXY) < MINIMUM_RESOLUTION_SQUARED) {
      // System.out.println(" Denominator is zero: no intersection");
      return;
    }
    constexpr double denom = 1.0 / denomXY;
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D + MINIMUM_RESOLUTION) * q->y -
                      this->y * -(q->D + MINIMUM_RESOLUTION)) *
                         denom,
                     (this->x * -(q->D + MINIMUM_RESOLUTION) +
                      (this->D + MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     0.0, {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D - MINIMUM_RESOLUTION) * q->y -
                      this->y * -(q->D + MINIMUM_RESOLUTION)) *
                         denom,
                     (this->x * -(q->D + MINIMUM_RESOLUTION) +
                      (this->D - MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     0.0, {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D + MINIMUM_RESOLUTION) * q->y -
                      this->y * -(q->D - MINIMUM_RESOLUTION)) *
                         denom,
                     (this->x * -(q->D - MINIMUM_RESOLUTION) +
                      (this->D + MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     0.0, {bounds});
    recordLineBounds(planetModel, boundsInfo, lineVectorX, lineVectorY,
                     lineVectorZ,
                     (-(this->D - MINIMUM_RESOLUTION) * q->y -
                      this->y * -(q->D - MINIMUM_RESOLUTION)) *
                         denom,
                     (this->x * -(q->D - MINIMUM_RESOLUTION) +
                      (this->D - MINIMUM_RESOLUTION) * q->x) *
                         denom,
                     0.0, {bounds});
  }
}

void Plane::recordLineBounds(shared_ptr<PlanetModel> planetModel,
                             shared_ptr<Bounds> boundsInfo,
                             double const lineVectorX, double const lineVectorY,
                             double const lineVectorZ, double const x0,
                             double const y0, double const z0,
                             deque<Membership> &bounds)
{
  // Once an intersecting line is determined, the next step is to intersect that
  // line with the ellipsoid, which will yield zero, one, or two points. The
  // ellipsoid equation: 1,0 = x^2/a^2 + y^2/b^2 + z^2/c^2 1.0 = (At+A0)^2/a^2 +
  // (Bt+B0)^2/b^2 + (Ct+C0)^2/c^2 A^2 t^2 / a^2 + 2AA0t / a^2 + A0^2 / a^2 +
  // B^2 t^2 / b^2 + 2BB0t / b^2 + B0^2 / b^2 + C^2 t^2 / c^2 + 2CC0t / c^2 +
  // C0^2 / c^2  - 1,0 = 0.0 [A^2 / a^2 + B^2 / b^2 + C^2 / c^2] t^2 + [2AA0 /
  // a^2 + 2BB0 / b^2 + 2CC0 / c^2] t + [A0^2 / a^2 + B0^2 / b^2 + C0^2 / c^2 -
  // 1,0] = 0.0 Use the quadratic formula to determine t values and candidate
  // point(s)
  constexpr double A =
      lineVectorX * lineVectorX * planetModel->inverseAbSquared +
      lineVectorY * lineVectorY * planetModel->inverseAbSquared +
      lineVectorZ * lineVectorZ * planetModel->inverseCSquared;
  constexpr double B = 2.0 * (lineVectorX * x0 * planetModel->inverseAbSquared +
                              lineVectorY * y0 * planetModel->inverseAbSquared +
                              lineVectorZ * z0 * planetModel->inverseCSquared);
  constexpr double C = x0 * x0 * planetModel->inverseAbSquared +
                       y0 * y0 * planetModel->inverseAbSquared +
                       z0 * z0 * planetModel->inverseCSquared - 1.0;

  constexpr double BsquaredMinus = B * B - 4.0 * A * C;
  if (abs(BsquaredMinus) < MINIMUM_RESOLUTION_SQUARED) {
    // System.err.println(" One point of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // One solution only
    constexpr double t = -B * inverse2A;
    // Maybe we can save ourselves the cost of construction of a point?
    constexpr double pointX = lineVectorX * t + x0;
    constexpr double pointY = lineVectorY * t + y0;
    constexpr double pointZ = lineVectorZ * t + z0;
    for (shared_ptr<Membership> bound : bounds) {
      if (!bound->isWithin(pointX, pointY, pointZ)) {
        return;
      }
    }
    boundsInfo->addPoint(make_shared<GeoPoint>(pointX, pointY, pointZ));
  } else if (BsquaredMinus > 0.0) {
    // System.err.println(" Two points of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // Two solutions
    constexpr double sqrtTerm = sqrt(BsquaredMinus);
    constexpr double t1 = (-B + sqrtTerm) * inverse2A;
    constexpr double t2 = (-B - sqrtTerm) * inverse2A;
    // Up to two points being returned.  Do what we can to save on object
    // creation though.
    constexpr double point1X = lineVectorX * t1 + x0;
    constexpr double point1Y = lineVectorY * t1 + y0;
    constexpr double point1Z = lineVectorZ * t1 + z0;
    constexpr double point2X = lineVectorX * t2 + x0;
    constexpr double point2Y = lineVectorY * t2 + y0;
    constexpr double point2Z = lineVectorZ * t2 + z0;
    bool point1Valid = true;
    bool point2Valid = true;
    for (shared_ptr<Membership> bound : bounds) {
      if (!bound->isWithin(point1X, point1Y, point1Z)) {
        point1Valid = false;
        break;
      }
    }
    for (shared_ptr<Membership> bound : bounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        point2Valid = false;
        break;
      }
    }

    if (point1Valid) {
      boundsInfo->addPoint(make_shared<GeoPoint>(point1X, point1Y, point1Z));
    }
    if (point2Valid) {
      boundsInfo->addPoint(make_shared<GeoPoint>(point2X, point2Y, point2Z));
    }
  } else {
    // If we can't intersect line with world, then it's outside the world, so
    // we have to assume everything is included.
    boundsInfo->noBound(planetModel);
  }
}

void Plane::recordBounds(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<XYZBounds> boundsInfo, shared_ptr<Plane> p,
                         deque<Membership> &bounds)
{
  findIntersectionBounds(planetModel, boundsInfo, p, {bounds});
}

void Plane::recordBounds(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<XYZBounds> boundsInfo,
                         deque<Membership> &bounds)
{
  // Basic plan is to do three intersections of the plane and the planet.
  // For min/max x, we intersect a vertical plane such that y = 0.
  // For min/max y, we intersect a vertical plane such that x = 0.
  // For min/max z, we intersect a vertical plane that is chosen to go through
  // the high point of the arc. For clarity, load local variables with good
  // names
  constexpr double A = this->x;
  constexpr double B = this->y;
  constexpr double C = this->z;

  // Do Z.  This can be done simply because it is symmetrical.
  if (!boundsInfo->isSmallestMinZ(planetModel) ||
      !boundsInfo->isLargestMaxZ(planetModel)) {
    // System.err.println("    computing Z bound");
    // Compute Z bounds for this arc
    // With ellipsoids, we really have only one viable way to do this
    // computation. Specifically, we compute an appropriate vertical plane,
    // based on the current plane's x-y orientation, and then intersect it with
    // this one and with the ellipsoid.  This gives us zero, one, or two points
    // to use as bounds. There is one special case: horizontal circles.  These
    // require TWO vertical planes: one for the x, and one for the y, and we use
    // all four resulting points in the bounds computation.
    if ((abs(A) >= MINIMUM_RESOLUTION || abs(B) >= MINIMUM_RESOLUTION)) {
      // NOT a degenerate case
      // System.err.println("    not degenerate");
      shared_ptr<Plane> *const normalizedZPlane =
          constructNormalizedZPlane({A, B});
      std::deque<std::shared_ptr<GeoPoint>> points =
          findIntersections(planetModel, normalizedZPlane, {bounds, NO_BOUNDS});
      for (auto point : points) {
        assert(planetModel->pointOnSurface(point));
        // System.err.println("      Point = "+point+";
        // this.evaluate(point)="+this.evaluate(point)+";
        // normalizedZPlane.evaluate(point)="+normalizedZPlane.evaluate(point));
        addPoint(boundsInfo, bounds, point);
      }
    } else {
      // Since a==b==0, any plane including the Z axis suffices.
      // System.err.println("      Perpendicular to z");
      std::deque<std::shared_ptr<GeoPoint>> points =
          findIntersections(planetModel, normalYPlane, {NO_BOUNDS, NO_BOUNDS});
      if (points.size() > 0) {
        boundsInfo->addZValue(points[0]);
      }
    }
  }

  // First, compute common subexpressions
  constexpr double k =
      1.0 / ((x * x + y * y) * planetModel->ab * planetModel->ab +
             z * z * planetModel->c * planetModel->c);
  constexpr double abSquared = planetModel->ab * planetModel->ab;
  constexpr double cSquared = planetModel->c * planetModel->c;
  constexpr double ASquared = A * A;
  constexpr double BSquared = B * B;
  constexpr double CSquared = C * C;

  constexpr double r = 2.0 * D * k;
  constexpr double rSquared = r * r;

  if (!boundsInfo->isSmallestMinX(planetModel) ||
      !boundsInfo->isLargestMaxX(planetModel)) {
    // For min/max x, we need to use lagrange multipliers.
    //
    // For this, we need grad(F(x,y,z)) = (dF/dx, dF/dy, dF/dz).
    //
    // Minimize and maximize f(x,y,z) = x, with respect to g(x,y,z) = Ax + By +
    // Cz - D and h(x,y,z) = x^2/ab^2 + y^2/ab^2 + z^2/c^2 - 1
    //
    // grad(f(x,y,z)) = (1,0,0)
    // grad(g(x,y,z)) = (A,B,C)
    // grad(h(x,y,z)) = (2x/ab^2,2y/ab^2,2z/c^2)
    //
    // Equations we need to simultaneously solve:
    //
    // grad(f(x,y,z)) = l * grad(g(x,y,z)) + m * grad(h(x,y,z))
    // g(x,y,z) = 0
    // h(x,y,z) = 0
    //
    // Equations:
    // 1 = l*A + m*2x/ab^2
    // 0 = l*B + m*2y/ab^2
    // 0 = l*C + m*2z/c^2
    // Ax + By + Cz + D = 0
    // x^2/ab^2 + y^2/ab^2 + z^2/c^2 - 1 = 0
    //
    // Solve for x,y,z in terms of (l, m):
    //
    // x = ((1 - l*A) * ab^2 ) / (2 * m)
    // y = (-l*B * ab^2) / ( 2 * m)
    // z = (-l*C * c^2)/ (2 * m)
    //
    // Two equations, two unknowns:
    //
    // A * (((1 - l*A) * ab^2 ) / (2 * m)) + B * ((-l*B * ab^2) / ( 2 * m)) + C
    // * ((-l*C * c^2)/ (2 * m)) + D = 0
    //
    // and
    //
    // (((1 - l*A) * ab^2 ) / (2 * m))^2/ab^2 + ((-l*B * ab^2) / ( 2 *
    // m))^2/ab^2 + ((-l*C * c^2)/ (2 * m))^2/c^2 - 1 = 0
    //
    // Simple: solve for l and m, then find x from it.
    //
    // (a) Use first equation to find l in terms of m.
    //
    // A * (((1 - l*A) * ab^2 ) / (2 * m)) + B * ((-l*B * ab^2) / ( 2 * m)) + C
    // * ((-l*C * c^2)/ (2 * m)) + D = 0 A * ((1 - l*A) * ab^2 ) + B * (-l*B *
    // ab^2) + C * (-l*C * c^2) + D * 2 * m = 0 A * ab^2 - l*A^2* ab^2 - B^2 * l
    // * ab^2 - C^2 * l * c^2 + D * 2 * m = 0
    // - l *(A^2* ab^2 + B^2 * ab^2 + C^2 * c^2) + (A * ab^2 + D * 2 * m) = 0
    // l = (A * ab^2 + D * 2 * m) / (A^2* ab^2 + B^2 * ab^2 + C^2 * c^2)
    // l = A * ab^2 / (A^2* ab^2 + B^2 * ab^2 + C^2 * c^2) + m * 2 * D / (A^2*
    // ab^2 + B^2 * ab^2 + C^2 * c^2)
    //
    // For convenience:
    //
    // k = 1.0 / (A^2* ab^2 + B^2 * ab^2 + C^2 * c^2)
    //
    // Then:
    //
    // l = A * ab^2 * k + m * 2 * D * k
    // l = k * (A*ab^2 + m*2*D)
    //
    // For further convenience:
    //
    // q = A*ab^2*k
    // r = 2*D*k
    //
    // l = (r*m + q)
    // l^2 = (r^2 * m^2 + 2*r*m*q + q^2)
    //
    // (b) Simplify the second equation before substitution
    //
    // (((1 - l*A) * ab^2 ) / (2 * m))^2/ab^2 + ((-l*B * ab^2) / ( 2 *
    // m))^2/ab^2 + ((-l*C * c^2)/ (2 * m))^2/c^2 - 1 = 0
    // ((1 - l*A) * ab^2 )^2/ab^2 + (-l*B * ab^2)^2/ab^2 + (-l*C * c^2)^2/c^2 =
    // 4 * m^2 (1 - l*A)^2 * ab^2 + (-l*B)^2 * ab^2 + (-l*C)^2 * c^2 = 4 * m^2
    // (1 - 2*l*A + l^2*A^2) * ab^2 + l^2*B^2 * ab^2 + l^2*C^2 * c^2 = 4 * m^2
    // ab^2 - 2*A*ab^2*l + A^2*ab^2*l^2 + B^2*ab^2*l^2 + C^2*c^2*l^2 - 4*m^2 = 0
    //
    // (c) Substitute for l, l^2
    //
    // ab^2 - 2*A*ab^2*(r*m + q) + A^2*ab^2*(r^2 * m^2 + 2*r*m*q + q^2) +
    // B^2*ab^2*(r^2 * m^2 + 2*r*m*q + q^2) + C^2*c^2*(r^2 * m^2 + 2*r*m*q +
    // q^2) - 4*m^2 = 0 ab^2 - 2*A*ab^2*r*m - 2*A*ab^2*q + A^2*ab^2*r^2*m^2 +
    // 2*A^2*ab^2*r*q*m +
    //        A^2*ab^2*q^2 + B^2*ab^2*r^2*m^2 + 2*B^2*ab^2*r*q*m + B^2*ab^2*q^2
    //        + C^2*c^2*r^2*m^2 + 2*C^2*c^2*r*q*m + C^2*c^2*q^2 - 4*m^2 = 0
    //
    // (d) Group
    //
    // m^2 * [A^2*ab^2*r^2 + B^2*ab^2*r^2 + C^2*c^2*r^2 - 4] +
    // m * [- 2*A*ab^2*r + 2*A^2*ab^2*r*q + 2*B^2*ab^2*r*q + 2*C^2*c^2*r*q] +
    // [ab^2 - 2*A*ab^2*q + A^2*ab^2*q^2 + B^2*ab^2*q^2 + C^2*c^2*q^2]  =  0

    // Useful subexpressions for this bound
    constexpr double q = A * abSquared * k;
    constexpr double qSquared = q * q;

    // Quadratic equation
    constexpr double a = ASquared * abSquared * rSquared +
                         BSquared * abSquared * rSquared +
                         CSquared * cSquared * rSquared - 4.0;
    constexpr double b =
        -2.0 * A * abSquared * r + 2.0 * ASquared * abSquared * r * q +
        2.0 * BSquared * abSquared * r * q + 2.0 * CSquared * cSquared * r * q;
    constexpr double c =
        abSquared - 2.0 * A * abSquared * q + ASquared * abSquared * qSquared +
        BSquared * abSquared * qSquared + CSquared * cSquared * qSquared;

    if (abs(a) >= MINIMUM_RESOLUTION_SQUARED) {
      constexpr double sqrtTerm = b * b - 4.0 * a * c;
      if (abs(sqrtTerm) < MINIMUM_RESOLUTION_SQUARED) {
        // One solution
        constexpr double m = -b / (2.0 * a);
        // Valid?
        if (abs(m) >= MINIMUM_RESOLUTION) {
          constexpr double l = r * m + q;
          // x = ((1 - l*A) * ab^2 ) / (2 * m)
          // y = (-l*B * ab^2) / ( 2 * m)
          // z = (-l*C * c^2)/ (2 * m)
          constexpr double denom0 = 0.5 / m;
          shared_ptr<GeoPoint> *const thePoint = make_shared<GeoPoint>(
              (1.0 - l * A) * abSquared * denom0, -l * B * abSquared * denom0,
              -l * C * cSquared * denom0);
          // Math is not quite accurate enough for this
          // assert planetModel.pointOnSurface(thePoint): "Point: "+thePoint+";
          // Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+" D="+D+"
          // planetfcn="+
          //  (thePoint.x*thePoint.x*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint.y*thePoint.y*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint.z*thePoint.z*planetModel.inverseC*planetModel.inverseC);
          // assert evaluateIsZero(thePoint): "Evaluation of point:
          // "+evaluate(thePoint);
          addPoint(boundsInfo, bounds, thePoint);
        } else {
          // This is a plane of the form A=n B=0 C=0.  We can set a bound only
          // by noting the D value.
          boundsInfo->addXValue(-D / A);
        }
      } else if (sqrtTerm > 0.0) {
        // Two solutions
        constexpr double sqrtResult = sqrt(sqrtTerm);
        constexpr double commonDenom = 0.5 / a;
        constexpr double m1 = (-b + sqrtResult) * commonDenom;
        assert(abs(a * m1 * m1 + b * m1 + c) < MINIMUM_RESOLUTION);
        constexpr double m2 = (-b - sqrtResult) * commonDenom;
        assert(abs(a * m2 * m2 + b * m2 + c) < MINIMUM_RESOLUTION);
        if (abs(m1) >= MINIMUM_RESOLUTION || abs(m2) >= MINIMUM_RESOLUTION) {
          constexpr double l1 = r * m1 + q;
          constexpr double l2 = r * m2 + q;
          // x = ((1 - l*A) * ab^2 ) / (2 * m)
          // y = (-l*B * ab^2) / ( 2 * m)
          // z = (-l*C * c^2)/ (2 * m)
          constexpr double denom1 = 0.5 / m1;
          constexpr double denom2 = 0.5 / m2;
          shared_ptr<GeoPoint> *const thePoint1 = make_shared<GeoPoint>(
              (1.0 - l1 * A) * abSquared * denom1, -l1 * B * abSquared * denom1,
              -l1 * C * cSquared * denom1);
          shared_ptr<GeoPoint> *const thePoint2 = make_shared<GeoPoint>(
              (1.0 - l2 * A) * abSquared * denom2, -l2 * B * abSquared * denom2,
              -l2 * C * cSquared * denom2);
          // Math is not quite accurate enough for this
          // assert planetModel.pointOnSurface(thePoint1): "Point1:
          // "+thePoint1+"; Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+"
          // D="+D+" planetfcn="+
          //  (thePoint1.x*thePoint1.x*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint1.y*thePoint1.y*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint1.z*thePoint1.z*planetModel.inverseC*planetModel.inverseC);
          // assert planetModel.pointOnSurface(thePoint2): "Point1:
          // "+thePoint2+"; Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+"
          // D="+D+" planetfcn="+
          //  (thePoint2.x*thePoint2.x*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint2.y*thePoint2.y*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint2.z*thePoint2.z*planetModel.inverseC*planetModel.inverseC);
          // assert evaluateIsZero(thePoint1): "Evaluation of point1:
          // "+evaluate(thePoint1); assert evaluateIsZero(thePoint2): "Evaluation
          // of point2: "+evaluate(thePoint2);
          addPoint(boundsInfo, bounds, thePoint1);
          addPoint(boundsInfo, bounds, thePoint2);
        } else {
          // This is a plane of the form A=n B=0 C=0.  We can set a bound only
          // by noting the D value.
          boundsInfo->addXValue(-D / A);
        }
      } else {
        // No solutions
      }
    } else if (abs(b) > MINIMUM_RESOLUTION_SQUARED) {
      // a = 0, so m = - c / b
      constexpr double m = -c / b;
      constexpr double l = r * m + q;
      // x = ((1 - l*A) * ab^2 ) / (2 * m)
      // y = (-l*B * ab^2) / ( 2 * m)
      // z = (-l*C * c^2)/ (2 * m)
      constexpr double denom0 = 0.5 / m;
      shared_ptr<GeoPoint> *const thePoint = make_shared<GeoPoint>(
          (1.0 - l * A) * abSquared * denom0, -l * B * abSquared * denom0,
          -l * C * cSquared * denom0);
      // Math is not quite accurate enough for this
      // assert planetModel.pointOnSurface(thePoint): "Point: "+thePoint+";
      // Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+" D="+D+"
      // planetfcn="+
      //  (thePoint.x*thePoint.x*planetModel.inverseAb*planetModel.inverseAb +
      //  thePoint.y*thePoint.y*planetModel.inverseAb*planetModel.inverseAb +
      //  thePoint.z*thePoint.z*planetModel.inverseC*planetModel.inverseC);
      // assert evaluateIsZero(thePoint): "Evaluation of point:
      // "+evaluate(thePoint);
      addPoint(boundsInfo, bounds, thePoint);
    } else {
      // Something went very wrong; a = b = 0
    }
  }

  // Do Y
  if (!boundsInfo->isSmallestMinY(planetModel) ||
      !boundsInfo->isLargestMaxY(planetModel)) {
    // For min/max x, we need to use lagrange multipliers.
    //
    // For this, we need grad(F(x,y,z)) = (dF/dx, dF/dy, dF/dz).
    //
    // Minimize and maximize f(x,y,z) = y, with respect to g(x,y,z) = Ax + By +
    // Cz - D and h(x,y,z) = x^2/ab^2 + y^2/ab^2 + z^2/c^2 - 1
    //
    // grad(f(x,y,z)) = (0,1,0)
    // grad(g(x,y,z)) = (A,B,C)
    // grad(h(x,y,z)) = (2x/ab^2,2y/ab^2,2z/c^2)
    //
    // Equations we need to simultaneously solve:
    //
    // grad(f(x,y,z)) = l * grad(g(x,y,z)) + m * grad(h(x,y,z))
    // g(x,y,z) = 0
    // h(x,y,z) = 0
    //
    // Equations:
    // 0 = l*A + m*2x/ab^2
    // 1 = l*B + m*2y/ab^2
    // 0 = l*C + m*2z/c^2
    // Ax + By + Cz + D = 0
    // x^2/ab^2 + y^2/ab^2 + z^2/c^2 - 1 = 0
    //
    // Solve for x,y,z in terms of (l, m):
    //
    // x = (-l*A * ab^2 ) / (2 * m)
    // y = ((1 - l*B) * ab^2) / ( 2 * m)
    // z = (-l*C * c^2)/ (2 * m)
    //
    // Two equations, two unknowns:
    //
    // A * ((-l*A * ab^2 ) / (2 * m)) + B * (((1 - l*B) * ab^2) / ( 2 * m)) + C
    // * ((-l*C * c^2)/ (2 * m)) + D = 0
    //
    // and
    //
    // ((-l*A * ab^2 ) / (2 * m))^2/ab^2 + (((1 - l*B) * ab^2) / ( 2 *
    // m))^2/ab^2 + ((-l*C * c^2)/ (2 * m))^2/c^2 - 1 = 0
    //
    // Simple: solve for l and m, then find y from it.
    //
    // (a) Use first equation to find l in terms of m.
    //
    // A * ((-l*A * ab^2 ) / (2 * m)) + B * (((1 - l*B) * ab^2) / ( 2 * m)) + C
    // * ((-l*C * c^2)/ (2 * m)) + D = 0 A * (-l*A * ab^2 ) + B * ((1-l*B) *
    // ab^2) + C * (-l*C * c^2) + D * 2 * m = 0 -A^2*l*ab^2 + B*ab^2 -
    // l*B^2*ab^2 - C^2*l*c^2 + D*2*m = 0
    // - l *(A^2* ab^2 + B^2 * ab^2 + C^2 * c^2) + (B * ab^2 + D * 2 * m) = 0
    // l = (B * ab^2 + D * 2 * m) / (A^2* ab^2 + B^2 * ab^2 + C^2 * c^2)
    // l = B * ab^2 / (A^2* ab^2 + B^2 * ab^2 + C^2 * c^2) + m * 2 * D / (A^2*
    // ab^2 + B^2 * ab^2 + C^2 * c^2)
    //
    // For convenience:
    //
    // k = 1.0 / (A^2* ab^2 + B^2 * ab^2 + C^2 * c^2)
    //
    // Then:
    //
    // l = B * ab^2 * k + m * 2 * D * k
    // l = k * (B*ab^2 + m*2*D)
    //
    // For further convenience:
    //
    // q = B*ab^2*k
    // r = 2*D*k
    //
    // l = (r*m + q)
    // l^2 = (r^2 * m^2 + 2*r*m*q + q^2)
    //
    // (b) Simplify the second equation before substitution
    //
    // ((-l*A * ab^2 ) / (2 * m))^2/ab^2 + (((1 - l*B) * ab^2) / ( 2 *
    // m))^2/ab^2 + ((-l*C * c^2)/ (2 * m))^2/c^2 - 1 = 0
    // (-l*A * ab^2 )^2/ab^2 + ((1 - l*B) * ab^2)^2/ab^2 + (-l*C * c^2)^2/c^2 =
    // 4 * m^2
    // (-l*A)^2 * ab^2 + (1 - l*B)^2 * ab^2 + (-l*C)^2 * c^2 = 4 * m^2
    // l^2*A^2 * ab^2 + (1 - 2*l*B + l^2*B^2) * ab^2 + l^2*C^2 * c^2 = 4 * m^2
    // A^2*ab^2*l^2 + ab^2 - 2*B*ab^2*l + B^2*ab^2*l^2 + C^2*c^2*l^2 - 4*m^2 = 0
    //
    // (c) Substitute for l, l^2
    //
    // A^2*ab^2*(r^2 * m^2 + 2*r*m*q + q^2) + ab^2 - 2*B*ab^2*(r*m + q) +
    // B^2*ab^2*(r^2 * m^2 + 2*r*m*q + q^2) + C^2*c^2*(r^2 * m^2 + 2*r*m*q +
    // q^2) - 4*m^2 = 0 A^2*ab^2*r^2*m^2 + 2*A^2*ab^2*r*q*m + A^2*ab^2*q^2 +
    // ab^2 - 2*B*ab^2*r*m - 2*B*ab^2*q + B^2*ab^2*r^2*m^2 +
    //    2*B^2*ab^2*r*q*m + B^2*ab^2*q^2 + C^2*c^2*r^2*m^2 + 2*C^2*c^2*r*q*m +
    //    C^2*c^2*q^2 - 4*m^2 = 0
    //
    // (d) Group
    //
    // m^2 * [A^2*ab^2*r^2 + B^2*ab^2*r^2 + C^2*c^2*r^2 - 4] +
    // m * [2*A^2*ab^2*r*q - 2*B*ab^2*r + 2*B^2*ab^2*r*q + 2*C^2*c^2*r*q] +
    // [A^2*ab^2*q^2 + ab^2 - 2*B*ab^2*q + B^2*ab^2*q^2 + C^2*c^2*q^2]  =  0

    // System.err.println("    computing Y bound");

    // Useful subexpressions for this bound
    constexpr double q = B * abSquared * k;
    constexpr double qSquared = q * q;

    // Quadratic equation
    constexpr double a = ASquared * abSquared * rSquared +
                         BSquared * abSquared * rSquared +
                         CSquared * cSquared * rSquared - 4.0;
    constexpr double b =
        2.0 * ASquared * abSquared * r * q - 2.0 * B * abSquared * r +
        2.0 * BSquared * abSquared * r * q + 2.0 * CSquared * cSquared * r * q;
    constexpr double c =
        ASquared * abSquared * qSquared + abSquared - 2.0 * B * abSquared * q +
        BSquared * abSquared * qSquared + CSquared * cSquared * qSquared;

    if (abs(a) >= MINIMUM_RESOLUTION_SQUARED) {
      constexpr double sqrtTerm = b * b - 4.0 * a * c;
      if (abs(sqrtTerm) < MINIMUM_RESOLUTION_SQUARED) {
        // One solution
        constexpr double m = -b / (2.0 * a);
        // Valid?
        if (abs(m) >= MINIMUM_RESOLUTION) {
          constexpr double l = r * m + q;
          // x = (-l*A * ab^2 ) / (2 * m)
          // y = ((1.0-l*B) * ab^2) / ( 2 * m)
          // z = (-l*C * c^2)/ (2 * m)
          constexpr double denom0 = 0.5 / m;
          shared_ptr<GeoPoint> *const thePoint = make_shared<GeoPoint>(
              -l * A * abSquared * denom0, (1.0 - l * B) * abSquared * denom0,
              -l * C * cSquared * denom0);
          // Math is not quite accurate enough for this
          // assert planetModel.pointOnSurface(thePoint): "Point: "+thePoint+";
          // Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+" D="+D+"
          // planetfcn="+
          //  (thePoint1.x*thePoint.x*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint.y*thePoint.y*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint.z*thePoint.z*planetModel.inverseC*planetModel.inverseC);
          // assert evaluateIsZero(thePoint): "Evaluation of point:
          // "+evaluate(thePoint);
          addPoint(boundsInfo, bounds, thePoint);
        } else {
          // This is a plane of the form A=0 B=n C=0.  We can set a bound only
          // by noting the D value.
          boundsInfo->addYValue(-D / B);
        }
      } else if (sqrtTerm > 0.0) {
        // Two solutions
        constexpr double sqrtResult = sqrt(sqrtTerm);
        constexpr double commonDenom = 0.5 / a;
        constexpr double m1 = (-b + sqrtResult) * commonDenom;
        assert(abs(a * m1 * m1 + b * m1 + c) < MINIMUM_RESOLUTION);
        constexpr double m2 = (-b - sqrtResult) * commonDenom;
        assert(abs(a * m2 * m2 + b * m2 + c) < MINIMUM_RESOLUTION);
        if (abs(m1) >= MINIMUM_RESOLUTION || abs(m2) >= MINIMUM_RESOLUTION) {
          constexpr double l1 = r * m1 + q;
          constexpr double l2 = r * m2 + q;
          // x = (-l*A * ab^2 ) / (2 * m)
          // y = ((1.0-l*B) * ab^2) / ( 2 * m)
          // z = (-l*C * c^2)/ (2 * m)
          constexpr double denom1 = 0.5 / m1;
          constexpr double denom2 = 0.5 / m2;
          shared_ptr<GeoPoint> *const thePoint1 = make_shared<GeoPoint>(
              -l1 * A * abSquared * denom1, (1.0 - l1 * B) * abSquared * denom1,
              -l1 * C * cSquared * denom1);
          shared_ptr<GeoPoint> *const thePoint2 = make_shared<GeoPoint>(
              -l2 * A * abSquared * denom2, (1.0 - l2 * B) * abSquared * denom2,
              -l2 * C * cSquared * denom2);
          // Math is not quite accurate enough for this
          // assert planetModel.pointOnSurface(thePoint1): "Point1:
          // "+thePoint1+"; Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+"
          // D="+D+" planetfcn="+
          //  (thePoint1.x*thePoint1.x*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint1.y*thePoint1.y*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint1.z*thePoint1.z*planetModel.inverseC*planetModel.inverseC);
          // assert planetModel.pointOnSurface(thePoint2): "Point2:
          // "+thePoint2+"; Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+"
          // D="+D+" planetfcn="+
          //  (thePoint2.x*thePoint2.x*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint2.y*thePoint2.y*planetModel.inverseAb*planetModel.inverseAb
          //  +
          //  thePoint2.z*thePoint2.z*planetModel.inverseC*planetModel.inverseC);
          // assert evaluateIsZero(thePoint1): "Evaluation of point1:
          // "+evaluate(thePoint1); assert evaluateIsZero(thePoint2): "Evaluation
          // of point2: "+evaluate(thePoint2);
          addPoint(boundsInfo, bounds, thePoint1);
          addPoint(boundsInfo, bounds, thePoint2);
        } else {
          // This is a plane of the form A=0 B=n C=0.  We can set a bound only
          // by noting the D value.
          boundsInfo->addYValue(-D / B);
        }
      } else {
        // No solutions
      }
    } else if (abs(b) > MINIMUM_RESOLUTION_SQUARED) {
      // a = 0, so m = - c / b
      constexpr double m = -c / b;
      constexpr double l = r * m + q;
      // x = ( -l*A * ab^2 ) / (2 * m)
      // y = ((1-l*B) * ab^2) / ( 2 * m)
      // z = (-l*C * c^2)/ (2 * m)
      constexpr double denom0 = 0.5 / m;
      shared_ptr<GeoPoint> *const thePoint = make_shared<GeoPoint>(
          -l * A * abSquared * denom0, (1.0 - l * B) * abSquared * denom0,
          -l * C * cSquared * denom0);
      // Math is not quite accurate enough for this
      // assert planetModel.pointOnSurface(thePoint): "Point: "+thePoint+";
      // Planetmodel="+planetModel+"; A="+A+" B="+B+" C="+C+" D="+D+"
      // planetfcn="+
      //  (thePoint.x*thePoint.x*planetModel.inverseAb*planetModel.inverseAb +
      //  thePoint.y*thePoint.y*planetModel.inverseAb*planetModel.inverseAb +
      //  thePoint.z*thePoint.z*planetModel.inverseC*planetModel.inverseC);
      // assert evaluateIsZero(thePoint): "Evaluation of point:
      // "+evaluate(thePoint);
      addPoint(boundsInfo, bounds, thePoint);
    } else {
      // Something went very wrong; a = b = 0
    }
  }
}

void Plane::recordBounds(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<LatLonBounds> boundsInfo,
                         shared_ptr<Plane> p, deque<Membership> &bounds)
{
  findIntersectionBounds(planetModel, boundsInfo, p, {bounds});
}

void Plane::recordBounds(shared_ptr<PlanetModel> planetModel,
                         shared_ptr<LatLonBounds> boundsInfo,
                         deque<Membership> &bounds)
{
  // For clarity, load local variables with good names
  constexpr double A = this->x;
  constexpr double B = this->y;
  constexpr double C = this->z;

  // Now compute latitude min/max points
  if (!boundsInfo->checkNoTopLatitudeBound() ||
      !boundsInfo->checkNoBottomLatitudeBound()) {
    // System.err.println("Looking at latitude for plane "+this);
    // With ellipsoids, we really have only one viable way to do this
    // computation. Specifically, we compute an appropriate vertical plane,
    // based on the current plane's x-y orientation, and then intersect it with
    // this one and with the ellipsoid.  This gives us zero, one, or two points
    // to use as bounds. There is one special case: horizontal circles.  These
    // require TWO vertical planes: one for the x, and one for the y, and we use
    // all four resulting points in the bounds computation.
    if ((abs(A) >= MINIMUM_RESOLUTION || abs(B) >= MINIMUM_RESOLUTION)) {
      // NOT a horizontal circle!
      // System.err.println(" Not a horizontal circle");
      shared_ptr<Plane> *const verticalPlane =
          constructNormalizedZPlane({A, B});
      std::deque<std::shared_ptr<GeoPoint>> points =
          findIntersections(planetModel, verticalPlane, {bounds, NO_BOUNDS});
      for (auto point : points) {
        addPoint(boundsInfo, bounds, point);
      }
    } else {
      // Horizontal circle.  Since a==b, any vertical plane suffices.
      std::deque<std::shared_ptr<GeoPoint>> points =
          findIntersections(planetModel, normalXPlane, {NO_BOUNDS, NO_BOUNDS});
      boundsInfo->addZValue(points[0]);
    }
    // System.err.println("Done latitude bounds");
  }

  // First, figure out our longitude bounds, unless we no longer need to
  // consider that
  if (!boundsInfo->checkNoLongitudeBound()) {
    // System.err.println("Computing longitude bounds for "+this);
    // System.out.println("A = "+A+" B = "+B+" C = "+C+" D = "+D);
    // Compute longitude bounds

    double a;
    double b;
    double c;

    if (abs(C) < MINIMUM_RESOLUTION) {
      // Degenerate; the equation describes a line
      // System.out.println("It's a zero-width ellipse");
      // Ax + By + D = 0
      if (abs(D) >= MINIMUM_RESOLUTION) {
        if (abs(A) > abs(B)) {
          // Use equation suitable for A != 0
          // We need to find the endpoints of the zero-width ellipse.
          // Geometrically, we have a line segment in x-y space.  We need to
          // locate the endpoints of that line.  But luckily, we know some
          // things: specifically, since it is a degenerate situation in
          // projection, the C value had to have been 0.  That means that our
          // line's endpoints will coincide with the projected ellipse.  All we
          // need to do then is to find the intersection of the projected
          // ellipse and the line equation:
          //
          // A x + B y + D = 0
          //
          // Since A != 0:
          // x = (-By - D)/A
          //
          // The projected ellipse:
          // x^2/a^2 + y^2/b^2 - 1 = 0
          // Substitute:
          // [(-By-D)/A]^2/a^2 + y^2/b^2 -1 = 0
          // Multiply through by A^2:
          // [-By - D]^2/a^2 + A^2*y^2/b^2 - A^2 = 0
          // Multiply out:
          // B^2*y^2/a^2 + 2BDy/a^2 + D^2/a^2 + A^2*y^2/b^2 - A^2 = 0
          // Group:
          // y^2 * [B^2/a^2 + A^2/b^2] + y [2BD/a^2] + [D^2/a^2-A^2] = 0

          a = B * B * planetModel->inverseAbSquared +
              A * A * planetModel->inverseAbSquared;
          b = 2.0 * B * D * planetModel->inverseAbSquared;
          c = D * D * planetModel->inverseAbSquared - A * A;

          double sqrtClause = b * b - 4.0 * a * c;

          if (abs(sqrtClause) < MINIMUM_RESOLUTION_SQUARED) {
            double y0 = -b / (2.0 * a);
            double x0 = (-D - B * y0) / A;
            double z0 = 0.0;
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0, y0, z0));
          } else if (sqrtClause > 0.0) {
            double sqrtResult = sqrt(sqrtClause);
            double denom = 1.0 / (2.0 * a);
            double Hdenom = 1.0 / A;

            double y0a = (-b + sqrtResult) * denom;
            double y0b = (-b - sqrtResult) * denom;

            double x0a = (-D - B * y0a) * Hdenom;
            double x0b = (-D - B * y0b) * Hdenom;

            double z0a = 0.0;
            double z0b = 0.0;

            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0a, y0a, z0a));
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0b, y0b, z0b));
          }

        } else {
          // Use equation suitable for B != 0
          // Since I != 0, we rewrite:
          // y = (-Ax - D)/B
          a = B * B * planetModel->inverseAbSquared +
              A * A * planetModel->inverseAbSquared;
          b = 2.0 * A * D * planetModel->inverseAbSquared;
          c = D * D * planetModel->inverseAbSquared - B * B;

          double sqrtClause = b * b - 4.0 * a * c;

          if (abs(sqrtClause) < MINIMUM_RESOLUTION_SQUARED) {
            double x0 = -b / (2.0 * a);
            double y0 = (-D - A * x0) / B;
            double z0 = 0.0;
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0, y0, z0));
          } else if (sqrtClause > 0.0) {
            double sqrtResult = sqrt(sqrtClause);
            double denom = 1.0 / (2.0 * a);
            double Idenom = 1.0 / B;

            double x0a = (-b + sqrtResult) * denom;
            double x0b = (-b - sqrtResult) * denom;
            double y0a = (-D - A * x0a) * Idenom;
            double y0b = (-D - A * x0b) * Idenom;
            double z0a = 0.0;
            double z0b = 0.0;

            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0a, y0a, z0a));
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0b, y0b, z0b));
          }
        }
      }

    } else {
      // System.err.println("General longitude bounds...");

      // NOTE WELL: The x,y,z values generated here are NOT on the unit sphere.
      // They are for lat/lon calculation purposes only.  x-y is meant to be
      // used for longitude determination, and z for latitude, and that's all
      // the values are good for.

      // (1) Intersect the plane and the ellipsoid, and project the results into
      // the x-y plane: From plane: z = (-Ax - By - D) / C From ellipsoid:
      // x^2/a^2 + y^2/b^2 + [(-Ax - By - D) / C]^2/c^2 = 1
      // Simplify/expand:
      // C^2*x^2/a^2 + C^2*y^2/b^2 + (-Ax - By - D)^2/c^2 = C^2
      //
      // x^2 * C^2/a^2 + y^2 * C^2/b^2 + x^2 * A^2/c^2 + ABxy/c^2 + ADx/c^2 +
      // ABxy/c^2 + y^2 * B^2/c^2 + BDy/c^2 + ADx/c^2 + BDy/c^2 + D^2/c^2 = C^2
      // Group:
      // [A^2/c^2 + C^2/a^2] x^2 + [B^2/c^2 + C^2/b^2] y^2 + [2AB/c^2]xy +
      // [2AD/c^2]x + [2BD/c^2]y + [D^2/c^2-C^2] = 0 For convenience, introduce
      // post-projection coefficient variables to make life easier. E x^2 + F
      // y^2 + G xy + H x + I y + J = 0
      double E = A * A * planetModel->inverseCSquared +
                 C * C * planetModel->inverseAbSquared;
      double F = B * B * planetModel->inverseCSquared +
                 C * C * planetModel->inverseAbSquared;
      double G = 2.0 * A * B * planetModel->inverseCSquared;
      double H = 2.0 * A * D * planetModel->inverseCSquared;
      double I = 2.0 * B * D * planetModel->inverseCSquared;
      double J = D * D * planetModel->inverseCSquared - C * C;

      // System.err.println("E = " + E + " F = " + F + " G = " + G + " H = "+ H
      // + " I = " + I + " J = " + J);

      // Check if the origin is within, by substituting x = 0, y = 0 and seeing
      // if less than zero
      if (abs(J) >= MINIMUM_RESOLUTION && J > 0.0) {
        // The derivative of the curve above is:
        // 2Exdx + 2Fydy + G(xdy+ydx) + Hdx + Idy = 0
        // (2Ex + Gy + H)dx + (2Fy + Gx + I)dy = 0
        // dy/dx = - (2Ex + Gy + H) / (2Fy + Gx + I)
        //
        // The equation of a line going through the origin with the slope dy/dx
        // is: y = dy/dx x y = - (2Ex + Gy + H) / (2Fy + Gx + I)  x Rearrange:
        // (2Fy + Gx + I) y + (2Ex + Gy + H) x = 0
        // 2Fy^2 + Gxy + Iy + 2Ex^2 + Gxy + Hx = 0
        // 2Ex^2 + 2Fy^2 + 2Gxy + Hx + Iy = 0
        //
        // Multiply the original equation by 2:
        // 2E x^2 + 2F y^2 + 2G xy + 2H x + 2I y + 2J = 0
        // Subtract one from the other, to remove the high-order terms:
        // Hx + Iy + 2J = 0
        // Now, we can substitute either x = or y = into the derivative
        // equation, or into the original equation. But we will need to base
        // this on which coefficient is non-zero

        if (abs(H) > abs(I)) {
          // System.err.println(" Using the y quadratic");
          // x = (-2J - Iy)/H

          // Plug into the original equation:
          // E [(-2J - Iy)/H]^2 + F y^2 + G [(-2J - Iy)/H]y + H [(-2J - Iy)/H] +
          // I y + J = 0 E [(-2J - Iy)/H]^2 + F y^2 + G [(-2J - Iy)/H]y - J = 0
          // Same equation as derivative equation, except for a factor of 2!  So
          // it doesn't matter which we pick.

          // Plug into derivative equation:
          // 2E[(-2J - Iy)/H]^2 + 2Fy^2 + 2G[(-2J - Iy)/H]y + H[(-2J - Iy)/H] +
          // Iy = 0 2E[(-2J - Iy)/H]^2 + 2Fy^2 + 2G[(-2J - Iy)/H]y - 2J = 0
          // E[(-2J - Iy)/H]^2 + Fy^2 + G[(-2J - Iy)/H]y - J = 0

          // Multiply by H^2 to make manipulation easier
          // E[(-2J - Iy)]^2 + F*H^2*y^2 + GH[(-2J - Iy)]y - J*H^2 = 0
          // Do the square
          // E[4J^2 + 4IJy + I^2*y^2] + F*H^2*y^2 + GH(-2Jy - I*y^2) - J*H^2 = 0

          // Multiply it out
          // 4E*J^2 + 4EIJy + E*I^2*y^2 + H^2*Fy^2 - 2GHJy - GH*I*y^2 - J*H^2 =
          // 0 Group: y^2 [E*I^2 - GH*I + F*H^2] + y [4EIJ - 2GHJ] + [4E*J^2 -
          // J*H^2] = 0

          a = E * I * I - G * H * I + F * H * H;
          b = 4.0 * E * I * J - 2.0 * G * H * J;
          c = 4.0 * E * J * J - J * H * H;

          // System.out.println("a="+a+" b="+b+" c="+c);
          double sqrtClause = b * b - 4.0 * a * c;
          // System.out.println("sqrtClause="+sqrtClause);

          if (abs(sqrtClause) < MINIMUM_RESOLUTION_CUBED) {
            // System.err.println(" One solution");
            double y0 = -b / (2.0 * a);
            double x0 = (-2.0 * J - I * y0) / H;
            double z0 = (-A * x0 - B * y0 - D) / C;

            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0, y0, z0));
          } else if (sqrtClause > 0.0) {
            // System.err.println(" Two solutions");
            double sqrtResult = sqrt(sqrtClause);
            double denom = 1.0 / (2.0 * a);
            double Hdenom = 1.0 / H;
            double Cdenom = 1.0 / C;

            double y0a = (-b + sqrtResult) * denom;
            double y0b = (-b - sqrtResult) * denom;
            double x0a = (-2.0 * J - I * y0a) * Hdenom;
            double x0b = (-2.0 * J - I * y0b) * Hdenom;
            double z0a = (-A * x0a - B * y0a - D) * Cdenom;
            double z0b = (-A * x0b - B * y0b - D) * Cdenom;

            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0a, y0a, z0a));
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0b, y0b, z0b));
          }

        } else {
          // System.err.println(" Using the x quadratic");
          // y = (-2J - Hx)/I

          // Plug into the original equation:
          // E x^2 + F [(-2J - Hx)/I]^2 + G x[(-2J - Hx)/I] - J = 0

          // Multiply by I^2 to make manipulation easier
          // E * I^2 * x^2 + F [(-2J - Hx)]^2 + GIx[(-2J - Hx)] - J * I^2 = 0
          // Do the square
          // E * I^2 * x^2 + F [ 4J^2 + 4JHx + H^2*x^2] + GI[(-2Jx - H*x^2)] - J
          // * I^2 = 0

          // Multiply it out
          // E * I^2 * x^2 + 4FJ^2 + 4FJHx + F*H^2*x^2 - 2GIJx - HGI*x^2 - J *
          // I^2 = 0 Group: x^2 [E*I^2 - GHI + F*H^2] + x [4FJH - 2GIJ] + [4FJ^2
          // - J*I^2] = 0

          // E x^2 + F y^2 + G xy + H x + I y + J = 0

          a = E * I * I - G * H * I + F * H * H;
          b = 4.0 * F * H * J - 2.0 * G * I * J;
          c = 4.0 * F * J * J - J * I * I;

          // System.out.println("a="+a+" b="+b+" c="+c);
          double sqrtClause = b * b - 4.0 * a * c;
          // System.out.println("sqrtClause="+sqrtClause);
          if (abs(sqrtClause) < MINIMUM_RESOLUTION_CUBED) {
            // System.err.println(" One solution; sqrt clause was "+sqrtClause);
            double x0 = -b / (2.0 * a);
            double y0 = (-2.0 * J - H * x0) / I;
            double z0 = (-A * x0 - B * y0 - D) / C;
            // Verify that x&y fulfill the equation
            // 2Ex^2 + 2Fy^2 + 2Gxy + Hx + Iy = 0
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0, y0, z0));
          } else if (sqrtClause > 0.0) {
            // System.err.println(" Two solutions");
            double sqrtResult = sqrt(sqrtClause);
            double denom = 1.0 / (2.0 * a);
            double Idenom = 1.0 / I;
            double Cdenom = 1.0 / C;

            double x0a = (-b + sqrtResult) * denom;
            double x0b = (-b - sqrtResult) * denom;
            double y0a = (-2.0 * J - H * x0a) * Idenom;
            double y0b = (-2.0 * J - H * x0b) * Idenom;
            double z0a = (-A * x0a - B * y0a - D) * Cdenom;
            double z0b = (-A * x0b - B * y0b - D) * Cdenom;

            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0a, y0a, z0a));
            addPoint(boundsInfo, bounds, make_shared<GeoPoint>(x0b, y0b, z0b));
          }
        }
      }
    }
  }
}

void Plane::addPoint(shared_ptr<Bounds> boundsInfo,
                     std::deque<std::shared_ptr<Membership>> &bounds,
                     shared_ptr<GeoPoint> point)
{
  // Make sure the discovered point is within the bounds
  for (auto bound : bounds) {
    if (!bound->isWithin(point)) {
      return;
    }
  }
  // Add the point
  boundsInfo->addPoint(point);
}

bool Plane::intersects(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> q,
    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
    std::deque<std::shared_ptr<GeoPoint>> &moreNotablePoints,
    std::deque<std::shared_ptr<Membership>> &bounds,
    deque<Membership> &moreBounds)
{
  // System.err.println("Does plane "+this+" intersect with plane "+q);
  // If the two planes are identical, then the math will find no points of
  // intersection. So a special case of this is to check for plane equality. But
  // that is not enough, because what we really need at that point is to
  // determine whether overlap occurs between the two parts of the intersection
  // of plane and circle.  That is, are there *any* points on the plane that are
  // within the bounds described?
  if (isNumericallyIdentical(q)) {
    // System.err.println(" Identical plane");
    // The only way to efficiently figure this out will be to have a deque of
    // trial points available to evaluate. We look for any point that fulfills
    // all the bounds.
    for (auto p : notablePoints) {
      if (meetsAllBounds(p, bounds, moreBounds)) {
        // System.err.println("  found a notable point in bounds, so
        // intersects");
        return true;
      }
    }
    for (auto p : moreNotablePoints) {
      if (meetsAllBounds(p, bounds, moreBounds)) {
        // System.err.println("  found a notable point in bounds, so
        // intersects");
        return true;
      }
    }
    // System.err.println("  no notable points inside found; no intersection");
    return false;
  }

  // Save on allocations; do inline instead of calling findIntersections
  // System.err.println("Looking for intersection between plane "+this+" and
  // plane "+q+" within bounds");
  // Unnormalized, unchecked...
  constexpr double lineVectorX = y * q->z - z * q->y;
  constexpr double lineVectorY = z * q->x - x * q->z;
  constexpr double lineVectorZ = x * q->y - y * q->x;

  if (abs(lineVectorX) < MINIMUM_RESOLUTION &&
      abs(lineVectorY) < MINIMUM_RESOLUTION &&
      abs(lineVectorZ) < MINIMUM_RESOLUTION) {
    // Degenerate case: parallel planes
    // System.err.println(" planes are parallel - no intersection");
    return false;
  }

  // The line will have the equation: A t + A0 = x, B t + B0 = y, C t + C0 = z.
  // We have A, B, and C.  In order to come up with A0, B0, and C0, we need to
  // find a point that is on both planes. To do this, we find the largest deque
  // value (either x, y, or z), and look for a point that solves both plane
  // equations simultaneous.  For example, let's say that the deque is
  // (0.5,0.5,1), and the two plane equations are: 0.7 x + 0.3 y + 0.1 z + 0.0 =
  // 0 and 0.9 x - 0.1 y + 0.2 z + 4.0 = 0 Then we'd pick z = 0, so the
  // equations to solve for x and y would be: 0.7 x + 0.3y = 0.0 0.9 x - 0.1y =
  // -4.0
  // ... which can readily be solved using standard linear algebra.  Generally:
  // Q0 x + R0 y = S0
  // Q1 x + R1 y = S1
  // ... can be solved by Cramer's rule:
  // x = det(S0 R0 / S1 R1) / det(Q0 R0 / Q1 R1)
  // y = det(Q0 S0 / Q1 S1) / det(Q0 R0 / Q1 R1)
  // ... where det( a b / c d ) = ad - bc, so:
  // x = (S0 * R1 - R0 * S1) / (Q0 * R1 - R0 * Q1)
  // y = (Q0 * S1 - S0 * Q1) / (Q0 * R1 - R0 * Q1)
  double x0;
  double y0;
  double z0;
  // We try to maximize the determinant in the denominator
  constexpr double denomYZ = this->y * q->z - this->z * q->y;
  constexpr double denomXZ = this->x * q->z - this->z * q->x;
  constexpr double denomXY = this->x * q->y - this->y * q->x;
  if (abs(denomYZ) >= abs(denomXZ) && abs(denomYZ) >= abs(denomXY)) {
    // X is the biggest, so our point will have x0 = 0.0
    if (abs(denomYZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return false;
    }
    constexpr double denom = 1.0 / denomYZ;
    x0 = 0.0;
    y0 = (-this->D * q->z - this->z * -q->D) * denom;
    z0 = (this->y * -q->D + this->D * q->y) * denom;
  } else if (abs(denomXZ) >= abs(denomXY) && abs(denomXZ) >= abs(denomYZ)) {
    // Y is the biggest, so y0 = 0.0
    if (abs(denomXZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return false;
    }
    constexpr double denom = 1.0 / denomXZ;
    x0 = (-this->D * q->z - this->z * -q->D) * denom;
    y0 = 0.0;
    z0 = (this->x * -q->D + this->D * q->x) * denom;
  } else {
    // Z is the biggest, so Z0 = 0.0
    if (abs(denomXY) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return false;
    }
    constexpr double denom = 1.0 / denomXY;
    x0 = (-this->D * q->y - this->y * -q->D) * denom;
    y0 = (this->x * -q->D + this->D * q->x) * denom;
    z0 = 0.0;
  }

  // Once an intersecting line is determined, the next step is to intersect that
  // line with the ellipsoid, which will yield zero, one, or two points. The
  // ellipsoid equation: 1,0 = x^2/a^2 + y^2/b^2 + z^2/c^2 1.0 = (At+A0)^2/a^2 +
  // (Bt+B0)^2/b^2 + (Ct+C0)^2/c^2 A^2 t^2 / a^2 + 2AA0t / a^2 + A0^2 / a^2 +
  // B^2 t^2 / b^2 + 2BB0t / b^2 + B0^2 / b^2 + C^2 t^2 / c^2 + 2CC0t / c^2 +
  // C0^2 / c^2  - 1,0 = 0.0 [A^2 / a^2 + B^2 / b^2 + C^2 / c^2] t^2 + [2AA0 /
  // a^2 + 2BB0 / b^2 + 2CC0 / c^2] t + [A0^2 / a^2 + B0^2 / b^2 + C0^2 / c^2 -
  // 1,0] = 0.0 Use the quadratic formula to determine t values and candidate
  // point(s)
  constexpr double A =
      lineVectorX * lineVectorX * planetModel->inverseAbSquared +
      lineVectorY * lineVectorY * planetModel->inverseAbSquared +
      lineVectorZ * lineVectorZ * planetModel->inverseCSquared;
  constexpr double B = 2.0 * (lineVectorX * x0 * planetModel->inverseAbSquared +
                              lineVectorY * y0 * planetModel->inverseAbSquared +
                              lineVectorZ * z0 * planetModel->inverseCSquared);
  constexpr double C = x0 * x0 * planetModel->inverseAbSquared +
                       y0 * y0 * planetModel->inverseAbSquared +
                       z0 * z0 * planetModel->inverseCSquared - 1.0;

  constexpr double BsquaredMinus = B * B - 4.0 * A * C;
  if (abs(BsquaredMinus) < MINIMUM_RESOLUTION_SQUARED) {
    // System.err.println(" One point of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // One solution only
    constexpr double t = -B * inverse2A;
    // Maybe we can save ourselves the cost of construction of a point?
    constexpr double pointX = lineVectorX * t + x0;
    constexpr double pointY = lineVectorY * t + y0;
    constexpr double pointZ = lineVectorZ * t + z0;
    for (auto bound : bounds) {
      if (!bound->isWithin(pointX, pointY, pointZ)) {
        return false;
      }
    }
    for (shared_ptr<Membership> bound : moreBounds) {
      if (!bound->isWithin(pointX, pointY, pointZ)) {
        return false;
      }
    }
    return true;
  } else if (BsquaredMinus > 0.0) {
    // System.err.println(" Two points of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // Two solutions
    constexpr double sqrtTerm = sqrt(BsquaredMinus);
    constexpr double t1 = (-B + sqrtTerm) * inverse2A;
    constexpr double t2 = (-B - sqrtTerm) * inverse2A;
    // Up to two points being returned.  Do what we can to save on object
    // creation though.
    constexpr double point1X = lineVectorX * t1 + x0;
    constexpr double point1Y = lineVectorY * t1 + y0;
    constexpr double point1Z = lineVectorZ * t1 + z0;
    bool point1Valid = true;
    for (auto bound : bounds) {
      if (!bound->isWithin(point1X, point1Y, point1Z)) {
        point1Valid = false;
        break;
      }
    }
    if (point1Valid) {
      for (shared_ptr<Membership> bound : moreBounds) {
        if (!bound->isWithin(point1X, point1Y, point1Z)) {
          point1Valid = false;
          break;
        }
      }
    }
    if (point1Valid) {
      return true;
    }
    constexpr double point2X = lineVectorX * t2 + x0;
    constexpr double point2Y = lineVectorY * t2 + y0;
    constexpr double point2Z = lineVectorZ * t2 + z0;
    for (auto bound : bounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        return false;
      }
    }
    for (shared_ptr<Membership> bound : moreBounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        return false;
      }
    }
    return true;
  } else {
    // System.err.println(" no solutions - no intersection");
    return false;
  }
}

bool Plane::crosses(shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> q,
                    std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                    std::deque<std::shared_ptr<GeoPoint>> &moreNotablePoints,
                    std::deque<std::shared_ptr<Membership>> &bounds,
                    deque<Membership> &moreBounds)
{
  // System.err.println("Does plane "+this+" cross plane "+q);
  // If the two planes are identical, then the math will find no points of
  // intersection. So a special case of this is to check for plane equality. But
  // that is not enough, because what we really need at that point is to
  // determine whether overlap occurs between the two parts of the intersection
  // of plane and circle.  That is, are there *any* points on the plane that are
  // within the bounds described?
  if (isNumericallyIdentical(q)) {
    // System.err.println(" Identical plane");
    // The only way to efficiently figure this out will be to have a deque of
    // trial points available to evaluate. We look for any point that fulfills
    // all the bounds.
    for (auto p : notablePoints) {
      if (meetsAllBounds(p, bounds, moreBounds)) {
        // System.err.println("  found a notable point in bounds, so
        // intersects");
        return true;
      }
    }
    for (auto p : moreNotablePoints) {
      if (meetsAllBounds(p, bounds, moreBounds)) {
        // System.err.println("  found a notable point in bounds, so
        // intersects");
        return true;
      }
    }
    // System.err.println("  no notable points inside found; no intersection");
    return false;
  }

  // Save on allocations; do inline instead of calling findIntersections
  // System.err.println("Looking for intersection between plane "+this+" and
  // plane "+q+" within bounds");
  // Unnormalized, unchecked...
  constexpr double lineVectorX = y * q->z - z * q->y;
  constexpr double lineVectorY = z * q->x - x * q->z;
  constexpr double lineVectorZ = x * q->y - y * q->x;

  if (abs(lineVectorX) < MINIMUM_RESOLUTION &&
      abs(lineVectorY) < MINIMUM_RESOLUTION &&
      abs(lineVectorZ) < MINIMUM_RESOLUTION) {
    // Degenerate case: parallel planes
    // System.err.println(" planes are parallel - no intersection");
    return false;
  }

  // The line will have the equation: A t + A0 = x, B t + B0 = y, C t + C0 = z.
  // We have A, B, and C.  In order to come up with A0, B0, and C0, we need to
  // find a point that is on both planes. To do this, we find the largest deque
  // value (either x, y, or z), and look for a point that solves both plane
  // equations simultaneous.  For example, let's say that the deque is
  // (0.5,0.5,1), and the two plane equations are: 0.7 x + 0.3 y + 0.1 z + 0.0 =
  // 0 and 0.9 x - 0.1 y + 0.2 z + 4.0 = 0 Then we'd pick z = 0, so the
  // equations to solve for x and y would be: 0.7 x + 0.3y = 0.0 0.9 x - 0.1y =
  // -4.0
  // ... which can readily be solved using standard linear algebra.  Generally:
  // Q0 x + R0 y = S0
  // Q1 x + R1 y = S1
  // ... can be solved by Cramer's rule:
  // x = det(S0 R0 / S1 R1) / det(Q0 R0 / Q1 R1)
  // y = det(Q0 S0 / Q1 S1) / det(Q0 R0 / Q1 R1)
  // ... where det( a b / c d ) = ad - bc, so:
  // x = (S0 * R1 - R0 * S1) / (Q0 * R1 - R0 * Q1)
  // y = (Q0 * S1 - S0 * Q1) / (Q0 * R1 - R0 * Q1)
  double x0;
  double y0;
  double z0;
  // We try to maximize the determinant in the denominator
  constexpr double denomYZ = this->y * q->z - this->z * q->y;
  constexpr double denomXZ = this->x * q->z - this->z * q->x;
  constexpr double denomXY = this->x * q->y - this->y * q->x;
  if (abs(denomYZ) >= abs(denomXZ) && abs(denomYZ) >= abs(denomXY)) {
    // X is the biggest, so our point will have x0 = 0.0
    if (abs(denomYZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return false;
    }
    constexpr double denom = 1.0 / denomYZ;
    x0 = 0.0;
    y0 = (-this->D * q->z - this->z * -q->D) * denom;
    z0 = (this->y * -q->D + this->D * q->y) * denom;
  } else if (abs(denomXZ) >= abs(denomXY) && abs(denomXZ) >= abs(denomYZ)) {
    // Y is the biggest, so y0 = 0.0
    if (abs(denomXZ) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return false;
    }
    constexpr double denom = 1.0 / denomXZ;
    x0 = (-this->D * q->z - this->z * -q->D) * denom;
    y0 = 0.0;
    z0 = (this->x * -q->D + this->D * q->x) * denom;
  } else {
    // Z is the biggest, so Z0 = 0.0
    if (abs(denomXY) < MINIMUM_RESOLUTION_SQUARED) {
      // System.err.println(" Denominator is zero: no intersection");
      return false;
    }
    constexpr double denom = 1.0 / denomXY;
    x0 = (-this->D * q->y - this->y * -q->D) * denom;
    y0 = (this->x * -q->D + this->D * q->x) * denom;
    z0 = 0.0;
  }

  // Once an intersecting line is determined, the next step is to intersect that
  // line with the ellipsoid, which will yield zero, one, or two points. The
  // ellipsoid equation: 1,0 = x^2/a^2 + y^2/b^2 + z^2/c^2 1.0 = (At+A0)^2/a^2 +
  // (Bt+B0)^2/b^2 + (Ct+C0)^2/c^2 A^2 t^2 / a^2 + 2AA0t / a^2 + A0^2 / a^2 +
  // B^2 t^2 / b^2 + 2BB0t / b^2 + B0^2 / b^2 + C^2 t^2 / c^2 + 2CC0t / c^2 +
  // C0^2 / c^2  - 1,0 = 0.0 [A^2 / a^2 + B^2 / b^2 + C^2 / c^2] t^2 + [2AA0 /
  // a^2 + 2BB0 / b^2 + 2CC0 / c^2] t + [A0^2 / a^2 + B0^2 / b^2 + C0^2 / c^2 -
  // 1,0] = 0.0 Use the quadratic formula to determine t values and candidate
  // point(s)
  constexpr double A =
      lineVectorX * lineVectorX * planetModel->inverseAbSquared +
      lineVectorY * lineVectorY * planetModel->inverseAbSquared +
      lineVectorZ * lineVectorZ * planetModel->inverseCSquared;
  constexpr double B = 2.0 * (lineVectorX * x0 * planetModel->inverseAbSquared +
                              lineVectorY * y0 * planetModel->inverseAbSquared +
                              lineVectorZ * z0 * planetModel->inverseCSquared);
  constexpr double C = x0 * x0 * planetModel->inverseAbSquared +
                       y0 * y0 * planetModel->inverseAbSquared +
                       z0 * z0 * planetModel->inverseCSquared - 1.0;

  constexpr double BsquaredMinus = B * B - 4.0 * A * C;
  if (abs(BsquaredMinus) < MINIMUM_RESOLUTION_SQUARED) {
    // System.err.println(" One point of intersection");
    // We're not interested in situations where there is only one solution;
    // these are intersections but not crossings
    return false;
  } else if (BsquaredMinus > 0.0) {
    // System.err.println(" Two points of intersection");
    constexpr double inverse2A = 1.0 / (2.0 * A);
    // Two solutions
    constexpr double sqrtTerm = sqrt(BsquaredMinus);
    constexpr double t1 = (-B + sqrtTerm) * inverse2A;
    constexpr double t2 = (-B - sqrtTerm) * inverse2A;
    // Up to two points being returned.  Do what we can to save on object
    // creation though.
    constexpr double point1X = lineVectorX * t1 + x0;
    constexpr double point1Y = lineVectorY * t1 + y0;
    constexpr double point1Z = lineVectorZ * t1 + z0;
    bool point1Valid = true;
    for (auto bound : bounds) {
      if (!bound->isWithin(point1X, point1Y, point1Z)) {
        point1Valid = false;
        break;
      }
    }
    if (point1Valid) {
      for (shared_ptr<Membership> bound : moreBounds) {
        if (!bound->isWithin(point1X, point1Y, point1Z)) {
          point1Valid = false;
          break;
        }
      }
    }
    if (point1Valid) {
      return true;
    }
    constexpr double point2X = lineVectorX * t2 + x0;
    constexpr double point2Y = lineVectorY * t2 + y0;
    constexpr double point2Z = lineVectorZ * t2 + z0;
    for (auto bound : bounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        return false;
      }
    }
    for (shared_ptr<Membership> bound : moreBounds) {
      if (!bound->isWithin(point2X, point2Y, point2Z)) {
        return false;
      }
    }
    return true;
  } else {
    // System.err.println(" no solutions - no intersection");
    return false;
  }
}

bool Plane::isFunctionallyIdentical(shared_ptr<Plane> p)
{
  // We can get the correlation by just doing a parallel plane check.  That's
  // basically finding out if the magnitude of the cross-product is "zero".
  constexpr double cross1 = this->y * p->z - this->z * p->y;
  constexpr double cross2 = this->z * p->x - this->x * p->z;
  constexpr double cross3 = this->x * p->y - this->y * p->x;
  // System.out.println("cross product magnitude = "+(cross1 * cross1 + cross2 *
  // cross2 + cross3 * cross3));
  // Should be MINIMUM_RESOLUTION_SQUARED, but that gives us planes that are
  // *almost* parallel, and those are problematic too, so we have a tighter
  // constraint on parallelism in this method.
  if (cross1 * cross1 + cross2 * cross2 + cross3 * cross3 >=
      MINIMUM_RESOLUTION) {
    return false;
  }

  // Now, see whether the parallel planes are in fact on top of one another.
  // The math:
  // We need a single point that fulfills:
  // Ax + By + Cz + D = 0
  // Pick:
  // x0 = -(A * D) / (A^2 + B^2 + C^2)
  // y0 = -(B * D) / (A^2 + B^2 + C^2)
  // z0 = -(C * D) / (A^2 + B^2 + C^2)
  // Check:
  // A (x0) + B (y0) + C (z0) + D =? 0
  // A (-(A * D) / (A^2 + B^2 + C^2)) + B (-(B * D) / (A^2 + B^2 + C^2)) + C
  // (-(C * D) / (A^2 + B^2 + C^2)) + D ?= 0 -D [ A^2 / (A^2 + B^2 + C^2) + B^2
  // / (A^2 + B^2 + C^2) + C^2 / (A^2 + B^2 + C^2)] + D ?= 0 Yes.
  constexpr double denom = 1.0 / (p->x * p->x + p->y * p->y + p->z * p->z);
  return evaluateIsZero(-p->x * p->D * denom, -p->y * p->D * denom,
                        -p->z * p->D * denom);
}

bool Plane::isNumericallyIdentical(shared_ptr<Plane> p)
{
  // We can get the correlation by just doing a parallel plane check.  That's
  // basically finding out if the magnitude of the cross-product is "zero".
  constexpr double cross1 = this->y * p->z - this->z * p->y;
  constexpr double cross2 = this->z * p->x - this->x * p->z;
  constexpr double cross3 = this->x * p->y - this->y * p->x;
  // System.out.println("cross product magnitude = "+(cross1 * cross1 + cross2 *
  // cross2 + cross3 * cross3));
  if (cross1 * cross1 + cross2 * cross2 + cross3 * cross3 >=
      MINIMUM_RESOLUTION_SQUARED) {
    return false;
  }
  /* Old method
  if (Math.abs(this.y * p.z - this.z * p.y) >= MINIMUM_RESOLUTION)
    return false;
  if (Math.abs(this.z * p.x - this.x * p.z) >= MINIMUM_RESOLUTION)
    return false;
  if (Math.abs(this.x * p.y - this.y * p.x) >= MINIMUM_RESOLUTION)
    return false;
  */

  // Now, see whether the parallel planes are in fact on top of one another.
  // The math:
  // We need a single point that fulfills:
  // Ax + By + Cz + D = 0
  // Pick:
  // x0 = -(A * D) / (A^2 + B^2 + C^2)
  // y0 = -(B * D) / (A^2 + B^2 + C^2)
  // z0 = -(C * D) / (A^2 + B^2 + C^2)
  // Check:
  // A (x0) + B (y0) + C (z0) + D =? 0
  // A (-(A * D) / (A^2 + B^2 + C^2)) + B (-(B * D) / (A^2 + B^2 + C^2)) + C
  // (-(C * D) / (A^2 + B^2 + C^2)) + D ?= 0 -D [ A^2 / (A^2 + B^2 + C^2) + B^2
  // / (A^2 + B^2 + C^2) + C^2 / (A^2 + B^2 + C^2)] + D ?= 0 Yes.
  constexpr double denom = 1.0 / (p->x * p->x + p->y * p->y + p->z * p->z);
  return evaluateIsZero(-p->x * p->D * denom, -p->y * p->D * denom,
                        -p->z * p->D * denom);
}

std::deque<std::shared_ptr<GeoPoint>> Plane::findArcDistancePoints(
    shared_ptr<PlanetModel> planetModel, double const arcDistanceValue,
    shared_ptr<GeoPoint> startPoint, deque<Membership> &bounds)
{
  if (abs(D) >= MINIMUM_RESOLUTION) {
    throw make_shared<IllegalStateException>(
        L"Can't find arc distance using plane that doesn't go through origin");
  }
  if (!evaluateIsZero(startPoint)) {
    throw invalid_argument(L"Start point is not on plane");
  }

  // The following assertion fails at times even for planes that were
  // *explicitly* normalized, so I've disabled the check.
  // assert Math.abs(x*x + y*y + z*z - 1.0) < MINIMUM_RESOLUTION_SQUARED :
  // "Plane needs to be normalized";

  // The first step is to rotate coordinates for the point so that the plane
  // lies on the x-y plane. To acheive this, there will need to be three
  // rotations: (1) rotate the plane in x-y so that the y axis lies in it. (2)
  // rotate the plane in x-z so that the plane lies on the x-y plane. (3) rotate
  // in x-y so that the starting deque points to (1,0,0).

  // This presumes a normalized plane!!
  constexpr double azimuthMagnitude =
      sqrt(this->x * this->x + this->y * this->y);
  constexpr double cosPlaneAltitude = this->z;
  constexpr double sinPlaneAltitude = azimuthMagnitude;
  constexpr double cosPlaneAzimuth = this->x / azimuthMagnitude;
  constexpr double sinPlaneAzimuth = this->y / azimuthMagnitude;

  assert((abs(sinPlaneAltitude * sinPlaneAltitude +
              cosPlaneAltitude * cosPlaneAltitude - 1.0) < MINIMUM_RESOLUTION,
          L"Improper sin/cos of altitude: " +
              to_wstring(sinPlaneAltitude * sinPlaneAltitude +
                         cosPlaneAltitude * cosPlaneAltitude)));
  assert((abs(sinPlaneAzimuth * sinPlaneAzimuth +
              cosPlaneAzimuth * cosPlaneAzimuth - 1.0) < MINIMUM_RESOLUTION,
          L"Improper sin/cos of azimuth: " +
              to_wstring(sinPlaneAzimuth * sinPlaneAzimuth +
                         cosPlaneAzimuth * cosPlaneAzimuth)));

  // Coordinate rotation formula:
  // xT = xS cos T - yS sin T
  // yT = xS sin T + yS cos T
  // But we're rotating backwards, so use:
  // sin (-T) = -sin (T)
  // cos (-T) = cos (T)

  // Now, rotate startpoint in x-y
  constexpr double x0 = startPoint->x;
  constexpr double y0 = startPoint->y;
  constexpr double z0 = startPoint->z;

  constexpr double x1 = x0 * cosPlaneAzimuth + y0 * sinPlaneAzimuth;
  constexpr double y1 = -x0 * sinPlaneAzimuth + y0 * cosPlaneAzimuth;
  constexpr double z1 = z0;

  // Rotate now in x-z
  constexpr double x2 = x1 * cosPlaneAltitude - z1 * sinPlaneAltitude;
  constexpr double y2 = y1;
  constexpr double z2 = +x1 * sinPlaneAltitude + z1 * cosPlaneAltitude;

  assert(
      (abs(z2) < MINIMUM_RESOLUTION,
       L"Rotation should have put startpoint on x-y plane, instead has value " +
           to_wstring(z2)));

  // Ok, we have the start point on the x-y plane.  To apply the arc distance,
  // we next need to convert to an angle (in radians).
  constexpr double startAngle = atan2(y2, x2);

  // To apply the arc distance, just add to startAngle.
  constexpr double point1Angle = startAngle + arcDistanceValue;
  constexpr double point2Angle = startAngle - arcDistanceValue;
  // Convert each point to x-y
  constexpr double point1x2 = cos(point1Angle);
  constexpr double point1y2 = sin(point1Angle);
  constexpr double point1z2 = 0.0;

  constexpr double point2x2 = cos(point2Angle);
  constexpr double point2y2 = sin(point2Angle);
  constexpr double point2z2 = 0.0;

  // Now, do the reverse rotations for both points
  // Altitude...
  constexpr double point1x1 =
      point1x2 * cosPlaneAltitude + point1z2 * sinPlaneAltitude;
  constexpr double point1y1 = point1y2;
  constexpr double point1z1 =
      -point1x2 * sinPlaneAltitude + point1z2 * cosPlaneAltitude;

  constexpr double point2x1 =
      point2x2 * cosPlaneAltitude + point2z2 * sinPlaneAltitude;
  constexpr double point2y1 = point2y2;
  constexpr double point2z1 =
      -point2x2 * sinPlaneAltitude + point2z2 * cosPlaneAltitude;

  // Azimuth...
  constexpr double point1x0 =
      point1x1 * cosPlaneAzimuth - point1y1 * sinPlaneAzimuth;
  constexpr double point1y0 =
      point1x1 * sinPlaneAzimuth + point1y1 * cosPlaneAzimuth;
  constexpr double point1z0 = point1z1;

  constexpr double point2x0 =
      point2x1 * cosPlaneAzimuth - point2y1 * sinPlaneAzimuth;
  constexpr double point2y0 =
      point2x1 * sinPlaneAzimuth + point2y1 * cosPlaneAzimuth;
  constexpr double point2z0 = point2z1;

  shared_ptr<GeoPoint> *const point1 =
      planetModel->createSurfacePoint(point1x0, point1y0, point1z0);
  shared_ptr<GeoPoint> *const point2 =
      planetModel->createSurfacePoint(point2x0, point2y0, point2z0);

  // Figure out what to return
  bool isPoint1Inside = meetsAllBounds(point1, bounds);
  bool isPoint2Inside = meetsAllBounds(point2, bounds);

  if (isPoint1Inside) {
    if (isPoint2Inside) {
      return std::deque<std::shared_ptr<GeoPoint>>{point1, point2};
    } else {
      return std::deque<std::shared_ptr<GeoPoint>>{point1};
    }
  } else {
    if (isPoint2Inside) {
      return std::deque<std::shared_ptr<GeoPoint>>{point2};
    } else {
      return std::deque<std::shared_ptr<GeoPoint>>(0);
    }
  }
}

bool Plane::meetsAllBounds(shared_ptr<Vector> p,
                           std::deque<std::shared_ptr<Membership>> &bounds)
{
  return meetsAllBounds(p->x, p->y, p->z, bounds);
}

bool Plane::meetsAllBounds(double const x, double const y, double const z,
                           std::deque<std::shared_ptr<Membership>> &bounds)
{
  for (auto bound : bounds) {
    if (!bound->isWithin(x, y, z)) {
      return false;
    }
  }
  return true;
}

bool Plane::meetsAllBounds(shared_ptr<Vector> p,
                           std::deque<std::shared_ptr<Membership>> &bounds,
                           std::deque<std::shared_ptr<Membership>> &moreBounds)
{
  return meetsAllBounds(p->x, p->y, p->z, bounds, moreBounds);
}

bool Plane::meetsAllBounds(double const x, double const y, double const z,
                           std::deque<std::shared_ptr<Membership>> &bounds,
                           std::deque<std::shared_ptr<Membership>> &moreBounds)
{
  return meetsAllBounds(x, y, z, bounds) && meetsAllBounds(x, y, z, moreBounds);
}

shared_ptr<GeoPoint>
Plane::getSampleIntersectionPoint(shared_ptr<PlanetModel> planetModel,
                                  shared_ptr<Plane> q)
{
  std::deque<std::shared_ptr<GeoPoint>> intersections =
      findIntersections(planetModel, q, {NO_BOUNDS, NO_BOUNDS});
  if (intersections.empty()) {
    return nullptr;
  }
  return intersections[0];
}

wstring Plane::toString()
{
  return L"[A=" + to_wstring(x) + L", B=" + to_wstring(y) + L"; C=" +
         to_wstring(z) + L"; D=" + to_wstring(D) + L"]";
}

bool Plane::equals(any o)
{
  if (!Vector::equals(o)) {
    return false;
  }
  if (!(std::dynamic_pointer_cast<Plane>(o) != nullptr)) {
    return false;
  }
  shared_ptr<Plane> other = any_cast<std::shared_ptr<Plane>>(o);
  return other->D == D;
}

int Plane::hashCode()
{
  int result = Vector::hashCode();
  int64_t temp;
  temp = Double::doubleToLongBits(D);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}
} // namespace org::apache::lucene::spatial3d::geom