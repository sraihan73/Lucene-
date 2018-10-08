using namespace std;

#include "SoftDeletesDirectoryReaderWrapper.h"

namespace org::apache::lucene::index
{
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

SoftDeletesDirectoryReaderWrapper::SoftDeletesDirectoryReaderWrapper(
    shared_ptr<DirectoryReader> in_, const wstring &field) 
    : SoftDeletesDirectoryReaderWrapper(
          in_, new SoftDeletesSubReaderWrapper(Collections::emptyMap(), field))
{
}

SoftDeletesDirectoryReaderWrapper::SoftDeletesDirectoryReaderWrapper(
    shared_ptr<DirectoryReader> in_,
    shared_ptr<SoftDeletesSubReaderWrapper> wrapper) 
    : FilterDirectoryReader(in_, wrapper), field(wrapper->field),
      readerCacheHelper(
          in_->getReaderCacheHelper() == nullptr
              ? nullptr
              : make_shared<DelegatingCacheHelper>(in_->getReaderCacheHelper()))
{
}

shared_ptr<DirectoryReader>
SoftDeletesDirectoryReaderWrapper::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  unordered_map<std::shared_ptr<CacheKey>, std::shared_ptr<LeafReader>>
      readerCache = unordered_map<std::shared_ptr<CacheKey>,
                                  std::shared_ptr<LeafReader>>();
  for (auto reader : getSequentialSubReaders()) {
    // we try to reuse the life docs instances here if the reader cache key
    // didn't change
    if (std::dynamic_pointer_cast<SoftDeletesFilterLeafReader>(reader) !=
            nullptr &&
        reader->getReaderCacheHelper() != nullptr) {
      readerCache.emplace(
          (std::static_pointer_cast<SoftDeletesFilterLeafReader>(reader))
              ->reader.getReaderCacheHelper()
              .getKey(),
          reader);
    } else if (std::dynamic_pointer_cast<SoftDeletesFilterCodecReader>(
                   reader) != nullptr &&
               reader->getReaderCacheHelper() != nullptr) {
      readerCache.emplace(
          (std::static_pointer_cast<SoftDeletesFilterCodecReader>(reader))
              ->reader.getReaderCacheHelper()
              .getKey(),
          reader);
    }
  }
  return make_shared<SoftDeletesDirectoryReaderWrapper>(
      in_, make_shared<SoftDeletesSubReaderWrapper>(readerCache, field));
}

shared_ptr<CacheHelper>
SoftDeletesDirectoryReaderWrapper::getReaderCacheHelper()
{
  return readerCacheHelper;
}

SoftDeletesDirectoryReaderWrapper::SoftDeletesSubReaderWrapper::
    SoftDeletesSubReaderWrapper(
        unordered_map<std::shared_ptr<CacheKey>, std::shared_ptr<LeafReader>>
            &oldReadersCache,
        const wstring &field)
    : mapping(oldReadersCache), field(field)
{
  Objects::requireNonNull(field, L"Field must not be null");
  assert(oldReadersCache.size() > 0);
}

shared_ptr<LeafReader>
SoftDeletesDirectoryReaderWrapper::SoftDeletesSubReaderWrapper::wrap(
    shared_ptr<LeafReader> reader)
{
  shared_ptr<CacheHelper> readerCacheHelper = reader->getReaderCacheHelper();
  if (readerCacheHelper != nullptr &&
      mapping.find(readerCacheHelper->getKey()) != mapping.end()) {
    // if the reader cache helper didn't change and we have it in the cache
    // don't bother creating a new one
    return mapping[readerCacheHelper->getKey()];
  }
  try {
    return SoftDeletesDirectoryReaderWrapper::wrap(reader, field);
  } catch (const IOException &e) {
    throw make_shared<UncheckedIOException>(e);
  }
}

shared_ptr<LeafReader>
SoftDeletesDirectoryReaderWrapper::wrap(shared_ptr<LeafReader> reader,
                                        const wstring &field) 
{
  shared_ptr<DocIdSetIterator> iterator =
      DocValuesFieldExistsQuery::getDocValuesDocIdSetIterator(field, reader);
  if (iterator == nullptr) {
    return reader;
  }
  shared_ptr<Bits> liveDocs = reader->getLiveDocs();
  shared_ptr<FixedBitSet> *const bits;
  if (liveDocs != nullptr) {
    bits = SoftDeletesRetentionMergePolicy::cloneLiveDocs(liveDocs);
  } else {
    bits = make_shared<FixedBitSet>(reader->maxDoc());
    bits->set(0, reader->maxDoc());
  }
  int numSoftDeletes = PendingSoftDeletes::applySoftDeletes(iterator, bits);
  int numDeletes = reader->numDeletedDocs() + numSoftDeletes;
  int numDocs = reader->maxDoc() - numDeletes;
  assert((assertDocCounts(numDocs, numSoftDeletes, reader)));
  return std::dynamic_pointer_cast<CodecReader>(reader) != nullptr
             ? make_shared<SoftDeletesFilterCodecReader>(
                   std::static_pointer_cast<CodecReader>(reader), bits, numDocs)
             : make_shared<SoftDeletesFilterLeafReader>(reader, bits, numDocs);
}

bool SoftDeletesDirectoryReaderWrapper::assertDocCounts(
    int expectedNumDocs, int numSoftDeletes, shared_ptr<LeafReader> reader)
{
  if (std::dynamic_pointer_cast<SegmentReader>(reader) != nullptr) {
    shared_ptr<SegmentReader> segmentReader =
        std::static_pointer_cast<SegmentReader>(reader);
    shared_ptr<SegmentCommitInfo> segmentInfo = segmentReader->getSegmentInfo();
    if (segmentReader->isNRT == false) {
      int numDocs = segmentInfo->info->maxDoc() -
                    segmentInfo->getSoftDelCount() - segmentInfo->getDelCount();
      assert((numDocs == expectedNumDocs,
              L"numDocs: " + to_wstring(numDocs) + L" expected: " +
                  to_wstring(expectedNumDocs) + L" maxDoc: " +
                  to_wstring(segmentInfo->info->maxDoc()) + L" getDelCount: " +
                  to_wstring(segmentInfo->getDelCount()) +
                  L" getSoftDelCount: " +
                  to_wstring(segmentInfo->getSoftDelCount()) +
                  L" numSoftDeletes: " + to_wstring(numSoftDeletes) +
                  L" reader.numDeletedDocs(): " +
                  to_wstring(reader->numDeletedDocs())));
    }
    // in the NRT case we don't have accurate numbers for getDelCount and
    // getSoftDelCount since they might not be flushed to disk when this reader
    // is opened. We don't necessarily flush deleted doc on reopen but we do for
    // docValues.
  }

  return true;
}

SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterLeafReader::
    SoftDeletesFilterLeafReader(shared_ptr<LeafReader> reader,
                                shared_ptr<FixedBitSet> bits, int numDocs)
    : FilterLeafReader(reader), reader(reader), bits(bits), numDocs(numDocs),
      readerCacheHelper(reader->getReaderCacheHelper() == nullptr
                            ? nullptr
                            : make_shared<DelegatingCacheHelper>(
                                  reader->getReaderCacheHelper()))
{
}

shared_ptr<Bits>
SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterLeafReader::getLiveDocs()
{
  return bits;
}

int SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterLeafReader::numDocs()
{
  return numDocs_;
}

shared_ptr<CacheHelper> SoftDeletesDirectoryReaderWrapper::
    SoftDeletesFilterLeafReader::getCoreCacheHelper()
{
  return reader->getCoreCacheHelper();
}

shared_ptr<CacheHelper> SoftDeletesDirectoryReaderWrapper::
    SoftDeletesFilterLeafReader::getReaderCacheHelper()
{
  return readerCacheHelper;
}

SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterCodecReader::
    SoftDeletesFilterCodecReader(shared_ptr<CodecReader> reader,
                                 shared_ptr<FixedBitSet> bits, int numDocs)
    : FilterCodecReader(reader), reader(reader), bits(bits), numDocs(numDocs),
      readerCacheHelper(reader->getReaderCacheHelper() == nullptr
                            ? nullptr
                            : make_shared<DelegatingCacheHelper>(
                                  reader->getReaderCacheHelper()))
{
}

shared_ptr<Bits>
SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterCodecReader::getLiveDocs()
{
  return bits;
}

int SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterCodecReader::numDocs()
{
  return numDocs_;
}

shared_ptr<CacheHelper> SoftDeletesDirectoryReaderWrapper::
    SoftDeletesFilterCodecReader::getCoreCacheHelper()
{
  return reader->getCoreCacheHelper();
}

shared_ptr<CacheHelper> SoftDeletesDirectoryReaderWrapper::
    SoftDeletesFilterCodecReader::getReaderCacheHelper()
{
  return readerCacheHelper;
}

SoftDeletesDirectoryReaderWrapper::DelegatingCacheHelper::DelegatingCacheHelper(
    shared_ptr<CacheHelper> delegate_)
    : delegate_(delegate_)
{
}

shared_ptr<CacheKey>
SoftDeletesDirectoryReaderWrapper::DelegatingCacheHelper::getKey()
{
  return cacheKey;
}

void SoftDeletesDirectoryReaderWrapper::DelegatingCacheHelper::
    addClosedListener(shared_ptr<ClosedListener> listener)
{
  // here we wrap the listener and call it with our cache key
  // this is important since this key will be used to cache the reader and
  // otherwise we won't free caches etc.
  delegate_->addClosedListener(
      [&](any unused) { listener->onClose(cacheKey); });
}
} // namespace org::apache::lucene::index