#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <deque>
#include "core/src/java/org/apache/lucene/util/BytesRef.h"
#include "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "core/src/java/org/apache/lucene/search/Scorer.h"
#include "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include "core/src/java/org/apache/lucene/search/LeafFieldComparator.h"

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

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Expert: a FieldComparator compares hits so as to determine their
 * sort order when collecting the top results with {@link
 * TopFieldCollector}.  The concrete public FieldComparator
 * classes here correspond to the SortField types.
 *
 * <p>The document IDs passed to these methods must only
 * move forwards, since they are using doc values iterators
 * to retrieve sort values.</p>
 *
 * <p>This API is designed to achieve high performance
 * sorting, by exposing a tight interaction with {@link
 * FieldValueHitQueue} as it visits hits.  Whenever a hit is
 * competitive, it's enrolled into a virtual slot, which is
 * an int ranging from 0 to numHits-1. Segment transitions are
 * handled by creating a dedicated per-segment
 * {@link LeafFieldComparator} which also needs to interact
 * with the {@link FieldValueHitQueue} but can optimize based
 * on the segment to collect.</p>
 *
 * <p>The following functions need to be implemented</p>
 * <ul>
 *  <li> {@link #compare} Compare a hit at 'slot a'
 *       with hit 'slot b'.
 *
 *  <li> {@link #setTopValue} This method is called by
 *       {@link TopFieldCollector} to notify the
 *       FieldComparator of the top most value, which is
 *       used by future calls to
 *       {@link LeafFieldComparator#compareTop}.
 *
 *  <li> {@link #getLeafComparator(org.apache.lucene.index.LeafReaderContext)}
 * Invoked when the search is switching to the next segment. You may need to
 * update internal state of the comparator, for example retrieving new values
 * from DocValues.
 *
 *  <li> {@link #value} Return the sort value stored in
 *       the specified slot.  This is only called at the end
 *       of the search, in order to populate {@link
 *       FieldDoc#fields} when returning the top results.
 * </ul>
 *
 * @see LeafFieldComparator
 * @lucene.experimental
 */
template <class T>
class FieldComparator : public std::enable_shared_from_this<FieldComparator<T>>
{
  GET_CLASS_NAME(FieldComparator)

  /**
   * Compare hit at slot1 with hit at slot2.
   *
   * @param slot1 first slot to compare
   * @param slot2 second slot to compare
   * @return any {@code N < 0} if slot2's value is sorted after
   * slot1, any {@code N > 0} if the slot2's value is sorted before
   * slot1 and {@code 0} if they are equal
   */
public:
  virtual int compare(int slot1, int slot2) = 0;

  /**
   * Record the top value, for future calls to {@link
   * LeafFieldComparator#compareTop}.  This is only called for searches that
   * use searchAfter (deep paging), and is called before any
   * calls to {@link #getLeafComparator(LeafReaderContext)}.
   */
  virtual void setTopValue(T value) = 0;

  /**
   * Return the actual value in the slot.
   *
   * @param slot the value
   * @return value in this slot
   */
  virtual T value(int slot) = 0;

  /**
   * Get a per-segment {@link LeafFieldComparator} to collect the given
   * {@link org.apache.lucene.index.LeafReaderContext}. All docIDs supplied to
   * this {@link LeafFieldComparator} are relative to the current reader (you
   * must add docBase if you need to map_obj it to a top-level docID).
   *
   * @param context current reader context
   * @return the comparator to use for this segment
   * @throws IOException if there is a low-level IO error
   */
  virtual std::shared_ptr<LeafFieldComparator>
  getLeafComparator(std::shared_ptr<LeafReaderContext> context) = 0;

  /** Returns a negative integer if first is less than second,
   *  0 if they are equal and a positive integer otherwise. Default
   *  impl to assume the type implements Comparable and
   *  invoke .compareTo; be sure to override this method if
   *  your FieldComparator's type isn't a Comparable or
   *  if your values may sometimes be null */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public int compareValues(T
  // first, T second)
  virtual int compareValues(T first, T second)
  {
    if (first == nullptr) {
      if (second == nullptr) {
        return 0;
      } else {
        return -1;
      }
    } else if (second == nullptr) {
      return 1;
    } else {
      return (first == second ? 0 : first < second ? -1 : 1);
    }
  }

};

} // #include  "core/src/java/org/apache/lucene/search/
