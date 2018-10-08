#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/NumberRangePrefixTree.h"

#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/Facets.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/UnitNRShape.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/FacetParentVal.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"

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
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using NumberRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::NumberRangePrefixTree;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape;

/** A PrefixTree based on Number/Date ranges. This isn't very "spatial" on the
 * surface (to the user) but it's implemented using spatial so that's why it's
 * here extending a SpatialStrategy. When using this class, you will use various
 * utility methods on the prefix tree implementation to convert objects/strings
 * to/from shapes.
 *
 * To use with dates, pass in {@link
 * org.apache.lucene.spatial.prefix.tree.DateRangePrefixTree}.
 *
 * @lucene.experimental
 */
class NumberRangePrefixTreeStrategy : public RecursivePrefixTreeStrategy
{
  GET_CLASS_NAME(NumberRangePrefixTreeStrategy)

public:
  NumberRangePrefixTreeStrategy(
      std::shared_ptr<NumberRangePrefixTree> prefixTree,
      const std::wstring &fieldName);

  std::shared_ptr<NumberRangePrefixTree> getGrid() override;

protected:
  bool isPointShape(std::shared_ptr<Shape> shape) override;

  bool isGridAlignedShape(std::shared_ptr<Shape> shape) override;

  /** Unsupported. */
public:
  std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) override;

  /** Calculates facets between {@code start} and {@code end} to a detail level
   * one greater than that provided by the arguments. For example providing
   * March to October of 2014 would return facets to the day level of those
   * months. This is just a convenience method.
   * @see #calcFacets(IndexReaderContext, Bits, Shape, int)
   */
  virtual std::shared_ptr<Facets>
  calcFacets(std::shared_ptr<IndexReaderContext> context,
             std::shared_ptr<Bits> topAcceptDocs,
             std::shared_ptr<NumberRangePrefixTree::UnitNRShape> start,
             std::shared_ptr<NumberRangePrefixTree::UnitNRShape>
                 end) ;

  /**
   * Calculates facets (aggregated counts) given a range shape (start-end span)
   * and a level, which specifies the detail. To get the level of an existing
   * shape, say a Calendar, call
   * {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree#toUnitShape(Object)}
   * then call
   * {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape#getLevel()}.
   * Facet computation is implemented by navigating the underlying indexed terms
   * efficiently.
   */
  virtual std::shared_ptr<Facets>
  calcFacets(std::shared_ptr<IndexReaderContext> context,
             std::shared_ptr<Bits> topAcceptDocs,
             std::shared_ptr<Shape> facetRange,
             int const level) ;

private:
  class FacetVisitorAnonymousInnerClass
      : public PrefixTreeFacetCounter::FacetVisitor
  {
    GET_CLASS_NAME(FacetVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<NumberRangePrefixTreeStrategy> outerInstance;

    int level = 0;
    std::shared_ptr<org::apache::lucene::spatial::prefix::
                        NumberRangePrefixTreeStrategy::Facets>
        facets;

  public:
    FacetVisitorAnonymousInnerClass(
        std::shared_ptr<NumberRangePrefixTreeStrategy> outerInstance, int level,
        std::shared_ptr<org::apache::lucene::spatial::prefix::
                            NumberRangePrefixTreeStrategy::Facets>
            facets);

    std::shared_ptr<Facets::FacetParentVal> parentFacet;
    std::shared_ptr<NumberRangePrefixTree::UnitNRShape> parentShape;

    void visit(std::shared_ptr<Cell> cell, int count) override;

  private:
    void
    setupParent(std::shared_ptr<NumberRangePrefixTree::UnitNRShape> unitShape);

  protected:
    std::shared_ptr<FacetVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FacetVisitorAnonymousInnerClass>(
          PrefixTreeFacetCounter.FacetVisitor::shared_from_this());
    }
  };

  /** Facet response information */
public:
  class Facets : public std::enable_shared_from_this<Facets>
  {
    GET_CLASS_NAME(Facets)
    // TODO consider a variable-level structure -- more general purpose.

  public:
    Facets(int detailLevel);

    /** The bottom-most detail-level counted, as requested. */
    const int detailLevel;

    /**
     * The count of documents with ranges that completely spanned the parents of
     * the detail level. In more technical terms, this is the count of leaf
     * cells 2 up and higher from the bottom. Usually you only care about counts
     * at detailLevel, and so you will add this number to all other counts
     * below, including to omitted/implied children counts of 0. If there are no
     * indexed ranges (just instances, i.e. fully specified dates) then this
     * value will always be 0.
     */
    int topLeaves = 0;

    /** Holds all the {@link FacetParentVal} instances in order of the key. This
     * is sparse; there won't be an instance if it's count and children are all
     * 0. The keys are {@link
     * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape}
     * shapes, which can be converted back to the original Object (i.e. a
     * Calendar) via
     * {@link
     * NumberRangePrefixTree#toObject(org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape)}.
     */
    const std::shared_ptr<
        SortedMap<std::shared_ptr<NumberRangePrefixTree::UnitNRShape>,
                  std::shared_ptr<FacetParentVal>>>
        parents = std::map_obj<std::shared_ptr<NumberRangePrefixTree::UnitNRShape>,
                           std::shared_ptr<FacetParentVal>>();

    /** Holds a block of detailLevel counts aggregated to their parent level. */
  public:
    class FacetParentVal : public std::enable_shared_from_this<FacetParentVal>
    {
      GET_CLASS_NAME(FacetParentVal)

      /** The count of ranges that span all of the childCounts.  In more
       * technical terms, this is the number of leaf
       * cells found at this parent.  Treat this like {@link Facets#topLeaves}.
       */
    public:
      int parentLeaves = 0;

      /** The length of {@link #childCounts}. If childCounts is not null then
       * this is childCounts.length, otherwise it says how long it would have
       * been if it weren't null. */
      int childCountsLen = 0;

      /** The detail level counts. It will be null if there are none, and thus
       * they are assumed 0. Most apps, when presenting the information, will
       * add {@link #topLeaves} and {@link #parentLeaves} to each count. */
      std::deque<int> childCounts;
      // assert childCountsLen == childCounts.length
    };

  public:
    virtual std::wstring toString();
  };

protected:
  std::shared_ptr<NumberRangePrefixTreeStrategy> shared_from_this()
  {
    return std::static_pointer_cast<NumberRangePrefixTreeStrategy>(
        RecursivePrefixTreeStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
