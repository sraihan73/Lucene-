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

/**
 * Dirichlet Priors normalization
 * @lucene.experimental
 */
class NormalizationH3 : public Normalization
{
  GET_CLASS_NAME(NormalizationH3)
private:
  const float mu;

  /**
   * Calls {@link #NormalizationH3(float) NormalizationH3(800)}
   */
public:
  NormalizationH3();

  /**
   * Creates NormalizationH3 with the supplied parameter <code>&mu;</code>.
   * @param mu smoothing parameter <code>&mu;</code>
   */
  NormalizationH3(float mu);

  float tfn(std::shared_ptr<BasicStats> stats, float tf, float len) override;

  virtual std::wstring toString();

  /**
   * Returns the parameter <code>&mu;</code>
   * @see #NormalizationH3(float)
   */
  virtual float getMu();

protected:
  std::shared_ptr<NormalizationH3> shared_from_this()
  {
    return std::static_pointer_cast<NormalizationH3>(
        Normalization::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
