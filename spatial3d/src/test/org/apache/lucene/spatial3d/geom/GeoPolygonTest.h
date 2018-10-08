#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::spatial3d::geom
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class GeoPolygonTest : public LuceneTestCase
{
  GET_CLASS_NAME(GeoPolygonTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonPointFiltering()
  virtual void testPolygonPointFiltering();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonPointFiltering2()
  virtual void testPolygonPointFiltering2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonClockwise()
  virtual void testPolygonClockwise();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonIntersects()
  virtual void testPolygonIntersects();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonPointWithin()
  virtual void testPolygonPointWithin();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonBounds()
  virtual void testPolygonBounds();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonBoundsCase1()
  virtual void testPolygonBoundsCase1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoPolygonBoundsCase2()
  virtual void testGeoPolygonBoundsCase2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoConcaveRelationshipCase1()
  virtual void testGeoConcaveRelationshipCase1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFactoryCase1()
  virtual void testPolygonFactoryCase1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFactoryCase2()
  virtual void testPolygonFactoryCase2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFactoryCase3() throws Exception
  virtual void testPolygonFactoryCase3() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFactoryCase4()
  virtual void testPolygonFactoryCase4();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFactoryCase5()
  virtual void testPolygonFactoryCase5();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLargePolygonFailureCase1()
  virtual void testLargePolygonFailureCase1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLargePolygonFailureCase2()
  virtual void testLargePolygonFailureCase2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFailureCase1()
  virtual void testPolygonFailureCase1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonFailureCase2()
  virtual void testPolygonFailureCase2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testConcavePolygon()
  virtual void testConcavePolygon();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPolygonWithHole()
  virtual void testPolygonWithHole();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testConvexPolygon()
  virtual void testConvexPolygon();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testConvexPolygonWithHole()
  virtual void testConvexPolygonWithHole();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8133()
  virtual void testLUCENE8133();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8140() throws Exception
  virtual void testLUCENE8140() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8211()
  virtual void testLUCENE8211();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCoplanarityTileConvex() throws
  // Exception
  virtual void testCoplanarityTileConvex() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCoplanarityConcave() throws Exception
  virtual void testCoplanarityConcave() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCoplanarityConvex2() throws Exception
  virtual void testCoplanarityConvex2() ;

  /*
   [lat=-0.63542308910253, lon=0.9853722928232957([X=0.4446759777403525,
  Y=0.6707549854468698, Z=-0.5934780737681111])], [lat=0.0,
  lon=0.0([X=1.0011188539924791, Y=0.0, Z=0.0])], [lat=0.45435018176633574,
  lon=3.141592653589793([X=-0.8989684544372841, Y=1.1009188402610632E-16,
  Z=0.4390846549572752])], [lat=-0.375870856827283,
  lon=2.9129132647718414([X=-0.9065744420970767, Y=0.21100590938346708,
  Z=-0.36732668582405886])], [lat=-1.2205765069413237,
  lon=3.141592653589793([X=-0.3424714964202101, Y=4.194066218902145E-17,
  Z=-0.9375649457139603])]}}

   [junit4]   1>       unquantized=[lat=-3.1780051348770987E-74,
  lon=-3.032608859187692([X=-0.9951793580358298, Y=-0.1088898762907205,
  Z=-3.181560858610375E-74])] [junit4]   1> quantized=[X=-0.9951793580415914,
  Y=-0.10888987641797832, Z=-2.3309121299774915E-10]
  */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8227() throws Exception
  virtual void testLUCENE8227() ;

  /*
   [junit4]   1>     doc=754 is contained by shape but is outside the returned
   XYZBounds [junit4]   1>       unquantized=[lat=2.4043303687704734E-204,
   lon=3.1342447995980507([X=-1.0010918284309325, Y=0.007356008974104805,
   Z=2.4070204634028112E-204])] [junit4]   1> quantized=[X=-1.0010918285430614,
   Y=0.007356008812298254, Z=2.3309121299774915E-10]

   [junit4]   1>     doc=3728 is contained by shape but is outside the returned
   XYZBounds [junit4]   1>       unquantized=[lat=2.4457272005608357E-47,
   lon=-3.1404077424936307([X=-1.001118151199965, Y=-0.0011862365610909341,
   Z=2.448463612203698E-47])] [junit4]   1> quantized=[X=-1.0011181510675629,
   Y=-0.001186236379718708, Z=2.3309121299774915E-10]

   [junit4]   1>   shape=GeoComplexPolygon: {planetmodel=PlanetModel.WGS84,
   number of shapes=1, address=7969cab3, testPoint=[X=-0.07416172733314662,
   Y=0.5686488061136892, Z=0.8178445379402641], testPointInSet=true, shapes={ {
   [lat=-1.5707963267948966, lon=-1.0755217966112058([X=2.903696886845155E-17,
   Y=-5.375400029710238E-17, Z=-0.997762292022105])], [lat=-1.327365682666958,
   lon=-2.9674513704178316([X=-0.23690293696956322, Y=-0.04167672037374933,
   Z=-0.9685334156912658])], [lat=0.32288591161895097,
   lon=3.141592653589793([X=-0.9490627533610154, Y=1.1622666630935417E-16,
   Z=0.3175519551883462])], [lat=0.0, lon=0.0([X=1.0011188539924791, Y=0.0,
   Z=0.0])], [lat=0.2839194570254642,
   lon=-1.2434404554202965([X=0.30893121415043073, Y=-0.9097632721627391,
   Z=0.2803596238536593])]}}
  */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8227_case2()
  virtual void testLUCENE8227_case2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE7642()
  virtual void testLUCENE7642();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testComplexPolygonPlaneOutsideWorld()
  virtual void testComplexPolygonPlaneOutsideWorld();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testComplexPolygonDegeneratedVector()
  virtual void testComplexPolygonDegeneratedVector();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAboveBelowCrossingDifferentEdges()
  virtual void testAboveBelowCrossingDifferentEdges();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBelowCrossingTwiceEdgePoint()
  virtual void testBelowCrossingTwiceEdgePoint();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8245()
  virtual void testLUCENE8245();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8245_case2()
  virtual void testLUCENE8245_case2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8245_case3()
  virtual void testLUCENE8245_case3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8245_case4()
  virtual void testLUCENE8245_case4();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8251()
  virtual void testLUCENE8251();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8257()
  virtual void testLUCENE8257();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8258()
  virtual void testLUCENE8258();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8266_case1()
  virtual void testLUCENE8266_case1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8266_case2()
  virtual void testLUCENE8266_case2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8266_case3()
  virtual void testLUCENE8266_case3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8276_case1()
  virtual void testLUCENE8276_case1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8276_case2()
  virtual void testLUCENE8276_case2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8276_case3()
  virtual void testLUCENE8276_case3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8281()
  virtual void testLUCENE8281();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8280()
  virtual void testLUCENE8280();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8337()
  virtual void testLUCENE8337();

protected:
  std::shared_ptr<GeoPolygonTest> shared_from_this()
  {
    return std::static_pointer_cast<GeoPolygonTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
