#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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
 * Normalization model that assumes a uniform distribution of the term
 * frequency. <p>While this model is parameterless in the <a
 * href="http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.101.742">
 * original article</a>, <a href="http://dl.acm.org/citation.cfm?id=1835490">
 * information-based models</a> (see {@link IBSimilarity}) introduced a
 * multiplying factor.
 * The default value for the {@code c} parameter is {@code 1}.</p>
 * @lucene.experimental
 */
class NormalizationH1 : public Normalization
{
  GET_CLASS_NAME(NormalizationH1)
private:
  const float c;

  /**
   * Creates NormalizationH1 with the supplied parameter <code>c</code>.
   * @param c hyper-parameter that controls the term frequency
   * normalization with respect to the document length.
   */
public:
  NormalizationH1(float c);

  /**
   * Calls {@link #NormalizationH1(float) NormalizationH1(1)}
   */
  NormalizationH1();

  float tfn(std::shared_ptr<BasicStats> stats, float tf,
            float len) override final;

  virtual std::wstring toString();

  /**
   * Returns the <code>c</code> parameter.
   * @see #NormalizationH1(float)
   */
  virtual float getC();

protected:
  std::shared_ptr<NormalizationH1> shared_from_this()
  {
    return std::static_pointer_cast<NormalizationH1>(
        Normalization::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
