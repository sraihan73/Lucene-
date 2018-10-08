using namespace std;

#include "LiveIndexWriterConfig.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Codec = org::apache::lucene::codecs::Codec;
using IndexingChain =
    org::apache::lucene::index::DocumentsWriterPerThread::IndexingChain;
using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Sort = org::apache::lucene::search::Sort;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using InfoStream = org::apache::lucene::util::InfoStream;

LiveIndexWriterConfig::LiveIndexWriterConfig(shared_ptr<Analyzer> analyzer)
    : analyzer(analyzer)
{
  ramBufferSizeMB = IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB;
  maxBufferedDocs = IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS;
  mergedSegmentWarmer.reset();
  delPolicy = make_shared<KeepOnlyLastCommitDeletionPolicy>();
  commit.reset();
  useCompoundFile = IndexWriterConfig::DEFAULT_USE_COMPOUND_FILE_SYSTEM;
  openMode = OpenMode::CREATE_OR_APPEND;
  similarity = IndexSearcher::getDefaultSimilarity();
  mergeScheduler = make_shared<ConcurrentMergeScheduler>();
  indexingChain = DocumentsWriterPerThread::defaultIndexingChain;
  codec = Codec::getDefault();
  if (codec == nullptr) {
    throw make_shared<NullPointerException>();
  }
  infoStream = InfoStream::getDefault();
  mergePolicy = make_shared<TieredMergePolicy>();
  flushPolicy = make_shared<FlushByRamOrCountsPolicy>();
  readerPooling = IndexWriterConfig::DEFAULT_READER_POOLING;
  indexerThreadPool = make_shared<DocumentsWriterPerThreadPool>();
  perThreadHardLimitMB =
      IndexWriterConfig::DEFAULT_RAM_PER_THREAD_HARD_LIMIT_MB;
}

shared_ptr<Analyzer> LiveIndexWriterConfig::getAnalyzer() { return analyzer; }

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<LiveIndexWriterConfig>
LiveIndexWriterConfig::setRAMBufferSizeMB(double ramBufferSizeMB)
{
  if (ramBufferSizeMB != IndexWriterConfig::DISABLE_AUTO_FLUSH &&
      ramBufferSizeMB <= 0.0) {
    throw invalid_argument(L"ramBufferSize should be > 0.0 MB when enabled");
  }
  if (ramBufferSizeMB == IndexWriterConfig::DISABLE_AUTO_FLUSH &&
      maxBufferedDocs == IndexWriterConfig::DISABLE_AUTO_FLUSH) {
    throw invalid_argument(
        L"at least one of ramBufferSize and maxBufferedDocs must be enabled");
  }
  this->ramBufferSizeMB = ramBufferSizeMB;
  return shared_from_this();
}

double LiveIndexWriterConfig::getRAMBufferSizeMB() { return ramBufferSizeMB; }

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<LiveIndexWriterConfig>
LiveIndexWriterConfig::setMaxBufferedDocs(int maxBufferedDocs)
{
  if (maxBufferedDocs != IndexWriterConfig::DISABLE_AUTO_FLUSH &&
      maxBufferedDocs < 2) {
    throw invalid_argument(L"maxBufferedDocs must at least be 2 when enabled");
  }
  if (maxBufferedDocs == IndexWriterConfig::DISABLE_AUTO_FLUSH &&
      ramBufferSizeMB == IndexWriterConfig::DISABLE_AUTO_FLUSH) {
    throw invalid_argument(
        L"at least one of ramBufferSize and maxBufferedDocs must be enabled");
  }
  this->maxBufferedDocs = maxBufferedDocs;
  return shared_from_this();
}

int LiveIndexWriterConfig::getMaxBufferedDocs() { return maxBufferedDocs; }

shared_ptr<LiveIndexWriterConfig>
LiveIndexWriterConfig::setMergePolicy(shared_ptr<MergePolicy> mergePolicy)
{
  if (mergePolicy == nullptr) {
    throw invalid_argument(L"mergePolicy must not be null");
  }
  this->mergePolicy = mergePolicy;
  return shared_from_this();
}

shared_ptr<LiveIndexWriterConfig> LiveIndexWriterConfig::setMergedSegmentWarmer(
    IndexReaderWarmer mergeSegmentWarmer)
{
  this->mergedSegmentWarmer = mergeSegmentWarmer;
  return shared_from_this();
}

IndexReaderWarmer LiveIndexWriterConfig::getMergedSegmentWarmer()
{
  return mergedSegmentWarmer;
}

OpenMode LiveIndexWriterConfig::getOpenMode() { return openMode; }

shared_ptr<IndexDeletionPolicy> LiveIndexWriterConfig::getIndexDeletionPolicy()
{
  return delPolicy;
}

shared_ptr<IndexCommit> LiveIndexWriterConfig::getIndexCommit()
{
  return commit;
}

shared_ptr<Similarity> LiveIndexWriterConfig::getSimilarity()
{
  return similarity;
}

shared_ptr<MergeScheduler> LiveIndexWriterConfig::getMergeScheduler()
{
  return mergeScheduler;
}

shared_ptr<Codec> LiveIndexWriterConfig::getCodec() { return codec; }

shared_ptr<MergePolicy> LiveIndexWriterConfig::getMergePolicy()
{
  return mergePolicy;
}

shared_ptr<DocumentsWriterPerThreadPool>
LiveIndexWriterConfig::getIndexerThreadPool()
{
  return indexerThreadPool;
}

bool LiveIndexWriterConfig::getReaderPooling() { return readerPooling; }

shared_ptr<IndexingChain> LiveIndexWriterConfig::getIndexingChain()
{
  return indexingChain;
}

int LiveIndexWriterConfig::getRAMPerThreadHardLimitMB()
{
  return perThreadHardLimitMB;
}

shared_ptr<FlushPolicy> LiveIndexWriterConfig::getFlushPolicy()
{
  return flushPolicy;
}

shared_ptr<InfoStream> LiveIndexWriterConfig::getInfoStream()
{
  return infoStream;
}

shared_ptr<LiveIndexWriterConfig>
LiveIndexWriterConfig::setUseCompoundFile(bool useCompoundFile)
{
  this->useCompoundFile = useCompoundFile;
  return shared_from_this();
}

bool LiveIndexWriterConfig::getUseCompoundFile() { return useCompoundFile; }

bool LiveIndexWriterConfig::getCommitOnClose() { return commitOnClose; }

shared_ptr<Sort> LiveIndexWriterConfig::getIndexSort() { return indexSort; }

shared_ptr<Set<wstring>> LiveIndexWriterConfig::getIndexSortFields()
{
  return indexSortFields;
}

bool LiveIndexWriterConfig::isCheckPendingFlushOnUpdate()
{
  return checkPendingFlushOnUpdate;
}

shared_ptr<LiveIndexWriterConfig>
LiveIndexWriterConfig::setCheckPendingFlushUpdate(
    bool checkPendingFlushOnUpdate)
{
  this->checkPendingFlushOnUpdate = checkPendingFlushOnUpdate;
  return shared_from_this();
}

wstring LiveIndexWriterConfig::getSoftDeletesField()
{
  return softDeletesField;
}

wstring LiveIndexWriterConfig::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"analyzer=")
      ->append(analyzer == nullptr ? L"null" : analyzer->getClassName())
      ->append(L"\n");
  sb->append(L"ramBufferSizeMB=")->append(getRAMBufferSizeMB())->append(L"\n");
  sb->append(L"maxBufferedDocs=")->append(getMaxBufferedDocs())->append(L"\n");
  sb->append(L"mergedSegmentWarmer=")
      ->append(getMergedSegmentWarmer())
      ->append(L"\n");
  sb->append(L"delPolicy=")
      ->append(getIndexDeletionPolicy()->getClassName())
      ->append(L"\n");
  shared_ptr<IndexCommit> commit = getIndexCommit();
  sb->append(L"commit=")
      ->append(commit == nullptr ? L"null" : commit)
      ->append(L"\n");
  sb->append(L"openMode=")->append(getOpenMode())->append(L"\n");
  sb->append(L"similarity=")
      ->append(getSimilarity()->getClassName())
      ->append(L"\n");
  sb->append(L"mergeScheduler=")->append(getMergeScheduler())->append(L"\n");
  sb->append(L"codec=")->append(getCodec())->append(L"\n");
  sb->append(L"infoStream=")
      ->append(getInfoStream()->getClassName())
      ->append(L"\n");
  sb->append(L"mergePolicy=")->append(getMergePolicy())->append(L"\n");
  sb->append(L"indexerThreadPool=")
      ->append(getIndexerThreadPool())
      ->append(L"\n");
  sb->append(L"readerPooling=")->append(getReaderPooling())->append(L"\n");
  sb->append(L"perThreadHardLimitMB=")
      ->append(getRAMPerThreadHardLimitMB())
      ->append(L"\n");
  sb->append(L"useCompoundFile=")->append(getUseCompoundFile())->append(L"\n");
  sb->append(L"commitOnClose=")->append(getCommitOnClose())->append(L"\n");
  sb->append(L"indexSort=")->append(getIndexSort())->append(L"\n");
  sb->append(L"checkPendingFlushOnUpdate=")
      ->append(isCheckPendingFlushOnUpdate())
      ->append(L"\n");
  sb->append(L"softDeletesField=")
      ->append(getSoftDeletesField())
      ->append(L"\n");
  return sb->toString();
}
} // namespace org::apache::lucene::index