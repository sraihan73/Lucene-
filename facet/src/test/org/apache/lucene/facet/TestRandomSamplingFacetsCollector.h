#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

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
namespace org::apache::lucene::facet
{

class TestRandomSamplingFacetsCollector : public FacetTestCase
{
  GET_CLASS_NAME(TestRandomSamplingFacetsCollector)

  // The first 50 chi-square value for p-value=0.05, taken from:
  // http://en.wikibooks.org/wiki/Engineering_Tables/Chi-Squared_Distibution
private:
  static std::deque<float> const CHI_SQUARE_VALUES;

public:
  virtual void testRandomSampling() ;

protected:
  std::shared_ptr<TestRandomSamplingFacetsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestRandomSamplingFacetsCollector>(
        FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet
