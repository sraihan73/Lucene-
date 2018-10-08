using namespace std;

#include "SegmentInfosSearcherManager.h"

namespace org::apache::lucene::replicator::nrt
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using StandardDirectoryReader =
    org::apache::lucene::index::StandardDirectoryReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Directory = org::apache::lucene::store::Directory;

SegmentInfosSearcherManager::SegmentInfosSearcherManager(
    shared_ptr<Directory> dir, shared_ptr<Node> node,
    shared_ptr<SegmentInfos> infosIn,
    shared_ptr<SearcherFactory> searcherFactory) 
    : dir(dir), node(node)
{
  if (searcherFactory == nullptr) {
    searcherFactory = make_shared<SearcherFactory>();
  }
  this->searcherFactory = searcherFactory;
  currentInfos = infosIn;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  node->message(L"SegmentInfosSearcherManager.init: use incoming infos=" +
                infosIn->toString());
  current = SearcherManager::getSearcher(
      searcherFactory,
      StandardDirectoryReader::open(dir, currentInfos, nullptr), nullptr);
  addReaderClosedListener(current->getIndexReader());
}

int SegmentInfosSearcherManager::getRefCount(shared_ptr<IndexSearcher> s)
{
  return s->getIndexReader()->getRefCount();
}

bool SegmentInfosSearcherManager::tryIncRef(shared_ptr<IndexSearcher> s)
{
  return s->getIndexReader()->tryIncRef();
}

void SegmentInfosSearcherManager::decRef(shared_ptr<IndexSearcher> s) throw(
    IOException)
{
  s->getIndexReader()->decRef();
}

shared_ptr<SegmentInfos> SegmentInfosSearcherManager::getCurrentInfos()
{
  return currentInfos;
}

void SegmentInfosSearcherManager::setCurrentInfos(
    shared_ptr<SegmentInfos> infos) 
{
  if (currentInfos->size() > 0) {
    // So that if we commit, we will go to the next
    // (unwritten so far) generation:
    infos->updateGeneration(currentInfos);
    node->message(L"mgr.setCurrentInfos: carry over infos gen=" +
                  infos->getSegmentsFileName());
  }
  currentInfos = infos;
  maybeRefresh();
}

shared_ptr<IndexSearcher> SegmentInfosSearcherManager::refreshIfNeeded(
    shared_ptr<IndexSearcher> old) 
{
  deque<std::shared_ptr<LeafReader>> subs;
  if (old == nullptr) {
    subs.clear();
  } else {
    subs = deque<>();
    for (auto ctx : old->getIndexReader()->leaves()) {
      subs.push_back(ctx->reader());
    }
  }

  // Open a new reader, sharing any common segment readers with the old one:
  shared_ptr<DirectoryReader> r =
      StandardDirectoryReader::open(dir, currentInfos, subs);
  addReaderClosedListener(r);
  node->message(L"refreshed to version=" +
                to_wstring(currentInfos->getVersion()) + L" r=" + r);
  return SearcherManager::getSearcher(
      searcherFactory, r,
      std::static_pointer_cast<DirectoryReader>(old->getIndexReader()));
}

void SegmentInfosSearcherManager::addReaderClosedListener(
    shared_ptr<IndexReader> r)
{
  shared_ptr<IndexReader::CacheHelper> cacheHelper = r->getReaderCacheHelper();
  if (cacheHelper == nullptr) {
    throw make_shared<IllegalStateException>(
        L"StandardDirectoryReader must support caching");
  }
  openReaderCount->incrementAndGet();
  cacheHelper->addClosedListener(
      [&](IndexReader::CacheKey cacheKey) { onReaderClosed(); });
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SegmentInfosSearcherManager::onReaderClosed()
{
  if (openReaderCount->decrementAndGet() == 0) {
    notifyAll();
  }
}
} // namespace org::apache::lucene::replicator::nrt