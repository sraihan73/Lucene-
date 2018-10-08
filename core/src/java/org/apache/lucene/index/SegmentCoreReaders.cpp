using namespace std;

#include "SegmentCoreReaders.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using CacheKey = org::apache::lucene::index::IndexReader::CacheKey;
using ClosedListener = org::apache::lucene::index::IndexReader::ClosedListener;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IOUtils = org::apache::lucene::util::IOUtils;

SegmentCoreReaders::CloseableThreadLocalAnonymousInnerClass::
    CloseableThreadLocalAnonymousInnerClass()
{
}

shared_ptr<StoredFieldsReader>
SegmentCoreReaders::CloseableThreadLocalAnonymousInnerClass::initialValue()
{
  return outerInstance->fieldsReaderOrig.clone();
}

SegmentCoreReaders::CloseableThreadLocalAnonymousInnerClass2::
    CloseableThreadLocalAnonymousInnerClass2()
{
}

shared_ptr<TermVectorsReader>
SegmentCoreReaders::CloseableThreadLocalAnonymousInnerClass2::initialValue()
{
  return (outerInstance->termVectorsReaderOrig == nullptr)
             ? nullptr
             : outerInstance->termVectorsReaderOrig.clone();
}

SegmentCoreReaders::SegmentCoreReaders(
    shared_ptr<Directory> dir, shared_ptr<SegmentCommitInfo> si,
    shared_ptr<IOContext> context) 
{

  shared_ptr<Codec> *const codec = si->info->getCodec();
  shared_ptr<Directory>
      *const cfsDir; // confusing name: if (cfs) it's the cfsdir, otherwise it's
                     // the segment's directory.
  bool success = false;

  try {
    if (si->info->getUseCompoundFile()) {
      cfsDir = cfsReader =
          codec->compoundFormat()->getCompoundReader(dir, si->info, context);
    } else {
      cfsReader.reset();
      cfsDir = dir;
    }

    segment = si->info->name;

    coreFieldInfos =
        codec->fieldInfosFormat()->read(cfsDir, si->info, L"", context);

    shared_ptr<SegmentReadState> *const segmentReadState =
        make_shared<SegmentReadState>(cfsDir, si->info, coreFieldInfos,
                                      context);
    shared_ptr<PostingsFormat> *const format = codec->postingsFormat();
    // Ask codec for its Fields
    fields = format->fieldsProducer(segmentReadState);
    assert(fields->size() > 0);
    // ask codec for its Norms:
    // TODO: since we don't write any norms file if there are no norms,
    // kinda jaky to assume the codec handles the case of no norms file at all
    // gracefully?!

    if (coreFieldInfos->hasNorms()) {
      normsProducer = codec->normsFormat()->normsProducer(segmentReadState);
      assert(normsProducer != nullptr);
    } else {
      normsProducer.reset();
    }

    fieldsReaderOrig = si->info->getCodec()->storedFieldsFormat()->fieldsReader(
        cfsDir, si->info, coreFieldInfos, context);

    if (coreFieldInfos->hasVectors()) { // open term deque files only as needed
      termVectorsReaderOrig =
          si->info->getCodec()->termVectorsFormat()->vectorsReader(
              cfsDir, si->info, coreFieldInfos, context);
    } else {
      termVectorsReaderOrig.reset();
    }

    if (coreFieldInfos->hasPointValues()) {
      pointsReader = codec->pointsFormat()->fieldsReader(segmentReadState);
    } else {
      pointsReader.reset();
    }
    success = true;
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (EOFException | FileNotFoundException e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<CorruptIndexException>(
        L"Problem reading index from " + dir, dir->toString(), e);
  } catch (const NoSuchFileException &e) {
    throw make_shared<CorruptIndexException>(L"Problem reading index.",
                                             e->getFile(), e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      decRef();
    }
  }
}

int SegmentCoreReaders::getRefCount() { return ref->get(); }

void SegmentCoreReaders::incRef()
{
  int count;
  while ((count = ref->get()) > 0) {
    if (ref->compareAndSet(count, count + 1)) {
      return;
    }
  }
  throw make_shared<AlreadyClosedException>(
      L"SegmentCoreReaders is already closed");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") void decRef() throws
// java.io.IOException
void SegmentCoreReaders::decRef() 
{
  if (ref->decrementAndGet() == 0) {
    runtime_error th = nullptr;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer =
    // this::notifyCoreClosedListeners)
    {
      java::io::Closeable finalizer =
          shared_from_this()::notifyCoreClosedListeners;
      IOUtils::close({termVectorsLocal, fieldsReaderLocal, fields,
                      termVectorsReaderOrig, fieldsReaderOrig, cfsReader,
                      normsProducer, pointsReader});
    }
  }
}

SegmentCoreReaders::CacheHelperAnonymousInnerClass::
    CacheHelperAnonymousInnerClass()
{
}

shared_ptr<CacheKey>
SegmentCoreReaders::CacheHelperAnonymousInnerClass::getKey()
{
  return cacheKey;
}

void SegmentCoreReaders::CacheHelperAnonymousInnerClass::addClosedListener(
    ClosedListener listener)
{
  outerInstance->coreClosedListeners->add(listener);
}

shared_ptr<IndexReader::CacheHelper> SegmentCoreReaders::getCacheHelper()
{
  return cacheHelper;
}

void SegmentCoreReaders::notifyCoreClosedListeners() 
{
  {
    lock_guard<mutex> lock(coreClosedListeners);
    IOUtils::applyToAll(coreClosedListeners,
                        [&](any l) { l::onClose(cacheHelper::getKey()); });
  }
}

wstring SegmentCoreReaders::toString()
{
  return L"SegmentCoreReader(" + segment + L")";
}
} // namespace org::apache::lucene::index