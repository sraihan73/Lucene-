#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"

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

using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Collects hits for subsequent faceting, using sampling if needed. Once you've
 * run a search and collect hits into this, instantiate one of the
 * {@link Facets} subclasses to do the facet counting. Note that this collector
 * does not collect the scores of matching docs (i.e.
 * {@link FacetsCollector.MatchingDocs#scores}) is {@code null}.
 * <p>
 * If you require the original set of hits, you can call
 * {@link #getOriginalMatchingDocs()}. Also, since the counts of the top-facets
 * is based on the sampled set, you can amortize the counts by calling
 * {@link #amortizeFacetCounts}.
 */
class RandomSamplingFacetsCollector : public FacetsCollector
{
  GET_CLASS_NAME(RandomSamplingFacetsCollector)

  /**
   * Faster alternative for java.util.Random, inspired by
   * http://dmurphy747.wordpress.com/2011/03/23/xorshift-vs-random-
   * performance-in-java/
   * <p>
   * Has a period of 2^64-1
   */
private:
  class XORShift64Random : public std::enable_shared_from_this<XORShift64Random>
  {
    GET_CLASS_NAME(XORShift64Random)

  private:
    int64_t x = 0;

    /** Creates a xorshift random generator using the provided seed */
  public:
    XORShift64Random(int64_t seed);

    /** Get the next random long value */
    virtual int64_t randomLong();

    /** Get the next random int, between 0 (inclusive) and n (exclusive) */
    virtual int nextInt(int n);
  };

private:
  static constexpr int NOT_CALCULATED = -1;

  const int sampleSize;
  const std::shared_ptr<XORShift64Random> random;

  double samplingRate = 0;
  std::deque<std::shared_ptr<MatchingDocs>> sampledDocs;
  int totalHits = NOT_CALCULATED;
  int leftoverBin = NOT_CALCULATED;
  int leftoverIndex = NOT_CALCULATED;

  /**
   * Constructor with the given sample size and default seed.
   *
   * @see #RandomSamplingFacetsCollector(int, long)
   */
public:
  RandomSamplingFacetsCollector(int sampleSize);

  /**
   * Constructor with the given sample size and seed.
   *
   * @param sampleSize
   *          The preferred sample size. If the number of hits is greater than
   *          the size, sampling will be done using a sample ratio of sampling
   *          size / totalN. For example: 1000 hits, sample size = 10 results in
   *          samplingRatio of 0.01. If the number of hits is lower, no sampling
   *          is done at all
   * @param seed
   *          The random seed. If {@code 0} then a seed will be chosen for you.
   */
  RandomSamplingFacetsCollector(int sampleSize, int64_t seed);

  /**
   * Returns the sampled deque of the matching documents. Note that a
   * {@link FacetsCollector.MatchingDocs} instance is returned per segment, even
   * if no hits from that segment are included in the sampled set.
   * <p>
   * Note: One or more of the MatchingDocs might be empty (not containing any
   * hits) as result of sampling.
   * <p>
   * Note: {@code MatchingDocs.totalHits} is copied from the original
   * MatchingDocs, scores is set to {@code null}
   */
  std::deque<std::shared_ptr<MatchingDocs>> getMatchingDocs() override;

  /** Returns the original matching documents. */
  virtual std::deque<std::shared_ptr<MatchingDocs>> getOriginalMatchingDocs();

  /** Create a sampled copy of the matching documents deque. */
private:
  std::deque<std::shared_ptr<MatchingDocs>> createSampledDocs(
      std::deque<std::shared_ptr<MatchingDocs>> &matchingDocsList);

  /** Create a sampled of the given hits. */
  std::shared_ptr<MatchingDocs>
  createSample(std::shared_ptr<MatchingDocs> docs);

  /**
   * Note: if you use a counting {@link Facets} implementation, you can amortize
   * the sampled counts by calling this method. Uses the {@link FacetsConfig}
   * and the {@link IndexSearcher} to determine the upper bound for each facet
   * value.
   */
public:
  virtual std::shared_ptr<FacetResult> amortizeFacetCounts(
      std::shared_ptr<FacetResult> res, std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<IndexSearcher> searcher) ;

  /** Returns the sampling rate that was used. */
  virtual double getSamplingRate();

protected:
  std::shared_ptr<RandomSamplingFacetsCollector> shared_from_this()
  {
    return std::static_pointer_cast<RandomSamplingFacetsCollector>(
        FacetsCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
