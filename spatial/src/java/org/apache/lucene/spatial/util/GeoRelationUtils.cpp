using namespace std;

#include "GeoRelationUtils.h"

namespace org::apache::lucene::spatial::util
{

GeoRelationUtils::GeoRelationUtils() {}

bool GeoRelationUtils::pointInRectPrecise(double const lat, double const lon,
                                          double const minLat,
                                          double const maxLat,
                                          double const minLon,
                                          double const maxLon)
{
  return lat >= minLat && lat <= maxLat && lon >= minLon && lon <= maxLon;
}

bool GeoRelationUtils::rectDisjoint(double const aMinLat, double const aMaxLat,
                                    double const aMinLon, double const aMaxLon,
                                    double const bMinLat, double const bMaxLat,
                                    double const bMinLon, double const bMaxLon)
{
  return (aMaxLon < bMinLon || aMinLon > bMaxLon || aMaxLat < bMinLat ||
          aMinLat > bMaxLat);
}

bool GeoRelationUtils::rectWithin(double const aMinLat, double const aMaxLat,
                                  double const aMinLon, double const aMaxLon,
                                  double const bMinLat, double const bMaxLat,
                                  double const bMinLon, double const bMaxLon)
{
  return !(aMinLon < bMinLon || aMinLat < bMinLat || aMaxLon > bMaxLon ||
           aMaxLat > bMaxLat);
}

bool GeoRelationUtils::rectCrosses(double const aMinLat, double const aMaxLat,
                                   double const aMinLon, double const aMaxLon,
                                   double const bMinLat, double const bMaxLat,
                                   double const bMinLon, double const bMaxLon)
{
  return !(rectDisjoint(aMinLat, aMaxLat, aMinLon, aMaxLon, bMinLat, bMaxLat,
                        bMinLon, bMaxLon) ||
           rectWithin(aMinLat, aMaxLat, aMinLon, aMaxLon, bMinLat, bMaxLat,
                      bMinLon, bMaxLon));
}

bool GeoRelationUtils::rectIntersects(
    double const aMinLat, double const aMaxLat, double const aMinLon,
    double const aMaxLon, double const bMinLat, double const bMaxLat,
    double const bMinLon, double const bMaxLon)
{
  return !((aMaxLon < bMinLon || aMinLon > bMaxLon || aMaxLat < bMinLat ||
            aMinLat > bMaxLat));
}
} // namespace org::apache::lucene::spatial::util