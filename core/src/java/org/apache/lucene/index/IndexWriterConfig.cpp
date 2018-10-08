using namespace std;

#include "IndexWriterConfig.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Field = org::apache::lucene::document::Field;
using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using InfoStream = org::apache::lucene::util::InfoStream;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using AlreadySetException =
    org::apache::lucene::util::SetOnce::AlreadySetException;
using SetOnce = org::apache::lucene::util::SetOnce;

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setIndexWriter(shared_ptr<IndexWriter> writer)
{
  if (this->writer->get() != nullptr) {
    throw make_shared<IllegalStateException>(
        L"do not share IndexWriterConfig instances across IndexWriters");
  }
  this->writer->set(writer);
  return shared_from_this();
}

IndexWriterConfig::IndexWriterConfig()
    : IndexWriterConfig(new StandardAnalyzer())
{
}

IndexWriterConfig::IndexWriterConfig(shared_ptr<Analyzer> analyzer)
    : LiveIndexWriterConfig(analyzer)
{
}

shared_ptr<IndexWriterConfig> IndexWriterConfig::setOpenMode(OpenMode openMode)
{
  if (openMode == nullptr) {
    throw invalid_argument(L"openMode must not be null");
  }
  this->openMode = openMode;
  return shared_from_this();
}

IndexWriterConfig::OpenMode IndexWriterConfig::getOpenMode()
{
  return openMode;
}

shared_ptr<IndexWriterConfig> IndexWriterConfig::setIndexDeletionPolicy(
    shared_ptr<IndexDeletionPolicy> delPolicy)
{
  if (delPolicy == nullptr) {
    throw invalid_argument(L"indexDeletionPolicy must not be null");
  }
  this->delPolicy = delPolicy;
  return shared_from_this();
}

shared_ptr<IndexDeletionPolicy> IndexWriterConfig::getIndexDeletionPolicy()
{
  return delPolicy;
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setIndexCommit(shared_ptr<IndexCommit> commit)
{
  this->commit = commit;
  return shared_from_this();
}

shared_ptr<IndexCommit> IndexWriterConfig::getIndexCommit() { return commit; }

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setSimilarity(shared_ptr<Similarity> similarity)
{
  if (similarity == nullptr) {
    throw invalid_argument(L"similarity must not be null");
  }
  this->similarity = similarity;
  return shared_from_this();
}

shared_ptr<Similarity> IndexWriterConfig::getSimilarity() { return similarity; }

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setMergeScheduler(shared_ptr<MergeScheduler> mergeScheduler)
{
  if (mergeScheduler == nullptr) {
    throw invalid_argument(L"mergeScheduler must not be null");
  }
  this->mergeScheduler = mergeScheduler;
  return shared_from_this();
}

shared_ptr<MergeScheduler> IndexWriterConfig::getMergeScheduler()
{
  return mergeScheduler;
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setCodec(shared_ptr<Codec> codec)
{
  if (codec == nullptr) {
    throw invalid_argument(L"codec must not be null");
  }
  this->codec = codec;
  return shared_from_this();
}

shared_ptr<Codec> IndexWriterConfig::getCodec() { return codec; }

shared_ptr<MergePolicy> IndexWriterConfig::getMergePolicy()
{
  return mergePolicy;
}

shared_ptr<IndexWriterConfig> IndexWriterConfig::setIndexerThreadPool(
    shared_ptr<DocumentsWriterPerThreadPool> threadPool)
{
  if (threadPool == nullptr) {
    throw invalid_argument(L"threadPool must not be null");
  }
  this->indexerThreadPool = threadPool;
  return shared_from_this();
}

shared_ptr<DocumentsWriterPerThreadPool>
IndexWriterConfig::getIndexerThreadPool()
{
  return indexerThreadPool;
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setReaderPooling(bool readerPooling)
{
  this->readerPooling = readerPooling;
  return shared_from_this();
}

bool IndexWriterConfig::getReaderPooling() { return readerPooling; }

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setFlushPolicy(shared_ptr<FlushPolicy> flushPolicy)
{
  if (flushPolicy == nullptr) {
    throw invalid_argument(L"flushPolicy must not be null");
  }
  this->flushPolicy = flushPolicy;
  return shared_from_this();
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setRAMPerThreadHardLimitMB(int perThreadHardLimitMB)
{
  if (perThreadHardLimitMB <= 0 || perThreadHardLimitMB >= 2048) {
    throw invalid_argument(
        L"PerThreadHardLimit must be greater than 0 and less than 2048MB");
  }
  this->perThreadHardLimitMB = perThreadHardLimitMB;
  return shared_from_this();
}

int IndexWriterConfig::getRAMPerThreadHardLimitMB()
{
  return perThreadHardLimitMB;
}

shared_ptr<FlushPolicy> IndexWriterConfig::getFlushPolicy()
{
  return flushPolicy;
}

shared_ptr<InfoStream> IndexWriterConfig::getInfoStream() { return infoStream; }

shared_ptr<Analyzer> IndexWriterConfig::getAnalyzer()
{
  return LiveIndexWriterConfig::getAnalyzer();
}

int IndexWriterConfig::getMaxBufferedDocs()
{
  return LiveIndexWriterConfig::getMaxBufferedDocs();
}

IndexReaderWarmer IndexWriterConfig::getMergedSegmentWarmer()
{
  return LiveIndexWriterConfig::getMergedSegmentWarmer();
}

double IndexWriterConfig::getRAMBufferSizeMB()
{
  return LiveIndexWriterConfig::getRAMBufferSizeMB();
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setInfoStream(shared_ptr<InfoStream> infoStream)
{
  if (infoStream == nullptr) {
    throw invalid_argument(
        wstring(L"Cannot set InfoStream implementation to null. ") +
        L"To disable logging use InfoStream.NO_OUTPUT");
  }
  this->infoStream = infoStream;
  return shared_from_this();
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setInfoStream(shared_ptr<PrintStream> printStream)
{
  if (printStream == nullptr) {
    throw invalid_argument(L"printStream must not be null");
  }
  return setInfoStream(make_shared<PrintStreamInfoStream>(printStream));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setMergePolicy(shared_ptr<MergePolicy> mergePolicy)
{
  return std::static_pointer_cast<IndexWriterConfig>(
      LiveIndexWriterConfig::setMergePolicy(mergePolicy));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setMaxBufferedDocs(int maxBufferedDocs)
{
  return std::static_pointer_cast<IndexWriterConfig>(
      LiveIndexWriterConfig::setMaxBufferedDocs(maxBufferedDocs));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setMergedSegmentWarmer(IndexReaderWarmer mergeSegmentWarmer)
{
  return std::static_pointer_cast<IndexWriterConfig>(
      LiveIndexWriterConfig::setMergedSegmentWarmer(mergeSegmentWarmer));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setRAMBufferSizeMB(double ramBufferSizeMB)
{
  return std::static_pointer_cast<IndexWriterConfig>(
      LiveIndexWriterConfig::setRAMBufferSizeMB(ramBufferSizeMB));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setUseCompoundFile(bool useCompoundFile)
{
  return std::static_pointer_cast<IndexWriterConfig>(
      LiveIndexWriterConfig::setUseCompoundFile(useCompoundFile));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setCommitOnClose(bool commitOnClose)
{
  this->commitOnClose = commitOnClose;
  return shared_from_this();
}

const shared_ptr<
    java::util::EnumSet<org::apache::lucene::search::SortField::Type>>
    IndexWriterConfig::ALLOWED_INDEX_SORT_TYPES = java::util::EnumSet::of(
        org::apache::lucene::search::SortField::Type::STRING,
        org::apache::lucene::search::SortField::Type::LONG,
        org::apache::lucene::search::SortField::Type::INT,
        org::apache::lucene::search::SortField::Type::DOUBLE,
        org::apache::lucene::search::SortField::Type::FLOAT);

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setIndexSort(shared_ptr<Sort> sort)
{
  for (auto sortField : sort->getSort()) {
    constexpr SortField::Type sortType = Sorter::getSortFieldType(sortField);
    if (ALLOWED_INDEX_SORT_TYPES->contains(sortType) == false) {
      throw invalid_argument(L"invalid SortField type: must be one of " +
                             ALLOWED_INDEX_SORT_TYPES + L" but got: " +
                             sortField);
    }
  }
  this->indexSort = sort;
  this->indexSortFields = Arrays::stream(sort->getSort())
                              .map_obj(SortField::getField)
                              .collect(Collectors::toSet());
  return shared_from_this();
}

wstring IndexWriterConfig::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(LiveIndexWriterConfig::toString());
  sb->append(L"writer=")->append(writer->get())->append(L"\n");
  return sb->toString();
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setCheckPendingFlushUpdate(bool checkPendingFlushOnUpdate)
{
  return std::static_pointer_cast<IndexWriterConfig>(
      LiveIndexWriterConfig::setCheckPendingFlushUpdate(
          checkPendingFlushOnUpdate));
}

shared_ptr<IndexWriterConfig>
IndexWriterConfig::setSoftDeletesField(const wstring &softDeletesField)
{
  this->softDeletesField = softDeletesField;
  return shared_from_this();
}
} // namespace org::apache::lucene::index