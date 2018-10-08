using namespace std;

#include "LRUQueryCache.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using TieredMergePolicy = org::apache::lucene::index::TieredMergePolicy;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BitDocIdSet = org::apache::lucene::util::BitDocIdSet;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using RoaringDocIdSet = org::apache::lucene::util::RoaringDocIdSet;

LRUQueryCache::LRUQueryCache(int maxSize, int64_t maxRamBytesUsed,
                             function<bool(LeafReaderContext *)> &leavesToCache)
    : maxSize(maxSize), maxRamBytesUsed(maxRamBytesUsed),
      leavesToCache(leavesToCache),
      uniqueQueries(make_shared<LinkedHashMap<>>(16, 0.75f, true)),
      mostRecentlyUsedQueries(uniqueQueries.keySet()),
      cache(make_shared<IdentityHashMap<>>()),
      lock(make_shared<ReentrantLock>())
{
  ramBytesUsed_ = 0;
}

LRUQueryCache::LRUQueryCache(int maxSize, int64_t maxRamBytesUsed)
    : LRUQueryCache(maxSize, maxRamBytesUsed,
                    new MinSegmentSizePredicate(10000, .03f))
{
}

LRUQueryCache::MinSegmentSizePredicate::MinSegmentSizePredicate(
    int minSize, float minSizeRatio)
    : minSize(minSize), minSizeRatio(minSizeRatio)
{
}

bool LRUQueryCache::MinSegmentSizePredicate::test(
    shared_ptr<LeafReaderContext> context)
{
  constexpr int maxDoc = context->reader()->maxDoc();
  if (maxDoc < minSize) {
    return false;
  }
  shared_ptr<IndexReaderContext> *const topLevelContext =
      ReaderUtil::getTopLevelContext(context);
  constexpr float sizeRatio = static_cast<float>(context->reader()->maxDoc()) /
                              topLevelContext->reader()->maxDoc();
  return sizeRatio >= minSizeRatio;
}

void LRUQueryCache::onHit(any readerCoreKey, shared_ptr<Query> query)
{
  assert(lock->isHeldByCurrentThread());
  hitCount += 1;
}

void LRUQueryCache::onMiss(any readerCoreKey, shared_ptr<Query> query)
{
  assert(lock->isHeldByCurrentThread());
  assert(query != nullptr);
  missCount += 1;
}

void LRUQueryCache::onQueryCache(shared_ptr<Query> query,
                                 int64_t ramBytesUsed)
{
  assert(lock->isHeldByCurrentThread());
  this->ramBytesUsed_ += ramBytesUsed;
}

void LRUQueryCache::onQueryEviction(shared_ptr<Query> query,
                                    int64_t ramBytesUsed)
{
  assert(lock->isHeldByCurrentThread());
  this->ramBytesUsed_ -= ramBytesUsed;
}

void LRUQueryCache::onDocIdSetCache(any readerCoreKey, int64_t ramBytesUsed)
{
  assert(lock->isHeldByCurrentThread());
  cacheSize += 1;
  cacheCount += 1;
  this->ramBytesUsed_ += ramBytesUsed;
}

void LRUQueryCache::onDocIdSetEviction(any readerCoreKey, int numEntries,
                                       int64_t sumRamBytesUsed)
{
  assert(lock->isHeldByCurrentThread());
  this->ramBytesUsed_ -= sumRamBytesUsed;
  cacheSize -= numEntries;
}

void LRUQueryCache::onClear()
{
  assert(lock->isHeldByCurrentThread());
  ramBytesUsed_ = 0;
  cacheSize = 0;
}

bool LRUQueryCache::requiresEviction()
{
  assert(lock->isHeldByCurrentThread());
  constexpr int size = mostRecentlyUsedQueries->size();
  if (size == 0) {
    return false;
  } else {
    return size > maxSize || ramBytesUsed() > maxRamBytesUsed;
  }
}

shared_ptr<DocIdSet>
LRUQueryCache::get(shared_ptr<Query> key, shared_ptr<LeafReaderContext> context,
                   shared_ptr<IndexReader::CacheHelper> cacheHelper)
{
  assert(lock->isHeldByCurrentThread());
  assert(std::dynamic_pointer_cast<BoostQuery>(key) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(key) != nullptr ==
         false);
  shared_ptr<IndexReader::CacheKey> *const readerKey = cacheHelper->getKey();
  shared_ptr<LeafCache> *const leafCache = cache[readerKey];
  if (leafCache == nullptr) {
    onMiss(readerKey, key);
    return nullptr;
  }
  // this get call moves the query to the most-recently-used position
  shared_ptr<Query> *const singleton = uniqueQueries[key];
  if (singleton == nullptr) {
    onMiss(readerKey, key);
    return nullptr;
  }
  shared_ptr<DocIdSet> *const cached = leafCache->get(singleton);
  if (cached == nullptr) {
    onMiss(readerKey, singleton);
  } else {
    onHit(readerKey, singleton);
  }
  return cached;
}

void LRUQueryCache::putIfAbsent(
    shared_ptr<Query> query, shared_ptr<LeafReaderContext> context,
    shared_ptr<DocIdSet> set, shared_ptr<IndexReader::CacheHelper> cacheHelper)
{
  assert(std::dynamic_pointer_cast<BoostQuery>(query) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr ==
         false);
  // under a lock to make sure that mostRecentlyUsedQueries and cache remain
  // sync'ed
  lock->lock();
  try {
    shared_ptr<Query> singleton = uniqueQueries.try_emplace(query, query);
    if (singleton == nullptr) {
      onQueryCache(query, LINKED_HASHTABLE_RAM_BYTES_PER_ENTRY +
                              QUERY_DEFAULT_RAM_BYTES_USED);
    } else {
      query = singleton;
    }
    shared_ptr<IndexReader::CacheKey> *const key = cacheHelper->getKey();
    shared_ptr<LeafCache> leafCache = cache[key];
    if (leafCache == nullptr) {
      leafCache = make_shared<LeafCache>(shared_from_this(), key);
      shared_ptr<LeafCache> *const previous = cache.emplace(key, leafCache);
      ramBytesUsed_ += HASHTABLE_RAM_BYTES_PER_ENTRY;
      assert(previous == nullptr);
      // we just created a new leaf cache, need to register a close listener
      cacheHelper->addClosedListener(shared_from_this()::clearCoreCacheKey);
    }
    leafCache->putIfAbsent(query, set);
    evictIfNecessary();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

void LRUQueryCache::evictIfNecessary()
{
  assert(lock->isHeldByCurrentThread());
  // under a lock to make sure that mostRecentlyUsedQueries and cache keep
  // sync'ed
  if (requiresEviction()) {

    Set<std::shared_ptr<Query>>::const_iterator iterator =
        mostRecentlyUsedQueries->begin();
    do {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      shared_ptr<Query> *const query = iterator->next();
      constexpr int size = mostRecentlyUsedQueries->size();
      iterator->remove();
      if (size == mostRecentlyUsedQueries->size()) {
        // size did not decrease, because the hash of the query changed since it
        // has been put into the cache
        // C++ TODO: The following line could not be converted:
        throw java.util.ConcurrentModificationException(
            L"Removal from the cache failed! This " +
            L"is probably due to a query which has been modified after having "
            L"been put into " +
            L" the cache or a badly implemented clone(). Query class: [" +
            query.getClass() + L"], query: [" + query + L"]");
      }
      onEviction(query);
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
    } while (iterator->hasNext() && requiresEviction());
  }
}

void LRUQueryCache::clearCoreCacheKey(any coreKey)
{
  lock->lock();
  try {
    shared_ptr<LeafCache> *const leafCache = cache.erase(coreKey);
    if (leafCache != nullptr) {
      ramBytesUsed_ -= HASHTABLE_RAM_BYTES_PER_ENTRY;
      constexpr int numEntries = leafCache->cache->size();
      if (numEntries > 0) {
        onDocIdSetEviction(coreKey, numEntries, leafCache->ramBytesUsed);
      } else {
        assert(numEntries == 0);
        assert(leafCache->ramBytesUsed == 0);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

void LRUQueryCache::clearQuery(shared_ptr<Query> query)
{
  lock->lock();
  try {
    shared_ptr<Query> *const singleton = uniqueQueries.erase(query);
    if (singleton != nullptr) {
      onEviction(singleton);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

void LRUQueryCache::onEviction(shared_ptr<Query> singleton)
{
  assert(lock->isHeldByCurrentThread());
  onQueryEviction(singleton, LINKED_HASHTABLE_RAM_BYTES_PER_ENTRY +
                                 QUERY_DEFAULT_RAM_BYTES_USED);
  for (auto leafCache : cache) {
    leafCache->second->remove(singleton);
  }
}

void LRUQueryCache::clear()
{
  lock->lock();
  try {
    cache.clear();
    // Note that this also clears the uniqueQueries map_obj since
    // mostRecentlyUsedQueries is the uniqueQueries.keySet view:
    mostRecentlyUsedQueries->clear();
    onClear();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

void LRUQueryCache::assertConsistent()
{
  lock->lock();
  try {
    if (requiresEviction()) {
      throw make_shared<AssertionError>(
          L"requires evictions: size=" + mostRecentlyUsedQueries->size() +
          L", maxSize=" + to_wstring(maxSize) + L", ramBytesUsed=" +
          to_wstring(ramBytesUsed()) + L", maxRamBytesUsed=" +
          to_wstring(maxRamBytesUsed));
    }
    for (auto leafCache : cache) {
      shared_ptr<Set<std::shared_ptr<Query>>> keys = Collections::newSetFromMap(
          make_shared<IdentityHashMap<std::shared_ptr<Query>>>());
      keys->addAll(leafCache->second.cache.keySet());
      keys->removeAll(mostRecentlyUsedQueries);
      if (!keys->isEmpty()) {
        throw make_shared<AssertionError>(
            L"One leaf cache contains more keys than the top-level cache: " +
            keys);
      }
    }
    int64_t recomputedRamBytesUsed =
        HASHTABLE_RAM_BYTES_PER_ENTRY * cache.size() +
        LINKED_HASHTABLE_RAM_BYTES_PER_ENTRY * uniqueQueries.size();
    recomputedRamBytesUsed +=
        mostRecentlyUsedQueries->size() * QUERY_DEFAULT_RAM_BYTES_USED;
    for (auto leafCache : cache) {
      recomputedRamBytesUsed +=
          HASHTABLE_RAM_BYTES_PER_ENTRY * leafCache->second.cache->size();
      for (shared_ptr<DocIdSet> set : leafCache->second.cache.values()) {
        recomputedRamBytesUsed += set->ramBytesUsed();
      }
    }
    if (recomputedRamBytesUsed != ramBytesUsed_) {
      throw make_shared<AssertionError>(L"ramBytesUsed mismatch : " +
                                        to_wstring(ramBytesUsed_) + L" != " +
                                        to_wstring(recomputedRamBytesUsed));
    }

    int64_t recomputedCacheSize = 0;
    for (auto leafCache : cache) {
      recomputedCacheSize += leafCache->second.cache->size();
    }
    if (recomputedCacheSize != getCacheSize()) {
      throw make_shared<AssertionError>(L"cacheSize mismatch : " +
                                        to_wstring(getCacheSize()) + L" != " +
                                        to_wstring(recomputedCacheSize));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

deque<std::shared_ptr<Query>> LRUQueryCache::cachedQueries()
{
  lock->lock();
  try {
    return deque<>(mostRecentlyUsedQueries);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

shared_ptr<Weight> LRUQueryCache::doCache(shared_ptr<Weight> weight,
                                          shared_ptr<QueryCachingPolicy> policy)
{
  while (std::dynamic_pointer_cast<CachingWrapperWeight>(weight) != nullptr) {
    weight = (std::static_pointer_cast<CachingWrapperWeight>(weight))->in_;
  }

  return make_shared<CachingWrapperWeight>(shared_from_this(), weight, policy);
}

int64_t LRUQueryCache::ramBytesUsed() { return ramBytesUsed_; }

shared_ptr<deque<std::shared_ptr<Accountable>>>
LRUQueryCache::getChildResources()
{
  lock->lock();
  try {
    return Accountables::namedAccountables(L"segment", cache);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

shared_ptr<DocIdSet> LRUQueryCache::cacheImpl(shared_ptr<BulkScorer> scorer,
                                              int maxDoc) 
{
  if (scorer->cost() * 100 >= maxDoc) {
    // FixedBitSet is faster for dense sets and will enable the random-access
    // optimization in ConjunctionDISI
    return cacheIntoBitSet(scorer, maxDoc);
  } else {
    return cacheIntoRoaringDocIdSet(scorer, maxDoc);
  }
}

shared_ptr<DocIdSet>
LRUQueryCache::cacheIntoBitSet(shared_ptr<BulkScorer> scorer,
                               int maxDoc) 
{
  shared_ptr<FixedBitSet> *const bitSet = make_shared<FixedBitSet>(maxDoc);
  std::deque<int64_t> cost(1);
  scorer->score(
      make_shared<LeafCollectorAnonymousInnerClass>(scorer, bitSet, cost),
      nullptr);
  return make_shared<BitDocIdSet>(bitSet, cost[0]);
}

LRUQueryCache::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<org::apache::lucene::search::BulkScorer> scorer,
        shared_ptr<FixedBitSet> bitSet, deque<int64_t> &cost)
{
  this->scorer = scorer;
  this->bitSet = bitSet;
  this->cost = cost;
}

void LRUQueryCache::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void LRUQueryCache::LeafCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  cost[0]++;
  bitSet->set(doc);
}

shared_ptr<DocIdSet>
LRUQueryCache::cacheIntoRoaringDocIdSet(shared_ptr<BulkScorer> scorer,
                                        int maxDoc) 
{
  shared_ptr<RoaringDocIdSet::Builder> builder =
      make_shared<RoaringDocIdSet::Builder>(maxDoc);
  scorer->score(make_shared<LeafCollectorAnonymousInnerClass2>(scorer, builder),
                nullptr);
  return builder->build();
}

LRUQueryCache::LeafCollectorAnonymousInnerClass2::
    LeafCollectorAnonymousInnerClass2(
        shared_ptr<org::apache::lucene::search::BulkScorer> scorer,
        shared_ptr<RoaringDocIdSet::Builder> builder)
{
  this->scorer = scorer;
  this->builder = builder;
}

void LRUQueryCache::LeafCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void LRUQueryCache::LeafCollectorAnonymousInnerClass2::collect(int doc) throw(
    IOException)
{
  builder->add(doc);
}

int64_t LRUQueryCache::getTotalCount()
{
  return getHitCount() + getMissCount();
}

int64_t LRUQueryCache::getHitCount() { return hitCount; }

int64_t LRUQueryCache::getMissCount() { return missCount; }

int64_t LRUQueryCache::getCacheSize() { return cacheSize; }

int64_t LRUQueryCache::getCacheCount() { return cacheCount; }

int64_t LRUQueryCache::getEvictionCount()
{
  return getCacheCount() - getCacheSize();
}

LRUQueryCache::LeafCache::LeafCache(shared_ptr<LRUQueryCache> outerInstance,
                                    any key)
    : key(key), cache(make_shared<IdentityHashMap<>>()),
      outerInstance(outerInstance)
{
  ramBytesUsed_ = 0;
}

void LRUQueryCache::LeafCache::onDocIdSetCache(int64_t ramBytesUsed)
{
  this->ramBytesUsed_ += ramBytesUsed;
  outerInstance->onDocIdSetCache(key, ramBytesUsed);
}

void LRUQueryCache::LeafCache::onDocIdSetEviction(int64_t ramBytesUsed)
{
  this->ramBytesUsed_ -= ramBytesUsed;
  outerInstance->onDocIdSetEviction(key, 1, ramBytesUsed);
}

shared_ptr<DocIdSet> LRUQueryCache::LeafCache::get(shared_ptr<Query> query)
{
  assert(std::dynamic_pointer_cast<BoostQuery>(query) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr ==
         false);
  return cache[query];
}

void LRUQueryCache::LeafCache::putIfAbsent(shared_ptr<Query> query,
                                           shared_ptr<DocIdSet> set)
{
  assert(std::dynamic_pointer_cast<BoostQuery>(query) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr ==
         false);
  if (cache.try_emplace(query, set) == nullptr) {
    // the set was actually put
    onDocIdSetCache(HASHTABLE_RAM_BYTES_PER_ENTRY + set->ramBytesUsed());
  }
}

void LRUQueryCache::LeafCache::remove(shared_ptr<Query> query)
{
  assert(std::dynamic_pointer_cast<BoostQuery>(query) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr ==
         false);
  shared_ptr<DocIdSet> removed = cache.erase(query);
  if (removed != nullptr) {
    onDocIdSetEviction(HASHTABLE_RAM_BYTES_PER_ENTRY + removed->ramBytesUsed());
  }
}

int64_t LRUQueryCache::LeafCache::ramBytesUsed() { return ramBytesUsed_; }

LRUQueryCache::CachingWrapperWeight::CachingWrapperWeight(
    shared_ptr<LRUQueryCache> outerInstance, shared_ptr<Weight> in_,
    shared_ptr<QueryCachingPolicy> policy)
    : ConstantScoreWeight(in_->getQuery(), 1.0f), in_(in_), policy(policy),
      used(make_shared<AtomicBoolean>(false)), outerInstance(outerInstance)
{
}

void LRUQueryCache::CachingWrapperWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  in_->extractTerms(terms);
}

shared_ptr<Matches> LRUQueryCache::CachingWrapperWeight::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return in_->matches(context, doc);
}

bool LRUQueryCache::CachingWrapperWeight::cacheEntryHasReasonableWorstCaseSize(
    int maxDoc)
{
  // The worst-case (dense) is a bit set which needs one bit per document
  constexpr int64_t worstCaseRamUsage = maxDoc / 8;
  constexpr int64_t totalRamAvailable = outerInstance->maxRamBytesUsed;
  // Imagine the worst-case that a cache entry is large than the size of
  // the cache: not only will this entry be trashed immediately but it
  // will also evict all current entries from the cache. For this reason
  // we only cache on an IndexReader if we have available room for
  // 5 different filters on this reader to avoid excessive trashing
  return worstCaseRamUsage * 5 < totalRamAvailable;
}

shared_ptr<DocIdSet> LRUQueryCache::CachingWrapperWeight::cache(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BulkScorer> *const scorer = in_->bulkScorer(context);
  if (scorer == nullptr) {
    return DocIdSet::EMPTY;
  } else {
    return outerInstance->cacheImpl(scorer, context->reader()->maxDoc());
  }
}

bool LRUQueryCache::CachingWrapperWeight::shouldCache(
    shared_ptr<LeafReaderContext> context) 
{
  return cacheEntryHasReasonableWorstCaseSize(
             ReaderUtil::getTopLevelContext(context)->reader()->maxDoc()) &&
         outerInstance->leavesToCache->test(context);
}

shared_ptr<ScorerSupplier> LRUQueryCache::CachingWrapperWeight::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  if (used->compareAndSet(false, true)) {
    policy->onUse(getQuery());
  }

  if (in_->isCacheable(context) == false) {
    // this segment is not suitable for caching
    return in_->scorerSupplier(context);
  }

  // Short-circuit: Check whether this segment is eligible for caching
  // before we take a lock because of #get
  if (shouldCache(context) == false) {
    return in_->scorerSupplier(context);
  }

  shared_ptr<IndexReader::CacheHelper> *const cacheHelper =
      context->reader()->getCoreCacheHelper();
  if (cacheHelper == nullptr) {
    // this reader has no cache helper
    return in_->scorerSupplier(context);
  }

  // If the lock is already busy, prefer using the uncached version than waiting
  if (outerInstance->lock->tryLock() == false) {
    return in_->scorerSupplier(context);
  }

  shared_ptr<DocIdSet> docIdSet;
  try {
    docIdSet = outerInstance->get(in_->getQuery(), context, cacheHelper);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    outerInstance->lock->unlock();
  }

  if (docIdSet == nullptr) {
    if (policy->shouldCache(in_->getQuery())) {
      docIdSet = cache(context);
      outerInstance->putIfAbsent(in_->getQuery(), context, docIdSet,
                                 cacheHelper);
    } else {
      return in_->scorerSupplier(context);
    }
  }

  assert(docIdSet != nullptr);
  if (docIdSet == DocIdSet::EMPTY) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> *const disi = docIdSet->begin();
  if (disi == nullptr) {
    return nullptr;
  }

  return make_shared<ScorerSupplierAnonymousInnerClass>(shared_from_this(),
                                                        disi);
}

LRUQueryCache::CachingWrapperWeight::ScorerSupplierAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass(
        shared_ptr<CachingWrapperWeight> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
}

shared_ptr<Scorer>
LRUQueryCache::CachingWrapperWeight::ScorerSupplierAnonymousInnerClass::get(
    int64_t LeadCost) 
{
  return make_shared<ConstantScoreScorer>(outerInstance, 0.0f, disi);
}

int64_t
LRUQueryCache::CachingWrapperWeight::ScorerSupplierAnonymousInnerClass::cost()
{
  return disi->cost();
}

shared_ptr<Scorer> LRUQueryCache::CachingWrapperWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = this->scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool LRUQueryCache::CachingWrapperWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return in_->isCacheable(ctx);
}

shared_ptr<BulkScorer> LRUQueryCache::CachingWrapperWeight::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  if (used->compareAndSet(false, true)) {
    policy->onUse(getQuery());
  }

  if (in_->isCacheable(context) == false) {
    // this segment is not suitable for caching
    return in_->bulkScorer(context);
  }

  // Short-circuit: Check whether this segment is eligible for caching
  // before we take a lock because of #get
  if (shouldCache(context) == false) {
    return in_->bulkScorer(context);
  }

  shared_ptr<IndexReader::CacheHelper> *const cacheHelper =
      context->reader()->getCoreCacheHelper();
  if (cacheHelper == nullptr) {
    // this reader has no cacheHelper
    return in_->bulkScorer(context);
  }

  // If the lock is already busy, prefer using the uncached version than waiting
  if (outerInstance->lock->tryLock() == false) {
    return in_->bulkScorer(context);
  }

  shared_ptr<DocIdSet> docIdSet;
  try {
    docIdSet = outerInstance->get(in_->getQuery(), context, cacheHelper);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    outerInstance->lock->unlock();
  }

  if (docIdSet == nullptr) {
    if (policy->shouldCache(in_->getQuery())) {
      docIdSet = cache(context);
      outerInstance->putIfAbsent(in_->getQuery(), context, docIdSet,
                                 cacheHelper);
    } else {
      return in_->bulkScorer(context);
    }
  }

  assert(docIdSet != nullptr);
  if (docIdSet == DocIdSet::EMPTY) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> *const disi = docIdSet->begin();
  if (disi == nullptr) {
    return nullptr;
  }

  return make_shared<DefaultBulkScorer>(
      make_shared<ConstantScoreScorer>(shared_from_this(), 0.0f, disi));
}
} // namespace org::apache::lucene::search