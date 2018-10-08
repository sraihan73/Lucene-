#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueSourceScorer.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/**
 * A Query wrapping a {@link ValueSource} that matches docs in which the values
 * in the value source match a configured range.  The score is the float value.
 * This can be a slow query if run by itself since it must visit all docs;
 * ideally it's combined with other queries.
 * It's mostly a wrapper around
 * {@link FunctionValues#getRangeScorer(LeafReaderContext, std::wstring, std::wstring,
 * bool, bool)}.
 *
 * A similar class is {@code org.apache.lucene.search.DocValuesRangeQuery} in
 * the sandbox module.  That one is constant scoring.
 *
 * @see FunctionQuery (constant scoring)
 * @lucene.experimental
 */
class FunctionRangeQuery : public Query
{
  GET_CLASS_NAME(FunctionRangeQuery)

private:
  const std::shared_ptr<ValueSource> valueSource;

  // These two are declared as strings because FunctionValues.getRangeScorer
  // takes std::wstring args and parses them.
  const std::wstring lowerVal;
  const std::wstring upperVal;
  const bool includeLower;
  const bool includeUpper;

public:
  FunctionRangeQuery(std::shared_ptr<ValueSource> valueSource,
                     std::shared_ptr<Number> lowerVal,
                     std::shared_ptr<Number> upperVal, bool includeLower,
                     bool includeUpper);

  FunctionRangeQuery(std::shared_ptr<ValueSource> valueSource,
                     const std::wstring &lowerVal, const std::wstring &upperVal,
                     bool includeLower, bool includeUpper);

  virtual std::shared_ptr<ValueSource> getValueSource();

  virtual std::wstring getLowerVal();

  virtual std::wstring getUpperVal();

  virtual bool isIncludeLower();

  virtual bool isIncludeUpper();

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<FunctionRangeQuery> other);

public:
  virtual int hashCode();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class FunctionRangeWeight : public Weight
  {
    GET_CLASS_NAME(FunctionRangeWeight)
  private:
    std::shared_ptr<FunctionRangeQuery> outerInstance;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("rawtypes") private final java.util.Map
    // vsContext;
    const std::unordered_map vsContext;

  public:
    FunctionRangeWeight(
        std::shared_ptr<FunctionRangeQuery> outerInstance,
        std::shared_ptr<IndexSearcher> searcher) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<ValueSourceScorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<FunctionRangeWeight> shared_from_this()
    {
      return std::static_pointer_cast<FunctionRangeWeight>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<FunctionRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<FunctionRangeQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/
