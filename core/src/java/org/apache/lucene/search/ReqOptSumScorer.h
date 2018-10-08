#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Scorer;
}

namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
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
namespace org::apache::lucene::search
{

/** A Scorer for queries with a required part and an optional part.
 * Delays skipTo() on the optional part until a score() is needed.
 */
class ReqOptSumScorer : public Scorer
{
  GET_CLASS_NAME(ReqOptSumScorer)
  /** The scorers passed from the constructor.
   * These are set to null as soon as their next() or skipTo() returns false.
   */
protected:
  const std::shared_ptr<Scorer> reqScorer;
  const std::shared_ptr<Scorer> optScorer;
  const std::shared_ptr<DocIdSetIterator> optIterator;

  /** Construct a <code>ReqOptScorer</code>.
   * @param reqScorer The required scorer. This must match.
   * @param optScorer The optional scorer. This is used for scoring only.
   */
public:
  ReqOptSumScorer(std::shared_ptr<Scorer> reqScorer,
                  std::shared_ptr<Scorer> optScorer);

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  int docID() override;

  /** Returns the score of the current document matching the query.
   * Initially invalid, until the {@link #iterator()} is advanced the first
   * time.
   * @return The score of the required scorer, eventually increased by the score
   * of the optional scorer when it also matches the current document.
   */
  float score()  override;

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren() override;

protected:
  std::shared_ptr<ReqOptSumScorer> shared_from_this()
  {
    return std::static_pointer_cast<ReqOptSumScorer>(
        Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
