#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

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
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using BytesRef = org::apache::lucene::util::BytesRef;

/** The base for the original two SPT's: Geohash and Quad. Don't subclass this
 * for new SPTs.
 * @lucene.internal */
class LegacyPrefixTree : public SpatialPrefixTree
{
  GET_CLASS_NAME(LegacyPrefixTree)
public:
  LegacyPrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels);

  double getDistanceForLevel(int level) override;

  /**
   * Returns the cell containing point {@code p} at the specified {@code level}.
   */
protected:
  virtual std::shared_ptr<Cell> getCell(std::shared_ptr<Point> p,
                                        int level) = 0;

public:
  std::shared_ptr<Cell> readCell(std::shared_ptr<BytesRef> term,
                                 std::shared_ptr<Cell> scratch) override;

  std::shared_ptr<CellIterator>
  getTreeCellIterator(std::shared_ptr<Shape> shape, int detailLevel) override;

protected:
  std::shared_ptr<LegacyPrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<LegacyPrefixTree>(
        SpatialPrefixTree::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix::tree
