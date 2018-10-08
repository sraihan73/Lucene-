#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}

namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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
namespace org::apache::lucene::queries::function
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

/**
 * {@link Scorer} which returns the result of {@link
 * FunctionValues#floatVal(int)} as the score for a document, and which filters
 * out documents that don't match {@link #matches(int)}. This Scorer has a
 * {@link TwoPhaseIterator}.  This is similar to {@link FunctionQuery}, but this
 * one has no {@link org.apache.lucene.search.Weight} normalization
 * factors/multipliers and that one doesn't filter either. <p> Note: If the
 * scores are needed, then the underlying value will probably be
 * fetched/computed twice -- once to filter and next to return the score.  If
 * that's non-trivial then consider wrapping it in an implementation that will
 * cache the current value.
 * </p>
 *
 * @see FunctionQuery
 * @lucene.experimental
 */
class ValueSourceScorer : public Scorer
{
  GET_CLASS_NAME(ValueSourceScorer)
protected:
  const std::shared_ptr<FunctionValues> values;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<TwoPhaseIterator> twoPhaseIterator_;
  const std::shared_ptr<DocIdSetIterator> disi;

protected:
  ValueSourceScorer(std::shared_ptr<LeafReaderContext> readerContext,
                    std::shared_ptr<FunctionValues> values); // no weight

  /** Override to decide if this document matches. It's called by {@link
   * TwoPhaseIterator#matches()}. */
  virtual bool matches(int doc) = 0;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

  int docID() override;

  float score()  override;

protected:
  std::shared_ptr<ValueSourceScorer> shared_from_this()
  {
    return std::static_pointer_cast<ValueSourceScorer>(
        org.apache.lucene.search.Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
