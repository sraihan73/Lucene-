using namespace std;

#include "SearcherTaxonomyManager.h"

namespace org::apache::lucene::facet::taxonomy
{
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;

SearcherTaxonomyManager::SearcherAndTaxonomy::SearcherAndTaxonomy(
    shared_ptr<IndexSearcher> searcher,
    shared_ptr<DirectoryTaxonomyReader> taxonomyReader)
    : searcher(searcher), taxonomyReader(taxonomyReader)
{
}

SearcherTaxonomyManager::SearcherTaxonomyManager(
    shared_ptr<IndexWriter> writer, shared_ptr<SearcherFactory> searcherFactory,
    shared_ptr<DirectoryTaxonomyWriter> taxoWriter) 
    : SearcherTaxonomyManager(writer, true, searcherFactory, taxoWriter)
{
}

SearcherTaxonomyManager::SearcherTaxonomyManager(
    shared_ptr<IndexWriter> writer, bool applyAllDeletes,
    shared_ptr<SearcherFactory> searcherFactory,
    shared_ptr<DirectoryTaxonomyWriter> taxoWriter) 
    : taxoEpoch(taxoWriter->getTaxonomyEpoch()), taxoWriter(taxoWriter)
{
  if (searcherFactory == nullptr) {
    searcherFactory = make_shared<SearcherFactory>();
  }
  this->searcherFactory = searcherFactory;
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);
  current = make_shared<SearcherAndTaxonomy>(
      SearcherManager::getSearcher(
          searcherFactory,
          DirectoryReader::open(writer, applyAllDeletes, false), nullptr),
      taxoReader);
}

SearcherTaxonomyManager::SearcherTaxonomyManager(
    shared_ptr<Directory> indexDir, shared_ptr<Directory> taxoDir,
    shared_ptr<SearcherFactory> searcherFactory) 
    : taxoEpoch(-1), taxoWriter(this->taxoWriter.reset())
{
  if (searcherFactory == nullptr) {
    searcherFactory = make_shared<SearcherFactory>();
  }
  this->searcherFactory = searcherFactory;
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  current = make_shared<SearcherAndTaxonomy>(
      SearcherManager::getSearcher(searcherFactory,
                                   DirectoryReader::open(indexDir), nullptr),
      taxoReader);
}

void SearcherTaxonomyManager::decRef(shared_ptr<SearcherAndTaxonomy> ref) throw(
    IOException)
{
  ref->searcher->getIndexReader()->decRef();

  // This decRef can fail, and then in theory we should
  // tryIncRef the searcher to put back the ref count
  // ... but 1) the below decRef should only fail because
  // it decRef'd to 0 and closed and hit some IOException
  // during close, in which case 2) very likely the
  // searcher was also just closed by the above decRef and
  // a tryIncRef would fail:
  ref->taxonomyReader->decRef();
}

bool SearcherTaxonomyManager::tryIncRef(
    shared_ptr<SearcherAndTaxonomy> ref) 
{
  if (ref->searcher->getIndexReader()->tryIncRef()) {
    if (ref->taxonomyReader->tryIncRef()) {
      return true;
    } else {
      ref->searcher->getIndexReader()->decRef();
    }
  }
  return false;
}

shared_ptr<SearcherAndTaxonomy> SearcherTaxonomyManager::refreshIfNeeded(
    shared_ptr<SearcherAndTaxonomy> ref) 
{
  // Must re-open searcher first, otherwise we may get a
  // new reader that references ords not yet known to the
  // taxonomy reader:
  shared_ptr<IndexReader> *const r = ref->searcher->getIndexReader();
  shared_ptr<IndexReader> *const newReader = DirectoryReader::openIfChanged(
      std::static_pointer_cast<DirectoryReader>(r));
  if (newReader == nullptr) {
    return nullptr;
  } else {
    shared_ptr<DirectoryTaxonomyReader> tr;
    try {
      tr = TaxonomyReader::openIfChanged(ref->taxonomyReader);
    } catch (const runtime_error &t1) {
      try {
        IOUtils::close({newReader});
      } catch (const runtime_error &t2) {
        t2.addSuppressed(t2);
      }
      throw t1;
    }
    if (tr == nullptr) {
      ref->taxonomyReader->incRef();
      tr = ref->taxonomyReader;
    } else if (taxoWriter != nullptr &&
               taxoWriter->getTaxonomyEpoch() != taxoEpoch) {
      IOUtils::close({newReader, tr});
      throw make_shared<IllegalStateException>(
          L"DirectoryTaxonomyWriter.replaceTaxonomy was called, which is not "
          L"allowed when using SearcherTaxonomyManager");
    }

    return make_shared<SearcherAndTaxonomy>(
        SearcherManager::getSearcher(searcherFactory, newReader, r), tr);
  }
}

int SearcherTaxonomyManager::getRefCount(
    shared_ptr<SearcherAndTaxonomy> reference)
{
  return reference->searcher->getIndexReader()->getRefCount();
}
} // namespace org::apache::lucene::facet::taxonomy