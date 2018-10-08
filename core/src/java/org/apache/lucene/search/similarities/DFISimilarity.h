#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/Independence.h"

#include  "core/src/java/org/apache/lucene/search/similarities/BasicStats.h"

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
 * Implements the <em>Divergence from Independence (DFI)</em> model based on
 * Chi-square statistics (i.e., standardized Chi-squared distance from
 * independence in term frequency tf). <p> DFI is both parameter-free and
 * non-parametric: <ul> <li>parameter-free: it does not require any parameter
 * tuning or training.</li> <li>non-parametric: it does not make any assumptions
 * about word frequency distributions on document collections.</li>
 * </ul>
 * <p>
 * It is highly recommended <b>not</b> to remove stopwords (very common terms:
 * the, of, and, to, a, in, for, is, on, that, etc) with this similarity. <p>
 * For more information see: <a
 * href="http://dx.doi.org/10.1007/s10791-013-9225-4">A nonparametric term
 * weighting method for information retrieval based on measuring the divergence
 * from independence</a>
 *
 * @lucene.experimental
 * @see org.apache.lucene.search.similarities.IndependenceStandardized
 * @see org.apache.lucene.search.similarities.IndependenceSaturated
 * @see org.apache.lucene.search.similarities.IndependenceChiSquared
 */

class DFISimilarity : public SimilarityBase
{
  GET_CLASS_NAME(DFISimilarity)
private:
  const std::shared_ptr<Independence> independence;

  /**
   * Create DFI with the specified divergence from independence measure
   * @param independenceMeasure measure of divergence from independence
   */
public:
  DFISimilarity(std::shared_ptr<Independence> independenceMeasure);

protected:
  float score(std::shared_ptr<BasicStats> stats, float freq,
              float docLen) override;

  /**
   * Returns the measure of independence
   */
public:
  virtual std::shared_ptr<Independence> getIndependence();

  virtual std::wstring toString();

protected:
  std::shared_ptr<DFISimilarity> shared_from_this()
  {
    return std::static_pointer_cast<DFISimilarity>(
        SimilarityBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
