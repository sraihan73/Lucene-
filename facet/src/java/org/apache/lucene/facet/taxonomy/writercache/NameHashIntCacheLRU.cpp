using namespace std;

#include "NameHashIntCacheLRU.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;

NameHashIntCacheLRU::NameHashIntCacheLRU(int maxCacheSize)
    : NameIntCacheLRU(maxCacheSize)
{
}

any NameHashIntCacheLRU::key(shared_ptr<FacetLabel> name)
{
  return optional<int64_t>(name->longHashCode());
}

any NameHashIntCacheLRU::key(shared_ptr<FacetLabel> name, int prefixLen)
{
  return optional<int64_t>(name->subpath(prefixLen)->longHashCode());
}
} // namespace org::apache::lucene::facet::taxonomy::writercache