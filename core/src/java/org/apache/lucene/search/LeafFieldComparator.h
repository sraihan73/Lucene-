#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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
 * Expert: comparator that gets instantiated on each leaf
 * from a top-level {@link FieldComparator} instance.
 *
 * <p>A leaf comparator must define these functions:</p>
 *
 * <ul>
 *
 *  <li> {@link #setBottom} This method is called by
 *       {@link FieldValueHitQueue} to notify the
 *       FieldComparator of the current weakest ("bottom")
 *       slot.  Note that this slot may not hold the weakest
 *       value according to your comparator, in cases where
 *       your comparator is not the primary one (ie, is only
 *       used to break ties from the comparators before it).
 *
 *  <li> {@link #compareBottom} Compare a new hit (docID)
 *       against the "weakest" (bottom) entry in the queue.
 *
 *  <li> {@link #compareTop} Compare a new hit (docID)
 *       against the top value previously set by a call to
 *       {@link FieldComparator#setTopValue}.
 *
 *  <li> {@link #copy} Installs a new hit into the
 *       priority queue.  The {@link FieldValueHitQueue}
 *       calls this method when a new hit is competitive.
 *
 * </ul>
 *
 * @see FieldComparator
 * @lucene.experimental
 */
class LeafFieldComparator
{
  GET_CLASS_NAME(LeafFieldComparator)

  /**
   * Set the bottom slot, ie the "weakest" (sorted last)
   * entry in the queue.  When {@link #compareBottom} is
   * called, you should compare against this slot.  This
   * will always be called before {@link #compareBottom}.
   *
   * @param slot the currently weakest (sorted last) slot in the queue
   */
public:
  virtual void setBottom(int const slot) = 0;

  /**
   * Compare the bottom of the queue with this doc.  This will
   * only invoked after setBottom has been called.  This
   * should return the same result as {@link
   * FieldComparator#compare(int,int)}} as if bottom were slot1 and the new
   * document were slot 2.
   *
   * <p>For a search that hits many results, this method
   * will be the hotspot (invoked by far the most
   * frequently).</p>
   *
   * @param doc that was hit
   * @return any {@code N < 0} if the doc's value is sorted after
   * the bottom entry (not competitive), any {@code N > 0} if the
   * doc's value is sorted before the bottom entry and {@code 0} if
   * they are equal.
   */
  virtual int compareBottom(int doc) = 0;

  /**
   * Compare the top value with this doc.  This will
   * only invoked after setTopValue has been called.  This
   * should return the same result as {@link
   * FieldComparator#compare(int,int)}} as if topValue were slot1 and the new
   * document were slot 2.  This is only called for searches that
   * use searchAfter (deep paging).
   *
   * @param doc that was hit
   * @return any {@code N < 0} if the doc's value is sorted after
   * the top entry (not competitive), any {@code N > 0} if the
   * doc's value is sorted before the top entry and {@code 0} if
   * they are equal.
   */
  virtual int compareTop(int doc) = 0;

  /**
   * This method is called when a new hit is competitive.
   * You should copy any state associated with this document
   * that will be required for future comparisons, into the
   * specified slot.
   *
   * @param slot which slot to copy the hit to
   * @param doc docID relative to current reader
   */
  virtual void copy(int slot, int doc) = 0;

  /** Sets the Scorer to use in case a document's score is
   *  needed.
   *
   * @param scorer Scorer instance that you should use to
   * obtain the current hit's score, if necessary. */
  virtual void setScorer(std::shared_ptr<Scorer> scorer) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/
