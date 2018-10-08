#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::search::intervals
{
class IntervalsSource;
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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Constructor functions for {@link IntervalsSource} types
 *
 * These sources implement minimum-interval algorithms taken from the paper
 * <a
 * href="http://vigna.di.unimi.it/ftp/papers/EfficientAlgorithmsMinimalIntervalSemantics.pdf">
 * Efficient Optimally Lazy Algorithms for Minimal-Interval Semantics</a>
 */
class Intervals final : public std::enable_shared_from_this<Intervals>
{
  GET_CLASS_NAME(Intervals)

private:
  Intervals();

  /**
   * Return an {@link IntervalsSource} exposing intervals for a term
   */
public:
  static std::shared_ptr<IntervalsSource> term(std::shared_ptr<BytesRef> term);

  /**
   * Return an {@link IntervalsSource} exposing intervals for a term
   */
  static std::shared_ptr<IntervalsSource> term(const std::wstring &term);

  /**
   * Return an {@link IntervalsSource} exposing intervals for a phrase
   * consisting of a deque of terms
   */
  static std::shared_ptr<IntervalsSource>
  phrase(std::deque<std::wstring> &terms);

  /**
   * Return an {@link IntervalsSource} exposing intervals for a phrase
   * consisting of a deque of IntervalsSources
   */
  static std::shared_ptr<IntervalsSource>
  phrase(std::deque<IntervalsSource> &subSources);

  /**
   * Return an {@link IntervalsSource} over the disjunction of a set of
   * sub-sources
   */
  static std::shared_ptr<IntervalsSource> or
      (std::deque<IntervalsSource> & subSources);

  /**
   * Create an {@link IntervalsSource} that filters a sub-source by the width of
   * its intervals
   * @param width       the maximum width of intervals in the sub-source ot
   * return
   * @param subSource   the sub-source to filter
   */
  static std::shared_ptr<IntervalsSource>
  maxwidth(int width, std::shared_ptr<IntervalsSource> subSource);

  /**
   * Create an ordered {@link IntervalsSource}
   *
   * Returns intervals in which the subsources all appear in the given order
   *
   * @param subSources  an ordered set of {@link IntervalsSource} objects
   */
  static std::shared_ptr<IntervalsSource>
  ordered(std::deque<IntervalsSource> &subSources);

  /**
   * Create an unordered {@link IntervalsSource}
   *
   * Returns intervals in which all the subsources appear.  The subsources may
   * overlap
   *
   * @param subSources  an unordered set of {@link IntervalsSource}s
   */
  static std::shared_ptr<IntervalsSource>
  unordered(std::deque<IntervalsSource> &subSources);

  /**
   * Create an unordered {@link IntervalsSource}
   *
   * Returns intervals in which all the subsources appear.
   *
   * @param subSources  an unordered set of {@link IntervalsSource}s
   * @param allowOverlaps whether or not the sources should be allowed to
   * overlap in a hit
   */
  static std::shared_ptr<IntervalsSource>
  unordered(bool allowOverlaps, std::deque<IntervalsSource> &subSources);

  /**
   * Create a non-overlapping IntervalsSource
   *
   * Returns intervals of the minuend that do not overlap with intervals from
   the subtrahend

   * @param minuend     the {@link IntervalsSource} to filter
   * @param subtrahend  the {@link IntervalsSource} to filter by
   */
  static std::shared_ptr<IntervalsSource>
  nonOverlapping(std::shared_ptr<IntervalsSource> minuend,
                 std::shared_ptr<IntervalsSource> subtrahend);

  /**
   * Create a not-within {@link IntervalsSource}
   *
   * Returns intervals of the minuend that do not appear within a set number of
   * positions of intervals from the subtrahend query
   *
   * @param minuend     the {@link IntervalsSource} to filter
   * @param positions   the maximum distance that intervals from the minuend may
   * occur from intervals of the subtrahend
   * @param subtrahend  the {@link IntervalsSource} to filter by
   */
  static std::shared_ptr<IntervalsSource>
  notWithin(std::shared_ptr<IntervalsSource> minuend, int positions,
            std::shared_ptr<IntervalsSource> subtrahend);

  /**
   * Create a not-containing {@link IntervalsSource}
   *
   * Returns intervals from the minuend that do not contain intervals of the
   * subtrahend
   *
   * @param minuend     the {@link IntervalsSource} to filter
   * @param subtrahend  the {@link IntervalsSource} to filter by
   */
  static std::shared_ptr<IntervalsSource>
  notContaining(std::shared_ptr<IntervalsSource> minuend,
                std::shared_ptr<IntervalsSource> subtrahend);

  /**
   * Create a containing {@link IntervalsSource}
   *
   * Returns intervals from the big source that contain one or more intervals
   * from the small source
   *
   * @param big     the {@link IntervalsSource} to filter
   * @param small   the {@link IntervalsSource} to filter by
   */
  static std::shared_ptr<IntervalsSource>
  containing(std::shared_ptr<IntervalsSource> big,
             std::shared_ptr<IntervalsSource> small);

  /**
   * Create a not-contained-by {@link IntervalsSource}
   *
   * Returns intervals from the small {@link IntervalsSource} that do not appear
   * within intervals from the big {@link IntervalsSource}.
   *
   * @param small   the {@link IntervalsSource} to filter
   * @param big     the {@link IntervalsSource} to filter by
   */
  static std::shared_ptr<IntervalsSource>
  notContainedBy(std::shared_ptr<IntervalsSource> small,
                 std::shared_ptr<IntervalsSource> big);

  /**
   * Create a contained-by {@link IntervalsSource}
   *
   * Returns intervals from the small query that appear within intervals of the
   * big query
   *
   * @param small     the {@link IntervalsSource} to filter
   * @param big       the {@link IntervalsSource} to filter by
   */
  static std::shared_ptr<IntervalsSource>
  containedBy(std::shared_ptr<IntervalsSource> small,
              std::shared_ptr<IntervalsSource> big);

  // TODO: beforeQuery, afterQuery, arbitrary IntervalFunctions
};

} // namespace org::apache::lucene::search::intervals
