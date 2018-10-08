#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/BooleanWeight.h"

#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"
#include  "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
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

using Occur = org::apache::lucene::search::BooleanClause::Occur;

class Boolean2ScorerSupplier final : public ScorerSupplier
{
  GET_CLASS_NAME(Boolean2ScorerSupplier)

private:
  const std::shared_ptr<BooleanWeight> weight;
  const std::unordered_map<BooleanClause::Occur,
                           std::deque<std::shared_ptr<ScorerSupplier>>>
      subs;
  const bool needsScores;
  const int minShouldMatch;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t cost_ = -1;

public:
  Boolean2ScorerSupplier(
      std::shared_ptr<BooleanWeight> weight,
      std::unordered_map<Occur, std::deque<std::shared_ptr<ScorerSupplier>>>
          &subs,
      bool needsScores, int minShouldMatch);

private:
  int64_t computeCost();

public:
  int64_t cost() override;

  std::shared_ptr<Scorer> get(int64_t leadCost)  override;

  /** Create a new scorer for the given required clauses. Note that
   *  {@code requiredScoring} is a subset of {@code required} containing
   *  required clauses that should participate in scoring. */
private:
  std::shared_ptr<Scorer>
  req(std::shared_ptr<std::deque<std::shared_ptr<ScorerSupplier>>>
          requiredNoScoring,
      std::shared_ptr<std::deque<std::shared_ptr<ScorerSupplier>>>
          requiredScoring,
      int64_t leadCost) ;

private:
  class FilterScorerAnonymousInnerClass : public FilterScorer
  {
    GET_CLASS_NAME(FilterScorerAnonymousInnerClass)
  private:
    std::shared_ptr<Boolean2ScorerSupplier> outerInstance;

  public:
    FilterScorerAnonymousInnerClass(
        std::shared_ptr<Boolean2ScorerSupplier> outerInstance,
        std::shared_ptr<org::apache::lucene::search::Scorer> req);

    float score()  override;

  protected:
    std::shared_ptr<FilterScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterScorerAnonymousInnerClass>(
          FilterScorer::shared_from_this());
    }
  };

private:
  std::shared_ptr<Scorer>
  excl(std::shared_ptr<Scorer> main,
       std::shared_ptr<std::deque<std::shared_ptr<ScorerSupplier>>> prohibited,
       int64_t leadCost) ;

  std::shared_ptr<Scorer>
  opt(std::shared_ptr<std::deque<std::shared_ptr<ScorerSupplier>>> optional,
      int minShouldMatch, bool needsScores,
      int64_t leadCost) ;

protected:
  std::shared_ptr<Boolean2ScorerSupplier> shared_from_this()
  {
    return std::static_pointer_cast<Boolean2ScorerSupplier>(
        ScorerSupplier::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
