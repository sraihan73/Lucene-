#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>

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
 * Random test to check relationship between GeoAreaShapes and GeoShapes.
 */
class RandomGeoShapeRelationshipTest : public RandomGeo3dShapeGenerator
{
  GET_CLASS_NAME(RandomGeoShapeRelationshipTest)

  /**
   * Test for WITHIN points. We build a WITHIN shape with respect the
   * geoAreaShape and create a point WITHIN the shape. The resulting shape
   * should be WITHIN the original shape.
   *
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 5) public void
  // testRandomPointWithin()
  virtual void testRandomPointWithin();

  /**
   * Test for NOT WITHIN points. We build a DIJOINT shape with respect the
   * geoAreaShape and create a point WITHIN that shape. The resulting shape
   * should not be WITHIN the original shape.
   *
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = 5) public void
  // testRandomPointNotWithin()
  virtual void testRandomPointNotWithin();

  /**
   * Test for disjoint shapes. We build a DISJOINT shape with respect the
   * geoAreaShape and create shapes WITHIN that shapes. The resulting shape
   * should be DISJOINT to the geoAreaShape.
   *
   * Note that both shapes cannot be concave.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 5) public void
  // testRandomDisjoint()
  virtual void testRandomDisjoint();

  /**
   * Test for within shapes. We build a shape WITHIN the geoAreaShape and create
   * shapes WITHIN that shape. The resulting shape should be WITHIN
   * to the geoAreaShape.
   *
   * Note that if the geoAreaShape is not concave the other shape must be not
   * concave.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 5) public void testRandomWithIn()
  virtual void testRandomWithIn();

  /**
   * Test for contains shapes. We build a shape containing the geoAreaShape and
   * create shapes WITHIN that shape. The resulting shape should CONTAIN the
   * geoAreaShape.
   *
   * Note that if the geoAreaShape is concave the other shape must be concave.
   * If shape is concave, the shape for reference should be concave as well.
   *
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 1) public void
  // testRandomContains()
  virtual void testRandomContains();

  /**
   * Test for overlapping shapes. We build a shape that contains part of the
   * geoAreaShape, is disjoint to other part and contains a disjoint shape. We
   * create shapes  according the criteria. The resulting shape should OVERLAP
   * the geoAreaShape.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 5) public void
  // testRandomOverlaps()
  virtual void testRandomOverlaps();

protected:
  std::shared_ptr<RandomGeoShapeRelationshipTest> shared_from_this()
  {
    return std::static_pointer_cast<RandomGeoShapeRelationshipTest>(
        RandomGeo3dShapeGenerator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
