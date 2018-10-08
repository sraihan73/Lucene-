#pragma once
#include "stringhelper.h"
#include <cmath>
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
 * Pareto-Zipf Normalization
 * @lucene.experimental
 */
class NormalizationZ : public Normalization
{
  GET_CLASS_NAME(NormalizationZ)
public:
  const float z;

  /**
   * Calls {@link #NormalizationZ(float) NormalizationZ(0.3)}
   */
  NormalizationZ();

  /**
   * Creates NormalizationZ with the supplied parameter <code>z</code>.
   * @param z represents <code>A/(A+1)</code> where <code>A</code>
   *          measures the specificity of the language.
   */
  NormalizationZ(float z);

  float tfn(std::shared_ptr<BasicStats> stats, float tf, float len) override;

  virtual std::wstring toString();

  /**
   * Returns the parameter <code>z</code>
   * @see #NormalizationZ(float)
   */
  virtual float getZ();

protected:
  std::shared_ptr<NormalizationZ> shared_from_this()
  {
    return std::static_pointer_cast<NormalizationZ>(
        Normalization::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
