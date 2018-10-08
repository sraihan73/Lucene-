#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class DocIdSet;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
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
namespace org::apache::lucene::spatial::prefix
{

using org::locationtech::spatial4j::shape::Shape;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;

/**
 * Finds docs where its indexed shape is {@link
 * org.apache.lucene.spatial.query.SpatialOperation#IsWithin WITHIN} the query
 * shape.  It works by looking at cells outside of the query shape to ensure
 * documents there are excluded. By default, it will examine all cells, and it's
 * fairly slow.  If you know that the indexed shapes are never comprised of
 * multiple disjoint parts (which also means it is not multi-valued), then you
 * can pass {@code SpatialPrefixTree.getDistanceForLevel(maxLevels)} as the
 * {@code queryBuffer} constructor parameter to minimally look this distance
 * beyond the query shape's edge.  Even if the indexed shapes are sometimes
 * comprised of multiple disjoint parts, you might want to use this option with
 * a large buffer as a faster approximation with minimal false-positives.
 *
 * @lucene.experimental
 */
class WithinPrefixTreeQuery : public AbstractVisitingPrefixTreeQuery
{
  GET_CLASS_NAME(WithinPrefixTreeQuery)
  // TODO LUCENE-4869: implement faster algorithm based on filtering out
  // false-positives of a
  //  minimal query buffer by looking in a DocValues cache holding a
  //  representative point of each disjoint component of a document's shape(s).

  // TODO Could the recursion in allCellsIntersectQuery() be eliminated when
  // non-fuzzy or other
  //  circumstances?

private:
  const std::shared_ptr<Shape>
      bufferedQueryShape; // if null then the whole world

  /**
   * See {@link
   * AbstractVisitingPrefixTreeQuery#AbstractVisitingPrefixTreeQuery(org.locationtech.spatial4j.shape.Shape,
   * std::wstring, org.apache.lucene.spatial.prefix.tree.SpatialPrefixTree, int,
   * int)}.
   * {@code queryBuffer} is the (minimum) distance beyond the query shape edge
   * where non-matching documents are looked for so they can be excluded. If
   * -1 is used then the whole world is examined (a good default for
   * correctness).
   */
public:
  WithinPrefixTreeQuery(std::shared_ptr<Shape> queryShape,
                        const std::wstring &fieldName,
                        std::shared_ptr<SpatialPrefixTree> grid,
                        int detailLevel, int prefixGridScanLevel,
                        double queryBuffer);

  virtual bool equals(std::any o);

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  /** Returns a new shape that is larger than shape by at distErr.
   */
  // TODO move this generic code elsewhere?  Spatial4j?
protected:
  virtual std::shared_ptr<Shape> bufferShape(std::shared_ptr<Shape> shape,
                                             double distErr);

  std::shared_ptr<DocIdSet> getDocIdSet(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class VisitorTemplateAnonymousInnerClass : public VisitorTemplate
  {
    GET_CLASS_NAME(VisitorTemplateAnonymousInnerClass)
  private:
    std::shared_ptr<WithinPrefixTreeQuery> outerInstance;

  public:
    VisitorTemplateAnonymousInnerClass(
        std::shared_ptr<WithinPrefixTreeQuery> outerInstance,
        std::shared_ptr<LeafReaderContext> context);

  private:
    std::shared_ptr<FixedBitSet> inside;
    std::shared_ptr<FixedBitSet> outside;

  protected:
    void start() override;

    std::shared_ptr<DocIdSet> finish() override;

    std::shared_ptr<CellIterator>
    findSubCellsToVisit(std::shared_ptr<Cell> cell) override;

    bool visitPrefix(std::shared_ptr<Cell> cell)  override;

    void visitLeaf(std::shared_ptr<Cell> cell)  override;

    /** Returns true if the provided cell, and all its sub-cells down to
     * detailLevel all intersect the queryShape.
     */
  private:
    bool allCellsIntersectQuery(std::shared_ptr<Cell> cell);

  protected:
    void visitScanned(std::shared_ptr<Cell> cell)  override;

  protected:
    std::shared_ptr<VisitorTemplateAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<VisitorTemplateAnonymousInnerClass>(
          VisitorTemplate::shared_from_this());
    }
  };

protected:
  std::shared_ptr<WithinPrefixTreeQuery> shared_from_this()
  {
    return std::static_pointer_cast<WithinPrefixTreeQuery>(
        AbstractVisitingPrefixTreeQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
