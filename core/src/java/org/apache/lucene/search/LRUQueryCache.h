#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <limits>
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/CacheKey.h"
#include  "core/src/java/org/apache/lucene/search/LeafCache.h"
#include  "core/src/java/org/apache/lucene/index/CacheHelper.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
#include  "core/src/java/org/apache/lucene/search/QueryCachingPolicy.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/search/BulkScorer.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/util/RoaringDocIdSet.h"
#include  "core/src/java/org/apache/lucene/util/Builder.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Matches.h"
#include  "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * A {@link QueryCache} that evicts queries using a LRU (least-recently-used)
 * eviction policy in order to remain under a given maximum size and number of
 * bytes used.
 *
 * This class is thread-safe.
 *
 * Note that query eviction runs in linear time with the total number of
 * segments that have cache entries so this cache works best with
 * {@link QueryCachingPolicy caching policies} that only cache on "large"
 * segments, and it is advised to not share this cache across too many indices.
 *
 * A default query cache and policy instance is used in IndexSearcher. If you
want to replace those defaults
 * it is typically done like this:
 * <pre class="prettyprint">
 *   final int maxNumberOfCachedQueries = 256;
 *   final long maxRamBytesUsed = 50 * 1024L * 1024L; // 50MB
 *   // these cache and policy instances can be shared across several queries
and readers
 *   // it is fine to eg. store them into static variables
 *   final QueryCache queryCache = new LRUQueryCache(maxNumberOfCachedQueries,
maxRamBytesUsed);
 *   final QueryCachingPolicy defaultCachingPolicy = new
UsageTrackingQueryCachingPolicy();
 *   indexSearcher.setQueryCache(queryCache);
 *   indexSearcher.setQueryCachingPolicy(defaultCachingPolicy);
 * </pre>
 *
 * This cache exposes some global statistics ({@link #getHitCount() hit count},
 * {@link #getMissCount() miss count}, {@link #getCacheSize() number of cache
 * entries}, {@link #getCacheCount() total number of DocIdSets that have ever
 * been cached}, {@link #getEvictionCount() number of evicted entries}). In
 * case you would like to have more fine-grained statistics, such as per-index
 * or per-query-class statistics, it is possible to override various callbacks:
 * {@link #onHit}, {@link #onMiss},
GET_CLASS_NAME(statistics,)
 * {@link #onQueryCache}, {@link #onQueryEviction},
 * {@link #onDocIdSetCache}, {@link #onDocIdSetEviction} and {@link #onClear}.
 * It is better to not perform heavy computations in these methods though since
 * they are called synchronously and under a lock.
 *
 * @see QueryCachingPolicy
 * @lucene.experimental
 */
class LRUQueryCache : public std::enable_shared_from_this<LRUQueryCache>,
                      public QueryCache,
                      public Accountable
{
  GET_CLASS_NAME(LRUQueryCache)

  // approximate memory usage that we assign to all queries
  // this maps roughly to a BooleanQuery with a couple term clauses
public:
  static constexpr int64_t QUERY_DEFAULT_RAM_BYTES_USED = 1024;

  static const int64_t HASHTABLE_RAM_BYTES_PER_ENTRY =
      2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF *
      2; // hash tables need to be oversized to avoid collisions, assume 2x
         // capacity

  static const int64_t LINKED_HASHTABLE_RAM_BYTES_PER_ENTRY =
      HASHTABLE_RAM_BYTES_PER_ENTRY +
      2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF; // previous & next references

private:
  const int maxSize;
  const int64_t maxRamBytesUsed;
  const std::function<bool(LeafReaderContext *)> leavesToCache;
  // maps queries that are contained in the cache to a singleton so that this
  // cache does not store several copies of the same query
  const std::unordered_map<std::shared_ptr<Query>, std::shared_ptr<Query>>
      uniqueQueries;
  // The contract between this set and the per-leaf caches is that per-leaf
  // caches are only allowed to store sub-sets of the queries that are contained
  // in mostRecentlyUsedQueries. This is why write operations are performed
  // under a lock
  const std::shared_ptr<Set<std::shared_ptr<Query>>> mostRecentlyUsedQueries;
  const std::unordered_map<std::shared_ptr<IndexReader::CacheKey>,
                           std::shared_ptr<LeafCache>>
      cache;
  const std::shared_ptr<ReentrantLock> lock;

  // these variables are volatile so that we do not need to sync reads
  // but increments need to be performed under the lock
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long ramBytesUsed;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t ramBytesUsed_ = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long hitCount;
  int64_t hitCount = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long missCount;
  int64_t missCount = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long cacheCount;
  int64_t cacheCount = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long cacheSize;
  int64_t cacheSize = 0;

  /**
   * Expert: Create a new instance that will cache at most <code>maxSize</code>
   * queries with at most <code>maxRamBytesUsed</code> bytes of memory, only on
   * leaves that satisfy {@code leavesToCache};
   */
public:
  LRUQueryCache(int maxSize, int64_t maxRamBytesUsed,
                std::function<bool(LeafReaderContext *)> &leavesToCache);

  /**
   * Create a new instance that will cache at most <code>maxSize</code> queries
   * with at most <code>maxRamBytesUsed</code> bytes of memory. Queries will
   * only be cached on leaves that have more than 10k documents and have more
   * than 3% of the total number of documents in the index.
   * This should guarantee that all leaves from the upper
   * {@link TieredMergePolicy tier} will be cached while ensuring that at most
   * <tt>33</tt> leaves can make it to the cache (very likely less than 10 in
   * practice), which is useful for this implementation since some operations
   * perform in linear time with the number of cached leaves.
   */
  LRUQueryCache(int maxSize, int64_t maxRamBytesUsed);

  // pkg-private for testing
public:
  class MinSegmentSizePredicate
      : public std::enable_shared_from_this<MinSegmentSizePredicate>,
        public std::function<bool(LeafReaderContext *)>
  {
    GET_CLASS_NAME(MinSegmentSizePredicate)
  private:
    const int minSize;
    const float minSizeRatio;

  public:
    MinSegmentSizePredicate(int minSize, float minSizeRatio);

    bool test(std::shared_ptr<LeafReaderContext> context) override;
  };

  /**
   * Expert: callback when there is a cache hit on a given query.
   * Implementing this method is typically useful in order to compute more
   * fine-grained statistics about the query cache.
   * @see #onMiss
   * @lucene.experimental
   */
protected:
  virtual void onHit(std::any readerCoreKey, std::shared_ptr<Query> query);

  /**
   * Expert: callback when there is a cache miss on a given query.
   * @see #onHit
   * @lucene.experimental
   */
  virtual void onMiss(std::any readerCoreKey, std::shared_ptr<Query> query);

  /**
   * Expert: callback when a query is added to this cache.
   * Implementing this method is typically useful in order to compute more
   * fine-grained statistics about the query cache.
   * @see #onQueryEviction
   * @lucene.experimental
   */
  virtual void onQueryCache(std::shared_ptr<Query> query,
                            int64_t ramBytesUsed);

  /**
   * Expert: callback when a query is evicted from this cache.
   * @see #onQueryCache
   * @lucene.experimental
   */
  virtual void onQueryEviction(std::shared_ptr<Query> query,
                               int64_t ramBytesUsed);

  /**
   * Expert: callback when a {@link DocIdSet} is added to this cache.
   * Implementing this method is typically useful in order to compute more
   * fine-grained statistics about the query cache.
   * @see #onDocIdSetEviction
   * @lucene.experimental
   */
  virtual void onDocIdSetCache(std::any readerCoreKey, int64_t ramBytesUsed);

  /**
   * Expert: callback when one or more {@link DocIdSet}s are removed from this
   * cache.
   * @see #onDocIdSetCache
   * @lucene.experimental
   */
  virtual void onDocIdSetEviction(std::any readerCoreKey, int numEntries,
                                  int64_t sumRamBytesUsed);

  /**
   * Expert: callback when the cache is completely cleared.
   * @lucene.experimental
   */
  virtual void onClear();

  /** Whether evictions are required. */
public:
  virtual bool requiresEviction();

  virtual std::shared_ptr<DocIdSet>
  get(std::shared_ptr<Query> key, std::shared_ptr<LeafReaderContext> context,
      std::shared_ptr<IndexReader::CacheHelper> cacheHelper);

  virtual void
  putIfAbsent(std::shared_ptr<Query> query,
              std::shared_ptr<LeafReaderContext> context,
              std::shared_ptr<DocIdSet> set,
              std::shared_ptr<IndexReader::CacheHelper> cacheHelper);

  virtual void evictIfNecessary();

  /**
   * Remove all cache entries for the given core cache key.
   */
  virtual void clearCoreCacheKey(std::any coreKey);

  /**
   * Remove all cache entries for the given query.
   */
  virtual void clearQuery(std::shared_ptr<Query> query);

private:
  void onEviction(std::shared_ptr<Query> singleton);

  /**
   * Clear the content of this cache.
   */
public:
  virtual void clear();

  // pkg-private for testing
  virtual void assertConsistent();

  // pkg-private for testing
  // return the deque of cached queries in LRU order
  virtual std::deque<std::shared_ptr<Query>> cachedQueries();

  std::shared_ptr<Weight>
  doCache(std::shared_ptr<Weight> weight,
          std::shared_ptr<QueryCachingPolicy> policy) override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  /**
   * Default cache implementation: uses {@link RoaringDocIdSet} for sets that
   * have a density &lt; 1% and a {@link BitDocIdSet} over a {@link FixedBitSet}
   * otherwise.
   */
protected:
  virtual std::shared_ptr<DocIdSet>
  cacheImpl(std::shared_ptr<BulkScorer> scorer, int maxDoc) ;

private:
  static std::shared_ptr<DocIdSet>
  cacheIntoBitSet(std::shared_ptr<BulkScorer> scorer,
                  int maxDoc) ;

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::BulkScorer> scorer;
    std::shared_ptr<FixedBitSet> bitSet;
    std::deque<int64_t> cost;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::BulkScorer> scorer,
        std::shared_ptr<FixedBitSet> bitSet, std::deque<int64_t> &cost);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

private:
  static std::shared_ptr<DocIdSet>
  cacheIntoRoaringDocIdSet(std::shared_ptr<BulkScorer> scorer,
                           int maxDoc) ;

private:
  class LeafCollectorAnonymousInnerClass2
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass2>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<org::apache::lucene::search::BulkScorer> scorer;
    std::shared_ptr<RoaringDocIdSet::Builder> builder;

  public:
    LeafCollectorAnonymousInnerClass2(
        std::shared_ptr<org::apache::lucene::search::BulkScorer> scorer,
        std::shared_ptr<RoaringDocIdSet::Builder> builder);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

  /**
   * Return the total number of times that a {@link Query} has been looked up
   * in this {@link QueryCache}. Note that this number is incremented once per
   * segment so running a cached query only once will increment this counter
   * by the number of segments that are wrapped by the searcher.
   * Note that by definition, {@link #getTotalCount()} is the sum of
   * {@link #getHitCount()} and {@link #getMissCount()}.
   * @see #getHitCount()
   * @see #getMissCount()
   */
public:
  int64_t getTotalCount();

  /**
   * Over the {@link #getTotalCount() total} number of times that a query has
   * been looked up, return how many times a cached {@link DocIdSet} has been
   * found and returned.
   * @see #getTotalCount()
   * @see #getMissCount()
   */
  int64_t getHitCount();

  /**
   * Over the {@link #getTotalCount() total} number of times that a query has
   * been looked up, return how many times this query was not contained in the
   * cache.
   * @see #getTotalCount()
   * @see #getHitCount()
   */
  int64_t getMissCount();

  /**
   * Return the total number of {@link DocIdSet}s which are currently stored
   * in the cache.
   * @see #getCacheCount()
   * @see #getEvictionCount()
   */
  int64_t getCacheSize();

  /**
   * Return the total number of cache entries that have been generated and put
   * in the cache. It is highly desirable to have a {@link #getHitCount() hit
   * count} that is much higher than the {@link #getCacheCount() cache count}
   * as the opposite would indicate that the query cache makes efforts in order
   * to cache queries but then they do not get reused.
   * @see #getCacheSize()
   * @see #getEvictionCount()
   */
  int64_t getCacheCount();

  /**
   * Return the number of cache entries that have been removed from the cache
   * either in order to stay under the maximum configured size/ram usage, or
   * because a segment has been closed. High numbers of evictions might mean
   * that queries are not reused or that the {@link QueryCachingPolicy
   * caching policy} caches too aggressively on NRT segments which get merged
   * early.
   * @see #getCacheCount()
   * @see #getCacheSize()
   */
  int64_t getEvictionCount();

  // this class is not thread-safe, everything but ramBytesUsed needs to be
  // called under a lock
private:
  class LeafCache : public std::enable_shared_from_this<LeafCache>,
                    public Accountable
  {
    GET_CLASS_NAME(LeafCache)
  private:
    std::shared_ptr<LRUQueryCache> outerInstance;

    const std::any key;
    const std::unordered_map<std::shared_ptr<Query>, std::shared_ptr<DocIdSet>>
        cache;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile long ramBytesUsed;
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t ramBytesUsed_ = 0;

  public:
    LeafCache(std::shared_ptr<LRUQueryCache> outerInstance, std::any key);

  private:
    void onDocIdSetCache(int64_t ramBytesUsed);

    void onDocIdSetEviction(int64_t ramBytesUsed);

  public:
    virtual std::shared_ptr<DocIdSet> get(std::shared_ptr<Query> query);

    virtual void putIfAbsent(std::shared_ptr<Query> query,
                             std::shared_ptr<DocIdSet> set);

    virtual void remove(std::shared_ptr<Query> query);

    int64_t ramBytesUsed() override;
  };

private:
  class CachingWrapperWeight : public ConstantScoreWeight
  {
    GET_CLASS_NAME(CachingWrapperWeight)
  private:
    std::shared_ptr<LRUQueryCache> outerInstance;

    const std::shared_ptr<Weight> in_;
    const std::shared_ptr<QueryCachingPolicy> policy;
    // we use an AtomicBoolean because Weight.scorer may be called from multiple
    // threads when IndexSearcher is created with threads
    const std::shared_ptr<AtomicBoolean> used;

  public:
    CachingWrapperWeight(std::shared_ptr<LRUQueryCache> outerInstance,
                         std::shared_ptr<Weight> in_,
                         std::shared_ptr<QueryCachingPolicy> policy);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

  private:
    bool cacheEntryHasReasonableWorstCaseSize(int maxDoc);

    std::shared_ptr<DocIdSet>
    cache(std::shared_ptr<LeafReaderContext> context) ;

    /** Check whether this segment is eligible for caching, regardless of the
     * query. */
    bool
    shouldCache(std::shared_ptr<LeafReaderContext> context) ;

  public:
    std::shared_ptr<ScorerSupplier> scorerSupplier(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
    private:
      std::shared_ptr<CachingWrapperWeight> outerInstance;

      std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> disi;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<CachingWrapperWeight> outerInstance,
          std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> disi);

      std::shared_ptr<Scorer>
      get(int64_t LeadCost)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            ScorerSupplier::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<CachingWrapperWeight> shared_from_this()
    {
      return std::static_pointer_cast<CachingWrapperWeight>(
          ConstantScoreWeight::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
