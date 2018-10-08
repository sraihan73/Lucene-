using namespace std;

#include "LruTaxonomyWriterCache.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;

LruTaxonomyWriterCache::LruTaxonomyWriterCache(int cacheSize)
    : LruTaxonomyWriterCache(cacheSize, LRUType::LRU_STRING)
{
  // TODO (Facet): choose between NameHashIntCacheLRU and NameIntCacheLRU.
  // For guaranteed correctness - not relying on no-collisions in the hash
  // function, NameIntCacheLRU should be used:
  // On the other hand, NameHashIntCacheLRU takes less RAM but if there
  // are collisions two different paths would be mapped to the same
  // ordinal...
}

LruTaxonomyWriterCache::LruTaxonomyWriterCache(int cacheSize, LRUType lruType)
{
  // TODO (Facet): choose between NameHashIntCacheLRU and NameIntCacheLRU.
  // For guaranteed correctness - not relying on no-collisions in the hash
  // function, NameIntCacheLRU should be used:
  // On the other hand, NameHashIntCacheLRU takes less RAM but if there
  // are collisions two different paths would be mapped to the same
  // ordinal...
  if (lruType == LRUType::LRU_HASHED) {
    this->cache = make_shared<NameHashIntCacheLRU>(cacheSize);
  } else {
    this->cache = make_shared<NameIntCacheLRU>(cacheSize);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool LruTaxonomyWriterCache::isFull()
{
  return cache->getSize() == cache->getMaxSize();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LruTaxonomyWriterCache::clear() { cache->clear(); }

// C++ WARNING: The following method was originally marked 'synchronized':
LruTaxonomyWriterCache::~LruTaxonomyWriterCache()
{
  cache->clear();
  cache.reset();
}

int LruTaxonomyWriterCache::size() { return cache->getSize(); }

// C++ WARNING: The following method was originally marked 'synchronized':
int LruTaxonomyWriterCache::get(shared_ptr<FacetLabel> categoryPath)
{
  optional<int> res = cache->get(categoryPath);
  if (!res) {
    return -1;
  }

  return res.value();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool LruTaxonomyWriterCache::put(shared_ptr<FacetLabel> categoryPath,
                                 int ordinal)
{
  bool ret = cache->put(categoryPath, optional<int>(ordinal));
  // If the cache is full, we need to clear one or more old entries
  // from the cache. However, if we delete from the cache a recent
  // addition that isn't yet in our reader, for this entry to be
  // visible to us we need to make sure that the changes have been
  // committed and we reopen the reader. Because this is a slow
  // operation, we don't delete entries one-by-one but rather in bulk
  // (put() removes the 2/3rd oldest entries).
  if (ret) {
    cache->makeRoomLRU();
  }
  return ret;
}
} // namespace org::apache::lucene::facet::taxonomy::writercache