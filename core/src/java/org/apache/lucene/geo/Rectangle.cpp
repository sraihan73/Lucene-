using namespace std;

#include "Rectangle.h"

namespace org::apache::lucene::geo
{
//    import static Math.PI;
//    import static Math.max;
//    import static Math.min;
//    import static org.apache.lucene.geo.GeoUtils.checkLatitude;
//    import static org.apache.lucene.geo.GeoUtils.checkLongitude;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_RADIANS;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LON_RADIANS;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_RADIANS;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LON_RADIANS;
//    import static org.apache.lucene.geo.GeoUtils.EARTH_MEAN_RADIUS_METERS;
//    import static org.apache.lucene.geo.GeoUtils.sloppySin;
//    import static org.apache.lucene.util.SloppyMath.TO_DEGREES;
//    import static org.apache.lucene.util.SloppyMath.asin;
//    import static org.apache.lucene.util.SloppyMath.cos;
//    import static org.apache.lucene.util.SloppyMath.toDegrees;
//    import static org.apache.lucene.util.SloppyMath.toRadians;

Rectangle::Rectangle(double minLat, double maxLat, double minLon, double maxLon)
    : minLat(minLat), minLon(minLon), maxLat(maxLat), maxLon(maxLon)
{
  GeoUtils::checkLatitude(minLat);
  GeoUtils::checkLatitude(maxLat);
  GeoUtils::checkLongitude(minLon);
  GeoUtils::checkLongitude(maxLon);
  assert(maxLat >= minLat);

  // NOTE: cannot assert maxLon >= minLon since this rect could cross the
  // dateline
}

wstring Rectangle::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"Rectangle(lat=");
  b->append(minLat);
  b->append(L" TO ");
  b->append(maxLat);
  b->append(L" lon=");
  b->append(minLon);
  b->append(L" TO ");
  b->append(maxLon);
  if (maxLon < minLon) {
    b->append(L" [crosses dateline!]");
  }
  b->append(L")");

  return b->toString();
}

bool Rectangle::crossesDateline() { return maxLon < minLon; }

shared_ptr<Rectangle> Rectangle::fromPointDistance(double const centerLat,
                                                   double const centerLon,
                                                   double const radiusMeters)
{
  checkLatitude(centerLat);
  checkLongitude(centerLon);
  constexpr double radLat = toRadians(centerLat);
  constexpr double radLon = toRadians(centerLon);
  // LUCENE-7143
  double radDistance = (radiusMeters + 7E-2) / EARTH_MEAN_RADIUS_METERS;
  double minLat = radLat - radDistance;
  double maxLat = radLat + radDistance;
  double minLon;
  double maxLon;

  if (minLat > MIN_LAT_RADIANS && maxLat < MAX_LAT_RADIANS) {
    double deltaLon = asin(sloppySin(radDistance) / cos(radLat));
    minLon = radLon - deltaLon;
    if (minLon < MIN_LON_RADIANS) {
      minLon += 2 * PI;
    }
    maxLon = radLon + deltaLon;
    if (maxLon > MAX_LON_RADIANS) {
      maxLon -= 2 * PI;
    }
  } else {
    // a pole is within the distance
    minLat = max(minLat, MIN_LAT_RADIANS);
    maxLat = min(maxLat, MAX_LAT_RADIANS);
    minLon = MIN_LON_RADIANS;
    maxLon = MAX_LON_RADIANS;
  }

  return make_shared<Rectangle>(toDegrees(minLat), toDegrees(maxLat),
                                toDegrees(minLon), toDegrees(maxLon));
}

const double Rectangle::AXISLAT_ERROR =
    0.1 / EARTH_MEAN_RADIUS_METERS * TO_DEGREES;

double Rectangle::axisLat(double centerLat, double radiusMeters)
{
  // A spherical triangle with:
  // r is the radius of the circle in radians
  // l1 is the latitude of the circle center
  // l2 is the latitude of the point at which the circle intersect's its bbox
  // longitudes We know r is tangent to the bbox meridians at l2, therefore it
  // is a right angle. So from the law of cosines, with the angle of l1 being
  // 90, we have: cos(l1) = cos(r) * cos(l2) + sin(r) * sin(l2) * cos(90) The
  // second part cancels out because cos(90) == 0, so we have: cos(l1) = cos(r)
  // * cos(l2) Solving for l2, we get: l2 = acos( cos(l1) / cos(r) ) We ensure r
  // is in the range (0, PI/2) and l1 in the range (0, PI/2]. This means we
  // cannot divide by 0, and we will always get a positive value in the range
  // [0, 1) as the argument to arc cosine, resulting in a range (0, PI/2].
  constexpr double PIO2 = M_PI / 2;
  double l1 = toRadians(centerLat);
  double r = (radiusMeters + 7E-2) / EARTH_MEAN_RADIUS_METERS;

  // if we are within radius range of a pole, the lat is the pole itself
  if (abs(l1) + r >= MAX_LAT_RADIANS) {
    return centerLat >= 0 ? MAX_LAT_INCL : MIN_LAT_INCL;
  }

  // adjust l1 as distance from closest pole, to form a right triangle with bbox
  // meridians and ensure it is in the range (0, PI/2]
  l1 = centerLat >= 0 ? PIO2 - l1 : l1 + PIO2;

  double l2 = acos(cos(l1) / cos(r));
  assert(!isnan(l2));

  // now adjust back to range [-pi/2, pi/2], ie latitude in radians
  l2 = centerLat >= 0 ? PIO2 - l2 : l2 - PIO2;

  return toDegrees(l2);
}

shared_ptr<Rectangle>
Rectangle::fromPolygon(std::deque<std::shared_ptr<Polygon>> &polygons)
{
  // compute bounding box
  double minLat = numeric_limits<double>::infinity();
  double maxLat = -numeric_limits<double>::infinity();
  double minLon = numeric_limits<double>::infinity();
  double maxLon = -numeric_limits<double>::infinity();

  for (int i = 0; i < polygons.size(); i++) {
    minLat = min(polygons[i]->minLat, minLat);
    maxLat = max(polygons[i]->maxLat, maxLat);
    minLon = min(polygons[i]->minLon, minLon);
    maxLon = max(polygons[i]->maxLon, maxLon);
  }

  return make_shared<Rectangle>(minLat, maxLat, minLon, maxLon);
}

bool Rectangle::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<Rectangle> rectangle = any_cast<std::shared_ptr<Rectangle>>(o);

  if (Double::compare(rectangle->minLat, minLat) != 0) {
    return false;
  }
  if (Double::compare(rectangle->minLon, minLon) != 0) {
    return false;
  }
  if (Double::compare(rectangle->maxLat, maxLat) != 0) {
    return false;
  }
  return Double::compare(rectangle->maxLon, maxLon) == 0;
}

int Rectangle::hashCode()
{
  int result;
  int64_t temp;
  temp = Double::doubleToLongBits(minLat);
  result = static_cast<int>(
      temp ^
      (static_cast<int64_t>(static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(minLon);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(maxLat);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(maxLon);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}
} // namespace org::apache::lucene::geo