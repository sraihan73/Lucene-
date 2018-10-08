#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

/**
 * Implements the Poisson approximation for the binomial model for DFR.
 * @lucene.experimental
 * <p>
 * WARNING: for terms that do not meet the expected random distribution
 * (e.g. stopwords), this model may give poor performance, such as
 * abnormally high scores for low tf values.
 */
class BasicModelP : public BasicModel
{
  GET_CLASS_NAME(BasicModelP)
  /** {@code log2(Math.E)}, precomputed. */
protected:
  static double LOG2_E;

  /** Sole constructor: parameter-free */
public:
  BasicModelP();

  float score(std::shared_ptr<BasicStats> stats, float tfn) override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<BasicModelP> shared_from_this()
  {
    return std::static_pointer_cast<BasicModelP>(
        BasicModel::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
