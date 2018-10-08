#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class CollectionModel;
}

namespace org::apache::lucene::search::similarities
{
class BasicStats;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search
{
class Explanation;
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

using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;

/**
 * Abstract superclass for language modeling Similarities. The following inner
 * types are introduced:
 * <ul>
 *   <li>{@link LMStats}, which defines a new statistic, the probability that
 *   the collection language model generates the current term;</li>
 *   <li>{@link CollectionModel}, which is a strategy interface for object that
 *   compute the collection language model {@code p(w|C)};</li>
 *   <li>{@link DefaultCollectionModel}, an implementation of the former, that
 *   computes the term probability as the number of occurrences of the term in
 * the collection, divided by the total number of tokens.</li>
 * </ul>
 *
 * @lucene.experimental
 */
class LMSimilarity : public SimilarityBase
{
  GET_CLASS_NAME(LMSimilarity)
  /** The collection model. */
protected:
  const std::shared_ptr<CollectionModel> collectionModel;

  /** Creates a new instance with the specified collection language model. */
public:
  LMSimilarity(std::shared_ptr<CollectionModel> collectionModel);

  /** Creates a new instance with the default collection language model. */
  LMSimilarity();

protected:
  std::shared_ptr<BasicStats> newStats(const std::wstring &field,
                                       float boost) override;

  /**
   * Computes the collection probability of the current term in addition to the
   * usual statistics.
   */
  void fillBasicStats(std::shared_ptr<BasicStats> stats,
                      std::shared_ptr<CollectionStatistics> collectionStats,
                      std::shared_ptr<TermStatistics> termStats) override;

  void explain(std::deque<std::shared_ptr<Explanation>> &subExpls,
               std::shared_ptr<BasicStats> stats, int doc, float freq,
               float docLen) override;

  /**
   * Returns the name of the LM method. The values of the parameters should be
   * included as well.
   * <p>Used in {@link #toString()}</p>.
   */
public:
  virtual std::wstring getName() = 0;

  /**
   * Returns the name of the LM method. If a custom collection model strategy is
   * used, its name is included as well.
   * @see #getName()
   * @see CollectionModel#getName()
   * @see DefaultCollectionModel
   */
  virtual std::wstring toString();

  /** Stores the collection distribution of the current term. */
public:
  class LMStats : public BasicStats
  {
    GET_CLASS_NAME(LMStats)
    /** The probability that the current term is generated by the collection. */
  private:
    float collectionProbability = 0;

    /**
     * Creates LMStats for the provided field and query-time boost
     */
  public:
    LMStats(const std::wstring &field, float boost);

    /**
     * Returns the probability that the current term is generated by the
     * collection.
     */
    float getCollectionProbability();

    /**
     * Sets the probability that the current term is generated by the
     * collection.
     */
    void setCollectionProbability(float collectionProbability);

  protected:
    std::shared_ptr<LMStats> shared_from_this()
    {
      return std::static_pointer_cast<LMStats>(BasicStats::shared_from_this());
    }
  };

  /** A strategy for computing the collection language model. */
public:
  class CollectionModel
  {
    GET_CLASS_NAME(CollectionModel)
    /**
     * Computes the probability {@code p(w|C)} according to the language model
     * strategy for the current term.
     */
  public:
    virtual float computeProbability(std::shared_ptr<BasicStats> stats) = 0;

    /** The name of the collection model strategy. */
    virtual std::wstring getName() = 0;
  };

  /**
   * Models {@code p(w|C)} as the number of occurrences of the term in the
   * collection, divided by the total number of tokens {@code + 1}.
   */
public:
  class DefaultCollectionModel
      : public std::enable_shared_from_this<DefaultCollectionModel>,
        public CollectionModel
  {
    GET_CLASS_NAME(DefaultCollectionModel)

    /** Sole constructor: parameter-free */
  public:
    DefaultCollectionModel();

    float computeProbability(std::shared_ptr<BasicStats> stats) override;

    std::wstring getName() override;
  };

protected:
  std::shared_ptr<LMSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<LMSimilarity>(
        SimilarityBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
