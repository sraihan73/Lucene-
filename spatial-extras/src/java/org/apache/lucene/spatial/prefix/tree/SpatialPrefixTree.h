#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::spatial::prefix::tree
{
class CellIterator;
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
namespace org::apache::lucene::spatial::prefix::tree
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A spatial Prefix Tree, or Trie, which decomposes shapes into prefixed strings
 * at variable lengths corresponding to variable precision.  Each string
 * corresponds to a rectangular spatial region.  This approach is
 * also referred to "Grids", "Tiles", and "Spatial Tiers".
 * <p>
 * Implementations of this class should be thread-safe and immutable once
 * initialized.
 *
 * @lucene.experimental
 */
class SpatialPrefixTree : public std::enable_shared_from_this<SpatialPrefixTree>
{
  GET_CLASS_NAME(SpatialPrefixTree)

protected:
  const int maxLevels;

  const std::shared_ptr<SpatialContext> ctx;

public:
  SpatialPrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels);

  virtual std::shared_ptr<SpatialContext> getSpatialContext();

  virtual int getMaxLevels();

  virtual std::wstring toString();

  /**
   * Returns the level of the largest grid in which its longest side is less
   * than or equal to the provided distance (in degrees). Consequently {@code
   * dist} acts as an error epsilon declaring the amount of detail needed in the
   * grid, such that you can get a grid with just the right amount of
   * precision.
   *
   * @param dist {@code >= 0}
   * @return level [1 to maxLevels]
   */
  virtual int getLevelForDistance(double dist) = 0;

  /**
   * Given a cell having the specified level, returns the distance from opposite
   * corners. Since this might vary depending on where the cell is, this method
   * may over-estimate.
   *
   * @param level [1 to maxLevels]
   * @return {@code > 0}
   */
  virtual double getDistanceForLevel(int level) = 0;

  /**
   * Returns the level 0 cell which encompasses all spatial data. Equivalent to
   * {@link #readCell(BytesRef,Cell)} with no bytes.
   */
  virtual std::shared_ptr<Cell>
  getWorldCell() = 0; // another possible name: getTopCell

  /**
   * This creates a new Cell (or re-using {@code scratch} if provided),
   * initialized to the state as read by the bytes. Warning: An implementation
   * may refer to the same byte array (no copy). If {@link Cell#setLeaf()} is
   * subsequently called, it would then modify these bytes.
   */
  virtual std::shared_ptr<Cell> readCell(std::shared_ptr<BytesRef> term,
                                         std::shared_ptr<Cell> scratch) = 0;

  /**
   * Gets the intersecting cells for the specified shape, without exceeding
   * detail level. If a cell is within the query shape then it's marked as a
   * leaf and none of its children are added. For cells at detailLevel, they are
   * marked as leaves too, unless it's a point. <p> IMPORTANT: Cells returned
   * from the iterator can be re-used for cells at the same level. So you can't
   * simply iterate to subsequent cells and still refer to the former cell nor
   * the bytes returned from the former cell, unless you know the former cell is
   * a parent.
   *
   * @param shape       the shape; possibly null but the caller should liberally
   * call
   *  {@code remove()} if so.
   * @param detailLevel the maximum detail level to get cells for
   * @return the matching cells
   */
  virtual std::shared_ptr<CellIterator>
  getTreeCellIterator(std::shared_ptr<Shape> shape, int detailLevel);
};

} // namespace org::apache::lucene::spatial::prefix::tree
