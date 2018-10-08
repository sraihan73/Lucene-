#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/VNode.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
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
namespace org::apache::lucene::spatial::prefix
{

using org::locationtech::spatial4j::shape::Shape;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Traverses a {@link SpatialPrefixTree} indexed field, using the template and
 * visitor design patterns for subclasses to guide the traversal and collect
 * matching documents.
 * <p>
 * Subclasses implement {@link
#getDocIdSet(org.apache.lucene.index.LeafReaderContext)}
 * by instantiating a custom {@link VisitorTemplate} subclass (i.e. an anonymous
inner class) GET_CLASS_NAME(es)
 * and implement the required methods.
 *
 * @lucene.internal
 */
class AbstractVisitingPrefixTreeQuery : public AbstractPrefixTreeQuery
{
  GET_CLASS_NAME(AbstractVisitingPrefixTreeQuery)

  // Historical note: this code resulted from a refactoring of
  // RecursivePrefixTreeQuery,
  // which in turn came out of SOLR-2155

  // This class perhaps could have been implemented in terms of
  // FilteredTermsEnum & MultiTermQuery.
  //  Maybe so for simple Intersects predicate but not for when we want to
  //  collect terms differently depending on cell state like IsWithin and for
  //  fuzzy/accurate collection planned improvements.  At least it would just
  //  make things more complicated.

protected:
  const int prefixGridScanLevel; // at least one less than grid.getMaxLevels()

public:
  AbstractVisitingPrefixTreeQuery(std::shared_ptr<Shape> queryShape,
                                  const std::wstring &fieldName,
                                  std::shared_ptr<SpatialPrefixTree> grid,
                                  int detailLevel, int prefixGridScanLevel);

  /**
   * An abstract class designed to make it easy to implement predicates or
   * other operations on a {@link SpatialPrefixTree} indexed field. An instance
GET_CLASS_NAME(designed)
   * of this class is not designed to be re-used across LeafReaderContext
   * instances so simply create a new one per-leaf.
   * The {@link #getDocIdSet()} method here starts the work. It first checks
   * that there are indexed terms; if not it quickly returns null. Then it calls
   * {@link #start()} so a subclass can set up a return value, like an
   * {@link org.apache.lucene.util.FixedBitSet}. Then it starts the traversal
GET_CLASS_NAME(can)
   * process, calling {@link
#findSubCellsToVisit(org.apache.lucene.spatial.prefix.tree.Cell)}
   * which by default finds the top cells that intersect {@code queryShape}. If
   * there isn't an indexed cell for a corresponding cell returned for this
   * method then it's short-circuited until it finds one, at which point
   * {@link #visitPrefix(org.apache.lucene.spatial.prefix.tree.Cell)} is called.
At
   * some depths, of the tree, the algorithm switches to a scanning mode that
   * calls {@link #visitScanned(org.apache.lucene.spatial.prefix.tree.Cell)}
   * for each leaf cell found.
   *
   * @lucene.internal
   */
public:
  class VisitorTemplate : public BaseTermsEnumTraverser
  {
    GET_CLASS_NAME(VisitorTemplate)
  private:
    std::shared_ptr<AbstractVisitingPrefixTreeQuery> outerInstance;

    /* Future potential optimizations:

    * Can a polygon query shape be optimized / made-simpler at recursive depths
      (e.g. intersection of shape + cell box)

    * RE "scan" vs divide & conquer performance decision:
      We should use termsEnum.docFreq() as an estimate on the number of places
    at this depth.  It would be nice if termsEnum knew how many terms start with
    the current term without having to repeatedly next() & test to find out.

    * Perhaps don't do intermediate seek()'s to cells above detailLevel that
    have Intersects relation because we won't be collecting those docs any way.
    However seeking does act as a short-circuit.  So maybe do some percent of
    the time or when the level is above some threshold.

    */

    //
    //  TODO MAJOR REFACTOR SIMPLIFICATION BASED ON TreeCellIterator  TODO
    //

    std::shared_ptr<VNode> curVNode; // current pointer, derived from query
                                     // shape
    std::shared_ptr<BytesRef> curVNodeTerm =
        std::make_shared<BytesRef>(); // curVNode.cell's term, without leaf. in
                                      // main loop only

    std::shared_ptr<BytesRef> thisTerm; // the result of termsEnum.term()
    std::shared_ptr<Cell> indexedCell;  // Cell wrapper of thisTerm. Always
                                       // updated when thisTerm is.

  public:
    VisitorTemplate(
        std::shared_ptr<AbstractVisitingPrefixTreeQuery> outerInstance,
        std::shared_ptr<LeafReaderContext> context) ;

    virtual std::shared_ptr<DocIdSet> getDocIdSet() ;

    /** Called initially, and whenever {@link
     * #visitPrefix(org.apache.lucene.spatial.prefix.tree.Cell)} returns true.
     */
  private:
    void addIntersectingChildren() ;

    /**
     * Called when doing a divide and conquer to find the next intersecting
     * cells of the query shape that are beneath {@code cell}. {@code cell} is
     * guaranteed to have an intersection and thus this must return some number
     * of nodes.
     */
  protected:
    virtual std::shared_ptr<CellIterator>
    findSubCellsToVisit(std::shared_ptr<Cell> cell);

    /**
     * Scans ({@code termsEnum.next()}) terms until a term is found that does
     * not start with curVNode's cell. If it finds a leaf cell or a cell at
     * level {@code scanDetailLevel} then it calls {@link
     * #visitScanned(org.apache.lucene.spatial.prefix.tree.Cell)}.
     */
    virtual void scan(int scanDetailLevel) ;

  private:
    bool nextTerm() ;

    /** Used for {@link VNode#children}. */
  private:
    class VNodeCellIterator
        : public std::enable_shared_from_this<VNodeCellIterator>,
          public Iterator<std::shared_ptr<VNode>>
    {
      GET_CLASS_NAME(VNodeCellIterator)
    private:
      std::shared_ptr<AbstractVisitingPrefixTreeQuery::VisitorTemplate>
          outerInstance;

    public:
      const std::shared_ptr<Iterator<std::shared_ptr<Cell>>> cellIter;

    private:
      const std::shared_ptr<VNode> vNode;

    public:
      VNodeCellIterator(
          std::shared_ptr<AbstractVisitingPrefixTreeQuery::VisitorTemplate>
              outerInstance,
          std::shared_ptr<Iterator<std::shared_ptr<Cell>>> cellIter,
          std::shared_ptr<VNode> vNode);

      bool hasNext() override;

      std::shared_ptr<VNode> next() override;

      void remove() override;
    };

    /** Called first to setup things. */
  protected:
    virtual void start() = 0;

    /** Called last to return the result. */
    virtual std::shared_ptr<DocIdSet> finish() = 0;

    /**
     * Visit an indexed non-leaf cell. The presence of a prefix cell implies
     * there are leaf cells at further levels. The cell passed should have it's
     * {@link org.apache.lucene.spatial.prefix.tree.Cell#getShapeRel()} set
     * relative to the filtered shape.
     *
     * @param cell An intersecting cell; not a leaf.
     * @return true to descend to more levels.
     */
    virtual bool visitPrefix(std::shared_ptr<Cell> cell) = 0;

    /**
     * Called when an indexed leaf cell is found. An
     * indexed leaf cell usually means associated documents won't be found at
     * further detail levels.  However, if a document has
     * multiple overlapping shapes at different resolutions, then this isn't
     * true.
     */
    virtual void visitLeaf(std::shared_ptr<Cell> cell) = 0;

    /**
     * The cell is either indexed as a leaf or is the last level of detail. It
     * might not even intersect the query shape, so be sure to check for that.
     * The default implementation will check that and if passes then call
     * {@link #visitLeaf(org.apache.lucene.spatial.prefix.tree.Cell)} or
     * {@link #visitPrefix(org.apache.lucene.spatial.prefix.tree.Cell)}.
     */
    virtual void visitScanned(std::shared_ptr<Cell> cell) ;

    virtual void preSiblings(std::shared_ptr<VNode> vNode) ;

    virtual void postSiblings(std::shared_ptr<VNode> vNode) ;

  protected:
    std::shared_ptr<VisitorTemplate> shared_from_this()
    {
      return std::static_pointer_cast<VisitorTemplate>(
          BaseTermsEnumTraverser::shared_from_this());
    }
  }; // class VisitorTemplate

  /**
   * A visitor node/cell found via the query shape for {@link VisitorTemplate}.
   * Sometimes these are reset(cell). It's like a LinkedList node but forms a
   * tree.
   *
   * @lucene.internal
   */
protected:
  class VNode : public std::enable_shared_from_this<VNode>
  {
    GET_CLASS_NAME(VNode)
    // Note: The VNode tree adds more code to debug/maintain v.s. a flattened
    // LinkedList that we used to have. There is more opportunity here for
    // custom behavior (see preSiblings & postSiblings) but that's not
    // leveraged yet. Maybe this is slightly more GC friendly.

  public:
    const std::shared_ptr<VNode> parent; // only null at the root
    std::shared_ptr<Iterator<std::shared_ptr<VNode>>>
        children;               // null, then sometimes set, then null
    std::shared_ptr<Cell> cell; // not null (except initially before reset())

    /**
     * call reset(cell) after to set the cell.
     */
    VNode(std::shared_ptr<VNode> parent);

    virtual void reset(std::shared_ptr<Cell> cell);
  };

protected:
  std::shared_ptr<AbstractVisitingPrefixTreeQuery> shared_from_this()
  {
    return std::static_pointer_cast<AbstractVisitingPrefixTreeQuery>(
        AbstractPrefixTreeQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
