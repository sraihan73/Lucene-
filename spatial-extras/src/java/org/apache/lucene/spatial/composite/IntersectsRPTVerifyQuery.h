#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/composite/IntersectsDifferentiatingQuery.h"

#include  "core/src/java/org/apache/lucene/spatial/util/ShapeValuesPredicate.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include  "core/src/java/org/apache/lucene/spatial/composite/IntersectsDifferentiatingVisitor.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
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
namespace org::apache::lucene::spatial::composite
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using AbstractVisitingPrefixTreeQuery =
    org::apache::lucene::spatial::prefix::AbstractVisitingPrefixTreeQuery;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using org::locationtech::spatial4j::shape::Shape;

/**
 * A spatial Intersects predicate that distinguishes an approximated match from
 * an exact match based on which cells are within the query shape. It exposes a
 * {@link TwoPhaseIterator} that will verify a match with a provided predicate
 * in the form of an ShapeValuesPredicate.
 *
 * @lucene.internal
 */
class IntersectsRPTVerifyQuery : public Query
{
  GET_CLASS_NAME(IntersectsRPTVerifyQuery)

private:
  const std::shared_ptr<IntersectsDifferentiatingQuery> intersectsDiffQuery;
  const std::shared_ptr<ShapeValuesPredicate> predicateValueSource;

public:
  IntersectsRPTVerifyQuery(
      std::shared_ptr<Shape> queryShape, const std::wstring &fieldName,
      std::shared_ptr<SpatialPrefixTree> grid, int detailLevel,
      int prefixGridScanLevel,
      std::shared_ptr<ShapeValuesPredicate> predicateValueSource);

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<IntersectsRPTVerifyQuery> other);

public:
  virtual int hashCode();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<IntersectsRPTVerifyQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<IntersectsRPTVerifyQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<LeafReaderContext> context;
      std::shared_ptr<DocIdSetIterator> approxDISI;
      std::shared_ptr<DocIdSetIterator> exactIterator;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReaderContext> context,
          std::shared_ptr<DocIdSetIterator> approxDISI,
          std::shared_ptr<DocIdSetIterator> exactIterator);

      const std::shared_ptr<TwoPhaseIterator> predFuncValues;

      bool matches()  override;

      float matchCost() override;

    protected:
      std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
            org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

  // This may be a "Query" but we don't use it as-such; the caller calls the
  // constructor and then compute() and examines
  // the results which consists of two parts -- the approximated results, and a
  // subset of exact matches. The difference needs to be verified.
  // TODO refactor AVPTQ to not be a Query?
private:
  class IntersectsDifferentiatingQuery : public AbstractVisitingPrefixTreeQuery
  {
    GET_CLASS_NAME(IntersectsDifferentiatingQuery)

  public:
    IntersectsDifferentiatingQuery(std::shared_ptr<Shape> queryShape,
                                   const std::wstring &fieldName,
                                   std::shared_ptr<SpatialPrefixTree> grid,
                                   int detailLevel, int prefixGridScanLevel);

    virtual std::shared_ptr<
        IntersectsDifferentiatingQuery::IntersectsDifferentiatingVisitor>
    compute(std::shared_ptr<LeafReaderContext> context) ;

    // TODO consider if IntersectsPrefixTreeQuery should simply do this and
    // provide both sets

  public:
    class IntersectsDifferentiatingVisitor : public VisitorTemplate
    {
      GET_CLASS_NAME(IntersectsDifferentiatingVisitor)
    private:
      std::shared_ptr<IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery>
          outerInstance;

    public:
      std::shared_ptr<DocIdSetBuilder> approxBuilder;
      std::shared_ptr<DocIdSetBuilder> exactBuilder;
      bool approxIsEmpty = true;
      bool exactIsEmpty = true;
      std::shared_ptr<DocIdSet> exactDocIdSet;
      std::shared_ptr<DocIdSet> approxDocIdSet;

      IntersectsDifferentiatingVisitor(
          std::shared_ptr<
              IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery>
              outerInstance,
          std::shared_ptr<LeafReaderContext> context) ;

    protected:
      void start()  override;

      std::shared_ptr<DocIdSet> finish()  override;

      bool visitPrefix(std::shared_ptr<Cell> cell)  override;

      void visitLeaf(std::shared_ptr<Cell> cell)  override;

    protected:
      std::shared_ptr<IntersectsDifferentiatingVisitor> shared_from_this()
      {
        return std::static_pointer_cast<IntersectsDifferentiatingVisitor>(
            VisitorTemplate::shared_from_this());
      }
    };

  public:
    std::shared_ptr<DocIdSet> getDocIdSet(
        std::shared_ptr<LeafReaderContext> context)  override;

    std::wstring toString(const std::wstring &field) override;

  protected:
    std::shared_ptr<IntersectsDifferentiatingQuery> shared_from_this()
    {
      return std::static_pointer_cast<IntersectsDifferentiatingQuery>(
          org.apache.lucene.spatial.prefix
              .AbstractVisitingPrefixTreeQuery::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IntersectsRPTVerifyQuery> shared_from_this()
  {
    return std::static_pointer_cast<IntersectsRPTVerifyQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/composite/
