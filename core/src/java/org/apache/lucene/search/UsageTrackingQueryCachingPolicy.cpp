using namespace std;

#include "UsageTrackingQueryCachingPolicy.h"

namespace org::apache::lucene::search
{
using FrequencyTrackingRingBuffer =
    org::apache::lucene::util::FrequencyTrackingRingBuffer;

bool UsageTrackingQueryCachingPolicy::isPointQuery(shared_ptr<Query> query)
{
  // we need to check for super classes because we occasionally use anonymous
  // sub classes of eg. PointRangeQuery
  for (type_info clazz = query->getClass(); clazz != Query::typeid;
       clazz = clazz.getSuperclass()) {
    const wstring simpleName = clazz.name();
    if (StringHelper::startsWith(simpleName, L"Point") &&
        StringHelper::endsWith(simpleName, L"Query")) {
      return true;
    }
  }
  return false;
}

bool UsageTrackingQueryCachingPolicy::isCostly(shared_ptr<Query> query)
{
  // This does not measure the cost of iterating over the filter (for this we
  // already have the DocIdSetIterator#cost API) but the cost to build the
  // DocIdSet in the first place
  return std::dynamic_pointer_cast<MultiTermQuery>(query) != nullptr ||
         std::dynamic_pointer_cast<MultiTermQueryConstantScoreWrapper>(query) !=
             nullptr ||
         std::dynamic_pointer_cast<TermInSetQuery>(query) != nullptr ||
         isPointQuery(query);
}

bool UsageTrackingQueryCachingPolicy::shouldNeverCache(shared_ptr<Query> query)
{
  if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr) {
    // We do not bother caching term queries since they are already plenty fast.
    return true;
  }

  if (std::dynamic_pointer_cast<MatchAllDocsQuery>(query) != nullptr) {
    // MatchAllDocsQuery has an iterator that is faster than what a bit set
    // could do.
    return true;
  }

  // For the below queries, it's cheap to notice they cannot match any docs so
  // we do not bother caching them.
  if (std::dynamic_pointer_cast<MatchNoDocsQuery>(query) != nullptr) {
    return true;
  }

  if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr) {
    shared_ptr<BooleanQuery> bq = std::static_pointer_cast<BooleanQuery>(query);
    if (bq->clauses().empty()) {
      return true;
    }
  }

  if (std::dynamic_pointer_cast<DisjunctionMaxQuery>(query) != nullptr) {
    shared_ptr<DisjunctionMaxQuery> dmq =
        std::static_pointer_cast<DisjunctionMaxQuery>(query);
    if (dmq->getDisjuncts().empty()) {
      return true;
    }
  }

  return false;
}

UsageTrackingQueryCachingPolicy::UsageTrackingQueryCachingPolicy(
    int historySize)
    : recentlyUsedFilters(
          make_shared<FrequencyTrackingRingBuffer>(historySize, SENTINEL))
{
}

UsageTrackingQueryCachingPolicy::UsageTrackingQueryCachingPolicy()
    : UsageTrackingQueryCachingPolicy(256)
{
}

int UsageTrackingQueryCachingPolicy::minFrequencyToCache(
    shared_ptr<Query> query)
{
  if (isCostly(query)) {
    return 2;
  } else {
    // default: cache after the filter has been seen 5 times
    int minFrequency = 5;
    if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr ||
        std::dynamic_pointer_cast<DisjunctionMaxQuery>(query) != nullptr) {
      // Say you keep reusing a bool query that looks like "A OR B" and
      // never use the A and B queries out of that context. 5 times after it
      // has been used, we would cache both A, B and A OR B, which is
      // wasteful. So instead we cache compound queries a bit earlier so that
      // we would only cache "A OR B" in that case.
      minFrequency--;
    }
    return minFrequency;
  }
}

void UsageTrackingQueryCachingPolicy::onUse(shared_ptr<Query> query)
{
  assert(std::dynamic_pointer_cast<BoostQuery>(query) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr ==
         false);

  if (shouldNeverCache(query)) {
    return;
  }

  // call hashCode outside of sync block
  // in case it's somewhat expensive:
  int hashCode = query->hashCode();

  // we only track hash codes to avoid holding references to possible
  // large queries; this may cause rare false positives, but at worse
  // this just means we cache a query that was not in fact used enough:
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this()) { recentlyUsedFilters->add(hashCode); }
}

int UsageTrackingQueryCachingPolicy::frequency(shared_ptr<Query> query)
{
  assert(std::dynamic_pointer_cast<BoostQuery>(query) != nullptr == false);
  assert(std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr ==
         false);

  // call hashCode outside of sync block
  // in case it's somewhat expensive:
  int hashCode = query->hashCode();

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    return recentlyUsedFilters->frequency(hashCode);
  }
}

bool UsageTrackingQueryCachingPolicy::shouldCache(
    shared_ptr<Query> query) 
{
  if (shouldNeverCache(query)) {
    return false;
  }
  constexpr int frequency = this->frequency(query);
  constexpr int minFrequency = minFrequencyToCache(query);
  return frequency >= minFrequency;
}
} // namespace org::apache::lucene::search