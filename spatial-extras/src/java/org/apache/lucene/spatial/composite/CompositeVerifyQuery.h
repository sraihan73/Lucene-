#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::spatial::util
{
class ShapeValuesPredicate;
}
namespace org::apache::lucene::index
{
class IndexReader;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;

/**
 * A Query that considers an "indexQuery" to have approximate results, and a
 * follow-on ShapeValuesSource is called to verify each hit from {@link
 * TwoPhaseIterator#matches()}.
 *
 * @lucene.experimental
 */
class CompositeVerifyQuery : public Query
{
  GET_CLASS_NAME(CompositeVerifyQuery)

private:
  const std::shared_ptr<Query>
      indexQuery; // approximation (matches more than needed)
  const std::shared_ptr<ShapeValuesPredicate> predicateValueSource;

public:
  CompositeVerifyQuery(
      std::shared_ptr<Query> indexQuery,
      std::shared_ptr<ShapeValuesPredicate> predicateValueSource);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<CompositeVerifyQuery> other);

public:
  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<CompositeVerifyQuery> outerInstance;

    std::shared_ptr<Weight> indexQueryWeight;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<CompositeVerifyQuery> outerInstance, float boost,
        std::shared_ptr<Weight> indexQueryWeight);

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

protected:
  std::shared_ptr<CompositeVerifyQuery> shared_from_this()
  {
    return std::static_pointer_cast<CompositeVerifyQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::composite
