#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <unordered_set>
#include "core/src/java/org/apache/lucene/search/SegmentCacheable.h"
#include "core/src/java/org/apache/lucene/search/Query.h"
#include "core/src/java/org/apache/lucene/search/Matches.h"
#include "core/src/java/org/apache/lucene/search/Explanation.h"
#include "core/src/java/org/apache/lucene/search/Scorer.h"
#include "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
#include "core/src/java/org/apache/lucene/search/BulkScorer.h"
#include "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include "core/src/java/org/apache/lucene/util/Bits.h"
#include "core/src/java/org/apache/lucene/index/Term.h"
#include "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Bits = org::apache::lucene::util::Bits;

/**
 * Expert: Calculate query weights and build query scorers.
 * <p>
 * The purpose of {@link Weight} is to ensure searching does not modify a
 * {@link Query}, so that a {@link Query} instance can be reused. <br>
 * {@link IndexSearcher} dependent state of the query should reside in the
 * {@link Weight}. <br>
 * {@link org.apache.lucene.index.LeafReader} dependent state should reside in
 * the {@link Scorer}. <p> Since {@link Weight} creates {@link Scorer} instances
 * for a given
 * {@link org.apache.lucene.index.LeafReaderContext} ({@link
 * #scorer(org.apache.lucene.index.LeafReaderContext)}) callers must maintain
 * the relationship between the searcher's top-level
 * {@link IndexReaderContext} and the context used to create a {@link Scorer}.
 * <p>
 * A <code>Weight</code> is used in the following way:
 * <ol>
 * <li>A <code>Weight</code> is constructed by a top-level query, given a
 * <code>IndexSearcher</code> ({@link Query#createWeight(IndexSearcher, bool,
 * float)}). <li>A <code>Scorer</code> is constructed by
 * {@link #scorer(org.apache.lucene.index.LeafReaderContext)}.
 * </ol>
 *
 * @since 2.9
 */
class Weight : public std::enable_shared_from_this<Weight>,
               public SegmentCacheable
{
  GET_CLASS_NAME(Weight)

protected:
  const std::shared_ptr<Query> parentQuery;

  /** Sole constructor, typically invoked by sub-classes.
   * @param query         the parent query
   */
  Weight(std::shared_ptr<Query> query);

  /**
   * Expert: adds all terms occurring in this query to the terms set. If the
   * {@link Weight} was created with {@code needsScores == true} then this
   * method will only extract terms which are used for scoring, otherwise it
   * will extract all terms which are used for matching.
   */
public:
  virtual void
  extractTerms(std::shared_ptr<std::unordered_set<std::shared_ptr<Term>>> terms) = 0;

  /**
   * Returns {@link Matches} for a specific document, or {@code null} if the
   * document does not match the parent query
   *
   * A query match that contains no position information (for example, a Point
   * or DocValues query) will return {@link Matches#MATCH_WITH_NO_TERMS}
   *
   * @param context the reader's context to create the {@link Matches} for
   * @param doc     the document's id relative to the given context's reader
   * @lucene.experimental
   */
  virtual std::shared_ptr<Matches>
  matches(std::shared_ptr<LeafReaderContext> context,
          int doc) ;

  /**
   * An explanation of the score computation for the named document.
   *
   * @param context the readers context to create the {@link Explanation} for.
   * @param doc the document's id relative to the given context's reader
   * @return an Explanation for the score
   * @throws IOException if an {@link IOException} occurs
   */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context, int doc) = 0;

  /** The query that this concerns. */
  std::shared_ptr<Query> getQuery();

  /**
   * Returns a {@link Scorer} which can iterate in order over all matching
   * documents and assign them a score.
   * <p>
   * <b>NOTE:</b> null can be returned if no documents will be scored by this
   * query.
   * <p>
   * <b>NOTE</b>: The returned {@link Scorer} does not have
   * {@link LeafReader#getLiveDocs()} applied, they need to be checked on top.
   *
   * @param context
   *          the {@link org.apache.lucene.index.LeafReaderContext} for which to
   * return the {@link Scorer}.
   *
   * @return a {@link Scorer} which scores documents in/out-of order.
   * @throws IOException if there is a low-level I/O error
   */
  virtual std::shared_ptr<Scorer>
  scorer(std::shared_ptr<LeafReaderContext> context) = 0;

  /**
   * Optional method.
   * Get a {@link ScorerSupplier}, which allows to know the cost of the {@link
   * Scorer} before building it. The default implementation calls {@link
   * #scorer} and builds a {@link ScorerSupplier} wrapper around it.
   * @see #scorer
   */
  virtual std::shared_ptr<ScorerSupplier>
  scorerSupplier(std::shared_ptr<LeafReaderContext> context) ;

private:
  class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
  {
    GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
  private:
    std::shared_ptr<Weight> outerInstance;

    std::shared_ptr<org::apache::lucene::search::Scorer> scorer;

  public:
    ScorerSupplierAnonymousInnerClass(
        std::shared_ptr<Weight> outerInstance,
        std::shared_ptr<org::apache::lucene::search::Scorer> scorer);

    std::shared_ptr<Scorer> get(int64_t leadCost) override;

    int64_t cost() override;

  protected:
    std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
          ScorerSupplier::shared_from_this());
    }
  };

  /**
   * Optional method, to return a {@link BulkScorer} to
   * score the query and send hits to a {@link Collector}.
   * Only queries that have a different top-level approach
   * need to override this; the default implementation
   * pulls a normal {@link Scorer} and iterates and
   * collects the resulting hits which are not marked as deleted.
   *
   * @param context
   *          the {@link org.apache.lucene.index.LeafReaderContext} for which to
   * return the {@link Scorer}.
   *
   * @return a {@link BulkScorer} which scores documents and
   * passes them to a collector.
   * @throws IOException if there is a low-level I/O error
   */
public:
  virtual std::shared_ptr<BulkScorer>
  bulkScorer(std::shared_ptr<LeafReaderContext> context) ;

  /** Just wraps a Scorer and performs top scoring using it.
   *  @lucene.internal */
protected:
  class DefaultBulkScorer : public BulkScorer
  {
    GET_CLASS_NAME(DefaultBulkScorer)
  private:
    const std::shared_ptr<Scorer> scorer;
    const std::shared_ptr<DocIdSetIterator> iterator;
    const std::shared_ptr<TwoPhaseIterator> twoPhase;

    /** Sole constructor. */
  public:
    DefaultBulkScorer(std::shared_ptr<Scorer> scorer);

    int64_t cost() override;

    int score(std::shared_ptr<LeafCollector> collector,
              std::shared_ptr<Bits> acceptDocs, int min,
              int max)  override;

    /** Specialized method to bulk-score a range of hits; we
     *  separate this from {@link #scoreAll} to help out
     *  hotspot.
     *  See <a
     * href="https://issues.apache.org/jira/browse/LUCENE-5487">LUCENE-5487</a>
     */
    static int scoreRange(std::shared_ptr<LeafCollector> collector,
                          std::shared_ptr<DocIdSetIterator> iterator,
                          std::shared_ptr<TwoPhaseIterator> twoPhase,
                          std::shared_ptr<Bits> acceptDocs, int currentDoc,
                          int end) ;

    /** Specialized method to bulk-score all hits; we
     *  separate this from {@link #scoreRange} to help out
     *  hotspot.
     *  See <a
     * href="https://issues.apache.org/jira/browse/LUCENE-5487">LUCENE-5487</a>
     */
    static void scoreAll(std::shared_ptr<LeafCollector> collector,
                         std::shared_ptr<DocIdSetIterator> iterator,
                         std::shared_ptr<TwoPhaseIterator> twoPhase,
                         std::shared_ptr<Bits> acceptDocs) ;

  protected:
    std::shared_ptr<DefaultBulkScorer> shared_from_this()
    {
      return std::static_pointer_cast<DefaultBulkScorer>(
          BulkScorer::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
