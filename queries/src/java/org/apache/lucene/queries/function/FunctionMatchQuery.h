#pragma once
#include "stringhelper.h"
#include <any>
#include <functional>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DoubleValuesSource;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class DoubleValues;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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

namespace org::apache::lucene::queries::function
{

using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/**
 * A query that retrieves all documents with a {@link DoubleValues} value
 * matching a predicate
 *
 * This query works by a linear scan of the index, and is best used in
 * conjunction with other queries that can restrict the number of
 * documents visited
 */
class FunctionMatchQuery final : public Query
{
  GET_CLASS_NAME(FunctionMatchQuery)

private:
  const std::shared_ptr<DoubleValuesSource> source;
  const std::function<bool(double)> filter;

  /**
   * Create a FunctionMatchQuery
   * @param source  a {@link DoubleValuesSource} to use for values
   * @param filter  the predicate to match against
   */
public:
  FunctionMatchQuery(std::shared_ptr<DoubleValuesSource> source,
                     std::function<bool(double)> &filter);

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<FunctionMatchQuery> outerInstance;

    std::shared_ptr<DoubleValuesSource> vs;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<FunctionMatchQuery> outerInstance, float boost,
        std::shared_ptr<DoubleValuesSource> vs);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<DoubleValues> values;
      std::shared_ptr<DocIdSetIterator> approximation;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<DocIdSetIterator> approximation,
          std::shared_ptr<DoubleValues> values);

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

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<FunctionMatchQuery> shared_from_this()
  {
    return std::static_pointer_cast<FunctionMatchQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
