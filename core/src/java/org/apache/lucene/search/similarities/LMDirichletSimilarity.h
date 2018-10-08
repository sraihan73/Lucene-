#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
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
 * Bayesian smoothing using Dirichlet priors. From Chengxiang Zhai and John
 * Lafferty. 2001. A study of smoothing methods for language models applied to
 * Ad Hoc information retrieval. In Proceedings of the 24th annual international
 * ACM SIGIR conference on Research and development in information retrieval
 * (SIGIR '01). ACM, New York, NY, USA, 334-342.
 * <p>
 * The formula as defined the paper assigns a negative score to documents that
 * contain the term, but with fewer occurrences than predicted by the collection
 * language model. The Lucene implementation returns {@code 0} for such
 * documents.
 * </p>
 *
 * @lucene.experimental
 */
class LMDirichletSimilarity : public LMSimilarity
{
  GET_CLASS_NAME(LMDirichletSimilarity)
  /** The &mu; parameter. */
private:
  const float mu;

  /** Instantiates the similarity with the provided &mu; parameter. */
public:
  LMDirichletSimilarity(std::shared_ptr<CollectionModel> collectionModel,
                        float mu);

  /** Instantiates the similarity with the provided &mu; parameter. */
  LMDirichletSimilarity(float mu);

  /** Instantiates the similarity with the default &mu; value of 2000. */
  LMDirichletSimilarity(std::shared_ptr<CollectionModel> collectionModel);

  /** Instantiates the similarity with the default &mu; value of 2000. */
  LMDirichletSimilarity();

protected:
  float score(std::shared_ptr<BasicStats> stats, float freq,
              float docLen) override;

  void explain(std::deque<std::shared_ptr<Explanation>> &subs,
               std::shared_ptr<BasicStats> stats, int doc, float freq,
               float docLen) override;

  /** Returns the &mu; parameter. */
public:
  virtual float getMu();

  std::wstring getName() override;

protected:
  std::shared_ptr<LMDirichletSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<LMDirichletSimilarity>(
        LMSimilarity::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
