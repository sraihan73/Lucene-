using namespace std;

#include "DirectoryTaxonomyReader.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using Document = org::apache::lucene::document::Document;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using LRUHashMap = org::apache::lucene::facet::taxonomy::LRUHashMap;
using ParallelTaxonomyArrays =
    org::apache::lucene::facet::taxonomy::ParallelTaxonomyArrays;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReader = org::apache::lucene::index::SegmentReader;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const shared_ptr<java::util::logging::Logger> DirectoryTaxonomyReader::logger =
    java::util::logging::Logger::getLogger(
        DirectoryTaxonomyReader::typeid->getName());

DirectoryTaxonomyReader::DirectoryTaxonomyReader(
    shared_ptr<DirectoryReader> indexReader,
    shared_ptr<DirectoryTaxonomyWriter> taxoWriter,
    shared_ptr<LRUHashMap<std::shared_ptr<FacetLabel>, int>> ordinalCache,
    shared_ptr<LRUHashMap<int, std::shared_ptr<FacetLabel>>> categoryCache,
    shared_ptr<TaxonomyIndexArrays> taxoArrays) 
    : taxoWriter(taxoWriter),
      taxoEpoch(taxoWriter == nullptr ? -1 : taxoWriter->getTaxonomyEpoch()),
      indexReader(indexReader)
{

  // use the same instance of the cache, note the protective code in getOrdinal
  // and getPath
  this->ordinalCache =
      ordinalCache == nullptr
          ? make_shared<LRUHashMap<std::shared_ptr<FacetLabel>, int>>(
                DEFAULT_CACHE_VALUE)
          : ordinalCache;
  this->categoryCache =
      categoryCache == nullptr
          ? make_shared<LRUHashMap<int, std::shared_ptr<FacetLabel>>>(
                DEFAULT_CACHE_VALUE)
          : categoryCache;

  this->taxoArrays =
      taxoArrays != nullptr
          ? make_shared<TaxonomyIndexArrays>(indexReader, taxoArrays)
          : nullptr;
}

DirectoryTaxonomyReader::DirectoryTaxonomyReader(
    shared_ptr<Directory> directory) 
    : taxoWriter(taxoWriter.reset()), taxoEpoch(-1),
      indexReader(openIndexReader(directory))
{

  // These are the default cache sizes; they can be configured after
  // construction with the cache's setMaxSize() method
  ordinalCache = make_shared<LRUHashMap<FacetLabel, int>>(DEFAULT_CACHE_VALUE);
  categoryCache = make_shared<LRUHashMap<int, FacetLabel>>(DEFAULT_CACHE_VALUE);
}

DirectoryTaxonomyReader::DirectoryTaxonomyReader(
    shared_ptr<DirectoryTaxonomyWriter> taxoWriter) 
    : taxoWriter(taxoWriter), taxoEpoch(taxoWriter->getTaxonomyEpoch()),
      indexReader(openIndexReader(taxoWriter->getInternalIndexWriter()))
{

  // These are the default cache sizes; they can be configured after
  // construction with the cache's setMaxSize() method
  ordinalCache = make_shared<LRUHashMap<FacetLabel, int>>(DEFAULT_CACHE_VALUE);
  categoryCache = make_shared<LRUHashMap<int, FacetLabel>>(DEFAULT_CACHE_VALUE);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirectoryTaxonomyReader::initTaxoArrays() 
{
  if (taxoArrays == nullptr) {
    // according to Java Concurrency in Practice, this might perform better on
    // some JVMs, because the array initialization doesn't happen on the
    // volatile member.
    shared_ptr<TaxonomyIndexArrays> tmpArrays =
        make_shared<TaxonomyIndexArrays>(indexReader);
    taxoArrays = tmpArrays;
  }
}

void DirectoryTaxonomyReader::doClose() 
{
  indexReader->close();
  taxoArrays.reset();
  // do not clear() the caches, as they may be used by other DTR instances.
  ordinalCache.reset();
  categoryCache.reset();
}

shared_ptr<DirectoryTaxonomyReader>
DirectoryTaxonomyReader::doOpenIfChanged() 
{
  ensureOpen();

  // This works for both NRT and non-NRT readers (i.e. an NRT reader remains
  // NRT).
  shared_ptr<DirectoryReader> *const r2 =
      DirectoryReader::openIfChanged(indexReader);
  if (r2 == nullptr) {
    return nullptr; // no changes, nothing to do
  }

  // check if the taxonomy was recreated
  bool success = false;
  try {
    bool recreated = false;
    if (taxoWriter == nullptr) {
      // not NRT, check epoch from commit data
      wstring t1 = indexReader->getIndexCommit()
                       ->getUserData()[DirectoryTaxonomyWriter::INDEX_EPOCH];
      wstring t2 = r2->getIndexCommit()
                       ->getUserData()[DirectoryTaxonomyWriter::INDEX_EPOCH];
      if (t1 == L"") {
        if (t2 != L"") {
          recreated = true;
        }
      } else if (t1 != t2) {
        // t1 != null and t2 must not be null b/c DirTaxoWriter always puts the
        // commit data. it's ok to use std::wstring.equals because we require the two
        // epoch values to be the same.
        recreated = true;
      }
    } else {
      // NRT, compare current taxoWriter.epoch() vs the one that was given at
      // construction
      if (taxoEpoch != taxoWriter->getTaxonomyEpoch()) {
        recreated = true;
      }
    }

    shared_ptr<DirectoryTaxonomyReader> *const newtr;
    if (recreated) {
      // if recreated, do not reuse anything from this instace. the information
      // will be lazily computed by the new instance when needed.
      newtr = make_shared<DirectoryTaxonomyReader>(r2, taxoWriter, nullptr,
                                                   nullptr, nullptr);
    } else {
      newtr = make_shared<DirectoryTaxonomyReader>(r2, taxoWriter, ordinalCache,
                                                   categoryCache, taxoArrays);
    }

    success = true;
    return newtr;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({r2});
    }
  }
}

shared_ptr<DirectoryReader> DirectoryTaxonomyReader::openIndexReader(
    shared_ptr<Directory> directory) 
{
  return DirectoryReader::open(directory);
}

shared_ptr<DirectoryReader> DirectoryTaxonomyReader::openIndexReader(
    shared_ptr<IndexWriter> writer) 
{
  return DirectoryReader::open(writer);
}

shared_ptr<DirectoryReader> DirectoryTaxonomyReader::getInternalIndexReader()
{
  ensureOpen();
  return indexReader;
}

shared_ptr<ParallelTaxonomyArrays>
DirectoryTaxonomyReader::getParallelTaxonomyArrays() 
{
  ensureOpen();
  if (taxoArrays == nullptr) {
    initTaxoArrays();
  }
  return taxoArrays;
}

unordered_map<wstring, wstring>
DirectoryTaxonomyReader::getCommitUserData() 
{
  ensureOpen();
  return indexReader->getIndexCommit()->getUserData();
}

int DirectoryTaxonomyReader::getOrdinal(shared_ptr<FacetLabel> cp) throw(
    IOException)
{
  ensureOpen();
  if (cp->length == 0) {
    return ROOT_ORDINAL;
  }

  {
    // First try to find the answer in the LRU cache:
    lock_guard<mutex> lock(ordinalCache);
    optional<int> res = ordinalCache->get(cp);
    if (res) {
      if (res.value() < indexReader->maxDoc()) {
        // Since the cache is shared with DTR instances allocated from
        // doOpenIfChanged, we need to ensure that the ordinal is one that
        // this DTR instance recognizes.
        return res.value();
      } else {
        // if we get here, it means that the category was found in the cache,
        // but is not recognized by this TR instance. Therefore there's no
        // need to continue search for the path on disk, because we won't find
        // it there too.
        return TaxonomyReader::INVALID_ORDINAL;
      }
    }
  }

  // If we're still here, we have a cache miss. We need to fetch the
  // value from disk, and then also put it in the cache:
  int ret = TaxonomyReader::INVALID_ORDINAL;
  shared_ptr<PostingsEnum> docs = MultiFields::getTermDocsEnum(
      indexReader, Consts::FULL,
      make_shared<BytesRef>(
          FacetsConfig::pathToString(cp->components, cp->length)),
      0);
  if (docs != nullptr && docs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    ret = docs->docID();

    // we only store the fact that a category exists, not its inexistence.
    // This is required because the caches are shared with new DTR instances
    // that are allocated from doOpenIfChanged. Therefore, if we only store
    // information about found categories, we cannot accidently tell a new
    // generation of DTR that a category does not exist.
    {
      lock_guard<mutex> lock(ordinalCache);
      ordinalCache->put(cp, static_cast<Integer>(ret));
    }
  }

  return ret;
}

shared_ptr<FacetLabel>
DirectoryTaxonomyReader::getPath(int ordinal) 
{
  ensureOpen();

  // Since the cache is shared with DTR instances allocated from
  // doOpenIfChanged, we need to ensure that the ordinal is one that this DTR
  // instance recognizes. Therefore we do this check up front, before we hit
  // the cache.
  if (ordinal < 0 || ordinal >= indexReader->maxDoc()) {
    return nullptr;
  }

  // TODO: can we use an int-based hash impl, such as IntToObjectMap,
  // wrapped as LRU?
  optional<int> catIDInteger = static_cast<Integer>(ordinal);
  {
    lock_guard<mutex> lock(categoryCache);
    shared_ptr<FacetLabel> res = categoryCache->get(catIDInteger);
    if (res != nullptr) {
      return res;
    }
  }

  shared_ptr<Document> doc = indexReader->document(ordinal);
  shared_ptr<FacetLabel> ret =
      make_shared<FacetLabel>(FacetsConfig::stringToPath(doc[Consts::FULL]));
  {
    lock_guard<mutex> lock(categoryCache);
    categoryCache->put(catIDInteger, ret);
  }

  return ret;
}

int DirectoryTaxonomyReader::getSize()
{
  ensureOpen();
  return indexReader->numDocs();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DirectoryTaxonomyReader::ramBytesUsed()
{
  ensureOpen();
  int64_t ramBytesUsed = 0;
  for (shared_ptr<LeafReaderContext> ctx : indexReader->leaves()) {
    ramBytesUsed += (std::static_pointer_cast<SegmentReader>(ctx->reader()))
                        ->ramBytesUsed();
  }
  if (taxoArrays != nullptr) {
    ramBytesUsed += taxoArrays->ramBytesUsed();
  }
  {
    lock_guard<mutex> lock(categoryCache);
    ramBytesUsed += BYTES_PER_CACHE_ENTRY * categoryCache->size();
  }

  {
    lock_guard<mutex> lock(ordinalCache);
    ramBytesUsed += BYTES_PER_CACHE_ENTRY * ordinalCache->size();
  }

  return ramBytesUsed;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectoryTaxonomyReader::getChildResources()
{
  const deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  int64_t ramBytesUsed = 0;
  for (shared_ptr<LeafReaderContext> ctx : indexReader->leaves()) {
    ramBytesUsed += (std::static_pointer_cast<SegmentReader>(ctx->reader()))
                        ->ramBytesUsed();
  }
  resources.push_back(
      Accountables::namedAccountable(L"indexReader", ramBytesUsed));
  if (taxoArrays != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"taxoArrays", taxoArrays));
  }

  {
    lock_guard<mutex> lock(categoryCache);
    resources.push_back(Accountables::namedAccountable(
        L"categoryCache", BYTES_PER_CACHE_ENTRY * categoryCache->size()));
  }

  {
    lock_guard<mutex> lock(ordinalCache);
    resources.push_back(Accountables::namedAccountable(
        L"ordinalCache", BYTES_PER_CACHE_ENTRY * ordinalCache->size()));
  }

  return Collections::unmodifiableList(resources);
}

void DirectoryTaxonomyReader::setCacheSize(int size)
{
  ensureOpen();
  {
    lock_guard<mutex> lock(categoryCache);
    categoryCache->setMaxSize(size);
  }
  {
    lock_guard<mutex> lock(ordinalCache);
    ordinalCache->setMaxSize(size);
  }
}

wstring DirectoryTaxonomyReader::toString(int max)
{
  ensureOpen();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int upperl = min(max, indexReader->maxDoc());
  for (int i = 0; i < upperl; i++) {
    try {
      shared_ptr<FacetLabel> category = this->getPath(i);
      if (category == nullptr) {
        sb->append(to_wstring(i) + L": NULL!! \n");
        continue;
      }
      if (category->length == 0) {
        sb->append(to_wstring(i) + L": EMPTY STRING!! \n");
        continue;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      sb->append(to_wstring(i) + L": " + category->toString() + L"\n");
    } catch (const IOException &e) {
      if (logger->isLoggable(Level::FINEST)) {
        logger->log(Level::FINEST, e->getMessage(), e);
      }
    }
  }
  return sb->toString();
}
} // namespace org::apache::lucene::facet::taxonomy::directory