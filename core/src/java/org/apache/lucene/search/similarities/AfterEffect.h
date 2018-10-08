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
 * This class acts as the base class for the implementations of the <em>first
 * normalization of the informative content</em> in the DFR framework. This
 * component is also called the <em>after effect</em> and is defined by the
 * formula <em>Inf<sub>2</sub> = 1 - Prob<sub>2</sub></em>, where
 * <em>Prob<sub>2</sub></em> measures the <em>information gain</em>.
 *
 * @see DFRSimilarity
 * @lucene.experimental
 */
class AfterEffect : public std::enable_shared_from_this<AfterEffect>
{
  GET_CLASS_NAME(AfterEffect)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  AfterEffect();

  /** Returns the aftereffect score. */
  virtual float score(std::shared_ptr<BasicStats> stats, float tfn) = 0;

  /** Returns an explanation for the score. */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<BasicStats> stats, float tfn) = 0;

  /** Implementation used when there is no aftereffect. */
public:
  class NoAfterEffect;

  /**
   * Subclasses must override this method to return the code of the
   * after effect formula. Refer to the original paper for the deque.
   */
public:
  std::wstring toString() = 0;
  override
};

} // namespace org::apache::lucene::search::similarities
class AfterEffect::NoAfterEffect final : public AfterEffect
{
  GET_CLASS_NAME(AfterEffect::NoAfterEffect)

  /** Sole constructor: parameter-free */
public:
  NoAfterEffect();

  float score(std::shared_ptr<BasicStats> stats, float tfn) override final;

  std::shared_ptr<Explanation> explain(std::shared_ptr<BasicStats> stats,
                                       float tfn) override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<NoAfterEffect> shared_from_this()
  {
    return std::static_pointer_cast<NoAfterEffect>(
        AfterEffect::shared_from_this());
  }
};
