#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class Similarity;
}

namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search::similarities
{
class SimWeight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class QueryCache;
}
namespace org::apache::lucene::search
{
class QueryCachingPolicy;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class IndexReaderContext;
}
namespace org::apache::lucene::search
{
class LeafSlice;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class TotalHitCountCollector;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search
{
class TopScoreDocCollector;
}
namespace org::apache::lucene::search
{
class Collector;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
}
namespace org::apache::lucene::search
{
class FieldDoc;
}
namespace org::apache::lucene::search
{
class TopFieldCollector;
}
namespace org::apache::lucene::search
{
template <typename Ctypename T>
class CollectorManager;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class TermContext;
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

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/** Implements search over a single IndexReader.
 *
 * <p>Applications usually need only call the inherited
 * {@link #search(Query,int)} method. For
 * performance reasons, if your index is unchanging, you
 * should share a single IndexSearcher instance across
 * multiple searches instead of creating a new one
 * per-search.  If your index has changed and you wish to
 * see the changes reflected in searching, you should
 * use {@link DirectoryReader#openIfChanged(DirectoryReader)}
 * to obtain a new reader and
 * then create a new IndexSearcher from that.  Also, for
 * low-latency turnaround it's best to use a near-real-time
 * reader ({@link DirectoryReader#open(IndexWriter)}).
 * Once you have a new {@link IndexReader}, it's relatively
 * cheap to create a new IndexSearcher from it.
 *
 * <a name="thread-safety"></a><p><b>NOTE</b>: <code>{@link
 * IndexSearcher}</code> instances are completely
 * thread safe, meaning multiple threads can call any of its
 * methods, concurrently.  If your application requires
 * external synchronization, you should <b>not</b>
 * synchronize on the <code>IndexSearcher</code> instance;
 * use your own (non-Lucene) objects instead.</p>
 */
class IndexSearcher : public std::enable_shared_from_this<IndexSearcher>
{
  GET_CLASS_NAME(IndexSearcher)

  /** A search-time {@link Similarity} that does not make use of scoring factors
   *  and may be used when scores are not needed. */
private:
  static const std::shared_ptr<Similarity> NON_SCORING_SIMILARITY;

private:
  class SimilarityAnonymousInnerClass : public Similarity
  {
    GET_CLASS_NAME(SimilarityAnonymousInnerClass)
  public:
    SimilarityAnonymousInnerClass();

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

  private:
    class SimWeightAnonymousInnerClass : public SimWeight
    {
      GET_CLASS_NAME(SimWeightAnonymousInnerClass)
    private:
      std::shared_ptr<SimilarityAnonymousInnerClass> outerInstance;

    public:
      SimWeightAnonymousInnerClass(
          std::shared_ptr<SimilarityAnonymousInnerClass> outerInstance);

    protected:
      std::shared_ptr<SimWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimWeightAnonymousInnerClass>(
            SimWeight::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<SimilarityAnonymousInnerClass> outerInstance;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<SimilarityAnonymousInnerClass> outerInstance);

      float score(int doc, float freq) override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            SimScorer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SimilarityAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimilarityAnonymousInnerClass>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

private:
  static std::shared_ptr<QueryCache> DEFAULT_QUERY_CACHE;
  static std::shared_ptr<QueryCachingPolicy> DEFAULT_CACHING_POLICY;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static IndexSearcher::StaticConstructor staticConstructor;

public:
  const std::shared_ptr<IndexReader> reader; // package private for testing!

  // NOTE: these members might change in incompatible ways
  // in the next release
protected:
  const std::shared_ptr<IndexReaderContext> readerContext;
  const std::deque<std::shared_ptr<LeafReaderContext>> leafContexts;
  /** used with executor - each slice holds a set of leafs executed within one
   * thread */
  std::deque<std::shared_ptr<LeafSlice>> const leafSlices;

  // These are only used for multi-threaded search
private:
  const std::shared_ptr<ExecutorService> executor;

  // the default Similarity
  static const std::shared_ptr<Similarity> defaultSimilarity;

  std::shared_ptr<QueryCache> queryCache = DEFAULT_QUERY_CACHE;
  std::shared_ptr<QueryCachingPolicy> queryCachingPolicy =
      DEFAULT_CACHING_POLICY;

  /**
   * Expert: returns a default Similarity instance.
   * In general, this method is only called to initialize searchers and writers.
   * User code and query implementations should respect
   * {@link IndexSearcher#getSimilarity(bool)}.
   * @lucene.internal
   */
public:
  static std::shared_ptr<Similarity> getDefaultSimilarity();

  /**
   * Expert: Get the default {@link QueryCache} or {@code null} if the cache is
   * disabled.
   * @lucene.internal
   */
  static std::shared_ptr<QueryCache> getDefaultQueryCache();

  /**
   * Expert: set the default {@link QueryCache} instance.
   * @lucene.internal
   */
  static void
  setDefaultQueryCache(std::shared_ptr<QueryCache> defaultQueryCache);

  /**
   * Expert: Get the default {@link QueryCachingPolicy}.
   * @lucene.internal
   */
  static std::shared_ptr<QueryCachingPolicy> getDefaultQueryCachingPolicy();

  /**
   * Expert: set the default {@link QueryCachingPolicy} instance.
   * @lucene.internal
   */
  static void setDefaultQueryCachingPolicy(
      std::shared_ptr<QueryCachingPolicy> defaultQueryCachingPolicy);

  /** The Similarity implementation used by this searcher. */
private:
  std::shared_ptr<Similarity> similarity = defaultSimilarity;

  /** Creates a searcher searching the provided index. */
public:
  IndexSearcher(std::shared_ptr<IndexReader> r);

  /** Runs searches for each segment separately, using the
   *  provided ExecutorService.  IndexSearcher will not
   *  close/awaitTermination this ExecutorService on
   *  close; you must do so, eventually, on your own.  NOTE:
   *  if you are using {@link NIOFSDirectory}, do not use
   *  the shutdownNow method of ExecutorService as this uses
   *  Thread.interrupt under-the-hood which can silently
   *  close file descriptors (see <a
   *  href="https://issues.apache.org/jira/browse/LUCENE-2239">LUCENE-2239</a>).
   *
   * @lucene.experimental */
  IndexSearcher(std::shared_ptr<IndexReader> r,
                std::shared_ptr<ExecutorService> executor);

  /**
   * Creates a searcher searching the provided top-level {@link
   * IndexReaderContext}. <p> Given a non-<code>null</code> {@link
   * ExecutorService} this method runs searches for each segment separately,
   * using the provided ExecutorService. IndexSearcher will not
   * close/awaitTermination this ExecutorService on close; you must do so,
   * eventually, on your own. NOTE: if you are using
   * {@link NIOFSDirectory}, do not use the shutdownNow method of
   * ExecutorService as this uses Thread.interrupt under-the-hood which can
   * silently close file descriptors (see <a
   * href="https://issues.apache.org/jira/browse/LUCENE-2239">LUCENE-2239</a>).
   *
   * @see IndexReaderContext
   * @see IndexReader#getContext()
   * @lucene.experimental
   */
  IndexSearcher(std::shared_ptr<IndexReaderContext> context,
                std::shared_ptr<ExecutorService> executor);

  /**
   * Creates a searcher searching the provided top-level {@link
   * IndexReaderContext}.
   *
   * @see IndexReaderContext
   * @see IndexReader#getContext()
   * @lucene.experimental
   */
  IndexSearcher(std::shared_ptr<IndexReaderContext> context);

  /**
   * Set the {@link QueryCache} to use when scores are not needed.
   * A value of {@code null} indicates that query matches should never be
   * cached. This method should be called <b>before</b> starting using this
   * {@link IndexSearcher}.
   * <p>NOTE: When using a query cache, queries should not be modified after
   * they have been passed to IndexSearcher.
   * @see QueryCache
   * @lucene.experimental
   */
  virtual void setQueryCache(std::shared_ptr<QueryCache> queryCache);

  /**
   * Return the query cache of this {@link IndexSearcher}. This will be either
   * the {@link #getDefaultQueryCache() default query cache} or the query cache
   * that was last set through {@link #setQueryCache(QueryCache)}. A return
   * value of {@code null} indicates that caching is disabled.
   * @lucene.experimental
   */
  virtual std::shared_ptr<QueryCache> getQueryCache();

  /**
   * Set the {@link QueryCachingPolicy} to use for query caching.
   * This method should be called <b>before</b> starting using this
   * {@link IndexSearcher}.
   * @see QueryCachingPolicy
   * @lucene.experimental
   */
  virtual void
  setQueryCachingPolicy(std::shared_ptr<QueryCachingPolicy> queryCachingPolicy);

  /**
   * Return the query cache of this {@link IndexSearcher}. This will be either
   * the {@link #getDefaultQueryCachingPolicy() default policy} or the policy
   * that was last set through {@link
   * #setQueryCachingPolicy(QueryCachingPolicy)}.
   * @lucene.experimental
   */
  virtual std::shared_ptr<QueryCachingPolicy> getQueryCachingPolicy();

  /**
   * Expert: Creates an array of leaf slices each holding a subset of the given
   * leaves. Each {@link LeafSlice} is executed in a single thread. By default
   * there will be one {@link LeafSlice} per leaf ({@link
   * org.apache.lucene.index.LeafReaderContext}).
   */
protected:
  virtual std::deque<std::shared_ptr<LeafSlice>>
  slices(std::deque<std::shared_ptr<LeafReaderContext>> &leaves);

  /** Return the {@link IndexReader} this searches. */
public:
  virtual std::shared_ptr<IndexReader> getIndexReader();

  /**
   * Sugar for <code>.getIndexReader().document(docID)</code>
   * @see IndexReader#document(int)
   */
  virtual std::shared_ptr<Document> doc(int docID) ;

  /**
   * Sugar for <code>.getIndexReader().document(docID, fieldVisitor)</code>
   * @see IndexReader#document(int, StoredFieldVisitor)
   */
  virtual void
  doc(int docID,
      std::shared_ptr<StoredFieldVisitor> fieldVisitor) ;

  /**
   * Sugar for <code>.getIndexReader().document(docID, fieldsToLoad)</code>
   * @see IndexReader#document(int, Set)
   */
  virtual std::shared_ptr<Document>
  doc(int docID,
      std::shared_ptr<Set<std::wstring>> fieldsToLoad) ;

  /** Expert: Set the Similarity implementation used by this IndexSearcher.
   *
   */
  virtual void setSimilarity(std::shared_ptr<Similarity> similarity);

  /** Expert: Get the {@link Similarity} to use to compute scores. When
   *  {@code needsScores} is {@code false}, this method will return a simple
   *  {@link Similarity} that does not leverage scoring factors such as norms.
   *  When {@code needsScores} is {@code true}, this returns the
   *  {@link Similarity} that has been set through {@link
   * #setSimilarity(Similarity)} or the {@link #getDefaultSimilarity()} default
   * {@link Similarity} if none has been set explicitly. */
  virtual std::shared_ptr<Similarity> getSimilarity(bool needsScores);

  /**
   * Count how many documents match the given query.
   */
  virtual int count(std::shared_ptr<Query> query) ;

private:
  class CollectorManagerAnonymousInnerClass
      : public std::enable_shared_from_this<
            CollectorManagerAnonymousInnerClass>,
        public CollectorManager<std::shared_ptr<TotalHitCountCollector>, int>
  {
    GET_CLASS_NAME(CollectorManagerAnonymousInnerClass)
  private:
    std::shared_ptr<IndexSearcher> outerInstance;

  public:
    CollectorManagerAnonymousInnerClass(
        std::shared_ptr<IndexSearcher> outerInstance);

    std::shared_ptr<TotalHitCountCollector> newCollector() ;

    std::optional<int>
    reduce(std::shared_ptr<std::deque<std::shared_ptr<TotalHitCountCollector>>>
               collectors) ;
  };

  /** Finds the top <code>n</code>
   * hits for <code>query</code> where all results are after a previous
   * result (<code>after</code>).
   * <p>
   * By passing the bottom result from a previous page as <code>after</code>,
   * this method can be used for efficient 'deep-paging' across potentially
   * large result sets.
   *
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
public:
  virtual std::shared_ptr<TopDocs> searchAfter(std::shared_ptr<ScoreDoc> after,
                                               std::shared_ptr<Query> query,
                                               int numHits) ;

private:
  class CollectorManagerAnonymousInnerClass2
      : public std::enable_shared_from_this<
            CollectorManagerAnonymousInnerClass2>,
        public CollectorManager<std::shared_ptr<TopScoreDocCollector>,
                                std::shared_ptr<TopDocs>>
  {
    GET_CLASS_NAME(CollectorManagerAnonymousInnerClass2)
  private:
    std::shared_ptr<IndexSearcher> outerInstance;

    std::shared_ptr<org::apache::lucene::search::ScoreDoc> after;
    int cappedNumHits = 0;

  public:
    CollectorManagerAnonymousInnerClass2(
        std::shared_ptr<IndexSearcher> outerInstance,
        std::shared_ptr<org::apache::lucene::search::ScoreDoc> after,
        int cappedNumHits);

    std::shared_ptr<TopScoreDocCollector> newCollector() ;

    std::shared_ptr<TopDocs>
    reduce(std::shared_ptr<std::deque<std::shared_ptr<TopScoreDocCollector>>>
               collectors) ;
  };

  /** Finds the top <code>n</code>
   * hits for <code>query</code>.
   *
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
public:
  virtual std::shared_ptr<TopDocs> search(std::shared_ptr<Query> query,
                                          int n) ;

  /** Lower-level search API.
   *
   * <p>{@link LeafCollector#collect(int)} is called for every matching
   * document.
   *
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
  virtual void search(std::shared_ptr<Query> query,
                      std::shared_ptr<Collector> results) ;

  /** Search implementation with arbitrary sorting, plus
   * control over whether hit scores and max score
   * should be computed.  Finds
   * the top <code>n</code> hits for <code>query</code>, and sorting
   * the hits by the criteria in <code>sort</code>.
   * If <code>doDocScores</code> is <code>true</code>
   * then the score of each hit will be computed and
   * returned.  If <code>doMaxScore</code> is
   * <code>true</code> then the maximum score over all
   * collected hits will be computed.
   *
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
  virtual std::shared_ptr<TopFieldDocs>
  search(std::shared_ptr<Query> query, int n, std::shared_ptr<Sort> sort,
         bool doDocScores, bool doMaxScore) ;

  /**
   * Search implementation with arbitrary sorting.
   * @param query The query to search for
   * @param n Return only the top n results
   * @param sort The {@link org.apache.lucene.search.Sort} object
   * @return The top docs, sorted according to the supplied {@link
   * org.apache.lucene.search.Sort} instance
   * @throws IOException if there is a low-level I/O error
   */
  virtual std::shared_ptr<TopFieldDocs>
  search(std::shared_ptr<Query> query, int n,
         std::shared_ptr<Sort> sort) ;

  /** Finds the top <code>n</code>
   * hits for <code>query</code> where all results are after a previous
   * result (<code>after</code>).
   * <p>
   * By passing the bottom result from a previous page as <code>after</code>,
   * this method can be used for efficient 'deep-paging' across potentially
   * large result sets.
   *
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
  virtual std::shared_ptr<TopDocs>
  searchAfter(std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> query,
              int n, std::shared_ptr<Sort> sort) ;

  /** Finds the top <code>n</code>
   * hits for <code>query</code> where all results are after a previous
   * result (<code>after</code>), allowing control over
   * whether hit scores and max score should be computed.
   * <p>
   * By passing the bottom result from a previous page as <code>after</code>,
   * this method can be used for efficient 'deep-paging' across potentially
   * large result sets.  If <code>doDocScores</code> is <code>true</code>
   * then the score of each hit will be computed and
   * returned.  If <code>doMaxScore</code> is
   * <code>true</code> then the maximum score over all
   * collected hits will be computed.
   *
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
  virtual std::shared_ptr<TopFieldDocs>
  searchAfter(std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> query,
              int numHits, std::shared_ptr<Sort> sort, bool doDocScores,
              bool doMaxScore) ;

private:
  std::shared_ptr<TopFieldDocs>
  searchAfter(std::shared_ptr<FieldDoc> after, std::shared_ptr<Query> query,
              int numHits, std::shared_ptr<Sort> sort, bool doDocScores,
              bool doMaxScore) ;

private:
  class CollectorManagerAnonymousInnerClass3
      : public std::enable_shared_from_this<
            CollectorManagerAnonymousInnerClass3>,
        public CollectorManager<std::shared_ptr<TopFieldCollector>,
                                std::shared_ptr<TopFieldDocs>>
  {
    GET_CLASS_NAME(CollectorManagerAnonymousInnerClass3)
  private:
    std::shared_ptr<IndexSearcher> outerInstance;

    std::shared_ptr<org::apache::lucene::search::FieldDoc> after;
    bool doDocScores = false;
    bool doMaxScore = false;
    int cappedNumHits = 0;
    std::shared_ptr<org::apache::lucene::search::Sort> rewrittenSort;

  public:
    CollectorManagerAnonymousInnerClass3(
        std::shared_ptr<IndexSearcher> outerInstance,
        std::shared_ptr<org::apache::lucene::search::FieldDoc> after,
        bool doDocScores, bool doMaxScore, int cappedNumHits,
        std::shared_ptr<org::apache::lucene::search::Sort> rewrittenSort);

    std::shared_ptr<TopFieldCollector> newCollector() ;

    std::shared_ptr<TopFieldDocs>
    reduce(std::shared_ptr<std::deque<std::shared_ptr<TopFieldCollector>>>
               collectors) ;
  };

  /**
   * Lower-level search API.
   * Search all leaves using the given {@link CollectorManager}. In contrast
   * to {@link #search(Query, Collector)}, this method will use the searcher's
   * {@link ExecutorService} in order to parallelize execution of the collection
   * on the configured {@link #leafSlices}.
   * @see CollectorManager
   * @lucene.experimental
   */
public:
  template <typename C, typename T>
  T search(std::shared_ptr<Query> query,
           std::shared_ptr<CollectorManager<C, T>>
               collectorManager) ;

private:
  class CallableAnonymousInnerClass : public Callable<std::shared_ptr<C>>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass)
  private:
    std::shared_ptr<IndexSearcher> outerInstance;

    std::shared_ptr<org::apache::lucene::search::Weight> weight;
    std::deque<std::shared_ptr<LeafReaderContext>> leaves;
    std::shared_ptr<Collector> collector;

  public:
    CallableAnonymousInnerClass(
        std::shared_ptr<IndexSearcher> outerInstance,
        std::shared_ptr<org::apache::lucene::search::Weight> weight,
        std::deque<std::shared_ptr<LeafReaderContext>> &leaves,
        std::shared_ptr<Collector> collector);

    std::shared_ptr<C> call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass>(
          java.util.concurrent.Callable<C>::shared_from_this());
    }
  };

  /**
   * Lower-level search API.
   *
   * <p>
   * {@link LeafCollector#collect(int)} is called for every document. <br>
   *
   * <p>
   * NOTE: this method executes the searches on all given leaves exclusively.
   * To search across all the searchers leaves use {@link #leafContexts}.
   *
   * @param leaves
   *          the searchers leaves to execute the searches on
   * @param weight
   *          to match documents
   * @param collector
   *          to receive hits
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
protected:
  virtual void search(std::deque<std::shared_ptr<LeafReaderContext>> &leaves,
                      std::shared_ptr<Weight> weight,
                      std::shared_ptr<Collector> collector) ;

  /** Expert: called to re-write queries into primitive queries.
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
public:
  virtual std::shared_ptr<Query>
  rewrite(std::shared_ptr<Query> original) ;

  /** Returns an Explanation that describes how <code>doc</code> scored against
   * <code>query</code>.
   *
   * <p>This is intended to be used in developing Similarity implementations,
   * and, for good performance, should not be displayed with every hit.
   * Computing an explanation is as expensive as executing the query over the
   * entire index.
   */
  virtual std::shared_ptr<Explanation> explain(std::shared_ptr<Query> query,
                                               int doc) ;

  /** Expert: low-level implementation method
   * Returns an Explanation that describes how <code>doc</code> scored against
   * <code>weight</code>.
   *
   * <p>This is intended to be used in developing Similarity implementations,
   * and, for good performance, should not be displayed with every hit.
   * Computing an explanation is as expensive as executing the query over the
   * entire index.
   * <p>Applications should call {@link IndexSearcher#explain(Query, int)}.
   * @throws BooleanQuery.TooManyClauses If a query would exceed
   *         {@link BooleanQuery#getMaxClauseCount()} clauses.
   */
protected:
  virtual std::shared_ptr<Explanation> explain(std::shared_ptr<Weight> weight,
                                               int doc) ;

  /**
   * Creates a normalized weight for a top-level {@link Query}.
   * The query is rewritten by this method and {@link Query#createWeight}
   * called, afterwards the {@link Weight} is normalized. The returned {@code
   * Weight} can then directly be used to get a {@link Scorer}.
   * @lucene.internal
   *
   * @deprecated Clients should rewrite the query and then call {@link
   * #createWeight(Query, bool, float)} with a boost value of 1f
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public Weight createNormalizedWeight(Query
  // query, bool needsScores) throws java.io.IOException
  virtual std::shared_ptr<Weight>
  createNormalizedWeight(std::shared_ptr<Query> query,
                         bool needsScores) ;

  /**
   * Creates a {@link Weight} for the given query, potentially adding caching
   * if possible and configured.
   * @lucene.experimental
   */
  virtual std::shared_ptr<Weight> createWeight(std::shared_ptr<Query> query,
                                               bool needsScores,
                                               float boost) ;

  /**
   * Returns this searchers the top-level {@link IndexReaderContext}.
   * @see IndexReader#getContext()
   */
  /* sugar for #getReader().getTopReaderContext() */
  virtual std::shared_ptr<IndexReaderContext> getTopReaderContext();

  /**
   * A class holding a subset of the {@link IndexSearcher}s leaf contexts to be
   * executed within a single thread.
   *
   * @lucene.experimental
   */
public:
  class LeafSlice : public std::enable_shared_from_this<LeafSlice>
  {
    GET_CLASS_NAME(LeafSlice)
  public:
    std::deque<std::shared_ptr<LeafReaderContext>> const leaves;

    LeafSlice(std::deque<LeafReaderContext> &leaves);
  };

public:
  virtual std::wstring toString();

  /**
   * Returns {@link TermStatistics} for a term.
   *
   * This can be overridden for example, to return a term's statistics
   * across a distributed collection.
   * @lucene.experimental
   */
  virtual std::shared_ptr<TermStatistics>
  termStatistics(std::shared_ptr<Term> term,
                 std::shared_ptr<TermContext> context) ;

  /**
   * Returns {@link CollectionStatistics} for a field.
   *
   * This can be overridden for example, to return a field's statistics
   * across a distributed collection.
   * @lucene.experimental
   */
  virtual std::shared_ptr<CollectionStatistics>
  collectionStatistics(const std::wstring &field) ;
};

} // namespace org::apache::lucene::search
