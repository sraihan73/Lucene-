#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoPolygon;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}

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
 * Random test for polygons.
 */
class RandomGeoPolygonTest : public RandomGeo3dShapeGenerator
{
  GET_CLASS_NAME(RandomGeoPolygonTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
  // testRandomLUCENE8157()
  virtual void testRandomLUCENE8157();

  virtual void testLUCENE8157();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCoplanarityTilePolygon()
  virtual void testCoplanarityTilePolygon();

  /**
   * Test comparing different polygon (Big) technologies using random
   * biased doubles.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
  // testCompareBigPolygons()
  virtual void testCompareBigPolygons();

  /**
   * Test comparing different polygon (Small) technologies using random
   * biased doubles.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
  // testCompareSmallPolygons()
  virtual void testCompareSmallPolygons();

private:
  void testComparePolygons(double limitDistance);

  void checkPoint(std::shared_ptr<GeoPolygon> polygon,
                  std::shared_ptr<GeoPolygon> largePolygon,
                  std::shared_ptr<GeoPoint> point,
                  std::deque<std::shared_ptr<GeoPoint>> &orderedPoints);

  std::shared_ptr<GeoPoint>
  getBiasedPoint(std::shared_ptr<PlanetModel> planetModel);

  std::wstring getWKT(std::deque<std::shared_ptr<GeoPoint>> &points);

  bool contains(std::shared_ptr<GeoPoint> p,
                std::deque<std::shared_ptr<GeoPoint>> &points);

  std::shared_ptr<GeoPoint>
  getCenterOfMass(std::shared_ptr<PlanetModel> planetModel,
                  std::deque<std::shared_ptr<GeoPoint>> &points);

protected:
  std::shared_ptr<RandomGeoPolygonTest> shared_from_this()
  {
    return std::static_pointer_cast<RandomGeoPolygonTest>(
        RandomGeo3dShapeGenerator::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
