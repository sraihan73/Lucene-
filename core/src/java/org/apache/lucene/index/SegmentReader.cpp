using namespace std;

#include "SegmentReader.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;
using IOUtils = org::apache::lucene::util::IOUtils;

SegmentReader::SegmentReader(shared_ptr<SegmentCommitInfo> si,
                             int createdVersionMajor,
                             shared_ptr<IOContext> context) 
    : si(si->clone()), originalSi(si),
      metaData(make_shared<LeafMetaData>(createdVersionMajor,
                                         si->info->getMinVersion(),
                                         si->info->getIndexSort())),
      core(make_shared<SegmentCoreReaders>(si->info->dir, si, context)),
      segDocValues(make_shared<SegmentDocValues>()), isNRT(false)
{

  // We pull liveDocs/DV updates from disk:

  bool success = false;
  shared_ptr<Codec> *const codec = si->info->getCodec();
  try {
    if (si->hasDeletions()) {
      // NOTE: the bitvector is stored using the regular directory, not cfs
      liveDocs = codec->liveDocsFormat()->readLiveDocs(directory(), si,
                                                       IOContext::READONCE);
    } else {
      assert(si->getDelCount() == 0);
      liveDocs.reset();
    }
    numDocs_ = si->info->maxDoc() - si->getDelCount();

    fieldInfos = initFieldInfos();
    docValuesProducer = initDocValuesProducer();

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // With lock-less commits, it's entirely possible (and
    // fine) to hit a FileNotFound exception above.  In
    // this case, we want to explicitly close any subset
    // of things that were opened so that we don't have to
    // wait for a GC to do so.
    if (!success) {
      doClose();
    }
  }
}

SegmentReader::SegmentReader(shared_ptr<SegmentCommitInfo> si,
                             shared_ptr<SegmentReader> sr) 
    : SegmentReader(si, sr,
                    si->hasDeletions()
                        ? si->info->getCodec()->liveDocsFormat()->readLiveDocs(
                              si->info->dir, si, IOContext::READONCE)
                        : nullptr,
                    si->info->maxDoc() - si->getDelCount(), false)
{
}

SegmentReader::SegmentReader(shared_ptr<SegmentCommitInfo> si,
                             shared_ptr<SegmentReader> sr,
                             shared_ptr<Bits> liveDocs,
                             int numDocs) 
    : SegmentReader(si, sr, liveDocs, numDocs, true)
{
}

SegmentReader::SegmentReader(shared_ptr<SegmentCommitInfo> si,
                             shared_ptr<SegmentReader> sr,
                             shared_ptr<Bits> liveDocs, int numDocs,
                             bool isNRT) 
    : si(si->clone()), originalSi(si), metaData(sr->getMetaData()),
      liveDocs(liveDocs), numDocs(numDocs), core(sr->core),
      segDocValues(sr->segDocValues), isNRT(isNRT)
{
  if (numDocs > si->info->maxDoc()) {
    throw invalid_argument(L"numDocs=" + to_wstring(numDocs) + L" but maxDoc=" +
                           to_wstring(si->info->maxDoc()));
  }
  if (liveDocs != nullptr && liveDocs->length() != si->info->maxDoc()) {
    throw invalid_argument(L"maxDoc=" + to_wstring(si->info->maxDoc()) +
                           L" but liveDocs.size()=" +
                           to_wstring(liveDocs->length()));
  }
  core->incRef();

  bool success = false;
  try {
    fieldInfos = initFieldInfos();
    docValuesProducer = initDocValuesProducer();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      doClose();
    }
  }
}

shared_ptr<DocValuesProducer>
SegmentReader::initDocValuesProducer() 
{

  if (fieldInfos->hasDocValues() == false) {
    return nullptr;
  } else {
    shared_ptr<Directory> dir;
    if (core->cfsReader != nullptr) {
      dir = core->cfsReader;
    } else {
      dir = si->info->dir;
    }
    if (si->hasFieldUpdates()) {
      return make_shared<SegmentDocValuesProducer>(
          si, dir, core->coreFieldInfos, fieldInfos, segDocValues);
    } else {
      // simple case, no DocValues updates
      return segDocValues->getDocValuesProducer(-1LL, si, dir, fieldInfos);
    }
  }
}

shared_ptr<FieldInfos> SegmentReader::initFieldInfos() 
{
  if (!si->hasFieldUpdates()) {
    return core->coreFieldInfos;
  } else {
    // updates always outside of CFS
    shared_ptr<FieldInfosFormat> fisFormat =
        si->info->getCodec()->fieldInfosFormat();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring segmentSuffix =
        Long::toString(si->getFieldInfosGen(), Character::MAX_RADIX);
    return fisFormat->read(si->info->dir, si->info, segmentSuffix,
                           IOContext::READONCE);
  }
}

shared_ptr<Bits> SegmentReader::getLiveDocs()
{
  ensureOpen();
  return liveDocs;
}

void SegmentReader::doClose() 
{
  // System.out.println("SR.close seg=" + si);
  try {
    core->decRef();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (std::dynamic_pointer_cast<SegmentDocValuesProducer>(
            docValuesProducer) != nullptr) {
      segDocValues->decRef((std::static_pointer_cast<SegmentDocValuesProducer>(
                                docValuesProducer))
                               ->dvGens);
    } else if (docValuesProducer != nullptr) {
      segDocValues->decRef(Collections::singletonList(-1LL));
    }
  }
}

shared_ptr<FieldInfos> SegmentReader::getFieldInfos()
{
  ensureOpen();
  return fieldInfos;
}

int SegmentReader::numDocs()
{
  // Don't call ensureOpen() here (it could affect performance)
  return numDocs_;
}

int SegmentReader::maxDoc()
{
  // Don't call ensureOpen() here (it could affect performance)
  return si->info->maxDoc();
}

shared_ptr<TermVectorsReader> SegmentReader::getTermVectorsReader()
{
  ensureOpen();
  return core->termVectorsLocal->get();
}

shared_ptr<StoredFieldsReader> SegmentReader::getFieldsReader()
{
  ensureOpen();
  return core->fieldsReaderLocal->get();
}

shared_ptr<PointsReader> SegmentReader::getPointsReader()
{
  ensureOpen();
  return core->pointsReader;
}

shared_ptr<NormsProducer> SegmentReader::getNormsReader()
{
  ensureOpen();
  return core->normsProducer;
}

shared_ptr<DocValuesProducer> SegmentReader::getDocValuesReader()
{
  ensureOpen();
  return docValuesProducer;
}

shared_ptr<FieldsProducer> SegmentReader::getPostingsReader()
{
  ensureOpen();
  return core->fields;
}

wstring SegmentReader::toString()
{
  // SegmentInfo.toString takes dir and number of
  // *pending* deletions; so we reverse compute that here:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return si->toString(si->info->maxDoc() - numDocs_ - si->getDelCount());
}

wstring SegmentReader::getSegmentName() { return si->info->name; }

shared_ptr<SegmentCommitInfo> SegmentReader::getSegmentInfo() { return si; }

shared_ptr<Directory> SegmentReader::directory()
{
  // Don't ensureOpen here -- in certain cases, when a
  // cloned/reopened reader needs to commit, it may call
  // this method on the closed original reader
  return si->info->dir;
}

void SegmentReader::notifyReaderClosedListeners() 
{
  {
    lock_guard<mutex> lock(readerClosedListeners);
    IOUtils::applyToAll(readerClosedListeners, [&](any l) {
      l::onClose(readerCacheHelper::getKey());
    });
  }
}

SegmentReader::CacheHelperAnonymousInnerClass::CacheHelperAnonymousInnerClass()
{
}

shared_ptr<CacheKey> SegmentReader::CacheHelperAnonymousInnerClass::getKey()
{
  return cacheKey;
}

void SegmentReader::CacheHelperAnonymousInnerClass::addClosedListener(
    ClosedListener listener)
{
  outerInstance->ensureOpen();
  outerInstance->readerClosedListeners->add(listener);
}

shared_ptr<CacheHelper> SegmentReader::getReaderCacheHelper()
{
  return readerCacheHelper;
}

SegmentReader::CacheHelperAnonymousInnerClass2::
    CacheHelperAnonymousInnerClass2()
{
}

shared_ptr<CacheKey> SegmentReader::CacheHelperAnonymousInnerClass2::getKey()
{
  return outerInstance->core.getCacheHelper().getKey();
}

void SegmentReader::CacheHelperAnonymousInnerClass2::addClosedListener(
    ClosedListener listener)
{
  outerInstance->ensureOpen();
  outerInstance->core.getCacheHelper().addClosedListener(listener);
}

shared_ptr<CacheHelper> SegmentReader::getCoreCacheHelper()
{
  return coreCacheHelper;
}

shared_ptr<LeafMetaData> SegmentReader::getMetaData() { return metaData; }

shared_ptr<SegmentCommitInfo> SegmentReader::getOriginalSegmentInfo()
{
  return originalSi;
}
} // namespace org::apache::lucene::index