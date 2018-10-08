#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/PointPrefixTreeFieldCacheProvider.h"
#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/CellToBytesRefIterator.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/HeatmapFacetCounter.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/Heatmap.h"

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

using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

/**
 * An abstract SpatialStrategy based on {@link SpatialPrefixTree}. The two
 * subclasses are {@link RecursivePrefixTreeStrategy} and {@link
 * TermQueryPrefixTreeStrategy}.  This strategy is most effective as a fast
 * approximate spatial search filter.
 * <p>
 * <b>Characteristics:</b>
 * <br>
 * <ul>
 * <li>Can index any shape; however only {@link RecursivePrefixTreeStrategy}
 * can effectively search non-point shapes.</li>
 * <li>Can index a variable number of shapes per field value. This strategy
 * can do it via multiple calls to {@link
 * #createIndexableFields(org.locationtech.spatial4j.shape.Shape)} for a
 * document or by giving it some sort of Shape aggregate (e.g. JTS WKT
 * MultiPoint).  The shape's boundary is approximated to a grid precision.
 * </li>
 * <li>Can query with any shape.  The shape's boundary is approximated to a grid
 * precision.</li>
 * <li>Only {@link org.apache.lucene.spatial.query.SpatialOperation#Intersects}
 * is supported.  If only points are indexed then this is effectively equivalent
 * to IsWithin.</li>
 * <li>The strategy supports {@link
 * #makeDistanceValueSource(org.locationtech.spatial4j.shape.Point,double)} even
 * for multi-valued data, so long as the indexed data is all points; the
 * behavior is undefined otherwise.  However, <em>it will likely be removed in
 * the future</em> in lieu of using another strategy with a more scalable
 * implementation.  Use of this call is the only
 * circumstance in which a cache is used.  The cache is simple but as such
 * it doesn't scale to large numbers of points nor is it real-time-search
 * friendly.</li>
 * </ul>
 * <p>
 * <b>Implementation:</b>
 * <p>
 * The {@link SpatialPrefixTree} does most of the work, for example returning
 * a deque of terms representing grids of various sizes for a supplied shape.
 * An important
 * configuration item is {@link #setDistErrPct(double)} which balances
 * shape precision against scalability.  See those javadocs.
 *
 * @lucene.experimental
 */
class PrefixTreeStrategy : public SpatialStrategy
{
  GET_CLASS_NAME(PrefixTreeStrategy)
protected:
  const std::shared_ptr<SpatialPrefixTree> grid;

private:
  const std::unordered_map<std::wstring,
                           std::shared_ptr<PointPrefixTreeFieldCacheProvider>>
      provider = std::make_shared<ConcurrentHashMap<
          std::wstring, std::shared_ptr<PointPrefixTreeFieldCacheProvider>>>();

protected:
  int defaultFieldValuesArrayLen = 2;
  double distErrPct = SpatialArgs::DEFAULT_DISTERRPCT; // [ 0 TO 0.5 ]
  bool pointsOnly = false; // if true, there are no leaves

public:
  PrefixTreeStrategy(std::shared_ptr<SpatialPrefixTree> grid,
                     const std::wstring &fieldName);

  virtual std::shared_ptr<SpatialPrefixTree> getGrid();

  /**
   * A memory hint used by {@link
   * #makeDistanceValueSource(org.locationtech.spatial4j.shape.Point)} for how
   * big the initial size of each Document's array should be. The default is 2.
   * Set this to slightly more than the default expected number of points per
   * document.
   */
  virtual void setDefaultFieldValuesArrayLen(int defaultFieldValuesArrayLen);

  virtual double getDistErrPct();

  /**
   * The default measure of shape precision affecting shapes at index and query
   * times. Points don't use this as they are always indexed at the configured
   * maximum precision ({@link
   * org.apache.lucene.spatial.prefix.tree.SpatialPrefixTree#getMaxLevels()});
   * this applies to all other shapes. Specific shapes at index and query time
   * can use something different than this default value.  If you don't set a
   * default then the default is {@link SpatialArgs#DEFAULT_DISTERRPCT} --
   * 2.5%.
   *
   * @see org.apache.lucene.spatial.query.SpatialArgs#getDistErrPct()
   */
  virtual void setDistErrPct(double distErrPct);

  virtual bool isPointsOnly();

  /** True if only indexed points shall be supported. There are no "leafs" in
   * such a case, except those at maximum precision. */
  virtual void setPointsOnly(bool pointsOnly);

  std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape) override;

  /**
   * Turns {@link SpatialPrefixTree#getTreeCellIterator(Shape, int)} into a
   * {@link org.apache.lucene.analysis.TokenStream}.
   */
  virtual std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape, double distErr);

  virtual std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape, int detailLevel);

protected:
  virtual std::shared_ptr<CellToBytesRefIterator> newCellToBytesRefIterator();

  virtual std::shared_ptr<Iterator<std::shared_ptr<Cell>>>
  createCellIteratorToIndex(
      std::shared_ptr<Shape> shape, int detailLevel,
      std::shared_ptr<Iterator<std::shared_ptr<Cell>>> reuse);

  /* Indexed, tokenized, not stored. */
public:
  static const std::shared_ptr<FieldType> FIELD_TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static PrefixTreeStrategy::StaticConstructor staticConstructor;

public:
  std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) override;

  /**
   * Computes spatial facets in two dimensions as a grid of numbers.  The data
   * is often visualized as a so-called "heatmap".
   *
   * @see HeatmapFacetCounter#calcFacets(PrefixTreeStrategy, IndexReaderContext,
   * Bits, Shape, int, int)
   */
  virtual std::shared_ptr<HeatmapFacetCounter::Heatmap>
  calcFacets(std::shared_ptr<IndexReaderContext> context,
             std::shared_ptr<Bits> topAcceptDocs,
             std::shared_ptr<Shape> inputShape, int const facetLevel,
             int maxCells) ;

  /**
   * Returns true if the {@code shape} is a {@link Point}.  For custom spatial
   * contexts, it may make sense to have certain other shapes return true.
   * @lucene.experimental
   */
protected:
  virtual bool isPointShape(std::shared_ptr<Shape> shape);

protected:
  std::shared_ptr<PrefixTreeStrategy> shared_from_this()
  {
    return std::static_pointer_cast<PrefixTreeStrategy>(
        org.apache.lucene.spatial.SpatialStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
