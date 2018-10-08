#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/RandomTwoPhaseView.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

/**
 * A {@link Query} that adds random approximations to its scorers.
 */
class RandomApproximationQuery : public Query
{
  GET_CLASS_NAME(RandomApproximationQuery)

private:
  const std::shared_ptr<Query> query;
  const std::shared_ptr<Random> random;

public:
  RandomApproximationQuery(std::shared_ptr<Query> query,
                           std::shared_ptr<Random> random);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any other) override;

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class RandomApproximationWeight : public FilterWeight
  {
    GET_CLASS_NAME(RandomApproximationWeight)

  private:
    const std::shared_ptr<Random> random;

  public:
    RandomApproximationWeight(std::shared_ptr<Weight> weight,
                              std::shared_ptr<Random> random);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<RandomApproximationWeight> shared_from_this()
    {
      return std::static_pointer_cast<RandomApproximationWeight>(
          FilterWeight::shared_from_this());
    }
  };

private:
  class RandomApproximationScorer : public Scorer
  {
    GET_CLASS_NAME(RandomApproximationScorer)

  private:
    const std::shared_ptr<Scorer> scorer;
    const std::shared_ptr<RandomTwoPhaseView> twoPhaseView;

  public:
    RandomApproximationScorer(std::shared_ptr<Scorer> scorer,
                              std::shared_ptr<Random> random);

    std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

    float score()  override;

    int docID() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

  protected:
    std::shared_ptr<RandomApproximationScorer> shared_from_this()
    {
      return std::static_pointer_cast<RandomApproximationScorer>(
          Scorer::shared_from_this());
    }
  };

private:
  class RandomTwoPhaseView : public TwoPhaseIterator
  {
    GET_CLASS_NAME(RandomTwoPhaseView)

  private:
    const std::shared_ptr<DocIdSetIterator> disi;
    int lastDoc = -1;
    const float randomMatchCost;

  public:
    RandomTwoPhaseView(std::shared_ptr<Random> random,
                       std::shared_ptr<DocIdSetIterator> disi);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<RandomTwoPhaseView> shared_from_this()
    {
      return std::static_pointer_cast<RandomTwoPhaseView>(
          TwoPhaseIterator::shared_from_this());
    }
  };

private:
  class RandomApproximation : public DocIdSetIterator
  {
    GET_CLASS_NAME(RandomApproximation)

  private:
    const std::shared_ptr<Random> random;
    const std::shared_ptr<DocIdSetIterator> disi;

  public:
    int doc = -1;

    RandomApproximation(std::shared_ptr<Random> random,
                        std::shared_ptr<DocIdSetIterator> disi);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<RandomApproximation> shared_from_this()
    {
      return std::static_pointer_cast<RandomApproximation>(
          DocIdSetIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<RandomApproximationQuery> shared_from_this()
  {
    return std::static_pointer_cast<RandomApproximationQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
