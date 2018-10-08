#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}

namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class Weight;
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

/**
 * A constant-scoring {@link Scorer}.
 * @lucene.internal
 */
class ConstantScoreScorer final : public Scorer
{
  GET_CLASS_NAME(ConstantScoreScorer)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const float score_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<TwoPhaseIterator> twoPhaseIterator_;
  const std::shared_ptr<DocIdSetIterator> disi;

  /** Constructor based on a {@link DocIdSetIterator} which will be used to
   *  drive iteration. Two phase iteration will not be supported.
   *  @param weight the parent weight
   *  @param score the score to return on each document
   *  @param disi the iterator that defines matching documents */
public:
  ConstantScoreScorer(std::shared_ptr<Weight> weight, float score,
                      std::shared_ptr<DocIdSetIterator> disi);

  /** Constructor based on a {@link TwoPhaseIterator}. In that case the
   *  {@link Scorer} will support two-phase iteration.
   *  @param weight the parent weight
   *  @param score the score to return on each document
   *  @param twoPhaseIterator the iterator that defines matching documents */
  ConstantScoreScorer(std::shared_ptr<Weight> weight, float score,
                      std::shared_ptr<TwoPhaseIterator> twoPhaseIterator);

  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

  int docID() override;

  float score()  override;

protected:
  std::shared_ptr<ConstantScoreScorer> shared_from_this()
  {
    return std::static_pointer_cast<ConstantScoreScorer>(
        Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
