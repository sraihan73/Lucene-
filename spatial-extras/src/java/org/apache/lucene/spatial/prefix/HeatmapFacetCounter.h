#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReaderContext;
}

namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::spatial::prefix
{
class PrefixTreeStrategy;
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
namespace org::apache::lucene::spatial::prefix
{

using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Computes spatial facets in two dimensions as a grid of numbers.  The data is
 * often visualized as a so-called "heatmap", hence the name.
 *
 * @lucene.experimental
 */
class HeatmapFacetCounter
    : public std::enable_shared_from_this<HeatmapFacetCounter>
{
  GET_CLASS_NAME(HeatmapFacetCounter)
  // TODO where should this code live? It could go to PrefixTreeFacetCounter, or
  // maybe here in its own class is fine.

  /** Maximum number of supported rows (or columns). */
public:
  static const int MAX_ROWS_OR_COLUMNS =
      static_cast<int>(std::sqrt(ArrayUtil::MAX_ARRAY_LENGTH));

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static HeatmapFacetCounter::StaticConstructor staticConstructor;

  /** Response structure */
public:
  class Heatmap : public std::enable_shared_from_this<Heatmap>
  {
    GET_CLASS_NAME(Heatmap)
  public:
    const int columns;
    const int rows;
    std::deque<int> const counts; // in order of 1st column (all rows) then 2nd
                                   // column (all rows) etc.
    const std::shared_ptr<Rectangle> region;

    Heatmap(int columns, int rows, std::shared_ptr<Rectangle> region);

    virtual int getCount(int x, int y);

    virtual std::wstring toString();
  };

  /**
   * Calculates spatial 2D facets (aggregated counts) in a grid, sometimes
   * called a heatmap. Facet computation is implemented by navigating the
   * underlying indexed terms efficiently. If you don't know exactly what
   * facetLevel to go to for a given input box but you have some sense of how
   * many cells there should be relative to the size of the shape, then consider
   * using the logic that {@link
   * org.apache.lucene.spatial.prefix.PrefixTreeStrategy} uses when
   * approximating what level to go to when indexing a shape given a distErrPct.
   *
   * @param context the IndexReader's context
   * @param topAcceptDocs a Bits to limit counted docs.  If null, live docs are
   * counted.
   * @param inputShape the shape to gather grid squares for; typically a {@link
   * Rectangle}. The <em>actual</em> heatmap area will usually be larger since
   * the cells on the edge that overlap are returned. We always return a
   * rectangle of integers even if the inputShape isn't a rectangle
   *                   -- the non-intersecting cells will all be 0.
   *                   If null is given, the entire world is assumed.
   * @param facetLevel the target depth (detail) of cells.
   * @param maxCells the maximum number of cells to return. If the cells exceed
   * this count, an
   */
public:
  static std::shared_ptr<Heatmap>
  calcFacets(std::shared_ptr<PrefixTreeStrategy> strategy,
             std::shared_ptr<IndexReaderContext> context,
             std::shared_ptr<Bits> topAcceptDocs,
             std::shared_ptr<Shape> inputShape, int const facetLevel,
             int maxCells) ;

private:
  class FacetVisitorAnonymousInnerClass
      : public PrefixTreeFacetCounter::FacetVisitor
  {
    GET_CLASS_NAME(FacetVisitorAnonymousInnerClass)
  private:
    int facetLevel = 0;
    double heatMinX = 0;
    double cellWidth = 0;
    double heatMinY = 0;
    double cellHeight = 0;
    std::shared_ptr<
        org::apache::lucene::spatial::prefix::HeatmapFacetCounter::Heatmap>
        heatmap;
    std::deque<int> allCellsAncestorCount;
    std::unordered_map<std::shared_ptr<Rectangle>, int> ancestors;

  public:
    FacetVisitorAnonymousInnerClass(
        int facetLevel, double heatMinX, double cellWidth, double heatMinY,
        double cellHeight,
        std::shared_ptr<
            org::apache::lucene::spatial::prefix::HeatmapFacetCounter::Heatmap>
            heatmap,
        std::deque<int> &allCellsAncestorCount,
        std::unordered_map<std::shared_ptr<Rectangle>, int> &ancestors);

    void visit(std::shared_ptr<Cell> cell, int count) override;

  protected:
    std::shared_ptr<FacetVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FacetVisitorAnonymousInnerClass>(
          PrefixTreeFacetCounter.FacetVisitor::shared_from_this());
    }
  };

private:
  static void intersectInterval(double heatMin, double heatMax,
                                double heatCellLen, int numCells,
                                double cellMin, double cellMax,
                                std::deque<int> &out);

  static void incrementRange(std::shared_ptr<Heatmap> heatmap, int startColumn,
                             int endColumn, int startRow, int endRow,
                             int count);

  /** Computes the number of intervals (rows or columns) to cover a range given
   * the sizes. */
  static int calcRowsOrCols(double cellRange, double cellMin,
                            double requestRange, double requestMin,
                            double worldRange);

  HeatmapFacetCounter();
};

} // namespace org::apache::lucene::spatial::prefix
