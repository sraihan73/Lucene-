#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class SpanCollector;
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
namespace org::apache::lucene::search::spans
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

/** Iterates through combinations of start/end positions per-doc.
 *  Each start/end position represents a range of term positions within the
 * current document. These are enumerated in order, by increasing document
 * number, within that by increasing start position and finally by increasing
 * end position.
 */
class Spans : public DocIdSetIterator
{
  GET_CLASS_NAME(Spans)

public:
  static const int NO_MORE_POSITIONS = std::numeric_limits<int>::max();

  /**
   * Returns the next start position for the current doc.
   * There is always at least one start/end position per doc.
   * After the last start/end position at the current doc this returns {@link
   * #NO_MORE_POSITIONS}.
   */
  virtual int nextStartPosition() = 0;

  /**
   * Returns the start position in the current doc, or -1 when {@link
   * #nextStartPosition} was not yet called on the current doc. After the last
   * start/end position at the current doc this returns {@link
   * #NO_MORE_POSITIONS}.
   */
  virtual int startPosition() = 0;

  /**
   * Returns the end position for the current start position, or -1 when {@link
   * #nextStartPosition} was not yet called on the current doc. After the last
   * start/end position at the current doc this returns {@link
   * #NO_MORE_POSITIONS}.
   */
  virtual int endPosition() = 0;

  /**
   * Return the width of the match, which is typically used to compute
   * the {@link SimScorer#computeSlopFactor(int) slop factor}. It is only legal
   * to call this method when the iterator is on a valid doc ID and positioned.
   * The return value must be positive, and lower values means that the match is
   * better.
   */
  virtual int width() = 0;

  /**
   * Collect postings data from the leaves of the current Spans.
   *
   * This method should only be called after {@link #nextStartPosition()}, and
   * before
   * {@link #NO_MORE_POSITIONS} has been reached.
   *
   * @param collector a SpanCollector
   *
   * @lucene.experimental
   */
  virtual void collect(std::shared_ptr<SpanCollector> collector) = 0;

  /**
   * Return an estimation of the cost of using the positions of
   * this {@link Spans} for any single document, but only after
   * {@link #asTwoPhaseIterator} returned {@code null}.
   * Otherwise this method should not be called.
   * The returned value is independent of the current document.
   *
   * @lucene.experimental
   */
  virtual float positionsCost() = 0;

  /**
   * Optional method: Return a {@link TwoPhaseIterator} view of this
   * {@link Scorer}. A return value of {@code null} indicates that
   * two-phase iteration is not supported.
   * @see Scorer#twoPhaseIterator()
   */
  virtual std::shared_ptr<TwoPhaseIterator> asTwoPhaseIterator();

  virtual std::wstring toString();

  /**
   * Called before the current doc's frequency is calculated
   */
protected:
  virtual void doStartCurrentDoc() ;

  /**
   * Called each time the scorer's SpanScorer is advanced during frequency
   * calculation
   */
  virtual void doCurrentSpans() ;

protected:
  std::shared_ptr<Spans> shared_from_this()
  {
    return std::static_pointer_cast<Spans>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
