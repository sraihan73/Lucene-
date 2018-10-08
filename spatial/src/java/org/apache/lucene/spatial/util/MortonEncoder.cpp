using namespace std;

#include "MortonEncoder.h"

namespace org::apache::lucene::spatial::util
{
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using BitUtil = org::apache::lucene::util::BitUtil;
//    import static org.apache.lucene.geo.GeoUtils.checkLatitude;
//    import static org.apache.lucene.geo.GeoUtils.checkLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitudeCeil;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitudeCeil;

MortonEncoder::MortonEncoder() {} // no instance

int64_t MortonEncoder::encode(double latitude, double longitude)
{
  checkLatitude(latitude);
  checkLongitude(longitude);
  // encode lat/lon flipping the sign bit so negative ints sort before positive
  // ints
  constexpr int latEnc =
      GeoEncodingUtils::encodeLatitude(latitude) ^ 0x80000000;
  constexpr int lonEnc =
      GeoEncodingUtils::encodeLongitude(longitude) ^ 0x80000000;
  return BitUtil::interleave(lonEnc, latEnc);
}

int64_t MortonEncoder::encodeCeil(double latitude, double longitude)
{
  checkLatitude(latitude);
  checkLongitude(longitude);
  // encode lat/lon flipping the sign bit so negative ints sort before positive
  // ints
  constexpr int latEnc =
      GeoEncodingUtils::encodeLatitudeCeil(latitude) ^ 0x80000000;
  constexpr int lonEnc =
      GeoEncodingUtils::encodeLongitudeCeil(longitude) ^ 0x80000000;
  return BitUtil::interleave(lonEnc, latEnc);
}

double MortonEncoder::decodeLatitude(int64_t const hash)
{
  // decode lat/lon flipping the sign bit so negative ints sort before positive
  // ints
  return GeoEncodingUtils::decodeLatitude(
      static_cast<int>(BitUtil::deinterleave(
          static_cast<int64_t>(static_cast<uint64_t>(hash) >> 1))) ^
      0x80000000);
}

double MortonEncoder::decodeLongitude(int64_t const hash)
{
  // decode lat/lon flipping the sign bit so negative ints sort before positive
  // ints
  return GeoEncodingUtils::decodeLongitude(
      static_cast<int>(BitUtil::deinterleave(hash)) ^ 0x80000000);
}

wstring MortonEncoder::geoTermToString(int64_t term)
{
  shared_ptr<StringBuilder> s = make_shared<StringBuilder>(64);
  constexpr int numberOfLeadingZeros = Long::numberOfLeadingZeros(term);
  for (int i = 0; i < numberOfLeadingZeros; i++) {
    s->append(L'0');
  }
  if (term != 0) {
    s->append(Long::toBinaryString(term));
  }
  return s->toString();
}
} // namespace org::apache::lucene::spatial::util