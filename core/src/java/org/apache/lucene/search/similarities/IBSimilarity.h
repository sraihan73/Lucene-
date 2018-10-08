#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/Distribution.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Lambda.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Normalization.h"
#include  "core/src/java/org/apache/lucene/search/similarities/BasicStats.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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

using Explanation = org::apache::lucene::search::Explanation;

/**
 * Provides a framework for the family of information-based models, as described
 * in St&eacute;phane Clinchant and Eric Gaussier. 2010. Information-based
 * models for ad hoc IR. In Proceeding of the 33rd international ACM SIGIR
 * conference on Research and development in information retrieval (SIGIR '10).
 * ACM, New York, NY, USA, 234-241.
 * <p>The retrieval function is of the form <em>RSV(q, d) = &sum;
 * -x<sup>q</sup><sub>w</sub> log Prob(X<sub>w</sub> &ge;
 * t<sup>d</sup><sub>w</sub> | &lambda;<sub>w</sub>)</em>, where
 * <ul>
 *   <li><em>x<sup>q</sup><sub>w</sub></em> is the query boost;</li>
 *   <li><em>X<sub>w</sub></em> is a random variable that counts the occurrences
 *   of word <em>w</em>;</li>
 *   <li><em>t<sup>d</sup><sub>w</sub></em> is the normalized term
 * frequency;</li> <li><em>&lambda;<sub>w</sub></em> is a parameter.</li>
 * </ul>
 * <p>The framework described in the paper has many similarities to the DFR
 * framework (see {@link DFRSimilarity}). It is possible that the two
 * Similarities will be merged at one point.</p>
 * <p>To construct an IBSimilarity, you must specify the implementations for
 * all three components of the Information-Based model.
 * <ol>
 *     <li>{@link Distribution}: Probabilistic distribution used to
 *         model term occurrence
 *         <ul>
 *             <li>{@link DistributionLL}: Log-logistic</li>
 *             <li>{@link DistributionLL}: Smoothed power-law</li>
 *         </ul>
 *     </li>
 *     <li>{@link Lambda}: &lambda;<sub>w</sub> parameter of the
 *         probability distribution
 *         <ul>
 *             <li>{@link LambdaDF}: <code>N<sub>w</sub>/N</code> or average
 *                 number of documents where w occurs</li>
 *             <li>{@link LambdaTTF}: <code>F<sub>w</sub>/N</code> or
 *                 average number of occurrences of w in the collection</li>
 *         </ul>
 *     </li>
 *     <li>{@link Normalization}: Term frequency normalization
 *         <blockquote>Any supported DFR normalization (listed in
 *                      {@link DFRSimilarity})</blockquote>
 *     </li>
 * </ol>
 * @see DFRSimilarity
 * @lucene.experimental
 */
class IBSimilarity : public SimilarityBase
{
  GET_CLASS_NAME(IBSimilarity)
  /** The probabilistic distribution used to model term occurrence. */
protected:
  const std::shared_ptr<Distribution> distribution;
  /** The <em>lambda (&lambda;<sub>w</sub>)</em> parameter. */
  const std::shared_ptr<Lambda> lambda;
  /** The term frequency normalization. */
  const std::shared_ptr<Normalization> normalization;

  /**
   * Creates IBSimilarity from the three components.
   * <p>
   * Note that <code>null</code> values are not allowed:
   * if you want no normalization, instead pass
   * {@link NoNormalization}.
   * @param distribution probabilistic distribution modeling term occurrence
   * @param lambda distribution's &lambda;<sub>w</sub> parameter
   * @param normalization term frequency normalization
   */
public:
  IBSimilarity(std::shared_ptr<Distribution> distribution,
               std::shared_ptr<Lambda> lambda,
               std::shared_ptr<Normalization> normalization);

protected:
  float score(std::shared_ptr<BasicStats> stats, float freq,
              float docLen) override;

  void explain(std::deque<std::shared_ptr<Explanation>> &subs,
               std::shared_ptr<BasicStats> stats, int doc, float freq,
               float docLen) override;

  /**
   * The name of IB methods follow the pattern
   * {@code IB <distribution> <lambda><normalization>}. The name of the
   * distribution is the same as in the original paper; for the names of lambda
   * parameters, refer to the javadoc of the {@link Lambda} classes.
   */
public:
  virtual std::wstring toString();

  /**
   * Returns the distribution
   */
  virtual std::shared_ptr<Distribution> getDistribution();

  /**
   * Returns the distribution's lambda parameter
   */
  virtual std::shared_ptr<Lambda> getLambda();

  /**
   * Returns the term frequency normalization
   */
  virtual std::shared_ptr<Normalization> getNormalization();

protected:
  std::shared_ptr<IBSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<IBSimilarity>(
        SimilarityBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
