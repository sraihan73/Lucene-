#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/LongValuesSource.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/LongValues.h"

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
namespace org::apache::lucene::facet::range
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/** Represents a range over long values.
 *
 * @lucene.experimental */
class LongRange final : public Range
{
  GET_CLASS_NAME(LongRange)
  /** Minimum (inclusive). */
public:
  const int64_t min;

  /** Maximum (inclusive). */
  const int64_t max;

  // TODO: can we require fewer args? (same for
  // Double/FloatRange too)

  /** Create a LongRange. */
  LongRange(const std::wstring &label, int64_t minIn, bool minInclusive,
            int64_t maxIn, bool maxInclusive);

  /** True if this range accepts the provided value. */
  bool accept(int64_t value);

  virtual std::wstring toString();

private:
  class ValueSourceQuery : public Query
  {
    GET_CLASS_NAME(ValueSourceQuery)
  private:
    const std::shared_ptr<LongRange> range;
    const std::shared_ptr<Query> fastMatchQuery;
    const std::shared_ptr<LongValuesSource> valueSource;

  public:
    ValueSourceQuery(std::shared_ptr<LongRange> range,
                     std::shared_ptr<Query> fastMatchQuery,
                     std::shared_ptr<LongValuesSource> valueSource);

    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<ValueSourceQuery> other);

  public:
    virtual int hashCode();

    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<ValueSourceQuery> outerInstance;

      std::shared_ptr<Weight> fastMatchWeight;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<ValueSourceQuery> outerInstance, float boost,
          std::shared_ptr<Weight> fastMatchWeight);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    private:
      class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
      {
        GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
      private:
        std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<DocIdSetIterator> approximation;
        std::shared_ptr<LongValues> values;

      public:
        TwoPhaseIteratorAnonymousInnerClass(
            std::shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<DocIdSetIterator> approximation,
            std::shared_ptr<LongValues> values);

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

  protected:
    std::shared_ptr<ValueSourceQuery> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

  /**
   * Create a Query that matches documents in this range
   *
   * The query will check all documents that match the provided match query,
   * or every document in the index if the match query is null.
   *
   * If the value source is static, eg an indexed numeric field, it may be
   * faster to use {@link org.apache.lucene.search.PointRangeQuery}
   *
   * @param fastMatchQuery a query to use as a filter
   * @param valueSource    the source of values for the range check
   */
public:
  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Query> fastMatchQuery,
           std::shared_ptr<LongValuesSource> valueSource);

protected:
  std::shared_ptr<LongRange> shared_from_this()
  {
    return std::static_pointer_cast<LongRange>(Range::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/range/
