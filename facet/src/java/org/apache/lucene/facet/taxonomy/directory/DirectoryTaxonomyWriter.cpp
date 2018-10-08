using namespace std;

#include "DirectoryTaxonomyWriter.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using UTF8TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::UTF8TaxonomyWriterCache;
using LruTaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::LruTaxonomyWriterCache;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LogByteSizeMergePolicy =
    org::apache::lucene::index::LogByteSizeMergePolicy;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using ReaderManager = org::apache::lucene::index::ReaderManager;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TieredMergePolicy = org::apache::lucene::index::TieredMergePolicy;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using BytesRef = org::apache::lucene::util::BytesRef;
using FutureObjects = org::apache::lucene::util::FutureObjects;
const wstring DirectoryTaxonomyWriter::INDEX_EPOCH = L"index.epoch";

unordered_map<wstring, wstring> DirectoryTaxonomyWriter::readCommitData(
    shared_ptr<Directory> dir) 
{
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
  return infos->getUserData();
}

DirectoryTaxonomyWriter::DirectoryTaxonomyWriter(
    shared_ptr<Directory> directory, OpenMode openMode,
    shared_ptr<TaxonomyWriterCache> cache) 
    : dir(directory), indexWriter(openIndexWriter(dir, config))
{

  shared_ptr<IndexWriterConfig> config = createIndexWriterConfig(openMode);

  // verify (to some extent) that merge policy in effect would preserve category
  // docids
  assert((!(std::dynamic_pointer_cast<TieredMergePolicy>(
                indexWriter->getConfig()->getMergePolicy()) != nullptr),
          L"for preserving category docids, merging none-adjacent segments is "
          L"not allowed"));

  // after we opened the writer, and the index is locked, it's safe to check
  // the commit data and read the index epoch
  openMode = config->getOpenMode();
  if (!DirectoryReader::indexExists(directory)) {
    indexEpoch = 1;
  } else {
    wstring epochStr = L"";
    unordered_map<wstring, wstring> commitData = readCommitData(directory);
    if (commitData.size() > 0) {
      epochStr = commitData[INDEX_EPOCH];
    }
    // no commit data, or no epoch in it means an old taxonomy, so set its epoch
    // to 1, for lack of a better value.
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: indexEpoch = epochStr == null ? 1 :
    // Long.parseLong(epochStr, 16);
    indexEpoch = epochStr == L"" ? 1 : int64_t ::valueOf(epochStr, 16);
  }

  if (openMode == OpenMode::CREATE) {
    ++indexEpoch;
  }

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setOmitNorms(true);
  parentStreamField =
      make_shared<Field>(Consts::FIELD_PAYLOADS, parentStream, ft);
  fullPathField =
      make_shared<StringField>(Consts::FULL, L"", Field::Store::YES);

  nextID = indexWriter->maxDoc();

  if (cache == nullptr) {
    cache = defaultTaxonomyWriterCache();
  }
  this->cache = cache;

  if (nextID == 0) {
    cacheIsComplete = true;
    // Make sure that the taxonomy always contain the root category
    // with category id 0.
    addCategory(make_shared<FacetLabel>());
  } else {
    // There are some categories on the disk, which we have not yet
    // read into the cache, and therefore the cache is incomplete.
    // We choose not to read all the categories into the cache now,
    // to avoid terrible performance when a taxonomy index is opened
    // to add just a single category. We will do it later, after we
    // notice a few cache misses.
    cacheIsComplete = false;
  }
}

shared_ptr<IndexWriter> DirectoryTaxonomyWriter::openIndexWriter(
    shared_ptr<Directory> directory,
    shared_ptr<IndexWriterConfig> config) 
{
  return make_shared<IndexWriter>(directory, config);
}

shared_ptr<IndexWriterConfig>
DirectoryTaxonomyWriter::createIndexWriterConfig(OpenMode openMode)
{
  // TODO: should we use a more optimized Codec?
  // The taxonomy has a unique structure, where each term is associated with one
  // document

  // Make sure we use a MergePolicy which always merges adjacent segments and
  // thus keeps the doc IDs ordered as well (this is crucial for the taxonomy
  // index).
  return (make_shared<IndexWriterConfig>(nullptr))
      ->setOpenMode(openMode)
      ->setMergePolicy(make_shared<LogByteSizeMergePolicy>());
}

void DirectoryTaxonomyWriter::initReaderManager() 
{
  if (!initializedReaderManager) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      // verify that the taxo-writer hasn't been closed on us.
      ensureOpen();
      if (!initializedReaderManager) {
        readerManager = make_shared<ReaderManager>(indexWriter, false, false);
        shouldRefreshReaderManager = false;
        initializedReaderManager = true;
      }
    }
  }
}

DirectoryTaxonomyWriter::DirectoryTaxonomyWriter(
    shared_ptr<Directory> directory, OpenMode openMode) 
    : DirectoryTaxonomyWriter(directory, openMode, defaultTaxonomyWriterCache())
{
}

shared_ptr<TaxonomyWriterCache>
DirectoryTaxonomyWriter::defaultTaxonomyWriterCache()
{
  return make_shared<UTF8TaxonomyWriterCache>();
}

DirectoryTaxonomyWriter::DirectoryTaxonomyWriter(shared_ptr<Directory> d) throw(
    IOException)
    : DirectoryTaxonomyWriter(d, OpenMode::CREATE_OR_APPEND)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
DirectoryTaxonomyWriter::~DirectoryTaxonomyWriter()
{
  if (!isClosed) {
    commit();
    delete indexWriter;
    doClose();
  }
}

void DirectoryTaxonomyWriter::doClose() 
{
  isClosed = true;
  closeResources();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirectoryTaxonomyWriter::closeResources() 
{
  if (initializedReaderManager) {
    delete readerManager;
    readerManager.reset();
    initializedReaderManager = false;
  }
  if (cache != nullptr) {
    cache->close();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DirectoryTaxonomyWriter::findCategory(
    shared_ptr<FacetLabel> categoryPath) 
{
  // If we can find the category in the cache, or we know the cache is
  // complete, we can return the response directly from it
  int res = cache->get(categoryPath);
  if (res >= 0 || cacheIsComplete) {
    return res;
  }

  cacheMisses->incrementAndGet();
  // After a few cache misses, it makes sense to read all the categories
  // from disk and into the cache. The reason not to do this on the first
  // cache miss (or even when opening the writer) is that it will
  // significantly slow down the case when a taxonomy is opened just to
  // add one category. The idea only spending a long time on reading
  // after enough time was spent on cache misses is known as an "online
  // algorithm".
  perhapsFillCache();
  res = cache->get(categoryPath);
  if (res >= 0 || cacheIsComplete) {
    // if after filling the cache from the info on disk, the category is in it
    // or the cache is complete, return whatever cache.get returned.
    return res;
  }

  // if we get here, it means the category is not in the cache, and it is not
  // complete, and therefore we must look for the category on disk.

  // We need to get an answer from the on-disk index.
  initReaderManager();

  int doc = -1;
  shared_ptr<DirectoryReader> reader = readerManager->acquire();
  try {
    shared_ptr<BytesRef> *const catTerm =
        make_shared<BytesRef>(FacetsConfig::pathToString(
            categoryPath->components, categoryPath->length));
    shared_ptr<PostingsEnum> docs = nullptr; // reuse
    for (shared_ptr<LeafReaderContext> ctx : reader->leaves()) {
      shared_ptr<Terms> terms = ctx->reader()->terms(Consts::FULL);
      if (terms != nullptr) {
        // TODO: share per-segment TermsEnum here!
        shared_ptr<TermsEnum> termsEnum = terms->begin();
        if (termsEnum->seekExact(catTerm)) {
          // liveDocs=null because the taxonomy has no deletes
          docs = termsEnum->postings(docs, 0);
          // if the term was found, we know it has exactly one document.
          doc = docs->nextDoc() + ctx->docBase;
          break;
        }
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    readerManager->release(reader);
  }
  if (doc > 0) {
    addToCache(categoryPath, doc);
  }
  return doc;
}

int DirectoryTaxonomyWriter::addCategory(
    shared_ptr<FacetLabel> categoryPath) 
{
  ensureOpen();
  // check the cache outside the synchronized block. this results in better
  // concurrency when categories are there.
  int res = cache->get(categoryPath);
  if (res < 0) {
    // the category is not in the cache - following code cannot be executed in
    // parallel.
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      res = findCategory(categoryPath);
      if (res < 0) {
        // This is a new category, and we need to insert it into the index
        // (and the cache). Actually, we might also need to add some of
        // the category's ancestors before we can add the category itself
        // (while keeping the invariant that a parent is always added to
        // the taxonomy before its child). internalAddCategory() does all
        // this recursively
        res = internalAddCategory(categoryPath);
      }
    }
  }
  return res;
}

int DirectoryTaxonomyWriter::internalAddCategory(
    shared_ptr<FacetLabel> cp) 
{
  // Find our parent's ordinal (recursively adding the parent category
  // to the taxonomy if it's not already there). Then add the parent
  // ordinal as payloads (rather than a stored field; payloads can be
  // more efficiently read into memory in bulk by LuceneTaxonomyReader)
  int parent;
  if (cp->length > 1) {
    shared_ptr<FacetLabel> parentPath = cp->subpath(cp->length - 1);
    parent = findCategory(parentPath);
    if (parent < 0) {
      parent = internalAddCategory(parentPath);
    }
  } else if (cp->length == 1) {
    parent = TaxonomyReader::ROOT_ORDINAL;
  } else {
    parent = TaxonomyReader::INVALID_ORDINAL;
  }
  int id = addCategoryDocument(cp, parent);

  return id;
}

void DirectoryTaxonomyWriter::ensureOpen()
{
  if (isClosed) {
    throw make_shared<AlreadyClosedException>(
        L"The taxonomy writer has already been closed");
  }
}

int DirectoryTaxonomyWriter::addCategoryDocument(
    shared_ptr<FacetLabel> categoryPath, int parent) 
{
  // Before Lucene 2.9, position increments >=0 were supported, so we
  // added 1 to parent to allow the parent -1 (the parent of the root).
  // Unfortunately, starting with Lucene 2.9, after LUCENE-1542, this is
  // no longer enough, since 0 is not encoded consistently either (see
  // comment in SinglePositionTokenStream). But because we must be
  // backward-compatible with existing indexes, we can't just fix what
  // we write here (e.g., to write parent+2), and need to do a workaround
  // in the reader (which knows that anyway only category 0 has a parent
  // -1).
  parentStream->set(max(parent + 1, 1));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(parentStreamField);

  fullPathField->setStringValue(FacetsConfig::pathToString(
      categoryPath->components, categoryPath->length));
  d->push_back(fullPathField);

  // Note that we do no pass an Analyzer here because the fields that are
  // added to the Document are untokenized or contains their own TokenStream.
  // Therefore the IndexWriter's Analyzer has no effect.
  indexWriter->addDocument(d);
  int id = nextID++;

  // added a category document, mark that ReaderManager is not up-to-date
  shouldRefreshReaderManager = true;

  // also add to the parent array
  taxoArrays = getTaxoArrays()->add(id, parent);

  // NOTE: this line must be executed last, or else the cache gets updated
  // before the parents array (LUCENE-4596)
  addToCache(categoryPath, id);

  return id;
}

DirectoryTaxonomyWriter::SinglePositionTokenStream::SinglePositionTokenStream(
    const wstring &word)
    : word(word)
{
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  returned = true;
}

void DirectoryTaxonomyWriter::SinglePositionTokenStream::set(int val)
{
  this->val = val;
  returned = false;
}

bool DirectoryTaxonomyWriter::SinglePositionTokenStream::incrementToken() throw(
    IOException)
{
  if (returned) {
    return false;
  }
  clearAttributes();
  posIncrAtt->setPositionIncrement(val);
  termAtt->setEmpty();
  termAtt->append(word);
  returned = true;
  return true;
}

void DirectoryTaxonomyWriter::addToCache(shared_ptr<FacetLabel> categoryPath,
                                         int id) 
{
  if (cache->put(categoryPath, id)) {
    // If cache.put() returned true, it means the cache was limited in
    // size, became full, and parts of it had to be evicted. It is
    // possible that a relatively-new category that isn't yet visible
    // to our 'reader' was evicted, and therefore we must now refresh
    // the reader.
    refreshReaderManager();
    cacheIsComplete = false;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirectoryTaxonomyWriter::refreshReaderManager() 
{
  // this method is synchronized since it cannot happen concurrently with
  // addCategoryDocument -- when this method returns, we must know that the
  // reader manager's state is current. also, it sets shouldRefresh to false,
  // and this cannot overlap with addCatDoc too.
  // NOTE: since this method is sync'ed, it can call maybeRefresh, instead of
  // maybeRefreshBlocking. If ever this is changed, make sure to change the
  // call too.
  if (shouldRefreshReaderManager && initializedReaderManager) {
    readerManager->maybeRefresh();
    shouldRefreshReaderManager = false;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DirectoryTaxonomyWriter::commit() 
{
  ensureOpen();
  // LUCENE-4972: if we always call setCommitData, we create empty commits

  unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
  deque<unordered_map::Entry<wstring, wstring>> iter =
      indexWriter->getLiveCommitData();
  if (iter.size() > 0) {
    for (auto ent : iter) {
      data.emplace(ent.first, ent.second);
    }
  }

  wstring epochStr = data[INDEX_EPOCH];
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: if (epochStr == null || Long.parseLong(epochStr, 16) !=
  // indexEpoch)
  if (epochStr == L"" || int64_t ::valueOf(epochStr, 16) != indexEpoch) {
    indexWriter->setLiveCommitData(
        combinedCommitData(indexWriter->getLiveCommitData()));
  }
  return indexWriter->commit();
}

deque<unordered_map::Entry<wstring, wstring>>
DirectoryTaxonomyWriter::combinedCommitData(
    deque<unordered_map::Entry<wstring, wstring>> &commitData)
{
  unordered_map<wstring, wstring> m = unordered_map<wstring, wstring>();
  if (commitData.size() > 0) {
    for (auto ent : commitData) {
      m.emplace(ent.first, ent.second);
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  m.emplace(INDEX_EPOCH, Long::toString(indexEpoch, 16));
  return m.entrySet();
}

void DirectoryTaxonomyWriter::setLiveCommitData(
    deque<unordered_map::Entry<wstring, wstring>> &commitUserData)
{
  indexWriter->setLiveCommitData(combinedCommitData(commitUserData));
}

deque<unordered_map::Entry<wstring, wstring>>
DirectoryTaxonomyWriter::getLiveCommitData()
{
  return combinedCommitData(indexWriter->getLiveCommitData());
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DirectoryTaxonomyWriter::prepareCommit() 
{
  ensureOpen();
  // LUCENE-4972: if we always call setCommitData, we create empty commits
  unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
  deque<unordered_map::Entry<wstring, wstring>> iter =
      indexWriter->getLiveCommitData();
  if (iter.size() > 0) {
    for (auto ent : iter) {
      data.emplace(ent.first, ent.second);
    }
  }
  wstring epochStr = data[INDEX_EPOCH];
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: if (epochStr == null || Long.parseLong(epochStr, 16) !=
  // indexEpoch)
  if (epochStr == L"" || int64_t ::valueOf(epochStr, 16) != indexEpoch) {
    indexWriter->setLiveCommitData(
        combinedCommitData(indexWriter->getLiveCommitData()));
  }
  return indexWriter->prepareCommit();
}

int DirectoryTaxonomyWriter::getSize()
{
  ensureOpen();
  return nextID;
}

void DirectoryTaxonomyWriter::setCacheMissesUntilFill(int i)
{
  ensureOpen();
  cacheMissesUntilFill = i;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirectoryTaxonomyWriter::perhapsFillCache() 
{
  if (cacheMisses->get() < cacheMissesUntilFill) {
    return;
  }

  if (!shouldFillCache) {
    // we already filled the cache once, there's no need to re-fill it
    return;
  }
  shouldFillCache = false;

  initReaderManager();

  bool aborted = false;
  shared_ptr<DirectoryReader> reader = readerManager->acquire();
  try {
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    for (shared_ptr<LeafReaderContext> ctx : reader->leaves()) {
      shared_ptr<Terms> terms = ctx->reader()->terms(Consts::FULL);
      if (terms != nullptr) { // cannot really happen, but be on the safe side
        // TODO: share per-segment TermsEnum here!
        shared_ptr<TermsEnum> termsEnum = terms->begin();
        while (termsEnum->next() != nullptr) {
          if (!cache->isFull()) {
            shared_ptr<BytesRef> t = termsEnum->term();
            // Since we guarantee uniqueness of categories, each term has
            // exactly one document. Also, since we do not allow removing
            // categories (and hence documents), there are no deletions in the
            // index. Therefore, it is sufficient to call next(), and then
            // doc(), exactly once with no 'validation' checks.
            shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(
                FacetsConfig::stringToPath(t->utf8ToString()));
            postingsEnum =
                termsEnum->postings(postingsEnum, PostingsEnum::NONE);
            bool res = cache->put(cp, postingsEnum->nextDoc() + ctx->docBase);
            assert(
                (!res, L"entries should not have been evicted from the cache"));
          } else {
            // the cache is full and the next put() will evict entries from it,
            // therefore abort the iteration.
            aborted = true;
            break;
          }
        }
      }
      if (aborted) {
        break;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    readerManager->release(reader);
  }

  cacheIsComplete = !aborted;
  if (cacheIsComplete) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      // everything is in the cache, so no need to keep readerManager open.
      // this block is executed in a sync block so that it works well with
      // initReaderManager called in parallel.
      delete readerManager;
      readerManager.reset();
      initializedReaderManager = false;
    }
  }
}

shared_ptr<TaxonomyIndexArrays>
DirectoryTaxonomyWriter::getTaxoArrays() 
{
  if (taxoArrays == nullptr) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      if (taxoArrays == nullptr) {
        initReaderManager();
        shared_ptr<DirectoryReader> reader = readerManager->acquire();
        try {
          // according to Java Concurrency, this might perform better on some
          // JVMs, since the object initialization doesn't happen on the
          // volatile member.
          shared_ptr<TaxonomyIndexArrays> tmpArrays =
              make_shared<TaxonomyIndexArrays>(reader);
          taxoArrays = tmpArrays;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          readerManager->release(reader);
        }
      }
    }
  }
  return taxoArrays;
}

int DirectoryTaxonomyWriter::getParent(int ordinal) 
{
  ensureOpen();
  // Note: the following if() just enforces that a user can never ask
  // for the parent of a nonexistant category - even if the parent array
  // was allocated bigger than it really needs to be.
  FutureObjects::checkIndex(ordinal, nextID);

  std::deque<int> parents = getTaxoArrays()->parents();
  assert((ordinal < parents.size(),
          L"requested ordinal (" + to_wstring(ordinal) +
              L"); parents.length (" + parents.size() + L") !"));
  return parents[ordinal];
}

void DirectoryTaxonomyWriter::addTaxonomy(
    shared_ptr<Directory> taxoDir,
    shared_ptr<OrdinalMap> map_obj) 
{
  ensureOpen();
  shared_ptr<DirectoryReader> r = DirectoryReader::open(taxoDir);
  try {
    constexpr int size = r->numDocs();
    shared_ptr<OrdinalMap> *const ordinalMap = map_obj;
    ordinalMap->setSize(size);
    int base = 0;
    shared_ptr<PostingsEnum> docs = nullptr;
    for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
      shared_ptr<LeafReader> *const ar = ctx->reader();
      shared_ptr<Terms> *const terms = ar->terms(Consts::FULL);
      // TODO: share per-segment TermsEnum here!
      shared_ptr<TermsEnum> te = terms->begin();
      while (te->next() != nullptr) {
        shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(
            FacetsConfig::stringToPath(te->term()->utf8ToString()));
        constexpr int ordinal = addCategory(cp);
        docs = te->postings(docs, PostingsEnum::NONE);
        ordinalMap->addMapping(docs->nextDoc() + base, ordinal);
      }
      base += ar->maxDoc(); // no deletions, so we're ok
    }
    ordinalMap->addDone();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    r->close();
  }
}

DirectoryTaxonomyWriter::MemoryOrdinalMap::MemoryOrdinalMap() {}

void DirectoryTaxonomyWriter::MemoryOrdinalMap::setSize(int taxonomySize)
{
  map_obj = std::deque<int>(taxonomySize);
}

void DirectoryTaxonomyWriter::MemoryOrdinalMap::addMapping(int origOrdinal,
                                                           int newOrdinal)
{
  map_obj[origOrdinal] = newOrdinal;
}

void DirectoryTaxonomyWriter::MemoryOrdinalMap::addDone() {}

std::deque<int> DirectoryTaxonomyWriter::MemoryOrdinalMap::getMap()
{
  return map_obj;
}

DirectoryTaxonomyWriter::DiskOrdinalMap::DiskOrdinalMap(
    shared_ptr<Path> tmpfile) 
{
  this->tmpfile = tmpfile;
  out = make_shared<DataOutputStream>(
      make_shared<BufferedOutputStream>(Files::newOutputStream(tmpfile)));
}

void DirectoryTaxonomyWriter::DiskOrdinalMap::addMapping(
    int origOrdinal, int newOrdinal) 
{
  out->writeInt(origOrdinal);
  out->writeInt(newOrdinal);
}

void DirectoryTaxonomyWriter::DiskOrdinalMap::setSize(int taxonomySize) throw(
    IOException)
{
  out->writeInt(taxonomySize);
}

void DirectoryTaxonomyWriter::DiskOrdinalMap::addDone() 
{
  if (out != nullptr) {
    out->close();
    out.reset();
  }
}

std::deque<int>
DirectoryTaxonomyWriter::DiskOrdinalMap::getMap() 
{
  if (map_obj.size() > 0) {
    return map_obj;
  }
  addDone(); // in case this wasn't previously called
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.DataInputStream in = new
  // java.io.DataInputStream(new
  // java.io.BufferedInputStream(java.nio.file.Files.newInputStream(tmpfile))))
  {
    java::io::DataInputStream in_ =
        java::io::DataInputStream(make_shared<java::io::BufferedInputStream>(
            java::nio::file::Files::newInputStream(tmpfile)));
    map_obj = std::deque<int>(in_.readInt());
    // NOTE: The current code assumes here that the map_obj is complete,
    // i.e., every ordinal gets one and exactly one value. Otherwise,
    // we may run into an EOF here, or vice versa, not read everything.
    for (int i = 0; i < map_obj.size(); i++) {
      int origordinal = in_.readInt();
      int newordinal = in_.readInt();
      map_obj[origordinal] = newordinal;
    }
  }

  // Delete the temporary file, which is no longer needed.
  Files::delete (tmpfile);

  return map_obj;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirectoryTaxonomyWriter::rollback() 
{
  ensureOpen();
  indexWriter->rollback();
  doClose();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirectoryTaxonomyWriter::replaceTaxonomy(
    shared_ptr<Directory> taxoDir) 
{
  // replace the taxonomy by doing IW optimized operations
  indexWriter->deleteAll();
  indexWriter->addIndexes({taxoDir});
  shouldRefreshReaderManager = true;
  initReaderManager(); // ensure that it's initialized
  refreshReaderManager();
  nextID = indexWriter->maxDoc();
  taxoArrays
      .reset(); // must nullify so that it's re-computed next time it's needed

  // need to clear the cache, so that addCategory won't accidentally return
  // old categories that are in the cache.
  cache->clear();
  cacheIsComplete = false;
  shouldFillCache = true;
  cacheMisses->set(0);

  // update indexEpoch as a taxonomy replace is just like it has be recreated
  ++indexEpoch;
}

shared_ptr<Directory> DirectoryTaxonomyWriter::getDirectory() { return dir; }

shared_ptr<IndexWriter> DirectoryTaxonomyWriter::getInternalIndexWriter()
{
  return indexWriter;
}

int64_t DirectoryTaxonomyWriter::getTaxonomyEpoch() { return indexEpoch; }
} // namespace org::apache::lucene::facet::taxonomy::directory