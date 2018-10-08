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
 * This class acts as the base class for the implementations of the term
 * frequency normalization methods in the DFR framework.
 *
 * @see DFRSimilarity
 * @lucene.experimental
 */
class Normalization : public std::enable_shared_from_this<Normalization>
{
  GET_CLASS_NAME(Normalization)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  Normalization();

  /** Returns the normalized term frequency.
   * @param len the field length. */
  virtual float tfn(std::shared_ptr<BasicStats> stats, float tf, float len) = 0;

  /** Returns an explanation for the normalized term frequency.
   * <p>The default normalization methods use the field length of the document
   * and the average field length to compute the normalized term frequency.
   * This method provides a generic explanation for such methods.
   * Subclasses that use other statistics must override this method.</p>
   */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<BasicStats> stats, float tf, float len);

  /** Implementation used when there is no normalization. */
public:
  class NoNormalization;

  /**
   * Subclasses must override this method to return the code of the
   * normalization formula. Refer to the original paper for the deque.
   */
public:
  std::wstring toString() = 0;
  override
};

} // namespace org::apache::lucene::search::similarities
class Normalization::NoNormalization final : public Normalization
{
  GET_CLASS_NAME(Normalization::NoNormalization)

  /** Sole constructor: parameter-free */
public:
  NoNormalization();

  float tfn(std::shared_ptr<BasicStats> stats, float tf,
            float len) override final;

  std::shared_ptr<Explanation> explain(std::shared_ptr<BasicStats> stats,
                                       float tf, float len) override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<NoNormalization> shared_from_this()
  {
    return std::static_pointer_cast<NoNormalization>(
        Normalization::shared_from_this());
  }
};
