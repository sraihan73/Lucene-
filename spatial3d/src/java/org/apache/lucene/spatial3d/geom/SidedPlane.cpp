using namespace std;

#include "SidedPlane.h"

namespace org::apache::lucene::spatial3d::geom
{

SidedPlane::SidedPlane(shared_ptr<SidedPlane> sidedPlane)
    : Plane(sidedPlane, sidedPlane->D), sigNum(-sidedPlane->sigNum)
{
}

SidedPlane::SidedPlane(double const pX, double const pY, double const pZ,
                       shared_ptr<Vector> A, shared_ptr<Vector> B)
    : Plane(A, B), sigNum(Math::signum(evaluate(pX, pY, pZ)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(shared_ptr<Vector> p, shared_ptr<Vector> A,
                       shared_ptr<Vector> B)
    : Plane(A, B), sigNum(Math::signum(evaluate(p)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(shared_ptr<Vector> A, shared_ptr<Vector> B)
    : Plane(A, B), sigNum(1.0)
{
}

SidedPlane::SidedPlane(shared_ptr<Vector> p, shared_ptr<Vector> A,
                       double const BX, double const BY, double const BZ)
    : Plane(A, BX, BY, BZ), sigNum(Math::signum(evaluate(p)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(shared_ptr<Vector> p, bool const onSide,
                       shared_ptr<Vector> A, shared_ptr<Vector> B)
    : Plane(A, B),
      sigNum(onSide ? Math::signum(evaluate(p)) : -Math::signum(evaluate(p)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(shared_ptr<Vector> p,
                       shared_ptr<PlanetModel> planetModel, double sinLat)
    : Plane(planetModel, sinLat), sigNum(Math::signum(evaluate(p)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(shared_ptr<Vector> p, double x, double y)
    : Plane(x, y), sigNum(Math::signum(evaluate(p)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(shared_ptr<Vector> p, shared_ptr<Vector> v, double D)
    : Plane(v, D), sigNum(Math::signum(evaluate(p)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

SidedPlane::SidedPlane(double pX, double pY, double pZ, shared_ptr<Vector> v,
                       double D)
    : Plane(v, D), sigNum(Math::signum(evaluate(pX, pY, pZ)))
{
  if (sigNum == 0.0) {
    throw invalid_argument(
        L"Cannot determine sidedness because check point is on plane.");
  }
}

shared_ptr<SidedPlane> SidedPlane::constructNormalizedPerpendicularSidedPlane(
    shared_ptr<Vector> insidePoint, shared_ptr<Vector> normalVector,
    shared_ptr<Vector> point1, shared_ptr<Vector> point2)
{
  shared_ptr<Vector> *const pointsVector = make_shared<Vector>(
      point1->x - point2->x, point1->y - point2->y, point1->z - point2->z);
  shared_ptr<Vector> *const newNormalVector =
      make_shared<Vector>(normalVector, pointsVector);
  try {
    // To construct the plane, we now just need D, which is simply the negative
    // of the evaluation of the circle normal deque at one of the points.
    return make_shared<SidedPlane>(insidePoint, newNormalVector,
                                   -newNormalVector->dotProduct(point1));
  } catch (const invalid_argument &e) {
    return nullptr;
  }
}

shared_ptr<SidedPlane> SidedPlane::constructNormalizedThreePointSidedPlane(
    shared_ptr<Vector> insidePoint, shared_ptr<Vector> point1,
    shared_ptr<Vector> point2, shared_ptr<Vector> point3)
{
  shared_ptr<SidedPlane> rval = nullptr;

  if (rval == nullptr) {
    try {
      shared_ptr<Vector> *const planeNormal = make_shared<Vector>(
          point1->x - point2->x, point1->y - point2->y, point1->z - point2->z,
          point2->x - point3->x, point2->y - point3->y, point2->z - point3->z);
      rval = make_shared<SidedPlane>(insidePoint, planeNormal,
                                     -planeNormal->dotProduct(point2));
    } catch (const invalid_argument &e) {
    }
  }

  if (rval == nullptr) {
    try {
      shared_ptr<Vector> *const planeNormal = make_shared<Vector>(
          point1->x - point3->x, point1->y - point3->y, point1->z - point3->z,
          point3->x - point2->x, point3->y - point2->y, point3->z - point2->z);
      rval = make_shared<SidedPlane>(insidePoint, planeNormal,
                                     -planeNormal->dotProduct(point3));
    } catch (const invalid_argument &e) {
    }
  }

  if (rval == nullptr) {
    try {
      shared_ptr<Vector> *const planeNormal = make_shared<Vector>(
          point3->x - point1->x, point3->y - point1->y, point3->z - point1->z,
          point1->x - point2->x, point1->y - point2->y, point1->z - point2->z);
      rval = make_shared<SidedPlane>(insidePoint, planeNormal,
                                     -planeNormal->dotProduct(point1));
    } catch (const invalid_argument &e) {
    }
  }

  return rval;
}

bool SidedPlane::isWithin(double x, double y, double z)
{
  double evalResult = evaluate(x, y, z);
  // System.out.println(Math.abs(evalResult));
  if (abs(evalResult) < MINIMUM_RESOLUTION) {
    return true;
  }
  double sigNum = Math::signum(evalResult);
  return sigNum == this->sigNum;
}

bool SidedPlane::strictlyWithin(shared_ptr<Vector> v)
{
  double evalResult = evaluate(v->x, v->y, v->z);
  double sigNum = Math::signum(evalResult);
  return sigNum == 0.0 || sigNum == this->sigNum;
}

bool SidedPlane::strictlyWithin(double x, double y, double z)
{
  double evalResult = evaluate(x, y, z);
  double sigNum = Math::signum(evalResult);
  return sigNum == 0.0 || sigNum == this->sigNum;
}

bool SidedPlane::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<SidedPlane>(o) != nullptr)) {
    return false;
  }
  if (!Plane::equals(o)) {
    return false;
  }

  shared_ptr<SidedPlane> that = any_cast<std::shared_ptr<SidedPlane>>(o);

  return Double::compare(that->sigNum, sigNum) == 0;
}

int SidedPlane::hashCode()
{
  int result = Plane::hashCode();
  int64_t temp;
  temp = Double::doubleToLongBits(sigNum);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring SidedPlane::toString()
{
  return L"[A=" + to_wstring(x) + L", B=" + to_wstring(y) + L", C=" +
         to_wstring(z) + L", D=" + to_wstring(D) + L", side=" +
         to_wstring(sigNum) + L"]";
}
} // namespace org::apache::lucene::spatial3d::geom