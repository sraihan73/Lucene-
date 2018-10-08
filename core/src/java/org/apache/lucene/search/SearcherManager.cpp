using namespace std;

#include "SearcherManager.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;

SearcherManager::SearcherManager(
    shared_ptr<IndexWriter> writer,
    shared_ptr<SearcherFactory> searcherFactory) 
    : SearcherManager(writer, true, false, searcherFactory)
{
}

SearcherManager::SearcherManager(
    shared_ptr<IndexWriter> writer, bool applyAllDeletes, bool writeAllDeletes,
    shared_ptr<SearcherFactory> searcherFactory) 
{
  if (searcherFactory == nullptr) {
    searcherFactory = make_shared<SearcherFactory>();
  }
  this->searcherFactory = searcherFactory;
  current = getSearcher(
      searcherFactory,
      DirectoryReader::open(writer, applyAllDeletes, writeAllDeletes), nullptr);
}

SearcherManager::SearcherManager(
    shared_ptr<Directory> dir,
    shared_ptr<SearcherFactory> searcherFactory) 
{
  if (searcherFactory == nullptr) {
    searcherFactory = make_shared<SearcherFactory>();
  }
  this->searcherFactory = searcherFactory;
  current = getSearcher(searcherFactory, DirectoryReader::open(dir), nullptr);
}

SearcherManager::SearcherManager(
    shared_ptr<DirectoryReader> reader,
    shared_ptr<SearcherFactory> searcherFactory) 
{
  if (searcherFactory == nullptr) {
    searcherFactory = make_shared<SearcherFactory>();
  }
  this->searcherFactory = searcherFactory;
  this->current = getSearcher(searcherFactory, reader, nullptr);
}

void SearcherManager::decRef(shared_ptr<IndexSearcher> reference) throw(
    IOException)
{
  reference->getIndexReader()->decRef();
}

shared_ptr<IndexSearcher> SearcherManager::refreshIfNeeded(
    shared_ptr<IndexSearcher> referenceToRefresh) 
{
  shared_ptr<IndexReader> *const r = referenceToRefresh->getIndexReader();
  assert((std::dynamic_pointer_cast<DirectoryReader>(r) != nullptr,
          L"searcher's IndexReader should be a DirectoryReader, but got " + r));
  shared_ptr<IndexReader> *const newReader = DirectoryReader::openIfChanged(
      std::static_pointer_cast<DirectoryReader>(r));
  if (newReader == nullptr) {
    return nullptr;
  } else {
    return getSearcher(searcherFactory, newReader, r);
  }
}

bool SearcherManager::tryIncRef(shared_ptr<IndexSearcher> reference)
{
  return reference->getIndexReader()->tryIncRef();
}

int SearcherManager::getRefCount(shared_ptr<IndexSearcher> reference)
{
  return reference->getIndexReader()->getRefCount();
}

bool SearcherManager::isSearcherCurrent() 
{
  shared_ptr<IndexSearcher> *const searcher = acquire();
  try {
    shared_ptr<IndexReader> *const r = searcher->getIndexReader();
    assert(
        (std::dynamic_pointer_cast<DirectoryReader>(r) != nullptr,
         L"searcher's IndexReader should be a DirectoryReader, but got " + r));
    return (std::static_pointer_cast<DirectoryReader>(r))->isCurrent();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    release(searcher);
  }
}

shared_ptr<IndexSearcher> SearcherManager::getSearcher(
    shared_ptr<SearcherFactory> searcherFactory, shared_ptr<IndexReader> reader,
    shared_ptr<IndexReader> previousReader) 
{
  bool success = false;
  shared_ptr<IndexSearcher> *const searcher;
  try {
    searcher = searcherFactory->newSearcher(reader, previousReader);
    if (searcher->getIndexReader() != reader) {
      throw make_shared<IllegalStateException>(
          L"SearcherFactory must wrap exactly the provided reader (got " +
          searcher->getIndexReader() + L" but expected " + reader + L")");
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      reader->decRef();
    }
  }
  return searcher;
}
} // namespace org::apache::lucene::search