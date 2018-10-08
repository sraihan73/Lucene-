#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::geo
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoTestUtil.nextLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoTestUtil.nextLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoTestUtil.nextPolygon;

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Test Polygon2D impl */
class TestPolygon2D : public LuceneTestCase
{
  GET_CLASS_NAME(TestPolygon2D)

  /** Three boxes, an island inside a hole inside a shape */
public:
  virtual void testMultiPolygon();

  virtual void testPacMan() ;

  virtual void testBoundingBox() ;

  // targets the bounding box directly
  virtual void testBoundingBoxEdgeCases() ;

  /** If polygon.contains(box) returns true, then any point in that box should
   * return true as well */
  virtual void testContainsRandom() ;

  /** If polygon.contains(box) returns true, then any point in that box should
   * return true as well */
  // different from testContainsRandom in that its not a purely random test. we
  // iterate the vertices of the polygon and generate boxes near each one of
  // those to try to be more efficient.
  virtual void testContainsEdgeCases() ;

  /** If polygon.intersects(box) returns false, then any point in that box
   * should return false as well */
  virtual void testIntersectRandom();

  /** If polygon.intersects(box) returns false, then any point in that box
   * should return false as well */
  // different from testIntersectsRandom in that its not a purely random test.
  // we iterate the vertices of the polygon and generate boxes near each one of
  // those to try to be more efficient.
  virtual void testIntersectEdgeCases();

  /** Tests edge case behavior with respect to insideness */
  virtual void testEdgeInsideness();

  /** Tests current impl against original algorithm */
  virtual void testContainsAgainstOriginal();

protected:
  std::shared_ptr<TestPolygon2D> shared_from_this()
  {
    return std::static_pointer_cast<TestPolygon2D>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::geo
