#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

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

namespace org::apache::lucene::search::intervals
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

/**
 * A {@link DocIdSetIterator} that also allows iteration over matching
 * intervals in a document.
 *
 * Once the iterator is positioned on a document by calling {@link
 * #advance(int)} or {@link #nextDoc()}, intervals may be retrieved by calling
 * {@link #nextInterval()} until {@link #NO_MORE_INTERVALS} is returned.
 *
 * The limits of the current interval are returned by {@link #start()} and
 * {@link #end()}. When the iterator has been moved to a new document, but
 * before {@link #nextInterval()} has been called, both these methods return
 * {@code -1}.
 *
 * Note that it is possible for a document to return {@link #NO_MORE_INTERVALS}
 * on the first call to {@link #nextInterval()}
 */
class IntervalIterator : public DocIdSetIterator
{
  GET_CLASS_NAME(IntervalIterator)

  /**
   * When returned from {@link #nextInterval()}, indicates that there are no
   * more matching intervals on the current document
   */
public:
  static const int NO_MORE_INTERVALS = std::numeric_limits<int>::max();

  /**
   * The start of the current interval
   *
   * Returns -1 if {@link #nextInterval()} has not yet been called
   */
  virtual int start() = 0;

  /**
   * The end of the current interval
   *
   * Returns -1 if {@link #nextInterval()} has not yet been called
   */
  virtual int end() = 0;

  /**
   * Advance the iterator to the next interval
   *
   * @return the start of the next interval, or {@link
   * IntervalIterator#NO_MORE_INTERVALS} if there are no more intervals on the
   * current document
   */
  virtual int nextInterval() = 0;

  /**
   * An indication of the average cost of iterating over all intervals in a
   * document
   *
   * @see TwoPhaseIterator#matchCost()
   */
  virtual float matchCost() = 0;

protected:
  std::shared_ptr<IntervalIterator> shared_from_this()
  {
    return std::static_pointer_cast<IntervalIterator>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
