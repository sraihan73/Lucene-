using namespace std;

#include "TestGeo3DDocValues.h"

namespace org::apache::lucene::spatial3d
{
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestGeo3DDocValues::testBasic() 
{
  checkPointEncoding(0.0, 0.0);
  checkPointEncoding(45.0, 72.0);
  checkPointEncoding(-45.0, -100.0);
  constexpr int testAmt = TestUtil::nextInt(random(), 1000, 2000);
  for (int i = 0; i < testAmt; i++) {
    checkPointEncoding(random()->nextDouble() * 180.0 - 90.0,
                       random()->nextDouble() * 360.0 - 180.0);
  }
}

void TestGeo3DDocValues::checkPointEncoding(double const latitude,
                                            double const longitude)
{
  shared_ptr<GeoPoint> *const point = make_shared<GeoPoint>(
      PlanetModel::WGS84, Geo3DUtil::fromDegrees(latitude),
      Geo3DUtil::fromDegrees(longitude));
  int64_t pointValue = Geo3DDocValuesField::encodePoint(point);
  constexpr double x = Geo3DDocValuesField::decodeXValue(pointValue);
  constexpr double y = Geo3DDocValuesField::decodeYValue(pointValue);
  constexpr double z = Geo3DDocValuesField::decodeZValue(pointValue);
  shared_ptr<GeoPoint> *const pointR = make_shared<GeoPoint>(x, y, z);
  // Check whether stable
  pointValue = Geo3DDocValuesField::encodePoint(x, y, z);
  assertEquals(x, Geo3DDocValuesField::decodeXValue(pointValue), 0.0);
  assertEquals(y, Geo3DDocValuesField::decodeYValue(pointValue), 0.0);
  assertEquals(z, Geo3DDocValuesField::decodeZValue(pointValue), 0.0);
  // Check whether has some relationship with original point
  assertEquals(0.0, point->arcDistance(pointR), 0.02);
}
} // namespace org::apache::lucene::spatial3d