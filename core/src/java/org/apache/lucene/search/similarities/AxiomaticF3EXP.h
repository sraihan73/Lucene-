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
 * F2EXP is defined as Sum(tf(term_doc_freq)*IDF(term)-gamma(docLen, queryLen))
 * where IDF(t) = pow((N+1)/df(t), k) N=total num of docs, df=doc freq
 * gamma(docLen, queryLen) = (docLen-queryLen)*queryLen*s/avdl
 *
 * @lucene.experimental
 */
class AxiomaticF3EXP : public Axiomatic
{
  GET_CLASS_NAME(AxiomaticF3EXP)

  /**
   * Constructor setting all Axiomatic hyperparameters
   *
   * @param s        hyperparam for the growth function
   * @param queryLen the query length
   * @param k        hyperparam for the primitive weighting function
   */
public:
  AxiomaticF3EXP(float s, int queryLen, float k);

  /**
   * Constructor setting s and queryLen, letting k to default
   *
   * @param s        hyperparam for the growth function
   * @param queryLen the query length
   */
  AxiomaticF3EXP(float s, int queryLen);

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
  std::shared_ptr<AxiomaticF3EXP> shared_from_this()
  {
    return std::static_pointer_cast<AxiomaticF3EXP>(
        Axiomatic::shared_from_this());
  }
};
} // namespace org::apache::lucene::search::similarities
