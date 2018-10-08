#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/BasicStats.h"

#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
 * This class acts as the base class for the specific <em>basic model</em>
 * implementations in the DFR framework. Basic models compute the
 * <em>informative content Inf<sub>1</sub> = -log<sub>2</sub>Prob<sub>1</sub>
 * </em>.
 *
 * @see DFRSimilarity
 * @lucene.experimental
 */
class BasicModel : public std::enable_shared_from_this<BasicModel>
{
  GET_CLASS_NAME(BasicModel)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  BasicModel();

  /** Returns the informative content score. */
  virtual float score(std::shared_ptr<BasicStats> stats, float tfn) = 0;

  /**
   * Returns an explanation for the score.
   * <p>Most basic models use the number of documents and the total term
   * frequency to compute Inf<sub>1</sub>. This method provides a generic
   * explanation for such models. Subclasses that use other statistics must
   * override this method.</p>
   */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<BasicStats> stats, float tfn);

  /**
   * Subclasses must override this method to return the code of the
   * basic model formula. Refer to the original paper for the deque.
   */
  std::wstring toString() = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
