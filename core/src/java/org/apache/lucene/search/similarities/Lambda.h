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
 * The <em>lambda (&lambda;<sub>w</sub>)</em> parameter in information-based
 * models.
 * @see IBSimilarity
 * @lucene.experimental
 */
class Lambda : public std::enable_shared_from_this<Lambda>
{
  GET_CLASS_NAME(Lambda)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  Lambda();

  /** Computes the lambda parameter. */
  virtual float lambda(std::shared_ptr<BasicStats> stats) = 0;
  /** Explains the lambda parameter. */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<BasicStats> stats) = 0;

  /**
   * Subclasses must override this method to return the code of the lambda
   * formula. Since the original paper is not very clear on this matter, and
   * also uses the DFR naming scheme incorrectly, the codes here were chosen
   * arbitrarily.
   */
  std::wstring toString() = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
