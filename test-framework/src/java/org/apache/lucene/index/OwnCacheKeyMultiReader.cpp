using namespace std;

#include "OwnCacheKeyMultiReader.h"

namespace org::apache::lucene::index
{
using IOUtils = org::apache::lucene::util::IOUtils;

OwnCacheKeyMultiReader::CacheHelperAnonymousInnerClass::
    CacheHelperAnonymousInnerClass()
{
}

shared_ptr<CacheKey>
OwnCacheKeyMultiReader::CacheHelperAnonymousInnerClass::getKey()
{
  return cacheKey;
}

void OwnCacheKeyMultiReader::CacheHelperAnonymousInnerClass::addClosedListener(
    shared_ptr<ClosedListener> listener)
{
  ensureOpen();
  outerInstance->readerClosedListeners->add(listener);
}

OwnCacheKeyMultiReader::OwnCacheKeyMultiReader(
    deque<IndexReader> &subReaders) 
    : MultiReader(subReaders)
{
}

shared_ptr<CacheHelper> OwnCacheKeyMultiReader::getReaderCacheHelper()
{
  return cacheHelper;
}

void OwnCacheKeyMultiReader::notifyReaderClosedListeners() 
{
  {
    lock_guard<mutex> lock(readerClosedListeners);
    IOUtils::applyToAll(readerClosedListeners,
                        [&](any l) { l::onClose(cacheHelper::getKey()); });
  }
}
} // namespace org::apache::lucene::index