using namespace std;

#include "GeoUtils.h"

namespace org::apache::lucene::geo
{
//    import static org.apache.lucene.util.SloppyMath.TO_RADIANS;
//    import static org.apache.lucene.util.SloppyMath.cos;
//    import static org.apache.lucene.util.SloppyMath.haversinMeters;
using PointValues = org::apache::lucene::index::PointValues;
using Relation = org::apache::lucene::index::PointValues::Relation;
using SloppyMath = org::apache::lucene::util::SloppyMath;
const double GeoUtils::MIN_LON_RADIANS = SloppyMath::TO_RADIANS * MIN_LON_INCL;
const double GeoUtils::MIN_LAT_RADIANS = SloppyMath::TO_RADIANS * MIN_LAT_INCL;
const double GeoUtils::MAX_LON_RADIANS = SloppyMath::TO_RADIANS * MAX_LON_INCL;
const double GeoUtils::MAX_LAT_RADIANS = SloppyMath::TO_RADIANS * MAX_LAT_INCL;

GeoUtils::GeoUtils() {}

void GeoUtils::checkLatitude(double latitude)
{
  if (isnan(latitude) || latitude < MIN_LAT_INCL || latitude > MAX_LAT_INCL) {
    throw invalid_argument(L"invalid latitude " + to_wstring(latitude) +
                           L"; must be between " + to_wstring(MIN_LAT_INCL) +
                           L" and " + to_wstring(MAX_LAT_INCL));
  }
}

void GeoUtils::checkLongitude(double longitude)
{
  if (isnan(longitude) || longitude < MIN_LON_INCL ||
      longitude > MAX_LON_INCL) {
    throw invalid_argument(L"invalid longitude " + to_wstring(longitude) +
                           L"; must be between " + to_wstring(MIN_LON_INCL) +
                           L" and " + to_wstring(MAX_LON_INCL));
  }
}

const double GeoUtils::PIO2 = M_PI / 2;

double GeoUtils::sloppySin(double a) { return SloppyMath::cos(a - PIO2); }

double GeoUtils::distanceQuerySortKey(double radius)
{
  // effectively infinite
  if (radius >= SloppyMath::haversinMeters(numeric_limits<double>::max())) {
    return SloppyMath::haversinMeters(numeric_limits<double>::max());
  }

  // this is a search through non-negative long space only
  int64_t lo = 0;
  int64_t hi = Double::doubleToRawLongBits(numeric_limits<double>::max());
  while (lo <= hi) {
    int64_t mid =
        static_cast<int64_t>(static_cast<uint64_t>((lo + hi)) >> 1);
    double sortKey = Double::longBitsToDouble(mid);
    double midRadius = SloppyMath::haversinMeters(sortKey);
    if (midRadius == radius) {
      return sortKey;
    } else if (midRadius > radius) {
      hi = mid - 1;
    } else {
      lo = mid + 1;
    }
  }

  // not found: this is because a user can supply an arbitrary radius, one that
  // we will never calculate exactly via our haversin method.
  double ceil = Double::longBitsToDouble(lo);
  assert(SloppyMath::haversinMeters(ceil) > radius);
  return ceil;
}

PointValues::Relation GeoUtils::relate(double minLat, double maxLat,
                                       double minLon, double maxLon, double lat,
                                       double lon, double distanceSortKey,
                                       double axisLat)
{

  if (minLon > maxLon) {
    throw invalid_argument(L"Box crosses the dateline");
  }

  if ((lon < minLon || lon > maxLon) &&
      (axisLat + Rectangle::AXISLAT_ERROR < minLat ||
       axisLat - Rectangle::AXISLAT_ERROR > maxLat)) {
    // circle not fully inside / crossing axis
    if (SloppyMath::haversinSortKey(lat, lon, minLat, minLon) >
            distanceSortKey &&
        SloppyMath::haversinSortKey(lat, lon, minLat, maxLon) >
            distanceSortKey &&
        SloppyMath::haversinSortKey(lat, lon, maxLat, minLon) >
            distanceSortKey &&
        SloppyMath::haversinSortKey(lat, lon, maxLat, maxLon) >
            distanceSortKey) {
      // no points inside
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }
  }

  if (within90LonDegrees(lon, minLon, maxLon) &&
      SloppyMath::haversinSortKey(lat, lon, minLat, minLon) <=
          distanceSortKey &&
      SloppyMath::haversinSortKey(lat, lon, minLat, maxLon) <=
          distanceSortKey &&
      SloppyMath::haversinSortKey(lat, lon, maxLat, minLon) <=
          distanceSortKey &&
      SloppyMath::haversinSortKey(lat, lon, maxLat, maxLon) <=
          distanceSortKey) {
    // we are fully enclosed, collect everything within this subtree
    return PointValues::Relation::CELL_INSIDE_QUERY;
  }

  return PointValues::Relation::CELL_CROSSES_QUERY;
}

bool GeoUtils::within90LonDegrees(double lon, double minLon, double maxLon)
{
  if (maxLon <= lon - 180) {
    lon -= 360;
  } else if (minLon >= lon + 180) {
    lon += 360;
  }
  return maxLon - lon < 90 && lon - minLon < 90;
}
} // namespace org::apache::lucene::geo