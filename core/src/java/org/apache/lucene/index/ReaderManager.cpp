using namespace std;

#include "ReaderManager.h"

namespace org::apache::lucene::index
{
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Directory = org::apache::lucene::store::Directory;

ReaderManager::ReaderManager(shared_ptr<IndexWriter> writer) 
    : ReaderManager(writer, true, false)
{
}

ReaderManager::ReaderManager(shared_ptr<IndexWriter> writer,
                             bool applyAllDeletes,
                             bool writeAllDeletes) 
{
  current = DirectoryReader::open(writer, applyAllDeletes, writeAllDeletes);
}

ReaderManager::ReaderManager(shared_ptr<Directory> dir) 
{
  current = DirectoryReader::open(dir);
}

ReaderManager::ReaderManager(shared_ptr<DirectoryReader> reader) throw(
    IOException)
{
  current = reader;
}

void ReaderManager::decRef(shared_ptr<DirectoryReader> reference) throw(
    IOException)
{
  reference->decRef();
}

shared_ptr<DirectoryReader> ReaderManager::refreshIfNeeded(
    shared_ptr<DirectoryReader> referenceToRefresh) 
{
  return DirectoryReader::openIfChanged(referenceToRefresh);
}

bool ReaderManager::tryIncRef(shared_ptr<DirectoryReader> reference)
{
  return reference->tryIncRef();
}

int ReaderManager::getRefCount(shared_ptr<DirectoryReader> reference)
{
  return reference->getRefCount();
}
} // namespace org::apache::lucene::index