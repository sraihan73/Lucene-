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
 * Log-logistic distribution.
 * <p>Unlike for DFR, the natural logarithm is used, as
 * it is faster to compute and the original paper does not express any
 * preference to a specific base.</p>
 * @lucene.experimental
 */
class DistributionLL : public Distribution
{
  GET_CLASS_NAME(DistributionLL)

  /** Sole constructor: parameter-free */
public:
  DistributionLL();

  float score(std::shared_ptr<BasicStats> stats, float tfn,
              float lambda) override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<DistributionLL> shared_from_this()
  {
    return std::static_pointer_cast<DistributionLL>(
        Distribution::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities