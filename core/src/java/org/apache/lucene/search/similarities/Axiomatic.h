#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class BasicStats;
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

using Explanation = org::apache::lucene::search::Explanation;

/**
 * Axiomatic approaches for IR. From Hui Fang and Chengxiang Zhai
 * 2005. An Exploration of Axiomatic Approaches to Information Retrieval.
 * In Proceedings of the 28th annual international ACM SIGIR
 * conference on Research and development in information retrieval
 * (SIGIR '05). ACM, New York, NY, USA, 480-487.
 * <p>
 * There are a family of models. All of them are based on BM25,
 * Pivoted Document Length Normalization and Language model with
 * Dirichlet prior. Some components (e.g. Term Frequency,
 * Inverted Document Frequency) in the original models are modified
 * so that they follow some axiomatic constraints.
 * </p>
 *
 * @lucene.experimental
 */
class Axiomatic : public SimilarityBase
{
  GET_CLASS_NAME(Axiomatic)
  /**
   * hyperparam for the growth function
   */
protected:
  const float s;

  /**
   * hyperparam for the primitive weighthing function
   */
  const float k;

  /**
   * the query length
   */
  const int queryLen;

  /**
   * Constructor setting all Axiomatic hyperparameters
   * @param s hyperparam for the growth function
   * @param queryLen the query length
   * @param k hyperparam for the primitive weighting function
   */
public:
  Axiomatic(float s, int queryLen, float k);

  /**
   * Constructor setting only s, letting k and queryLen to default
   * @param s hyperparam for the growth function
   */
  Axiomatic(float s);

  /**
   * Constructor setting s and queryLen, letting k to default
   * @param s hyperparam for the growth function
   * @param queryLen the query length
   */
  Axiomatic(float s, int queryLen);

  /**
   * Default constructor
   */
  Axiomatic();

  float score(std::shared_ptr<BasicStats> stats, float freq,
              float docLen) override;

protected:
  void explain(std::deque<std::shared_ptr<Explanation>> &subs,
               std::shared_ptr<BasicStats> stats, int doc, float freq,
               float docLen) override;

  /**
   * Name of the axiomatic method.
   */
public:
  std::wstring toString() = 0;
  override override;

  /**
   * compute the term frequency component
   */
protected:
  virtual float tf(std::shared_ptr<BasicStats> stats, float freq,
                   float docLen) = 0;

  /**
   * compute the document length component
   */
  virtual float ln(std::shared_ptr<BasicStats> stats, float freq,
                   float docLen) = 0;

  /**
   * compute the mixed term frequency and document length component
   */
  virtual float tfln(std::shared_ptr<BasicStats> stats, float freq,
                     float docLen) = 0;

  /**
   * compute the inverted document frequency component
   */
  virtual float idf(std::shared_ptr<BasicStats> stats, float freq,
                    float docLen) = 0;

  /**
   * compute the gamma component (only for F3EXp and F3LOG)
   */
  virtual float gamma(std::shared_ptr<BasicStats> stats, float freq,
                      float docLen) = 0;

protected:
  std::shared_ptr<Axiomatic> shared_from_this()
  {
    return std::static_pointer_cast<Axiomatic>(
        SimilarityBase::shared_from_this());
  }
};
} // namespace org::apache::lucene::search::similarities
