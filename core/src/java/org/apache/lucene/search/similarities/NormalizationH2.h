#pragma once
#include "stringhelper.h"
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

/**
 * Normalization model in which the term frequency is inversely related to the
 * length.
 * <p>While this model is parameterless in the
 * <a href="http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.101.742">
 * original article</a>, the <a href="http://theses.gla.ac.uk/1570/">thesis</a>
 * introduces the parameterized variant.
 * The default value for the {@code c} parameter is {@code 1}.</p>
 * @lucene.experimental
 */
class NormalizationH2 : public Normalization
{
  GET_CLASS_NAME(NormalizationH2)
private:
  const float c;

  /**
   * Creates NormalizationH2 with the supplied parameter <code>c</code>.
   * @param c hyper-parameter that controls the term frequency
   * normalization with respect to the document length.
   */
public:
  NormalizationH2(float c);

  /**
   * Calls {@link #NormalizationH2(float) NormalizationH2(1)}
   */
  NormalizationH2();

  float tfn(std::shared_ptr<BasicStats> stats, float tf,
            float len) override final;

  virtual std::wstring toString();

  /**
   * Returns the <code>c</code> parameter.
   * @see #NormalizationH2(float)
   */
  virtual float getC();

protected:
  std::shared_ptr<NormalizationH2> shared_from_this()
  {
    return std::static_pointer_cast<NormalizationH2>(
        Normalization::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
