#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class Weight;
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

/** Scorer for conjunctions, sets of queries, all of which are required. */
class ConjunctionScorer : public Scorer
{
  GET_CLASS_NAME(ConjunctionScorer)

public:
  const std::shared_ptr<DocIdSetIterator> disi;
  std::deque<std::shared_ptr<Scorer>> const scorers;
  const std::shared_ptr<std::deque<std::shared_ptr<Scorer>>> required;

  /** Create a new {@link ConjunctionScorer}, note that {@code scorers} must be
   * a subset of {@code required}. */
  ConjunctionScorer(
      std::shared_ptr<Weight> weight,
      std::shared_ptr<std::deque<std::shared_ptr<Scorer>>> required,
      std::shared_ptr<std::deque<std::shared_ptr<Scorer>>> scorers);

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  int docID() override;

  float score()  override;

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren() override;

public:
  class DocsAndFreqs final : public std::enable_shared_from_this<DocsAndFreqs>
  {
    GET_CLASS_NAME(DocsAndFreqs)
  public:
    const int64_t cost;
    const std::shared_ptr<DocIdSetIterator> iterator;
    int doc = -1;

    DocsAndFreqs(std::shared_ptr<DocIdSetIterator> iterator);
  };

protected:
  std::shared_ptr<ConjunctionScorer> shared_from_this()
  {
    return std::static_pointer_cast<ConjunctionScorer>(
        Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
