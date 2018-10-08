using namespace std;

#include "GeoEncodingUtils.h"

namespace org::apache::lucene::geo
{
using Relation = org::apache::lucene::index::PointValues::Relation;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using SloppyMath = org::apache::lucene::util::SloppyMath;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LON_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LON_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_INCL;
//    import static org.apache.lucene.geo.GeoUtils.checkLatitude;
//    import static org.apache.lucene.geo.GeoUtils.checkLongitude;
const double GeoEncodingUtils::LAT_SCALE = (0x1LL << BITS) / 180.0;
const double GeoEncodingUtils::LAT_DECODE = 1 / LAT_SCALE;
const double GeoEncodingUtils::LON_SCALE = (0x1LL << BITS) / 360.0;
const double GeoEncodingUtils::LON_DECODE = 1 / LON_SCALE;

GeoEncodingUtils::GeoEncodingUtils() {}

int GeoEncodingUtils::encodeLatitude(double latitude)
{
  checkLatitude(latitude);
  // the maximum possible value cannot be encoded without overflow
  if (latitude == 90.0) {
    latitude = Math::nextDown(latitude);
  }
  return static_cast<int>(floor(latitude / LAT_DECODE));
}

int GeoEncodingUtils::encodeLatitudeCeil(double latitude)
{
  GeoUtils::checkLatitude(latitude);
  // the maximum possible value cannot be encoded without overflow
  if (latitude == 90.0) {
    latitude = Math::nextDown(latitude);
  }
  return static_cast<int>(ceil(latitude / LAT_DECODE));
}

int GeoEncodingUtils::encodeLongitude(double longitude)
{
  checkLongitude(longitude);
  // the maximum possible value cannot be encoded without overflow
  if (longitude == 180.0) {
    longitude = Math::nextDown(longitude);
  }
  return static_cast<int>(floor(longitude / LON_DECODE));
}

int GeoEncodingUtils::encodeLongitudeCeil(double longitude)
{
  GeoUtils::checkLongitude(longitude);
  // the maximum possible value cannot be encoded without overflow
  if (longitude == 180.0) {
    longitude = Math::nextDown(longitude);
  }
  return static_cast<int>(ceil(longitude / LON_DECODE));
}

double GeoEncodingUtils::decodeLatitude(int encoded)
{
  double result = encoded * LAT_DECODE;
  assert(result >= MIN_LAT_INCL && result < MAX_LAT_INCL);
  return result;
}

double GeoEncodingUtils::decodeLatitude(std::deque<char> &src, int offset)
{
  return decodeLatitude(NumericUtils::sortableBytesToInt(src, offset));
}

double GeoEncodingUtils::decodeLongitude(int encoded)
{
  double result = encoded * LON_DECODE;
  assert(result >= MIN_LON_INCL && result < MAX_LON_INCL);
  return result;
}

double GeoEncodingUtils::decodeLongitude(std::deque<char> &src, int offset)
{
  return decodeLongitude(NumericUtils::sortableBytesToInt(src, offset));
}

shared_ptr<DistancePredicate>
GeoEncodingUtils::createDistancePredicate(double lat, double lon,
                                          double radiusMeters)
{
  shared_ptr<Rectangle> *const boundingBox =
      Rectangle::fromPointDistance(lat, lon, radiusMeters);
  constexpr double axisLat = Rectangle::axisLat(lat, radiusMeters);
  constexpr double distanceSortKey =
      GeoUtils::distanceQuerySortKey(radiusMeters);
  const function<Relation(Rectangle *)> boxToRelation = [&](Rectangle *box) {
    return GeoUtils::relate(box::minLat, box::maxLat, box::minLon, box::maxLon,
                            lat, lon, distanceSortKey, axisLat);
  };
  shared_ptr<Grid> *const subBoxes = createSubBoxes(boundingBox, boxToRelation);

  return make_shared<DistancePredicate>(
      subBoxes->latShift, subBoxes->lonShift, subBoxes->latBase,
      subBoxes->lonBase, subBoxes->maxLatDelta, subBoxes->maxLonDelta,
      subBoxes->relations, lat, lon, distanceSortKey);
}

shared_ptr<PolygonPredicate> GeoEncodingUtils::createPolygonPredicate(
    std::deque<std::shared_ptr<Polygon>> &polygons, shared_ptr<Polygon2D> tree)
{
  shared_ptr<Rectangle> *const boundingBox = Rectangle::fromPolygon(polygons);
  const function<Relation(Rectangle *)> boxToRelation = [&](Rectangle *box) {
    return tree->relate(box::minLat, box::maxLat, box::minLon, box::maxLon);
  };
  shared_ptr<Grid> *const subBoxes = createSubBoxes(boundingBox, boxToRelation);

  return make_shared<PolygonPredicate>(
      subBoxes->latShift, subBoxes->lonShift, subBoxes->latBase,
      subBoxes->lonBase, subBoxes->maxLatDelta, subBoxes->maxLonDelta,
      subBoxes->relations, tree);
}

shared_ptr<Grid>
GeoEncodingUtils::createSubBoxes(shared_ptr<Rectangle> boundingBox,
                                 function<Relation(Rectangle *)> &boxToRelation)
{
  constexpr int minLat = encodeLatitudeCeil(boundingBox->minLat);
  constexpr int maxLat = encodeLatitude(boundingBox->maxLat);
  constexpr int minLon = encodeLongitudeCeil(boundingBox->minLon);
  constexpr int maxLon = encodeLongitude(boundingBox->maxLon);

  if (maxLat < minLat ||
      (boundingBox->crossesDateline() == false && maxLon < minLon)) {
    // the box cannot match any quantized point
    return make_shared<Grid>(1, 1, 0, 0, 0, 0, std::deque<char>(0));
  }

  constexpr int latShift, lonShift;
  constexpr int latBase, lonBase;
  constexpr int maxLatDelta, maxLonDelta;
  {
    int64_t minLat2 =
        static_cast<int64_t>(minLat) - numeric_limits<int>::min();
    int64_t maxLat2 =
        static_cast<int64_t>(maxLat) - numeric_limits<int>::min();
    latShift = computeShift(minLat2, maxLat2);
    latBase = static_cast<int>(static_cast<int64_t>(
        static_cast<uint64_t>(minLat2) >> latShift));
    maxLatDelta = static_cast<int>(static_cast<int64_t>(
                      static_cast<uint64_t>(maxLat2) >> latShift)) -
                  latBase + 1;
    assert(maxLatDelta > 0);
  }
  {
    int64_t minLon2 =
        static_cast<int64_t>(minLon) - numeric_limits<int>::min();
    int64_t maxLon2 =
        static_cast<int64_t>(maxLon) - numeric_limits<int>::min();
    if (boundingBox->crossesDateline()) {
      maxLon2 += 1LL << 32; // wrap
    }
    lonShift = computeShift(minLon2, maxLon2);
    lonBase = static_cast<int>(static_cast<int64_t>(
        static_cast<uint64_t>(minLon2) >> lonShift));
    maxLonDelta = static_cast<int>(static_cast<int64_t>(
                      static_cast<uint64_t>(maxLon2) >> lonShift)) -
                  lonBase + 1;
    assert(maxLonDelta > 0);
  }

  const std::deque<char> relations =
      std::deque<char>(maxLatDelta * maxLonDelta);
  for (int i = 0; i < maxLatDelta; ++i) {
    for (int j = 0; j < maxLonDelta; ++j) {
      constexpr int boxMinLat =
          ((latBase + i) << latShift) + numeric_limits<int>::min();
      constexpr int boxMinLon =
          ((lonBase + j) << lonShift) + numeric_limits<int>::min();
      constexpr int boxMaxLat = boxMinLat + (1 << latShift) - 1;
      constexpr int boxMaxLon = boxMinLon + (1 << lonShift) - 1;

      relations[i * maxLonDelta + j] = static_cast<char>(
          boxToRelation(make_shared<Rectangle>(decodeLatitude(boxMinLat),
                                               decodeLatitude(boxMaxLat),
                                               decodeLongitude(boxMinLon),
                                               decodeLongitude(boxMaxLon)))
              .ordinal());
    }
  }

  return make_shared<Grid>(latShift, lonShift, latBase, lonBase, maxLatDelta,
                           maxLonDelta, relations);
}

int GeoEncodingUtils::computeShift(int64_t a, int64_t b)
{
  assert(a <= b);
  // We enforce a shift of at least 1 so that when we work with unsigned ints
  // by doing (lat - MIN_VALUE), the result of the shift (lat - MIN_VALUE) >>>
  // shift can be used for comparisons without particular care: the sign bit has
  // been cleared so comparisons work the same for signed and unsigned ints
  for (int shift = 1;; ++shift) {
    constexpr int64_t delta =
        (static_cast<int64_t>(static_cast<uint64_t>(b) >> shift)) -
        (static_cast<int64_t>(static_cast<uint64_t>(a) >> shift));
    if (delta >= 0 && delta < Grid::ARITY) {
      return shift;
    }
  }
}

GeoEncodingUtils::Grid::Grid(int latShift, int lonShift, int latBase,
                             int lonBase, int maxLatDelta, int maxLonDelta,
                             std::deque<char> &relations)
    : latShift(latShift), lonShift(lonShift), latBase(latBase),
      lonBase(lonBase), maxLatDelta(maxLatDelta), maxLonDelta(maxLonDelta),
      relations(relations)
{
  if (latShift < 1 || latShift > 31) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }
  if (lonShift < 1 || lonShift > 31) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }
}

GeoEncodingUtils::DistancePredicate::DistancePredicate(
    int latShift, int lonShift, int latBase, int lonBase, int maxLatDelta,
    int maxLonDelta, std::deque<char> &relations, double lat, double lon,
    double distanceKey)
    : Grid(latShift, lonShift, latBase, lonBase, maxLatDelta, maxLonDelta,
           relations),
      lat(lat), lon(lon), distanceKey(distanceKey)
{
}

bool GeoEncodingUtils::DistancePredicate::test(int lat, int lon)
{
  constexpr int lat2 = (static_cast<int>(
      static_cast<unsigned int>((lat - numeric_limits<int>::min())) >>
      latShift));
  if (lat2 < latBase || lat2 >= latBase + maxLatDelta) {
    return false;
  }
  int lon2 = (static_cast<int>(
      static_cast<unsigned int>((lon - numeric_limits<int>::min())) >>
      lonShift));
  if (lon2 < lonBase) { // wrap
    lon2 += 1 << (32 - lonShift);
  }
  assert(Integer::toUnsignedLong(lon2) >= lonBase);
  assert(lon2 - lonBase >= 0);
  if (lon2 - lonBase >= maxLonDelta) {
    return false;
  }

  constexpr int relation =
      relations[(lat2 - latBase) * maxLonDelta + (lon2 - lonBase)];
  if (relation == static_cast<int>(Relation::CELL_CROSSES_QUERY)) {
    return SloppyMath::haversinSortKey(decodeLatitude(lat),
                                       decodeLongitude(lon), this->lat,
                                       this->lon) <= distanceKey;
  } else {
    return relation == static_cast<int>(Relation::CELL_INSIDE_QUERY);
  }
}

GeoEncodingUtils::PolygonPredicate::PolygonPredicate(
    int latShift, int lonShift, int latBase, int lonBase, int maxLatDelta,
    int maxLonDelta, std::deque<char> &relations, shared_ptr<Polygon2D> tree)
    : Grid(latShift, lonShift, latBase, lonBase, maxLatDelta, maxLonDelta,
           relations),
      tree(tree)
{
}

bool GeoEncodingUtils::PolygonPredicate::test(int lat, int lon)
{
  constexpr int lat2 = (static_cast<int>(
      static_cast<unsigned int>((lat - numeric_limits<int>::min())) >>
      latShift));
  if (lat2 < latBase || lat2 >= latBase + maxLatDelta) {
    return false;
  }
  int lon2 = (static_cast<int>(
      static_cast<unsigned int>((lon - numeric_limits<int>::min())) >>
      lonShift));
  if (lon2 < lonBase) { // wrap
    lon2 += 1 << (32 - lonShift);
  }
  assert(Integer::toUnsignedLong(lon2) >= lonBase);
  assert(lon2 - lonBase >= 0);
  if (lon2 - lonBase >= maxLonDelta) {
    return false;
  }

  constexpr int relation =
      relations[(lat2 - latBase) * maxLonDelta + (lon2 - lonBase)];
  if (relation == static_cast<int>(Relation::CELL_CROSSES_QUERY)) {
    return tree->contains(decodeLatitude(lat), decodeLongitude(lon));
  } else {
    return relation == static_cast<int>(Relation::CELL_INSIDE_QUERY);
  }
}
} // namespace org::apache::lucene::geo