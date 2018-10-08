#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/BasicModel.h"

#include  "core/src/java/org/apache/lucene/search/similarities/AfterEffect.h"
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
 * Implements the <em>divergence from randomness (DFR)</em> framework
 * introduced in Gianni Amati and Cornelis Joost Van Rijsbergen. 2002.
 * Probabilistic models of information retrieval based on measuring the
 * divergence from randomness. ACM Trans. Inf. Syst. 20, 4 (October 2002),
 * 357-389.
 * <p>The DFR scoring formula is composed of three separate components: the
 * <em>basic model</em>, the <em>aftereffect</em> and an additional
 * <em>normalization</em> component, represented by the classes
 * {@code BasicModel}, {@code AfterEffect} and {@code Normalization},
GET_CLASS_NAME(es)
 * respectively. The names of these classes were chosen to match the names of
 * their counterparts in the Terrier IR engine.</p>
 * <p>To construct a DFRSimilarity, you must specify the implementations for
 * all three components of DFR:
 * <ol>
 *    <li>{@link BasicModel}: Basic model of information content:
 *        <ul>
 *           <li>{@link BasicModelBE}: Limiting form of Bose-Einstein
 *           <li>{@link BasicModelG}: Geometric approximation of Bose-Einstein
 *           <li>{@link BasicModelP}: Poisson approximation of the Binomial
 *           <li>{@link BasicModelD}: Divergence approximation of the Binomial
 *           <li>{@link BasicModelIn}: Inverse document frequency
 *           <li>{@link BasicModelIne}: Inverse expected document
 *               frequency [mixture of Poisson and IDF]
 *           <li>{@link BasicModelIF}: Inverse term frequency
 *               [approximation of I(ne)]
 *        </ul>
 *    <li>{@link AfterEffect}: First normalization of information
 *        gain:
 *        <ul>
 *           <li>{@link AfterEffectL}: Laplace's law of succession
 *           <li>{@link AfterEffectB}: Ratio of two Bernoulli processes
 *           <li>{@link NoAfterEffect}: no first normalization
 *        </ul>
 *    <li>{@link Normalization}: Second (length) normalization:
 *        <ul>
 *           <li>{@link NormalizationH1}: Uniform distribution of term
 *               frequency
 *           <li>{@link NormalizationH2}: term frequency density inversely
 *               related to length
 *           <li>{@link NormalizationH3}: term frequency normalization
 *               provided by Dirichlet prior
 *           <li>{@link NormalizationZ}: term frequency normalization provided
 *                by a Zipfian relation
 *           <li>{@link NoNormalization}: no second normalization
 *        </ul>
 * </ol>
 * <p>Note that <em>qtf</em>, the multiplicity of term-occurrence in the query,
 * is not handled by this implementation.</p>
 * @see BasicModel
 * @see AfterEffect
 * @see Normalization
 * @lucene.experimental
 */
class DFRSimilarity : public SimilarityBase
{
  GET_CLASS_NAME(DFRSimilarity)
  /** The basic model for information content. */
protected:
  const std::shared_ptr<BasicModel> basicModel;
  /** The first normalization of the information content. */
  const std::shared_ptr<AfterEffect> afterEffect;
  /** The term frequency normalization. */
  const std::shared_ptr<Normalization> normalization;

  /**
   * Creates DFRSimilarity from the three components.
   * <p>
   * Note that <code>null</code> values are not allowed:
   * if you want no normalization or after-effect, instead pass
   * {@link NoNormalization} or {@link NoAfterEffect} respectively.
   * @param basicModel Basic model of information content
   * @param afterEffect First normalization of information gain
   * @param normalization Second (length) normalization
   */
public:
  DFRSimilarity(std::shared_ptr<BasicModel> basicModel,
                std::shared_ptr<AfterEffect> afterEffect,
                std::shared_ptr<Normalization> normalization);

protected:
  float score(std::shared_ptr<BasicStats> stats, float freq,
              float docLen) override;

  void explain(std::deque<std::shared_ptr<Explanation>> &subs,
               std::shared_ptr<BasicStats> stats, int doc, float freq,
               float docLen) override;

public:
  virtual std::wstring toString();

  /**
   * Returns the basic model of information content
   */
  virtual std::shared_ptr<BasicModel> getBasicModel();

  /**
   * Returns the first normalization
   */
  virtual std::shared_ptr<AfterEffect> getAfterEffect();

  /**
   * Returns the second normalization
   */
  virtual std::shared_ptr<Normalization> getNormalization();

protected:
  std::shared_ptr<DFRSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<DFRSimilarity>(
        SimilarityBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
