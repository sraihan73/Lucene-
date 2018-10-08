#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A {@link SpatialPrefixTree} which uses a
 * <a href="http://en.wikipedia.org/wiki/Quadtree">quad tree</a> in which an
 * indexed term will be generated for each cell, 'A', 'B', 'C', 'D'.
 *
 * @lucene.experimental
 */
class QuadPrefixTree : public LegacyPrefixTree
{
  GET_CLASS_NAME(QuadPrefixTree)

  /**
   * Factory for creating {@link QuadPrefixTree} instances with useful defaults
   */
public:
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

public:
  static constexpr int MAX_LEVELS_POSSIBLE =
      50; // not really sure how big this should be

  static constexpr int DEFAULT_MAX_LEVELS = 12;

protected:
  const double xmin;
  const double xmax;
  const double ymin;
  const double ymax;
  const double xmid;
  const double ymid;

  const double gridW;

public:
  const double gridH;

  std::deque<double> const levelW;
  std::deque<double> const levelH;
  std::deque<int> const levelS; // side
  std::deque<int> const levelN; // number

  QuadPrefixTree(std::shared_ptr<SpatialContext> ctx,
                 std::shared_ptr<Rectangle> bounds, int maxLevels);

  QuadPrefixTree(std::shared_ptr<SpatialContext> ctx);

  QuadPrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels);

  std::shared_ptr<Cell> getWorldCell() override;

  virtual void printInfo(std::shared_ptr<PrintStream> out);

  int getLevelForDistance(double dist) override;

  std::shared_ptr<Cell> getCell(std::shared_ptr<Point> p, int level) override;

private:
  void build(double x, double y, int level,
             std::deque<std::shared_ptr<Cell>> &matches,
             std::shared_ptr<BytesRef> str, std::shared_ptr<Shape> shape,
             int maxLevel);

protected:
  virtual void checkBattenberg(wchar_t c, double cx, double cy, int level,
                               std::deque<std::shared_ptr<Cell>> &matches,
                               std::shared_ptr<BytesRef> str,
                               std::shared_ptr<Shape> shape, int maxLevel);

protected:
  class QuadCell : public LegacyCell
  {
    GET_CLASS_NAME(QuadCell)
  private:
    std::shared_ptr<QuadPrefixTree> outerInstance;

  public:
    QuadCell(std::shared_ptr<QuadPrefixTree> outerInstance,
             std::deque<char> &bytes, int off, int len);

    QuadCell(std::shared_ptr<QuadPrefixTree> outerInstance,
             std::shared_ptr<BytesRef> str,
             std::shared_ptr<SpatialRelation> shapeRel);

  protected:
    std::shared_ptr<QuadPrefixTree> getGrid() override;
    int getMaxLevels() override;
    std::shared_ptr<std::deque<std::shared_ptr<Cell>>> getSubCells() override;

    virtual std::shared_ptr<BytesRef> concat(std::shared_ptr<BytesRef> source,
                                             char b);

  public:
    int getSubCellsSize() override;

  protected:
    std::shared_ptr<QuadCell> getSubCell(std::shared_ptr<Point> p) override;

  public:
    std::shared_ptr<Shape> getShape() override;

  protected:
    virtual std::shared_ptr<Rectangle> makeShape();

  protected:
    std::shared_ptr<QuadCell> shared_from_this()
    {
      return std::static_pointer_cast<QuadCell>(LegacyCell::shared_from_this());
    }
  }; // QuadCell

protected:
  std::shared_ptr<QuadPrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<QuadPrefixTree>(
        LegacyPrefixTree::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
