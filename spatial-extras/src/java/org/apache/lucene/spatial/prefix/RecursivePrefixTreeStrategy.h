#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}

namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::spatial::query
{
class SpatialArgs;
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
namespace org::apache::lucene::spatial::prefix
{

using Query = org::apache::lucene::search::Query;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using org::locationtech::spatial4j::shape::Shape;

/**
 * A {@link PrefixTreeStrategy} which uses {@link
 * AbstractVisitingPrefixTreeQuery}. This strategy has support for searching
 * non-point shapes (note: not tested). Even a query shape with distErrPct=0
 * (fully precise to the grid) should have good performance for typical data,
 * unless there is a lot of indexed data coincident with the shape's edge.
 *
 * @lucene.experimental
 */
class RecursivePrefixTreeStrategy : public PrefixTreeStrategy
{
  GET_CLASS_NAME(RecursivePrefixTreeStrategy)
  /* Future potential optimizations:

    Each shape.relate(otherShape) result could be cached since much of the same
    relations will be invoked when multiple segments are involved. Do this for
    "complex" shapes, not cheap ones, and don't cache when disjoint to bbox
    because it's a cheap calc. This is one advantage TermQueryPrefixTreeStrategy
    has over RPT.

   */

protected:
  int prefixGridScanLevel = 0;

  // Formerly known as simplifyIndexedCells. Eventually will be removed. Only
  // compatible with RPT
  // and cells implementing CellCanPrune, otherwise ignored.
  bool pruneLeafyBranches = true;

  bool multiOverlappingIndexedShapes = true;

public:
  RecursivePrefixTreeStrategy(std::shared_ptr<SpatialPrefixTree> grid,
                              const std::wstring &fieldName);

  virtual int getPrefixGridScanLevel();

  /**
   * Sets the grid level [1-maxLevels] at which indexed terms are scanned
   * brute-force instead of by grid decomposition.  By default this is maxLevels
   * - 4.  The final level, maxLevels, is always scanned.
   *
   * @param prefixGridScanLevel 1 to maxLevels
   */
  virtual void setPrefixGridScanLevel(int prefixGridScanLevel);

  virtual bool isMultiOverlappingIndexedShapes();

  /** See {@link ContainsPrefixTreeQuery#multiOverlappingIndexedShapes}. */
  virtual void
  setMultiOverlappingIndexedShapes(bool multiOverlappingIndexedShapes);

  virtual bool isPruneLeafyBranches();

  /**
   * An optional hint affecting non-point shapes and tree cells implementing
   * {@link CellCanPrune}, otherwise ignored. <p> It will prune away a complete
   * set sibling leaves to their parent (recursively), resulting in ~20-50%
   * fewer indexed cells, and consequently that much less disk and that much
   * faster indexing. So if it's a quad tree and all 4 sub-cells are there
   * marked as a leaf, then they will be removed (pruned) and the parent is
   * marked as a leaf instead.  This occurs recursively on up.  Unfortunately,
   * the current implementation will buffer all cells to do this, so consider
   * disabling for high precision (low distErrPct) shapes. (default=true)
   */
  virtual void setPruneLeafyBranches(bool pruneLeafyBranches);

  virtual std::wstring toString();

protected:
  std::shared_ptr<Iterator<std::shared_ptr<Cell>>> createCellIteratorToIndex(
      std::shared_ptr<Shape> shape, int detailLevel,
      std::shared_ptr<Iterator<std::shared_ptr<Cell>>> reuse) override;

  /** Returns true if cell was added as a leaf. If it wasn't it recursively
   * descends. */
private:
  bool recursiveTraverseAndPrune(std::shared_ptr<Cell> cell,
                                 std::shared_ptr<Shape> shape, int detailLevel,
                                 std::deque<std::shared_ptr<Cell>> &result);

public:
  std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialArgs> args) override;

  /**
   * A quick check of the shape to see if it is perfectly aligned to a grid.
   * Points always are as they are indivisible.  It's okay to return false
   * if the shape actually is aligned; this is an optimization hint.
   */
protected:
  virtual bool isGridAlignedShape(std::shared_ptr<Shape> shape);

  /** {@link #makeQuery(SpatialArgs)} specialized for the query being a grid
   * square. */
  virtual std::shared_ptr<Query>
  makeGridShapeIntersectsQuery(std::shared_ptr<Shape> gridShape);

protected:
  std::shared_ptr<RecursivePrefixTreeStrategy> shared_from_this()
  {
    return std::static_pointer_cast<RecursivePrefixTreeStrategy>(
        PrefixTreeStrategy::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
