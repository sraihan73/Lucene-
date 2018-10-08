#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class Similarity;
}

namespace org::apache::lucene::search
{
class BooleanQuery;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Matches;
}
namespace org::apache::lucene::search
{
class BulkScorer;
}
namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::search
{
class FakeScorer;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class ScorerSupplier;
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * Expert: the Weight for BooleanQuery, used to
 * normalize, score and explain these queries.
 */
class BooleanWeight final : public Weight
{
  GET_CLASS_NAME(BooleanWeight)
  /** The Similarity implementation. */
public:
  const std::shared_ptr<Similarity> similarity;
  const std::shared_ptr<BooleanQuery> query;

  const std::deque<std::shared_ptr<Weight>> weights;
  const bool needsScores;

  BooleanWeight(std::shared_ptr<BooleanQuery> query,
                std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                float boost) ;

  void extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

  std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                   int doc)  override;

  static std::shared_ptr<BulkScorer>
  disableScoring(std::shared_ptr<BulkScorer> scorer);

private:
  class BulkScorerAnonymousInnerClass : public BulkScorer
  {
    GET_CLASS_NAME(BulkScorerAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::BulkScorer> scorer;

  public:
    BulkScorerAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::BulkScorer> scorer);

    int score(std::shared_ptr<LeafCollector> collector,
              std::shared_ptr<Bits> acceptDocs, int min,
              int max)  override;

  private:
    class LeafCollectorAnonymousInnerClass
        : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
          public LeafCollector
    {
      GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<BulkScorerAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::search::LeafCollector> collector;

    public:
      LeafCollectorAnonymousInnerClass(
          std::shared_ptr<BulkScorerAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::search::LeafCollector>
              collector);

      std::shared_ptr<FakeScorer> fake;

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

      void collect(int doc)  override;
    };

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<BulkScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BulkScorerAnonymousInnerClass>(
          BulkScorer::shared_from_this());
    }
  };

  // Return a BulkScorer for the optional clauses only,
  // or null if it is not applicable
  // pkg-private for forcing use of BooleanScorer in tests
public:
  std::shared_ptr<BulkScorer> optionalBulkScorer(
      std::shared_ptr<LeafReaderContext> context) ;

  // Return a BulkScorer for the required clauses only,
  // or null if it is not applicable
private:
  std::shared_ptr<BulkScorer> requiredBulkScorer(
      std::shared_ptr<LeafReaderContext> context) ;

  /** Try to build a bool scorer for this weight. Returns null if {@link
   * BooleanScorer} cannot be used. */
public:
  std::shared_ptr<BulkScorer>
  booleanScorer(std::shared_ptr<LeafReaderContext> context) ;

  std::shared_ptr<BulkScorer> bulkScorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  std::shared_ptr<Scorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<ScorerSupplier> scorerSupplier(
      std::shared_ptr<LeafReaderContext> context)  override;

protected:
  std::shared_ptr<BooleanWeight> shared_from_this()
  {
    return std::static_pointer_cast<BooleanWeight>(Weight::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
