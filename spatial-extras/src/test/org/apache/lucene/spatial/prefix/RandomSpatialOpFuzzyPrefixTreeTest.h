#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/RecursivePrefixTreeStrategy.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"

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
namespace org::apache::lucene::spatial::prefix
{

using Document = org::apache::lucene::document::Document;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeCollection;
using org::locationtech::spatial4j::shape::SpatialRelation;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomBoolean;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.locationtech.spatial4j.shape.SpatialRelation.CONTAINS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.locationtech.spatial4j.shape.SpatialRelation.DISJOINT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.locationtech.spatial4j.shape.SpatialRelation.INTERSECTS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.locationtech.spatial4j.shape.SpatialRelation.WITHIN;

/** Randomized PrefixTree test that considers the fuzziness of the
 * results introduced by grid approximation. */
class RandomSpatialOpFuzzyPrefixTreeTest : public StrategyTestCase
{
  GET_CLASS_NAME(RandomSpatialOpFuzzyPrefixTreeTest)

public:
  static constexpr int ITERATIONS = 10;

protected:
  std::shared_ptr<SpatialPrefixTree> grid;

private:
  std::shared_ptr<SpatialContext> ctx2D;

public:
  virtual void setupGrid(int maxLevels) ;

private:
  void setupCtx2D(std::shared_ptr<SpatialContext> ctx);

  void setupQuadGrid(int maxLevels, bool packedQuadPrefixTree);

public:
  virtual void setupGeohashGrid(int maxLevels);

protected:
  virtual std::shared_ptr<RecursivePrefixTreeStrategy> newRPT();

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
  // testIntersects() throws java.io.IOException
  virtual void testIntersects() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
  // testWithin() throws java.io.IOException
  virtual void testWithin() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
  // testContains() throws java.io.IOException
  virtual void testContains() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPackedQuadPointsOnlyBug() throws
  // java.io.IOException
  virtual void testPackedQuadPointsOnlyBug() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPointsOnlyOptBug() throws
  // java.io.IOException
  virtual void testPointsOnlyOptBug() ;

  /** See LUCENE-5062, {@link
   * ContainsPrefixTreeQuery#multiOverlappingIndexedShapes}. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testContainsPairOverlap() throws
  // java.io.IOException
  virtual void testContainsPairOverlap() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithinDisjointParts() throws
  // java.io.IOException
  virtual void testWithinDisjointParts() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithinLeafApproxRule() throws
  // java.io.IOException
  virtual void testWithinLeafApproxRule() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testShapePair()
  virtual void testShapePair();

  // Override so we can index parts of a pair separately, resulting in the
  // detailLevel
  // being independent for each shape vs the whole thing
protected:
  std::shared_ptr<Document> newDoc(const std::wstring &id,
                                   std::shared_ptr<Shape> shape) override;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") private void doTest(final
  // org.apache.lucene.spatial.query.SpatialOperation operation) throws
  // java.io.IOException
  void doTest(std::shared_ptr<SpatialOperation> operation) ;

  std::shared_ptr<Shape> randomShapePairRect(bool biasContains);

  void fail(
      const std::wstring &label, const std::wstring &id,
      std::unordered_map<std::wstring, std::shared_ptr<Shape>> &indexedShapes,
      std::unordered_map<std::wstring, std::shared_ptr<Shape>> &indexedShapesGS,
      std::shared_ptr<Shape> queryShape);

  //  private Rectangle inset(Rectangle r) {
  //    //typically inset by 1 (whole numbers are easy to read)
  //    double d = Math.min(1.0, grid.getDistanceForLevel(grid.getMaxLevels()) /
  //    4); return ctx.makeRectangle(r.getMinX() + d, r.getMaxX() - d,
  //    r.getMinY() + d, r.getMaxY() - d);
  //  }

protected:
  virtual std::shared_ptr<Shape> gridSnap(std::shared_ptr<Shape> snapMe);

  /**
   * An aggregate of 2 shapes. Unfortunately we can't simply use a
   * ShapeCollection because: (a) ambiguity between CONTAINS and WITHIN for
   * equal shapes, and (b) adjacent pairs could as a whole contain the input
   * shape. The tests here are sensitive to these matters, although in practice
   * ShapeCollection is fine.
   */
private:
  class ShapePair : public ShapeCollection<std::shared_ptr<Shape>>
  {
    GET_CLASS_NAME(ShapePair)
  private:
    std::shared_ptr<RandomSpatialOpFuzzyPrefixTreeTest> outerInstance;

  public:
    const std::shared_ptr<Shape> shape1, shape2;
    const std::shared_ptr<Shape> shape1_2D, shape2_2D; // not geo (bit of a
                                                       // hack)
    const bool biasContainsThenWithin;

    ShapePair(std::shared_ptr<RandomSpatialOpFuzzyPrefixTreeTest> outerInstance,
              std::shared_ptr<Shape> shape1, std::shared_ptr<Shape> shape2,
              bool containsThenWithin);

  private:
    std::shared_ptr<Shape> toNonGeo(std::shared_ptr<Shape> shape);

  public:
    std::shared_ptr<SpatialRelation>
    relate(std::shared_ptr<Shape> other) override;

  private:
    bool cornerContainsNonGeo(double x, double y);

    std::shared_ptr<SpatialRelation> relateApprox(std::shared_ptr<Shape> other);

  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<ShapePair> shared_from_this()
    {
      return std::static_pointer_cast<ShapePair>(
          org.locationtech.spatial4j.shape.ShapeCollection<
              org.locationtech.spatial4j.shape.Shape>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<RandomSpatialOpFuzzyPrefixTreeTest> shared_from_this()
  {
    return std::static_pointer_cast<RandomSpatialOpFuzzyPrefixTreeTest>(
        org.apache.lucene.spatial.StrategyTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
