#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/S2ShapeFactory.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/CellIterator.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Spatial prefix tree for <a href="https://s2geometry.io/">S2 Geometry</a>.
 * Shape factories for the given {@link SpatialContext} must implement the
 * interface {@link S2ShapeFactory}.
 *
 * The tree can be configured on how it divided itself by providing an arity.
 * The default arity is 1 which divided every sub-cell in 4 (except the first
 * level that is always divided by 6) . Arity 2 divides sub-cells in 16 and
 * arity 3 in 64 sub-cells.
 *
 * @lucene.experimental
 */
class S2PrefixTree : public SpatialPrefixTree
{
  GET_CLASS_NAME(S2PrefixTree)

  /**
   * Factory for creating {@link S2PrefixTree} instances with useful defaults
   */
protected:
  class Factory : public SpatialPrefixTreeFactory
  {
    GET_CLASS_NAME(Factory)

  protected:
    int getLevelForDistance(double degrees) override;

    std::shared_ptr<SpatialPrefixTree> newSPT() override;

  protected:
    std::shared_ptr<Factory> shared_from_this()
    {
      return std::static_pointer_cast<Factory>(
          SpatialPrefixTreeFactory::shared_from_this());
    }
  };

  // factory to generate S2 cell shapes
protected:
  const std::shared_ptr<S2ShapeFactory> s2ShapeFactory;
  const int arity;

  /**
   * Creates a S2 spatial tree with arity 1.
   *
   * @param ctx The provided spatial context. The shape factor of the spatial
   * context must implement {@link S2ShapeFactory}
   * @param maxLevels The provided maximum level for this tree.
   */
public:
  S2PrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels);

  /**
   * Creates a S2 spatial tree with provided arity.
   *
   * @param ctx The provided spatial context. The shape factor of the spatial
   * context must implement {@link S2ShapeFactory}
   * @param maxLevels The provided maximum level for this tree.
   * @param arity The arity of the tree.
   */
  S2PrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels, int arity);

  /**
   * Get max levels for this spatial tree.
   *
   * @param arity The arity of the tree.
   * @return The maximum number of levels by the provided arity.
   */
  static int getMaxLevels(int arity);

  int getLevelForDistance(double dist) override;

  double getDistanceForLevel(int level) override;

  std::shared_ptr<Cell> getWorldCell() override;

  std::shared_ptr<Cell> readCell(std::shared_ptr<BytesRef> term,
                                 std::shared_ptr<Cell> scratch) override;

  std::shared_ptr<CellIterator>
  getTreeCellIterator(std::shared_ptr<Shape> shape, int detailLevel) override;

protected:
  std::shared_ptr<S2PrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<S2PrefixTree>(
        SpatialPrefixTree::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
