using namespace std;

#include "NameIntCacheLRU.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;

NameIntCacheLRU::NameIntCacheLRU(int maxCacheSize)
{
  this->maxCacheSize = maxCacheSize;
  createCache(maxCacheSize);
}

int NameIntCacheLRU::getMaxSize() { return maxCacheSize; }

int NameIntCacheLRU::getSize() { return cache.size(); }

void NameIntCacheLRU::createCache(int maxSize)
{
  if (maxSize < numeric_limits<int>::max()) {
    cache = make_shared<LinkedHashMap<>>(1000, static_cast<float>(0.7),
                                         true); // for LRU
  } else {
    cache = unordered_map<any, int>(1000,
                                    static_cast<float>(0.7)); // no need for LRU
  }
}

optional<int> NameIntCacheLRU::get(shared_ptr<FacetLabel> name)
{
  optional<int> res = cache[key(name)];
  if (!res) {
    nMisses++;
  } else {
    nHits++;
  }
  return res;
}

any NameIntCacheLRU::key(shared_ptr<FacetLabel> name) { return name; }

any NameIntCacheLRU::key(shared_ptr<FacetLabel> name, int prefixLen)
{
  return name->subpath(prefixLen);
}

bool NameIntCacheLRU::put(shared_ptr<FacetLabel> name, optional<int> &val)
{
  cache.emplace(key(name), val);
  return isCacheFull();
}

bool NameIntCacheLRU::put(shared_ptr<FacetLabel> name, int prefixLen,
                          optional<int> &val)
{
  cache.emplace(key(name, prefixLen), val);
  return isCacheFull();
}

bool NameIntCacheLRU::isCacheFull() { return cache.size() > maxCacheSize; }

void NameIntCacheLRU::clear() { cache.clear(); }

wstring NameIntCacheLRU::stats()
{
  return L"#miss=" + to_wstring(nMisses) + L" #hit=" + to_wstring(nHits);
}

bool NameIntCacheLRU::makeRoomLRU()
{
  if (!isCacheFull()) {
    return false;
  }
  int n = cache.size() - (2 * maxCacheSize) / 3;
  if (n <= 0) {
    return false;
  }
  Iterator<any> it = cache.keySet().begin();
  int i = 0;
  while (i < n && it->hasNext()) {
    it->next();
    it->remove();
    i++;
  }
  return true;
}
} // namespace org::apache::lucene::facet::taxonomy::writercache