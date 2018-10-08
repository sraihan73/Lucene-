#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPolygon;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertEquals;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertTrue;

/**
 * Check relationship between polygon and GeoShapes of basic polygons. Normally
 * we construct the convex, concave counterpart and the convex polygon as a
 * complex polygon.
 */
class SimpleGeoPolygonRelationshipsTest
    : public std::enable_shared_from_this<SimpleGeoPolygonRelationshipsTest>
{
  GET_CLASS_NAME(SimpleGeoPolygonRelationshipsTest)

  /**
   * Test with two shapes with no crossing edges and no points in common in
   * convex case.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygon1()
  virtual void testGeoSimplePolygon1();

  /**
   * Test with two shapes with crossing edges and some points inside in convex
   * case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygon2()
  virtual void testGeoSimplePolygon2();

  /**
   * Test with two shapes with no crossing edges and all points inside in convex
   * case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygon3()
  virtual void testGeoSimplePolygon3();

  /**
   * Test with two shapes with crossing edges and no points inside in convex
   * case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygon4()
  virtual void testGeoSimplePolygon4();

  /**
   * Test with two shapes with no crossing edges and polygon in hole  in convex
   * case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole1()
  virtual void testGeoSimplePolygonWithHole1();

  /**
   * Test with two shapes with crossing edges in hole and some points inside in
   * convex case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole2()
  virtual void testGeoSimplePolygonWithHole2();

  /**
   * Test with two shapes with crossing edges and some points inside in convex
   * case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole3()
  virtual void testGeoSimplePolygonWithHole3();

  /**
   * Test with two shapes with no crossing edges and all points inside in convex
   * case.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole4()
  virtual void testGeoSimplePolygonWithHole4();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithCircle()
  virtual void testGeoSimplePolygonWithCircle();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithBBox()
  virtual void testGeoSimplePolygonWithBBox();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoSimplePolygonWithComposite()
  virtual void testGeoSimplePolygonWithComposite();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDegeneratedPointIntersectShape()
  virtual void testDegeneratedPointIntersectShape();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDegeneratedPointInPole()
  virtual void testDegeneratedPointInPole();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDegeneratePathShape()
  virtual void testDegeneratePathShape();

private:
  std::shared_ptr<GeoPolygon> buildConvexGeoPolygon(double lon1, double lat1,
                                                    double lon2, double lat2,
                                                    double lon3, double lat3,
                                                    double lon4, double lat4);

  std::shared_ptr<GeoPolygon> buildConcaveGeoPolygon(double lon1, double lat1,
                                                     double lon2, double lat2,
                                                     double lon3, double lat3,
                                                     double lon4, double lat4);

  std::shared_ptr<GeoPolygon> buildComplexGeoPolygon(double lon1, double lat1,
                                                     double lon2, double lat2,
                                                     double lon3, double lat3,
                                                     double lon4, double lat4);

  std::shared_ptr<GeoPolygon> buildConvexGeoPolygonWithHole(
      double lon1, double lat1, double lon2, double lat2, double lon3,
      double lat3, double lon4, double lat4, std::shared_ptr<GeoPolygon> hole);

  std::shared_ptr<GeoPolygon> buildConcaveGeoPolygonWithHole(
      double lon1, double lat1, double lon2, double lat2, double lon3,
      double lat3, double lon4, double lat4, std::shared_ptr<GeoPolygon> hole);

  std::shared_ptr<GeoShape> getCompositeShape();
};

} // namespace org::apache::lucene::spatial3d::geom
