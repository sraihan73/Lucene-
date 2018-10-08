#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search::spans
{
class SpanWeight;
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
namespace org::apache::lucene::search::spans
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

/**
 * Counterpart of {@link BoostQuery} for spans.
 */
class SpanBoostQuery final : public SpanQuery
{
  GET_CLASS_NAME(SpanBoostQuery)

private:
  const std::shared_ptr<SpanQuery> query;
  const float boost;

  /** Sole constructor: wrap {@code query} in such a way that the produced
   *  scores will be boosted by {@code boost}. */
public:
  SpanBoostQuery(std::shared_ptr<SpanQuery> query, float boost);

  /**
   * Return the wrapped {@link SpanQuery}.
   */
  std::shared_ptr<SpanQuery> getQuery();

  /**
   * Return the applied boost.
   */
  float getBoost();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<SpanBoostQuery> other);

public:
  virtual int hashCode();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  std::wstring getField() override;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

protected:
  std::shared_ptr<SpanBoostQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanBoostQuery>(
        SpanQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
