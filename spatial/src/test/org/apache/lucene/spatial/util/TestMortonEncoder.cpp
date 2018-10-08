using namespace std;

#include "TestMortonEncoder.h"

namespace org::apache::lucene::spatial::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.spatial.util.MortonEncoder.decodeLatitude;
//    import static
//    org.apache.lucene.spatial.util.MortonEncoder.decodeLongitude; import
//    static org.apache.lucene.spatial.util.MortonEncoder.encode; import static
//    org.apache.lucene.spatial.util.MortonEncoder.encodeCeil; import static
//    org.apache.lucene.util.BitUtil.deinterleave; import static
//    org.apache.lucene.util.BitUtil.interleave;

void TestMortonEncoder::testMortonEncoding() 
{
  constexpr int64_t TRANSLATE = 1LL << 31;
  constexpr double LATITUDE_DECODE = 180.0 / (0x1LL << 32);
  constexpr double LONGITUDE_DECODE = 360.0 / (0x1LL << 32);
  shared_ptr<Random> random = TestMortonEncoder::random();
  for (int i = 0; i < 10000; ++i) {
    int64_t encoded = TestMortonEncoder::random()->nextLong();
    int64_t encodedLat = deinterleave(
        static_cast<int64_t>(static_cast<uint64_t>(encoded) >> 1));
    int64_t encodedLon = deinterleave(encoded);
    double expectedLat =
        decodeLatitude(static_cast<int>(encodedLat - TRANSLATE));
    double decodedLat = decodeLatitude(encoded);
    double expectedLon =
        decodeLongitude(static_cast<int>(encodedLon - TRANSLATE));
    double decodedLon = decodeLongitude(encoded);
    assertEquals(expectedLat, decodedLat, 0.0);
    assertEquals(expectedLon, decodedLon, 0.0);
    // should round-trip
    TestUtil::assertEquals(encoded, encode(decodedLat, decodedLon));

    // test within the range
    if (encoded != 0xFFFFFFFFFFFFFFFFLL) {
      // this is the next representable value
      // all double values between [min .. max) should encode to the current
      // integer all double values between (min .. max] should encodeCeil to the
      // next integer.
      double maxLat = expectedLat + LATITUDE_DECODE;
      encodedLat += 1;
      assertEquals(maxLat,
                   decodeLatitude(static_cast<int>(encodedLat - TRANSLATE)),
                   0.0);
      double maxLon = expectedLon + LONGITUDE_DECODE;
      encodedLon += 1;
      assertEquals(maxLon,
                   decodeLongitude(static_cast<int>(encodedLon - TRANSLATE)),
                   0.0);
      int64_t encodedNext = encode(maxLat, maxLon);
      TestUtil::assertEquals(interleave(static_cast<int>(encodedLon),
                                        static_cast<int>(encodedLat)),
                             encodedNext);

      // first and last doubles in range that will be quantized
      double minEdgeLat = Math::nextUp(expectedLat);
      double minEdgeLon = Math::nextUp(expectedLon);
      int64_t encodedMinEdge = encode(minEdgeLat, minEdgeLon);
      int64_t encodedMinEdgeCeil = encodeCeil(minEdgeLat, minEdgeLon);
      double maxEdgeLat = Math::nextDown(maxLat);
      double maxEdgeLon = Math::nextDown(maxLon);
      int64_t encodedMaxEdge = encode(maxEdgeLat, maxEdgeLon);
      int64_t encodedMaxEdgeCeil = encodeCeil(maxEdgeLat, maxEdgeLon);

      TestUtil::assertEquals(
          encodedLat - 1,
          deinterleave(static_cast<int64_t>(
              static_cast<uint64_t>(encodedMinEdge) >> 1)));
      TestUtil::assertEquals(
          encodedLat,
          deinterleave(static_cast<int64_t>(
              static_cast<uint64_t>(encodedMinEdgeCeil) >> 1)));
      TestUtil::assertEquals(encodedLon - 1, deinterleave(encodedMinEdge));
      TestUtil::assertEquals(encodedLon, deinterleave(encodedMinEdgeCeil));

      TestUtil::assertEquals(
          encodedLat - 1,
          deinterleave(static_cast<int64_t>(
              static_cast<uint64_t>(encodedMaxEdge) >> 1)));
      TestUtil::assertEquals(
          encodedLat,
          deinterleave(static_cast<int64_t>(
              static_cast<uint64_t>(encodedMaxEdgeCeil) >> 1)));
      TestUtil::assertEquals(encodedLon - 1, deinterleave(encodedMaxEdge));
      TestUtil::assertEquals(encodedLon, deinterleave(encodedMaxEdgeCeil));

      // check random values within the double range
      int64_t minBitsLat = NumericUtils::doubleToSortableLong(minEdgeLat);
      int64_t maxBitsLat = NumericUtils::doubleToSortableLong(maxEdgeLat);
      int64_t minBitsLon = NumericUtils::doubleToSortableLong(minEdgeLon);
      int64_t maxBitsLon = NumericUtils::doubleToSortableLong(maxEdgeLon);
      for (int j = 0; j < 100; j++) {
        double valueLat = NumericUtils::sortableLongToDouble(
            TestUtil::nextLong(random, minBitsLat, maxBitsLat));
        double valueLon = NumericUtils::sortableLongToDouble(
            TestUtil::nextLong(random, minBitsLon, maxBitsLon));
        // round down
        TestUtil::assertEquals(encoded, encode(valueLat, valueLon));
        // round up
        TestUtil::assertEquals(interleave(static_cast<int>(encodedLon),
                                          static_cast<int>(encodedLat)),
                               encodeCeil(valueLat, valueLon));
      }
    }
  }
}
} // namespace org::apache::lucene::spatial::util