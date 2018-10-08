#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class Similarity;
}

namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::search::similarities
{

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Implements the CombSUM method for combining evidence from multiple
 * similarity values described in: Joseph A. Shaw, Edward A. Fox.
 * In Text REtrieval Conference (1993), pp. 243-252
 * @lucene.experimental
 */
class MultiSimilarity : public Similarity
{
  GET_CLASS_NAME(MultiSimilarity)
  /** the sub-similarities used to create the combined score */
protected:
  std::deque<std::shared_ptr<Similarity>> const sims;

  /** Creates a MultiSimilarity which will sum the scores
   * of the provided <code>sims</code>. */
public:
  MultiSimilarity(std::deque<std::shared_ptr<Similarity>> &sims);

  int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

  std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) override;

  std::shared_ptr<SimScorer> simScorer(
      std::shared_ptr<SimWeight> stats,
      std::shared_ptr<LeafReaderContext> context)  override;

public:
  class MultiSimScorer : public SimScorer
  {
    GET_CLASS_NAME(MultiSimScorer)
  private:
    std::deque<std::shared_ptr<SimScorer>> const subScorers;

  public:
    MultiSimScorer(std::deque<std::shared_ptr<SimScorer>> &subScorers);

    float score(int doc, float freq)  override;

    std::shared_ptr<Explanation>
    explain(int doc,
            std::shared_ptr<Explanation> freq)  override;

    float computeSlopFactor(int distance) override;

    float computePayloadFactor(int doc, int start, int end,
                               std::shared_ptr<BytesRef> payload) override;

  protected:
    std::shared_ptr<MultiSimScorer> shared_from_this()
    {
      return std::static_pointer_cast<MultiSimScorer>(
          SimScorer::shared_from_this());
    }
  };

public:
  class MultiStats : public SimWeight
  {
    GET_CLASS_NAME(MultiStats)
  public:
    std::deque<std::shared_ptr<SimWeight>> const subStats;

    MultiStats(std::deque<std::shared_ptr<SimWeight>> &subStats);

  protected:
    std::shared_ptr<MultiStats> shared_from_this()
    {
      return std::static_pointer_cast<MultiStats>(
          SimWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MultiSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<MultiSimilarity>(
        Similarity::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
