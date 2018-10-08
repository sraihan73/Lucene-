#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
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

using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * A {@link Query} wrapper that allows to give a boost to the wrapped query.
 * Boost values that are less than one will give less importance to this
 * query compared to other ones while values that are greater than one will
 * give more importance to the scores returned by this query.
 *
 * More complex boosts can be applied by using FunctionScoreQuery in the
 * lucene-queries module
 */
class BoostQuery final : public Query
{
  GET_CLASS_NAME(BoostQuery)

private:
  const std::shared_ptr<Query> query;
  const float boost;

  /** Sole constructor: wrap {@code query} in such a way that the produced
   *  scores will be boosted by {@code boost}. */
public:
  BoostQuery(std::shared_ptr<Query> query, float boost);

  /**
   * Return the wrapped {@link Query}.
   */
  std::shared_ptr<Query> getQuery();

  /**
   * Return the applied boost.
   */
  float getBoost();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<BoostQuery> other);

public:
  virtual int hashCode();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

protected:
  std::shared_ptr<BoostQuery> shared_from_this()
  {
    return std::static_pointer_cast<BoostQuery>(Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
