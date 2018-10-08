#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search::highlight
{
class WeightedTerm;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class BooleanQuery;
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
namespace org::apache::lucene::search::highlight
{

using IndexReader = org::apache::lucene::index::IndexReader;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

/**
 * Utility class used to extract the terms used in a query, plus any weights.
 * This class will not find terms for MultiTermQuery, TermRangeQuery and
 * PrefixQuery classes so the caller must pass a rewritten query (see
 * Query.rewrite) to obtain a deque of expanded terms.
 *
 */
class QueryTermExtractor final
    : public std::enable_shared_from_this<QueryTermExtractor>
{
  GET_CLASS_NAME(QueryTermExtractor)

  /** for term extraction */
private:
  static const std::shared_ptr<IndexSearcher> EMPTY_INDEXSEARCHER;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static QueryTermExtractor::StaticConstructor staticConstructor;

  /**
   * Extracts all terms texts of a given Query into an array of WeightedTerms
   *
   * @param query      Query to extract term texts from
   * @return an array of the terms used in a query, plus their weights.
   */
public:
  static std::deque<std::shared_ptr<WeightedTerm>>
  getTerms(std::shared_ptr<Query> query);

  /**
   * Extracts all terms texts of a given Query into an array of WeightedTerms
   *
   * @param query      Query to extract term texts from
   * @param reader used to compute IDF which can be used to a) score selected
   * fragments better b) use graded highlights eg changing intensity of font
   * color
   * @param fieldName the field on which Inverse Document Frequency (IDF)
   * calculations are based
   * @return an array of the terms used in a query, plus their weights.
   */
  static std::deque<std::shared_ptr<WeightedTerm>>
  getIdfWeightedTerms(std::shared_ptr<Query> query,
                      std::shared_ptr<IndexReader> reader,
                      const std::wstring &fieldName);

  /**
   * Extracts all terms texts of a given Query into an array of WeightedTerms
   *
   * @param query      Query to extract term texts from
   * @param prohibited <code>true</code> to extract "prohibited" terms, too
   * @param fieldName  The fieldName used to filter query terms
   * @return an array of the terms used in a query, plus their weights.
   */
  static std::deque<std::shared_ptr<WeightedTerm>>
  getTerms(std::shared_ptr<Query> query, bool prohibited,
           const std::wstring &fieldName);

  /**
   * Extracts all terms texts of a given Query into an array of WeightedTerms
   *
   * @param query      Query to extract term texts from
   * @param prohibited <code>true</code> to extract "prohibited" terms, too
   * @return an array of the terms used in a query, plus their weights.
   */
  static std::deque<std::shared_ptr<WeightedTerm>>
  getTerms(std::shared_ptr<Query> query, bool prohibited);

private:
  static void getTerms(std::shared_ptr<Query> query, float boost,
                       std::unordered_set<std::shared_ptr<WeightedTerm>> &terms,
                       bool prohibited, const std::wstring &fieldName);

  /**
   * extractTerms is currently the only query-independent means of introspecting
   * queries but it only reveals a deque of terms for that query - not the boosts
   * each individual term in that query may or may not have. "Container" queries
   * such as BooleanQuery should be unwrapped to get at the boost info held in
   * each child element. Some discussion around this topic here:
   * http://www.gossamer-threads.com/lists/lucene/java-dev/34208?search_string=introspection;#34208
   * Unfortunately there seemed to be limited interest in requiring all Query
   * objects to implement something common which would allow access to child
   * queries so what follows here are query-specific implementations for
   * accessing embedded query elements.
   */
  static void getTermsFromBooleanQuery(
      std::shared_ptr<BooleanQuery> query, float boost,
      std::unordered_set<std::shared_ptr<WeightedTerm>> &terms, bool prohibited,
      const std::wstring &fieldName);
};

} // namespace org::apache::lucene::search::highlight
