#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class BasicStats;
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

/**
 * F2EXP is defined as Sum(tfln(term_doc_freq, docLen)*IDF(term))
 * where IDF(t) = pow((N+1)/df(t), k) N=total num of docs, df=doc freq
 *
 * @lucene.experimental
 */
class AxiomaticF2EXP : public Axiomatic
{
  GET_CLASS_NAME(AxiomaticF2EXP)
  /**
   * Constructor setting s and k, letting queryLen to default
   * @param s hyperparam for the growth function
   * @param k hyperparam for the primitive weighting function
   */
public:
  AxiomaticF2EXP(float s, float k);

  /**
   * Constructor setting s only, letting k and queryLen to default
   * @param s hyperparam for the growth function
   */
  AxiomaticF2EXP(float s);

  /**
   * Default constructor
   */
  AxiomaticF2EXP();

  virtual std::wstring toString();

  /**
   * compute the term frequency component
   */
protected:
  float tf(std::shared_ptr<BasicStats> stats, float freq,
           float docLen) override;

  /**
   * compute the document length component
   */
  float ln(std::shared_ptr<BasicStats> stats, float freq,
           float docLen) override;

  /**
   * compute the mixed term frequency and document length component
   */
  float tfln(std::shared_ptr<BasicStats> stats, float freq,
             float docLen) override;

  /**
   * compute the inverted document frequency component
   */
  float idf(std::shared_ptr<BasicStats> stats, float freq,
            float docLen) override;

  /**
   * compute the gamma component
   */
  float gamma(std::shared_ptr<BasicStats> stats, float freq,
              float docLen) override;

protected:
  std::shared_ptr<AxiomaticF2EXP> shared_from_this()
  {
    return std::static_pointer_cast<AxiomaticF2EXP>(
        Axiomatic::shared_from_this());
  }
};
} // namespace org::apache::lucene::search::similarities
