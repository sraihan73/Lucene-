#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class CellIterator;
}

namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
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

using org::locationtech::spatial4j::shape::Shape;

/**
 * Navigates a {@link org.apache.lucene.spatial.prefix.tree.SpatialPrefixTree}
 * from a given cell (typically the world cell) down to a maximum number of
 * configured levels, filtered by a given shape. Intermediate non-leaf cells are
 * returned. It supports {@link #remove()} for skipping traversal of subcells of
 * the current cell.
 *
 * @lucene.internal
 */
class TreeCellIterator : public CellIterator
{
  GET_CLASS_NAME(TreeCellIterator)
  // This class uses a stack approach, which is more efficient than creating
  // linked nodes. And it might more easily
  // pave the way for re-using Cell & CellIterator at a given level in the
  // future.

private:
  const std::shared_ptr<Shape> shapeFilter; // possibly null
  std::deque<std::shared_ptr<CellIterator>> const
      iterStack;    // starts at level 1
  int stackIdx = 0; //-1 when done
  bool descend = false;

public:
  TreeCellIterator(std::shared_ptr<Shape> shapeFilter, int detailLevel,
                   std::shared_ptr<Cell> parentCell);

  bool hasNext() override;

  void remove() override;

  // TODO implement a smart nextFrom() that looks at the parent's bytes first

protected:
  std::shared_ptr<TreeCellIterator> shared_from_this()
  {
    return std::static_pointer_cast<TreeCellIterator>(
        CellIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix::tree
