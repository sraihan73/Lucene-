#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include "core/src/java/org/apache/lucene/search/Weight.h"

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
 * Expert: Common scoring functionality for different types of queries.
 *
 * <p>
 * A <code>Scorer</code> exposes an {@link #iterator()} over documents
 * matching a query in increasing order of doc Id.
 * </p>
 * <p>
 * Document scores are computed using a given <code>Similarity</code>
 * implementation.
 * </p>
 *
 * <p><b>NOTE</b>: The values Float.Nan,
 * Float.NEGATIVE_INFINITY and Float.POSITIVE_INFINITY are
 * not valid scores.  Certain collectors (eg {@link
 * TopScoreDocCollector}) will not properly collect hits
 * with these scores.
 */
class Scorer : public std::enable_shared_from_this<Scorer>
{
  GET_CLASS_NAME(Scorer)
  /** A child Scorer and its relationship to its parent.
   * the meaning of the relationship depends upon the parent query.
   * @lucene.experimental */
public:
  class ChildScorer : public std::enable_shared_from_this<ChildScorer>
  {
    GET_CLASS_NAME(ChildScorer)
    /**
     * Child Scorer. (note this is typically a direct child, and may
     * itself also have children).
     */
  public:
    const std::shared_ptr<Scorer> child;
    /**
     * An arbitrary string relating this scorer to the parent.
     */
    const std::wstring relationship;

    /**
     * Creates a new ChildScorer node with the specified relationship.
     * <p>
     * The relationship can be any be any string that makes sense to
     * the parent Scorer.
     */
    ChildScorer(std::shared_ptr<Scorer> child,
                const std::wstring &relationship);
  };

  /** the Scorer's parent Weight. in some cases this may be null */
  // TODO can we clean this up?
protected:
  const std::shared_ptr<Weight> weight;

  /**
   * Constructs a Scorer
   * @param weight The scorers <code>Weight</code>.
   */
  Scorer(std::shared_ptr<Weight> weight);

  /**
   * Returns the doc ID that is currently being scored.
   * This will return {@code -1} if the {@link #iterator()} is not positioned
   * or {@link DocIdSetIterator#NO_MORE_DOCS} if it has been entirely consumed.
   * @see DocIdSetIterator#docID()
   */
public:
  virtual int docID() = 0;

  /** Returns the score of the current document matching the query.
   * Initially invalid, until {@link DocIdSetIterator#nextDoc()} or
   * {@link DocIdSetIterator#advance(int)} is called on the {@link #iterator()}
   * the first time, or when called from within {@link LeafCollector#collect}.
   */
  virtual float score() = 0;

  /** returns parent Weight
   * @lucene.experimental
   */
  virtual std::shared_ptr<Weight> getWeight();

  /**
   * Returns child sub-scorers positioned on the current document
   *
   * Note that this method should not be called on Scorers passed to {@link
   * LeafCollector#setScorer(Scorer)}, as these may be synthetic Scorers
   * produced by {@link BulkScorer} which will throw an Exception.
   *
   * @lucene.experimental
   */
  virtual std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren() ;

  /**
   * Return a {@link DocIdSetIterator} over matching documents.
   *
   * The returned iterator will either be positioned on {@code -1} if no
   * documents have been scored yet, {@link DocIdSetIterator#NO_MORE_DOCS}
   * if all documents have been scored already, or the last document id that
   * has been scored otherwise.
   *
   * The returned iterator is a view: calling this method several times will
   * return iterators that have the same state.
   */
public:
  virtual std::shared_ptr<DocIdSetIterator> iterator() = 0;

  /**
   * Optional method: Return a {@link TwoPhaseIterator} view of this
   * {@link Scorer}. A return value of {@code null} indicates that
   * two-phase iteration is not supported.
   *
   * Note that the returned {@link TwoPhaseIterator}'s
   * {@link TwoPhaseIterator#approximation() approximation} must
   * advance synchronously with the {@link #iterator()}: advancing the
   * approximation must advance the iterator and vice-versa.
   *
   * Implementing this method is typically useful on {@link Scorer}s
   * that have a high per-document overhead in order to confirm matches.
   *
   * The default implementation returns {@code null}.
   */
  virtual std::shared_ptr<TwoPhaseIterator> twoPhaseIterator();
};

} // #include  "core/src/java/org/apache/lucene/search/
