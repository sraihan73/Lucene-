#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReaderContext;
}

namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
}
namespace org::apache::lucene::index
{
class Term;
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

namespace org::apache::lucene::search::intervals
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

/**
 * A helper class for {@link IntervalQuery} that provides an {@link
 * IntervalIterator} for a given field and segment
 *
 * Static constructor functions for various different sources can be found in
 * the
 * {@link Intervals} class
 */
class IntervalsSource : public std::enable_shared_from_this<IntervalsSource>
{
  GET_CLASS_NAME(IntervalsSource)

  /**
   * Create an {@link IntervalIterator} exposing the minimum intervals defined
   * by this {@link IntervalsSource}
   *
   * Returns {@code null} if no intervals for this field exist in this segment
   *
   * @param field the field to read positions from
   * @param ctx   the context for which to return the iterator
   */
public:
  virtual std::shared_ptr<IntervalIterator>
  intervals(const std::wstring &field,
            std::shared_ptr<LeafReaderContext> ctx) = 0;

  /**
   * Expert: collect {@link Term} objects from this source, to be used for
   * top-level term scoring
   * @param field the field to be scored
   * @param terms a {@link Set} which terms should be added to
   */
  virtual void
  extractTerms(const std::wstring &field,
               std::shared_ptr<Set<std::shared_ptr<Term>>> terms) = 0;

  int hashCode() = 0;
  override

      bool
      equals(std::any other) = 0;
  override

      std::wstring
      toString() = 0;
  override
};

} // namespace org::apache::lucene::search::intervals
