#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/util/BulkAdder.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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
namespace org::apache::lucene::search
{

/**
 * Abstract class for range queries against single or multidimensional points
such as
 * {@link IntPoint}.
GET_CLASS_NAME(for)
 * <p>
 * This is for subclasses and works on the underlying binary encoding: to
 * create range queries for lucene's standard {@code Point} types, refer to
factory GET_CLASS_NAME(es)
 * methods on those classes, e.g. {@link IntPoint#newRangeQuery
IntPoint.newRangeQuery()} for
 * fields indexed with {@link IntPoint}.
GET_CLASS_NAME(es,)
 * <p>
 * For a single-dimensional field this query is a simple range query; in a
multi-dimensional field it's a box shape.
 * @see PointValues
 * @lucene.experimental
 */
class PointRangeQuery : public Query
{
  GET_CLASS_NAME(PointRangeQuery)
public:
  const std::wstring field;
  const int numDims;
  const int bytesPerDim;
  std::deque<char> const lowerPoint;
  std::deque<char> const upperPoint;

  /**
   * Expert: create a multidimensional range query for point values.
   *
   * @param field field name. must not be {@code null}.
   * @param lowerPoint lower portion of the range (inclusive).
   * @param upperPoint upper portion of the range (inclusive).
   * @param numDims number of dimensions.
   * @throws IllegalArgumentException if {@code field} is null, or if {@code
   * lowerValue.length != upperValue.length}
   */
protected:
  PointRangeQuery(const std::wstring &field, std::deque<char> &lowerPoint,
                  std::deque<char> &upperPoint, int numDims);

  /**
   * Check preconditions for all factory methods
   * @throws IllegalArgumentException if {@code field}, {@code lowerPoint} or
   * {@code upperPoint} are null.
   */
public:
  static void checkArgs(const std::wstring &field, std::any lowerPoint,
                        std::any upperPoint);

  std::shared_ptr<Weight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override final;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<PointRangeQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<PointRangeQuery> outerInstance, float boost);

  private:
    std::shared_ptr<PointValues::IntersectVisitor>
    getIntersectVisitor(std::shared_ptr<DocIdSetBuilder> result);

  private:
    class IntersectVisitorAnonymousInnerClass
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass>,
          public PointValues::IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<DocIdSetBuilder> result;

    public:
      IntersectVisitorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<DocIdSetBuilder> result);

      std::shared_ptr<DocIdSetBuilder::BulkAdder> adder;

      void grow(int count);

      void visit(int docID) override;

      void visit(int docID, std::deque<char> &packedValue) override;

      PointValues::Relation compare(std::deque<char> &minPackedValue,
                                    std::deque<char> &maxPackedValue);
    };

    /**
     * Create a visitor that clears documents that do NOT match the range.
     */
  private:
    std::shared_ptr<PointValues::IntersectVisitor>
    getInverseIntersectVisitor(std::shared_ptr<FixedBitSet> result,
                               std::deque<int> &cost);

  private:
    class IntersectVisitorAnonymousInnerClass2
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass2>,
          public PointValues::IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass2)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<FixedBitSet> result;
      std::deque<int> cost;

    public:
      IntersectVisitorAnonymousInnerClass2(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<FixedBitSet> result, std::deque<int> &cost);

      void visit(int docID) override;

      void visit(int docID, std::deque<char> &packedValue) override;

      PointValues::Relation compare(std::deque<char> &minPackedValue,
                                    std::deque<char> &maxPackedValue);
    };

  public:
    std::shared_ptr<ScorerSupplier> scorerSupplier(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<LeafReader> reader;
      std::shared_ptr<org::apache::lucene::search::Weight> weight;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReader> reader,
          std::shared_ptr<org::apache::lucene::search::Weight> weight);

      std::shared_ptr<Scorer> get(int64_t leadCost) override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            ScorerSupplier::shared_from_this());
      }
    };

  private:
    class ScorerSupplierAnonymousInnerClass2 : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass2)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<LeafReader> reader;
      std::shared_ptr<PointValues> values;
      std::shared_ptr<org::apache::lucene::search::Weight> weight;

    public:
      ScorerSupplierAnonymousInnerClass2(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReader> reader,
          std::shared_ptr<PointValues> values,
          std::shared_ptr<org::apache::lucene::search::Weight> weight);

      const std::shared_ptr<DocIdSetBuilder> result;
      const std::shared_ptr<PointValues::IntersectVisitor> visitor;
      int64_t cost = 0;

      std::shared_ptr<Scorer>
      get(int64_t leadCost)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass2>(
            ScorerSupplier::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

public:
  virtual std::wstring getField();

  virtual int getNumDims();

  virtual int getBytesPerDim();

  virtual std::deque<char> getLowerPoint();

  virtual std::deque<char> getUpperPoint();

  int hashCode() override final;

  bool equals(std::any o) override final;

private:
  bool equalsTo(std::shared_ptr<PointRangeQuery> other);

public:
  std::wstring toString(const std::wstring &field) override final;

  /**
   * Returns a string of a single value in a human-readable format for
   * debugging. This is used by {@link #toString()}.
   *
   * @param dimension dimension of the particular value
   * @param value single value, never null
   * @return human readable value for debugging
   */
protected:
  virtual std::wstring toString(int dimension, std::deque<char> &value) = 0;

protected:
  std::shared_ptr<PointRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<PointRangeQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
