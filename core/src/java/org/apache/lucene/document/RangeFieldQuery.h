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
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/util/BulkAdder.h"
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
namespace org::apache::lucene::document
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/**
 * Query class for searching {@code RangeField} types by a defined {@link
 * Relation}.
 */
class RangeFieldQuery : public Query
{
  GET_CLASS_NAME(RangeFieldQuery)
  /** field name */
public:
  const std::wstring field;
  /** query relation
   * intersects: {@code CELL_CROSSES_QUERY},
   * contains: {@code CELL_CONTAINS_QUERY},
   * within: {@code CELL_WITHIN_QUERY} */
  const QueryType queryType;
  /** number of dimensions - max 4 */
  const int numDims;
  /** ranges encoded as a sortable byte array */
  std::deque<char> const ranges;
  /** number of bytes per dimension */
  const int bytesPerDim;

  /** Used by {@code RangeFieldQuery} to check how each internal or leaf node
   * relates to the query. */
public:
  class QueryType final
  {
    GET_CLASS_NAME(QueryType)
    /** Use this for intersects queries. */
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        INTERSECTS
    //        {
    //            org.apache.lucene.index.PointValues.Relation compare(byte[]
    //            queryPackedValue, byte[] minPackedValue, byte[]
    //            maxPackedValue, int numDims, int bytesPerDim, int dim)
    //            {
    //                int minOffset = dim * bytesPerDim;
    //                int maxOffset = minOffset + bytesPerDim * numDims;
    //                if(org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, minPackedValue, minOffset) <
    //                0 ||
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, maxPackedValue, maxOffset) >
    //                0)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY;
    //                }
    //                if(org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, maxPackedValue, minOffset) >=
    //                0 &&
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, minPackedValue, maxOffset) <=
    //                0)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_INSIDE_QUERY;
    //                }
    //                return
    //                org.apache.lucene.index.PointValues.Relation.CELL_CROSSES_QUERY;
    //            }
    //            bool matches(byte[] queryPackedValue, byte[] packedValue,
    //            int numDims, int bytesPerDim, int dim)
    //            {
    //                int minOffset = dim * bytesPerDim;
    //                int maxOffset = minOffset + bytesPerDim * numDims;
    //                return
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, packedValue, minOffset) >= 0
    //                &&
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, packedValue, maxOffset) <= 0;
    //            }
    //        },
    /** Use this for within queries. */
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        WITHIN
    //        {
    //            org.apache.lucene.index.PointValues.Relation compare(byte[]
    //            queryPackedValue, byte[] minPackedValue, byte[]
    //            maxPackedValue, int numDims, int bytesPerDim, int dim)
    //            {
    //                int minOffset = dim * bytesPerDim;
    //                int maxOffset = minOffset + bytesPerDim * numDims;
    //                if(org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, minPackedValue, maxOffset) <
    //                0 ||
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, maxPackedValue, minOffset) >
    //                0)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY;
    //                }
    //                if(org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, maxPackedValue, maxOffset) >=
    //                0 &&
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, minPackedValue, minOffset) <=
    //                0)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_INSIDE_QUERY;
    //                }
    //                return
    //                org.apache.lucene.index.PointValues.Relation.CELL_CROSSES_QUERY;
    //            }
    //            bool matches(byte[] queryPackedValue, byte[] packedValue,
    //            int numDims, int bytesPerDim, int dim)
    //            {
    //                int minOffset = dim * bytesPerDim;
    //                int maxOffset = minOffset + bytesPerDim * numDims;
    //                return
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, packedValue, minOffset) <= 0
    //                &&
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, packedValue, maxOffset) >= 0;
    //            }
    //        },
    /** Use this for contains */
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        CONTAINS
    //        {
    //            org.apache.lucene.index.PointValues.Relation compare(byte[]
    //            queryPackedValue, byte[] minPackedValue, byte[]
    //            maxPackedValue, int numDims, int bytesPerDim, int dim)
    //            {
    //                int minOffset = dim * bytesPerDim;
    //                int maxOffset = minOffset + bytesPerDim * numDims;
    //                if(org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, maxPackedValue, maxOffset) >
    //                0 ||
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, minPackedValue, minOffset) <
    //                0)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY;
    //                }
    //                if(org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, minPackedValue, maxOffset) <=
    //                0 &&
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, maxPackedValue, minOffset) >=
    //                0)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_INSIDE_QUERY;
    //                }
    //                return
    //                org.apache.lucene.index.PointValues.Relation.CELL_CROSSES_QUERY;
    //            }
    //            bool matches(byte[] queryPackedValue, byte[] packedValue,
    //            int numDims, int bytesPerDim, int dim)
    //            {
    //                int minOffset = dim * bytesPerDim;
    //                int maxOffset = minOffset + bytesPerDim * numDims;
    //                return
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, minOffset, packedValue, minOffset) >= 0
    //                &&
    //                org.apache.lucene.util.StringHelper.compare(bytesPerDim,
    //                queryPackedValue, maxOffset, packedValue, maxOffset) <= 0;
    //            }
    //        },
    /** Use this for crosses queries */
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        CROSSES
    //        {
    //            org.apache.lucene.index.PointValues.Relation compare(byte[]
    //            queryPackedValue, byte[] minPackedValue, byte[]
    //            maxPackedValue, int numDims, int bytesPerDim, int dim)
    //            {
    //                throw new UnsupportedOperationException();
    //            }
    //            bool matches(byte[] queryPackedValue, byte[] packedValue,
    //            int numDims, int bytesPerDim, int dim)
    //            {
    //                throw new UnsupportedOperationException();
    //            }
    //            org.apache.lucene.index.PointValues.Relation compare(byte[]
    //            queryPackedValue, byte[] minPackedValue, byte[]
    //            maxPackedValue, int numDims, int bytesPerDim)
    //            {
    //                org.apache.lucene.index.PointValues.Relation
    //                intersectRelation =
    //                QueryType.INTERSECTS.compare(queryPackedValue,
    //                minPackedValue, maxPackedValue, numDims, bytesPerDim);
    //                if(intersectRelation ==
    //                org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY;
    //                }
    //                org.apache.lucene.index.PointValues.Relation
    //                withinRelation =
    //                QueryType.WITHIN.compare(queryPackedValue, minPackedValue,
    //                maxPackedValue, numDims, bytesPerDim); if(withinRelation
    //                ==
    //                org.apache.lucene.index.PointValues.Relation.CELL_INSIDE_QUERY)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY;
    //                }
    //                if(intersectRelation ==
    //                org.apache.lucene.index.PointValues.Relation.CELL_INSIDE_QUERY
    //                && withinRelation ==
    //                org.apache.lucene.index.PointValues.Relation.CELL_OUTSIDE_QUERY)
    //                {
    //                    return
    //                    org.apache.lucene.index.PointValues.Relation.CELL_INSIDE_QUERY;
    //                }
    //                return
    //                org.apache.lucene.index.PointValues.Relation.CELL_CROSSES_QUERY;
    //            }
    //            bool matches(byte[] queryPackedValue, byte[] packedValue,
    //            int numDims, int bytesPerDim)
    //            {
    //                return INTERSECTS.matches(queryPackedValue, packedValue,
    //                numDims, bytesPerDim) && WITHIN.matches(queryPackedValue,
    //                packedValue, numDims, bytesPerDim) == false;
    //            }
    //        };

  private:
    static std::deque<QueryType> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

    QueryType(const std::wstring &name, InnerEnum innerEnum);

  public:
    virtual org::apache::lucene::index::PointValues::Relation
    compare(std::deque<char> &queryPackedValue,
            std::deque<char> &minPackedValue,
            std::deque<char> &maxPackedValue, int numDims, int bytesPerDim,
            int dim) = 0;

    virtual org::apache::lucene::index::PointValues::Relation
    compare(std::deque<char> &queryPackedValue,
            std::deque<char> &minPackedValue,
            std::deque<char> &maxPackedValue, int numDims, int bytesPerDim);

    virtual bool matches(std::deque<char> &queryPackedValue,
                         std::deque<char> &packedValue, int numDims,
                         int bytesPerDim, int dim) = 0;

    virtual bool matches(std::deque<char> &queryPackedValue,
                         std::deque<char> &packedValue, int numDims,
                         int bytesPerDim);

  public:
    bool operator==(const QueryType &other);

    bool operator!=(const QueryType &other);

    static std::deque<QueryType> values();

    int ordinal();

    std::wstring toString();

    static QueryType valueOf(const std::wstring &name);
  };

  /**
   * Create a query for searching indexed ranges that match the provided
   * relation.
   * @param field field name. must not be null.
   * @param ranges encoded range values; this is done by the {@code RangeField}
   * implementation
   * @param queryType the query relation
   */
public:
  RangeFieldQuery(const std::wstring &field, std::deque<char> &ranges,
                  int const numDims, QueryType const queryType);

  /** check input arguments */
private:
  static void checkArgs(const std::wstring &field, std::deque<char> &ranges,
                        int const numDims);

  /** Check indexed field info against the provided query data. */
  void checkFieldInfo(std::shared_ptr<FieldInfo> fieldInfo);

public:
  std::shared_ptr<Weight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override final;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<RangeFieldQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<RangeFieldQuery> outerInstance, float boost);

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
      void visit(int docID)  override;
      void visit(int docID,
                 std::deque<char> &leaf)  override;
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
      std::shared_ptr<Weight> weight;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReader> reader, std::shared_ptr<Weight> weight);

      std::shared_ptr<Scorer> get(int64_t leadCost) override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            org.apache.lucene.search.ScorerSupplier::shared_from_this());
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
      std::shared_ptr<Weight> weight;

    public:
      ScorerSupplierAnonymousInnerClass2(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReader> reader,
          std::shared_ptr<PointValues> values, std::shared_ptr<Weight> weight);

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
            org.apache.lucene.search.ScorerSupplier::shared_from_this());
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
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

public:
  virtual int hashCode();

  bool equals(std::any o) override final;

protected:
  virtual bool equalsTo(std::shared_ptr<RangeFieldQuery> other);

public:
  std::wstring toString(const std::wstring &field) override;

  /**
   * Returns a string of a single value in a human-readable format for
   * debugging. This is used by {@link #toString()}.
   *
   * @param dimension dimension of the particular value
   * @param ranges encoded ranges, never null
   * @return human readable value for debugging
   */
protected:
  virtual std::wstring toString(std::deque<char> &ranges, int dimension) = 0;

protected:
  std::shared_ptr<RangeFieldQuery> shared_from_this()
  {
    return std::static_pointer_cast<RangeFieldQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
