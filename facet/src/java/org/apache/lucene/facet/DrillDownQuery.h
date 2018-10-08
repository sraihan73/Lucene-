#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::facet
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;

/**
 * A {@link Query} for drill-down over facet categories. You
 * should call {@link #add(std::wstring, std::wstring...)} for every group of categories you
 * want to drill-down over.
 * <p>
 * <b>NOTE:</b> if you choose to create your own {@link Query} by calling
 * {@link #term}, it is recommended to wrap it in a {@link BoostQuery}
 * with a boost of {@code 0.0f},
 * so that it does not affect the scores of the documents.
 *
 * @lucene.experimental
 */
class DrillDownQuery final : public Query
{
  GET_CLASS_NAME(DrillDownQuery)

  /** Creates a drill-down term. */
public:
  static std::shared_ptr<Term> term(const std::wstring &field,
                                    const std::wstring &dim,
                                    std::deque<std::wstring> &path);

private:
  const std::shared_ptr<FacetsConfig> config;
  const std::shared_ptr<Query> baseQuery;
  const std::deque<std::shared_ptr<BooleanQuery::Builder>> dimQueries =
      std::deque<std::shared_ptr<BooleanQuery::Builder>>();
  const std::unordered_map<std::wstring, int> drillDownDims =
      std::make_shared<LinkedHashMap<std::wstring, int>>();

  /** Used by clone() and DrillSideways */
public:
  DrillDownQuery(
      std::shared_ptr<FacetsConfig> config, std::shared_ptr<Query> baseQuery,
      std::deque<std::shared_ptr<BooleanQuery::Builder>> &dimQueries,
      std::unordered_map<std::wstring, int> &drillDownDims);

  /** Used by DrillSideways */
  DrillDownQuery(std::shared_ptr<FacetsConfig> config,
                 std::shared_ptr<Query> filter,
                 std::shared_ptr<DrillDownQuery> other);

  /** Creates a new {@code DrillDownQuery} without a base query,
   *  to perform a pure browsing query (equivalent to using
   *  {@link MatchAllDocsQuery} as base). */
  DrillDownQuery(std::shared_ptr<FacetsConfig> config);

  /** Creates a new {@code DrillDownQuery} over the given base query. Can be
   *  {@code null}, in which case the result {@link Query} from
   *  {@link #rewrite(IndexReader)} will be a pure browsing query, filtering on
   *  the added categories only. */
  DrillDownQuery(std::shared_ptr<FacetsConfig> config,
                 std::shared_ptr<Query> baseQuery);

  /** Adds one dimension of drill downs; if you pass the same
   *  dimension more than once it is OR'd with the previous
   *  cofnstraints on that dimension, and all dimensions are
   *  AND'd against each other and the base query. */
  void add(const std::wstring &dim, std::deque<std::wstring> &path);

  /** Expert: add a custom drill-down subQuery.  Use this
   *  when you have a separate way to drill-down on the
   *  dimension than the indexed facet ordinals. */
  void add(const std::wstring &dim, std::shared_ptr<Query> subQuery);

  std::shared_ptr<DrillDownQuery> clone() override;

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<DrillDownQuery> other);

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> r)  override;

  std::wstring toString(const std::wstring &field) override;

private:
  std::shared_ptr<BooleanQuery> getBooleanQuery();

public:
  std::shared_ptr<Query> getBaseQuery();

  std::deque<std::shared_ptr<Query>> getDrillDownQueries();

  std::unordered_map<std::wstring, int> getDims();

protected:
  std::shared_ptr<DrillDownQuery> shared_from_this()
  {
    return std::static_pointer_cast<DrillDownQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
