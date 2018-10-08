#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
 * The probabilistic distribution used to model term occurrence
 * in information-based models.
 * @see IBSimilarity
 * @lucene.experimental
 */
class Distribution : public std::enable_shared_from_this<Distribution>
{
  GET_CLASS_NAME(Distribution)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  Distribution();

  /** Computes the score. */
  virtual float score(std::shared_ptr<BasicStats> stats, float tfn,
                      float lambda) = 0;

  /** Explains the score. Returns the name of the model only, since
   * both {@code tfn} and {@code lambda} are explained elsewhere. */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<BasicStats> stats, float tfn, float lambda);

  /**
   * Subclasses must override this method to return the name of the
   * distribution.
   */
  std::wstring toString() = 0;
  override
};

} // namespace org::apache::lucene::search::similarities
