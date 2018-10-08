using namespace std;

#include "TestGeoEncodingUtils.h"

namespace org::apache::lucene::geo
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitudeCeil;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitudeCeil;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MAX_LON_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_INCL;
//    import static org.apache.lucene.geo.GeoUtils.MIN_LON_INCL;

void TestGeoEncodingUtils::testLatitudeQuantization() 
{
  constexpr double LATITUDE_DECODE = 180.0 / (0x1LL << 32);
  shared_ptr<Random> random = TestGeoEncodingUtils::random();
  for (int i = 0; i < 10000; i++) {
    int encoded = random->nextInt();
    double min =
        MIN_LAT_INCL +
        (encoded - static_cast<int64_t>(numeric_limits<int>::min())) *
            LATITUDE_DECODE;
    double decoded = decodeLatitude(encoded);
    // should exactly equal expected value
    assertEquals(min, decoded, 0.0);
    // should round-trip
    TestUtil::assertEquals(encoded, encodeLatitude(decoded));
    TestUtil::assertEquals(encoded, encodeLatitudeCeil(decoded));
    // test within the range
    if (encoded != numeric_limits<int>::max()) {
      // this is the next representable value
      // all double values between [min .. max) should encode to the current
      // integer all double values between (min .. max] should encodeCeil to the
      // next integer.
      double max = min + LATITUDE_DECODE;
      assertEquals(max, decodeLatitude(encoded + 1), 0.0);
      TestUtil::assertEquals(encoded + 1, encodeLatitude(max));
      TestUtil::assertEquals(encoded + 1, encodeLatitudeCeil(max));

      // first and last doubles in range that will be quantized
      double minEdge = Math::nextUp(min);
      double maxEdge = Math::nextDown(max);
      TestUtil::assertEquals(encoded, encodeLatitude(minEdge));
      TestUtil::assertEquals(encoded + 1, encodeLatitudeCeil(minEdge));
      TestUtil::assertEquals(encoded, encodeLatitude(maxEdge));
      TestUtil::assertEquals(encoded + 1, encodeLatitudeCeil(maxEdge));

      // check random values within the double range
      int64_t minBits = NumericUtils::doubleToSortableLong(minEdge);
      int64_t maxBits = NumericUtils::doubleToSortableLong(maxEdge);
      for (int j = 0; j < 100; j++) {
        double value = NumericUtils::sortableLongToDouble(
            TestUtil::nextLong(random, minBits, maxBits));
        // round down
        TestUtil::assertEquals(encoded, encodeLatitude(value));
        // round up
        TestUtil::assertEquals(encoded + 1, encodeLatitudeCeil(value));
      }
    }
  }
}

void TestGeoEncodingUtils::testLongitudeQuantization() 
{
  constexpr double LONGITUDE_DECODE = 360.0 / (0x1LL << 32);
  shared_ptr<Random> random = TestGeoEncodingUtils::random();
  for (int i = 0; i < 10000; i++) {
    int encoded = random->nextInt();
    double min =
        MIN_LON_INCL +
        (encoded - static_cast<int64_t>(numeric_limits<int>::min())) *
            LONGITUDE_DECODE;
    double decoded = decodeLongitude(encoded);
    // should exactly equal expected value
    assertEquals(min, decoded, 0.0);
    // should round-trip
    TestUtil::assertEquals(encoded, encodeLongitude(decoded));
    TestUtil::assertEquals(encoded, encodeLongitudeCeil(decoded));
    // test within the range
    if (encoded != numeric_limits<int>::max()) {
      // this is the next representable value
      // all double values between [min .. max) should encode to the current
      // integer all double values between (min .. max] should encodeCeil to the
      // next integer.
      double max = min + LONGITUDE_DECODE;
      assertEquals(max, decodeLongitude(encoded + 1), 0.0);
      TestUtil::assertEquals(encoded + 1, encodeLongitude(max));
      TestUtil::assertEquals(encoded + 1, encodeLongitudeCeil(max));

      // first and last doubles in range that will be quantized
      double minEdge = Math::nextUp(min);
      double maxEdge = Math::nextDown(max);
      TestUtil::assertEquals(encoded, encodeLongitude(minEdge));
      TestUtil::assertEquals(encoded + 1, encodeLongitudeCeil(minEdge));
      TestUtil::assertEquals(encoded, encodeLongitude(maxEdge));
      TestUtil::assertEquals(encoded + 1, encodeLongitudeCeil(maxEdge));

      // check random values within the double range
      int64_t minBits = NumericUtils::doubleToSortableLong(minEdge);
      int64_t maxBits = NumericUtils::doubleToSortableLong(maxEdge);
      for (int j = 0; j < 100; j++) {
        double value = NumericUtils::sortableLongToDouble(
            TestUtil::nextLong(random, minBits, maxBits));
        // round down
        TestUtil::assertEquals(encoded, encodeLongitude(value));
        // round up
        TestUtil::assertEquals(encoded + 1, encodeLongitudeCeil(value));
      }
    }
  }
}

void TestGeoEncodingUtils::testEncodeEdgeCases()
{
  TestUtil::assertEquals(numeric_limits<int>::min(),
                         encodeLatitude(MIN_LAT_INCL));
  TestUtil::assertEquals(numeric_limits<int>::min(),
                         encodeLatitudeCeil(MIN_LAT_INCL));
  TestUtil::assertEquals(numeric_limits<int>::max(),
                         encodeLatitude(MAX_LAT_INCL));
  TestUtil::assertEquals(numeric_limits<int>::max(),
                         encodeLatitudeCeil(MAX_LAT_INCL));

  TestUtil::assertEquals(numeric_limits<int>::min(),
                         encodeLongitude(MIN_LON_INCL));
  TestUtil::assertEquals(numeric_limits<int>::min(),
                         encodeLongitudeCeil(MIN_LON_INCL));
  TestUtil::assertEquals(numeric_limits<int>::max(),
                         encodeLongitude(MAX_LON_INCL));
  TestUtil::assertEquals(numeric_limits<int>::max(),
                         encodeLongitudeCeil(MAX_LON_INCL));
}
} // namespace org::apache::lucene::geo