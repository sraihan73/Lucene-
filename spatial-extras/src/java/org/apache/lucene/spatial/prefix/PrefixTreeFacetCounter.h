#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"

#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/PrefixTreeStrategy.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"

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
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using Bits = org::apache::lucene::util::Bits;

/**
 * Computes facets on cells for {@link
 * org.apache.lucene.spatial.prefix.PrefixTreeStrategy}. <p> <em>NOTE:</em> If
 * for a given document and a given field using
 * {@link org.apache.lucene.spatial.prefix.RecursivePrefixTreeStrategy}
 * multiple values are indexed (i.e. multi-valued) and at least one of them is a
 * non-point, then there is a possibility of double-counting the document in the
 * facet results.  Since each shape is independently turned into grid cells at
 * a resolution chosen by the shape's size, it's possible they will be indexed
 * at different resolutions.  This means the document could be present in BOTH
 * the postings for a cell in both its prefix and leaf variants.  To avoid this,
 * use a single valued field with a {@link
 * org.locationtech.spatial4j.shape.ShapeCollection} (or WKT equivalent).  Or
 * calculate a suitable level/distErr to index both and call
 * {@link
 * org.apache.lucene.spatial.prefix.PrefixTreeStrategy#createIndexableFields(org.locationtech.spatial4j.shape.Shape,
 * int)} with the same value for all shapes for a given document/field.
 *
 * @lucene.experimental
 */
class PrefixTreeFacetCounter
    : public std::enable_shared_from_this<PrefixTreeFacetCounter>
{
  GET_CLASS_NAME(PrefixTreeFacetCounter)

  /** A callback/visitor of facet counts. */
public:
  class FacetVisitor : public std::enable_shared_from_this<FacetVisitor>
  {
    GET_CLASS_NAME(FacetVisitor)
    /** Called at the start of the segment, if there is indexed data. */
  public:
    virtual void startOfSegment();

    /** Called for cells with a leaf, or cells at the target facet level. {@code
     * count} is greater than zero. When an ancestor cell is given with non-zero
     * count, the count can be considered to be added to all cells below. You
     * won't necessarily get a cell at level {@code facetLevel} if the indexed
     * data is courser (bigger).
     */
    virtual void visit(std::shared_ptr<Cell> cell, int count) = 0;
  };

private:
  PrefixTreeFacetCounter();

  /**
   * Computes facets using a callback/visitor style design, allowing flexibility
   * for the caller to determine what to do with each underlying count.
   * @param strategy the prefix tree strategy (contains the field reference,
   * grid, max levels)
   * @param context the IndexReader's context
   * @param topAcceptDocs a Bits to limit counted docs. If null, live docs are
   * counted.
   * @param queryShape the shape to limit the range of facet counts to
   * @param facetLevel the maximum depth (detail) of faceted cells
   * @param facetVisitor the visitor/callback to receive the counts
   */
public:
  static void
  compute(std::shared_ptr<PrefixTreeStrategy> strategy,
          std::shared_ptr<IndexReaderContext> context,
          std::shared_ptr<Bits> topAcceptDocs,
          std::shared_ptr<Shape> queryShape, int facetLevel,
          std::shared_ptr<FacetVisitor> facetVisitor) ;

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<Bits> topAcceptDocs;
    std::shared_ptr<LeafReaderContext> leafCtx;

  public:
    BitsAnonymousInnerClass(std::shared_ptr<Bits> topAcceptDocs,
                            std::shared_ptr<LeafReaderContext> leafCtx);

    bool get(int index) override;

    int length() override;
  };

  /** Lower-level per-leaf segment method. */
public:
  static void
  compute(std::shared_ptr<PrefixTreeStrategy> strategy,
          std::shared_ptr<LeafReaderContext> context,
          std::shared_ptr<Bits> acceptDocs, std::shared_ptr<Shape> queryShape,
          int const facetLevel,
          std::shared_ptr<FacetVisitor> facetVisitor) ;

private:
  class AbstractVisitingPrefixTreeQueryAnonymousInnerClass
      : public AbstractVisitingPrefixTreeQuery
  {
    GET_CLASS_NAME(AbstractVisitingPrefixTreeQueryAnonymousInnerClass)
  private:
    std::shared_ptr<LeafReaderContext> context;
    std::shared_ptr<Bits> acceptDocs;
    int facetLevel = 0;
    std::shared_ptr<org::apache::lucene::spatial::prefix::
                        PrefixTreeFacetCounter::FacetVisitor>
        facetVisitor;

  public:
    AbstractVisitingPrefixTreeQueryAnonymousInnerClass(
        std::shared_ptr<Shape> queryShape, const std::wstring &getFieldName,
        int facetLevel, std::shared_ptr<LeafReaderContext> context,
        std::shared_ptr<Bits> acceptDocs,
        std::shared_ptr<org::apache::lucene::spatial::prefix::
                            PrefixTreeFacetCounter::FacetVisitor>
            facetVisitor);

    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<DocIdSet>
    getDocIdSet(std::shared_ptr<LeafReaderContext> contexts) throw(
        IOException) override;

  private:
    class VisitorTemplateAnonymousInnerClass : public VisitorTemplate
    {
      GET_CLASS_NAME(VisitorTemplateAnonymousInnerClass)
    private:
      std::shared_ptr<AbstractVisitingPrefixTreeQueryAnonymousInnerClass>
          outerInstance;

    public:
      VisitorTemplateAnonymousInnerClass(
          std::shared_ptr<AbstractVisitingPrefixTreeQueryAnonymousInnerClass>
              outerInstance,
          std::shared_ptr<LeafReaderContext> context);

    protected:
      void start()  override;

      std::shared_ptr<DocIdSet> finish()  override;

      bool visitPrefix(std::shared_ptr<Cell> cell)  override;

      void visitLeaf(std::shared_ptr<Cell> cell)  override;

    private:
      int countDocsAtThisTerm() ;

      bool hasDocsAtThisTerm() ;

    protected:
      std::shared_ptr<VisitorTemplateAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<VisitorTemplateAnonymousInnerClass>(
            VisitorTemplate::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AbstractVisitingPrefixTreeQueryAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AbstractVisitingPrefixTreeQueryAnonymousInnerClass>(
          AbstractVisitingPrefixTreeQuery::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
