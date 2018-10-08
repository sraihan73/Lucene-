#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPolygon.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"

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

/**
 * Check relationship between polygon and GeoShapes of composite polygons.
 * Normally we construct the composite polygon (when possible) and the complex
 * one.
 */
class CompositeGeoPolygonRelationshipsTest
    : public std::enable_shared_from_this<CompositeGeoPolygonRelationshipsTest>
{
  GET_CLASS_NAME(CompositeGeoPolygonRelationshipsTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon1()
  virtual void testGeoCompositePolygon1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon2()
  virtual void testGeoCompositePolygon2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon3()
  virtual void testGeoCompositePolygon3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon4()
  virtual void testGeoCompositePolygon4();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon5()
  virtual void testGeoCompositePolygon5();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon6()
  virtual void testGeoCompositePolygon6();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon7()
  virtual void testGeoCompositePolygon7();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCompositePolygon8()
  virtual void testGeoCompositePolygon8();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoPolygonPole1()
  virtual void testGeoPolygonPole1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoPolygonPole2()
  virtual void testGeoPolygonPole2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoPolygonPole3()
  virtual void testGeoPolygonPole3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiPolygon1()
  virtual void testMultiPolygon1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiPolygon2()
  virtual void testMultiPolygon2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiPolygon3()
  virtual void testMultiPolygon3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiPolygon4()
  virtual void testMultiPolygon4();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiPolygon5()
  virtual void testMultiPolygon5();

private:
  std::shared_ptr<GeoPolygon> buildGeoPolygon(double lon1, double lat1,
                                              double lon2, double lat2,
                                              double lon3, double lat3,
                                              double lon4, double lat4,
                                              double lon5, double lat5);

  std::shared_ptr<GeoPolygon> buildConcaveGeoPolygon(double lon1, double lat1,
                                                     double lon2, double lat2,
                                                     double lon3, double lat3,
                                                     double lon4, double lat4);

  std::shared_ptr<GeoPolygon> getCompositePolygon();

  std::shared_ptr<GeoPolygon> getComplexPolygon();

  std::shared_ptr<GeoPolygon> getMultiPolygon();

public:
  virtual std::shared_ptr<GeoShape> getInsideCompositeShape();
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
