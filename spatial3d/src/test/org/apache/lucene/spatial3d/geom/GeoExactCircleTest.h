#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "stringbuilder.h"
#include <cmath>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

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

/**
 * Tests for GeoExactCircle.
 */
class GeoExactCircleTest : public RandomGeo3dShapeGenerator
{
  GET_CLASS_NAME(GeoExactCircleTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactCircle()
  virtual void testExactCircle();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSurfacePointOnBearingScale()
  virtual void testSurfacePointOnBearingScale();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
  // RandomPointBearingWGS84Test()
  virtual void RandomPointBearingWGS84Test();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
  // RandomPointBearingCardinalTest()
  virtual void RandomPointBearingCardinalTest();

private:
  void checkBearingPoint(std::shared_ptr<PlanetModel> planetModel,
                         std::shared_ptr<GeoPoint> center, double radius,
                         double bearingAngle);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactCircleBounds()
  virtual void testExactCircleBounds();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void exactCircleLargeTest()
  virtual void exactCircleLargeTest();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactCircleDoesNotFit()
  virtual void testExactCircleDoesNotFit();

  virtual void testBigCircleInSphere();

  /**
   * in LUCENE-8054 we have problems with exact circles that have
   * edges that are close together. This test creates those circles with the
   * same center and slightly different radius.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
  // testRandomLUCENE8054()
  virtual void testRandomLUCENE8054();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8054()
  virtual void testLUCENE8054();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8056()
  virtual void testLUCENE8056();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactCircleLUCENE8054()
  virtual void testExactCircleLUCENE8054();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLUCENE8065()
  virtual void testLUCENE8065();

  virtual void testLUCENE8080();

protected:
  std::shared_ptr<GeoExactCircleTest> shared_from_this()
  {
    return std::static_pointer_cast<GeoExactCircleTest>(
        RandomGeo3dShapeGenerator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
