using namespace std;

#include "IndexWriter.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Codec = org::apache::lucene::codecs::Codec;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using Field = org::apache::lucene::document::Field;
using BinaryDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::BinaryDocValuesUpdate;
using NumericDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::NumericDocValuesUpdate;
using FieldNumbers = org::apache::lucene::index::FieldInfos::FieldNumbers;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using Lock = org::apache::lucene::store::Lock;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using LockValidatingDirectoryWrapper =
    org::apache::lucene::store::LockValidatingDirectoryWrapper;
using MMapDirectory = org::apache::lucene::store::MMapDirectory;
using MergeInfo = org::apache::lucene::store::MergeInfo;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using StringHelper = org::apache::lucene::util::StringHelper;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using Version = org::apache::lucene::util::Version;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
int IndexWriter::actualMaxDocs = MAX_DOCS;

void IndexWriter::setMaxDocs(int maxDocs)
{
  if (maxDocs > MAX_DOCS) {
    // Cannot go higher than the hard max:
    throw invalid_argument(L"maxDocs must be <= IndexWriter.MAX_DOCS=" +
                           to_wstring(MAX_DOCS) + L"; got: " +
                           to_wstring(maxDocs));
  }
  IndexWriter::actualMaxDocs = maxDocs;
}

int IndexWriter::getActualMaxDocs() { return IndexWriter::actualMaxDocs; }

const wstring IndexWriter::WRITE_LOCK_NAME = L"write.lock";
const wstring IndexWriter::SOURCE = L"source";
const wstring IndexWriter::SOURCE_MERGE = L"merge";
const wstring IndexWriter::SOURCE_FLUSH = L"flush";
const wstring IndexWriter::SOURCE_ADDINDEXES_READERS =
    L"addIndexes(CodecReader...)";

IndexWriter::FlushNotificationsAnonymousInnerClass::
    FlushNotificationsAnonymousInnerClass()
{
}

void IndexWriter::FlushNotificationsAnonymousInnerClass::deleteUnusedFiles(
    shared_ptr<deque<wstring>> files)
{
  outerInstance->eventQueue.push_back([&](any w) { w::deleteNewFiles(files); });
}

void IndexWriter::FlushNotificationsAnonymousInnerClass::flushFailed(
    shared_ptr<SegmentInfo> info)
{
  outerInstance->eventQueue.push_back([&](any w) { w::flushFailed(info); });
}

void IndexWriter::FlushNotificationsAnonymousInnerClass::
    afterSegmentsFlushed() 
{
  try {
    outerInstance->publishFlushedSegments(false);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (false) {
      outerInstance->maybeMerge(outerInstance->config.getMergePolicy(),
                                MergeTrigger::SEGMENT_FLUSH,
                                UNBOUNDED_MAX_MERGE_SEGMENTS);
    }
  }
}

void IndexWriter::FlushNotificationsAnonymousInnerClass::onTragicEvent(
    runtime_error event_, const wstring &message)
{
  outerInstance->onTragicEvent(event_, message);
}

void IndexWriter::FlushNotificationsAnonymousInnerClass::onDeletesApplied()
{
  outerInstance->eventQueue.push_back([&](any w) {
    try {
      w::publishFlushedSegments(true);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      outerInstance->flushCount.incrementAndGet();
    }
  });
}

void IndexWriter::FlushNotificationsAnonymousInnerClass::onTicketBacklog()
{
  outerInstance->eventQueue.push_back(
      [&](any w) { w::publishFlushedSegments(true); });
}

shared_ptr<DirectoryReader> IndexWriter::getReader() 
{
  return getReader(true, false);
}

shared_ptr<DirectoryReader>
IndexWriter::getReader(bool applyAllDeletes,
                       bool writeAllDeletes) 
{
  ensureOpen();

  if (writeAllDeletes && applyAllDeletes == false) {
    throw invalid_argument(
        L"applyAllDeletes must be true when writeAllDeletes=true");
  }

  constexpr int64_t tStart = System::currentTimeMillis();

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"flush at getReader");
  }
  // Do this up front before flushing so that the readers
  // obtained during this flush are pooled, the first time
  // this method is called:
  readerPool->enableReaderPooling();
  shared_ptr<DirectoryReader> r = nullptr;
  doBeforeFlush();
  bool anyChanges = false;
  /*
   * for releasing a NRT reader we must ensure that
   * DW doesn't add any segments or deletes until we are
   * done with creating the NRT DirectoryReader.
   * We release the two stage full flush after we are done opening the
   * directory reader!
   */
  bool success2 = false;
  try {
    bool success = false;
    {
      lock_guard<mutex> lock(fullFlushLock);
      try {
        // TODO: should we somehow make this available in the returned NRT
        // reader?
        int64_t seqNo = docWriter->flushAllThreads();
        if (seqNo < 0) {
          anyChanges = true;
          seqNo = -seqNo;
        } else {
          anyChanges = false;
        }
        if (anyChanges == false) {
          // prevent double increment since docWriter#doFlush increments the
          // flushcount if we flushed anything.
          flushCount->incrementAndGet();
        }
        publishFlushedSegments(true);
        processEvents(false);

        if (applyAllDeletes) {
          applyAllDeletesAndUpdates();
        }

        // C++ TODO: Multithread locking on 'this' is not converted to native
        // C++:
        synchronized(shared_from_this())
        {

          // NOTE: we cannot carry doc values updates in memory yet, so we
          // always must write them through to disk and re-open each
          // SegmentReader:

          // TODO: we could instead just clone SIS and pull/incref readers in
          // sync'd block, and then do this w/o IW's lock? Must do this sync'd
          // on IW to prevent a merge from completing at the last second and
          // failing to write its DV updates:
          writeReaderPool(writeAllDeletes);

          // Prevent segmentInfos from changing while opening the
          // reader; in theory we could instead do similar retry logic,
          // just like we do when loading segments_N

          r = StandardDirectoryReader::open(shared_from_this(), segmentInfos,
                                            applyAllDeletes, writeAllDeletes);
          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(L"IW", L"return reader version=" +
                                           to_wstring(r->getVersion()) +
                                           L" reader=" + r);
          }
        }
        success = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // Done: finish the full flush!
        assert(holdsFullFlushLock());
        docWriter->finishFullFlush(success);
        if (success) {
          processEvents(false);
          doAfterFlush();
        } else {
          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(L"IW", L"hit exception during NRT reader");
          }
        }
      }
    }
    anyChanges |= maybeMerge_->getAndSet(false);
    if (anyChanges) {
      maybeMerge(config->getMergePolicy(), MergeTrigger::FULL_FLUSH,
                 UNBOUNDED_MAX_MERGE_SEGMENTS);
    }
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"getReader took " +
                                     (System::currentTimeMillis() - tStart) +
                                     L" msec");
    }
    success2 = true;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"getReader");
    throw tragedy;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success2) {
      try {
        IOUtils::closeWhileHandlingException({r});
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        maybeCloseOnTragicEvent();
      }
    }
  }
  return r;
}

int64_t IndexWriter::ramBytesUsed()
{
  ensureOpen();
  return docWriter->ramBytesUsed();
}

int64_t IndexWriter::getReaderPoolRamBytesUsed()
{
  return readerPool->ramBytesUsed();
}

void IndexWriter::writeSomeDocValuesUpdates() 
{
  if (writeDocValuesLock->compareAndSet(false, true)) {
    try {
      constexpr double ramBufferSizeMB = config->getRAMBufferSizeMB();
      // If the reader pool is > 50% of our IW buffer, then write the updates:
      if (ramBufferSizeMB != IndexWriterConfig::DISABLE_AUTO_FLUSH) {
        int64_t startNS = System::nanoTime();

        int64_t ramBytesUsed = getReaderPoolRamBytesUsed();
        if (ramBytesUsed > 0.5 * ramBufferSizeMB * 1024 * 1024) {
          if (infoStream->isEnabled(L"BD")) {
            infoStream->message(
                L"BD", wstring::format(Locale::ROOT,
                                       L"now write some pending DV updates: "
                                       L"%.2f MB used vs IWC Buffer %.2f MB",
                                       ramBytesUsed / 1024.0 / 1024.0,
                                       ramBufferSizeMB));
          }

          // Sort by largest ramBytesUsed:
          const deque<std::shared_ptr<ReadersAndUpdates>> deque =
              readerPool->getReadersByRam();
          int count = 0;
          for (auto rld : deque) {

            if (ramBytesUsed <= 0.5 * ramBufferSizeMB * 1024 * 1024) {
              break;
            }
            // We need to do before/after because not all RAM in this RAU is
            // used by DV updates, and not all of those bytes can be written
            // here:
            int64_t bytesUsedBefore = rld->ramBytesUsed->get();
            if (bytesUsedBefore == 0) {
              continue; // nothing to do here - lets not acquire the lock
            }
            // Only acquire IW lock on each write, since this is a time
            // consuming operation.  This way other threads get a chance to run
            // in between our writes.
            // C++ TODO: Multithread locking on 'this' is not converted to
            // native C++:
            synchronized(shared_from_this())
            {
              // It's possible that the segment of a reader returned by
              // readerPool#getReadersByRam is dropped before being processed
              // here. If it happens, we need to skip that reader. this is also
              // best effort to free ram, there might be some other thread
              // writing this rld concurrently which wins and then if
              // readerPooling is off this rld will be dropped.
              if (readerPool->get(rld->info, false) == nullptr) {
                continue;
              }
              if (rld->writeFieldUpdates(
                      directory, globalFieldNumberMap,
                      bufferedUpdatesStream->getCompletedDelGen(),
                      infoStream)) {
                checkpointNoSIS();
              }
            }
            int64_t bytesUsedAfter = rld->ramBytesUsed->get();
            ramBytesUsed -= bytesUsedBefore - bytesUsedAfter;
            count++;
          }

          if (infoStream->isEnabled(L"BD")) {
            infoStream->message(
                L"BD", wstring::format(
                           Locale::ROOT,
                           L"done write some DV updates for %d segments: now "
                           L"%.2f MB used vs IWC Buffer %.2f MB; took %.2f sec",
                           count, getReaderPoolRamBytesUsed() / 1024.0 / 1024.0,
                           ramBufferSizeMB,
                           ((System::nanoTime() - startNS) / 1000000000.0)));
          }
        }
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      writeDocValuesLock->set(false);
    }
  }
}

int IndexWriter::numDeletedDocs(shared_ptr<SegmentCommitInfo> info)
{
  ensureOpen(false);
  validate(info);
  shared_ptr<ReadersAndUpdates> *const rld = getPooledInstance(info, false);
  if (rld != nullptr) {
    return rld->getDelCount(); // get the full count from here since SCI might
                               // change concurrently
  } else {
    constexpr int delCount = info->getDelCount(softDeletesEnabled);
    assert((delCount <= info->info->maxDoc(),
            L"delCount: " + to_wstring(delCount) + L" maxDoc: " +
                to_wstring(info->info->maxDoc())));
    return delCount;
  }
}

void IndexWriter::ensureOpen(bool failIfClosing) 
{
  if (closed || (failIfClosing && closing)) {
    throw make_shared<AlreadyClosedException>(L"this IndexWriter is closed",
                                              tragedy->get());
  }
}

void IndexWriter::ensureOpen() 
{
  ensureOpen(true);
}

IndexWriter::IndexWriter(shared_ptr<Directory> d,
                         shared_ptr<IndexWriterConfig> conf) 
    : enableTestPoints(isEnableTestPoints()), config(conf),
      softDeletesEnabled(config->getSoftDeletesField() != L""),
      infoStream(config->getInfoStream())
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  conf->setIndexWriter(shared_from_this()); // prevent reuse by other instances
  // obtain the write.lock. If the user configured a timeout,
  // we wrap with a sleeper and this might take some time.
  writeLock = d->obtainLock(WRITE_LOCK_NAME);

  bool success = false;
  try {
    directoryOrig = d;
    directory = make_shared<LockValidatingDirectoryWrapper>(d, writeLock);

    analyzer = config->getAnalyzer();
    mergeScheduler = config->getMergeScheduler();
    mergeScheduler->setInfoStream(infoStream);
    codec = config->getCodec();
    OpenMode mode = config->getOpenMode();
    bool create;
    if (mode == OpenMode::CREATE) {
      create = true;
    } else if (mode == OpenMode::APPEND) {
      create = false;
    } else {
      // CREATE_OR_APPEND - create only if an index does not exist
      create = !DirectoryReader::indexExists(directory);
    }

    // If index is too old, reading the segments will throw
    // IndexFormatTooOldException.

    bool initialIndexExists = true;

    std::deque<wstring> files = directory->listAll();

    // Set up our initial SegmentInfos:
    shared_ptr<IndexCommit> commit = config->getIndexCommit();

    // Set up our initial SegmentInfos:
    shared_ptr<StandardDirectoryReader> reader;
    if (commit == nullptr) {
      reader.reset();
    } else {
      reader = commit->getReader();
    }

    if (create) {

      if (config->getIndexCommit() != nullptr) {
        // We cannot both open from a commit point and create:
        if (mode == OpenMode::CREATE) {
          throw invalid_argument(
              L"cannot use IndexWriterConfig.setIndexCommit() with "
              L"OpenMode.CREATE");
        } else {
          throw invalid_argument(
              L"cannot use IndexWriterConfig.setIndexCommit() when index has "
              L"no commit");
        }
      }

      // Try to read first.  This is to allow create
      // against an index that's currently open for
      // searching.  In this case we write the next
      // segments_N file with no segments:
      shared_ptr<SegmentInfos> *const sis =
          make_shared<SegmentInfos>(Version::LATEST->major);
      try {
        shared_ptr<SegmentInfos> *const previous =
            SegmentInfos::readLatestCommit(directory);
        sis->updateGenerationVersionAndCounter(previous);
      } catch (const IOException &e) {
        // Likely this means it's a fresh directory
        initialIndexExists = false;
      }

      segmentInfos = sis;
      rollbackSegments = segmentInfos->createBackupSegmentInfos();

      // Record that we have a change (zero out all
      // segments) pending:
      changed();

    } else if (reader != nullptr) {
      // Init from an existing already opened NRT or non-NRT reader:

      if (reader->directory() != commit->getDirectory()) {
        throw invalid_argument(L"IndexCommit's reader must have the same "
                               L"directory as the IndexCommit");
      }

      if (reader->directory() != directoryOrig) {
        throw invalid_argument(L"IndexCommit's reader must have the same "
                               L"directory passed to IndexWriter");
      }

      if (reader->segmentInfos->getLastGeneration() == 0) {
        // TODO: maybe we could allow this?  It's tricky...
        throw invalid_argument(
            L"index must already have an initial commit to open from reader");
      }

      // Must clone because we don't want the incoming NRT reader to "see" any
      // changes this writer now makes:
      segmentInfos = reader->segmentInfos->clone();

      shared_ptr<SegmentInfos> lastCommit;
      try {
        lastCommit = SegmentInfos::readCommit(
            directoryOrig, segmentInfos->getSegmentsFileName());
      } catch (const IOException &ioe) {
        throw invalid_argument(
            L"the provided reader is stale: its prior commit file \"" +
            segmentInfos->getSegmentsFileName() + L"\" is missing from index");
      }

      if (reader->writer != nullptr) {

        // The old writer better be closed (we have the write lock now!):
        assert(reader->writer->closed);

        // In case the old writer wrote further segments (which we are now
        // dropping), update SIS metadata so we remain write-once:
        segmentInfos->updateGenerationVersionAndCounter(
            reader->writer->segmentInfos);
        lastCommit->updateGenerationVersionAndCounter(
            reader->writer->segmentInfos);
      }

      rollbackSegments = lastCommit->createBackupSegmentInfos();
    } else {
      // Init from either the latest commit point, or an explicit prior commit
      // point:

      wstring lastSegmentsFile =
          SegmentInfos::getLastCommitSegmentsFileName(files);
      if (lastSegmentsFile == L"") {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw make_shared<IndexNotFoundException>(
            L"no segments* file found in " + directory + L": files: " +
            Arrays->toString(files));
      }

      // Do not use SegmentInfos.read(Directory) since the spooky
      // retrying it does is not necessary here (we hold the write lock):
      segmentInfos = SegmentInfos::readCommit(directoryOrig, lastSegmentsFile);

      if (commit != nullptr) {
        // Swap out all segments, but, keep metadata in
        // SegmentInfos, like version & generation, to
        // preserve write-once.  This is important if
        // readers are open against the future commit
        // points.
        if (commit->getDirectory() != directoryOrig) {
          throw invalid_argument(
              L"IndexCommit's directory doesn't match my directory, expected=" +
              directoryOrig + L", got=" + commit->getDirectory());
        }

        shared_ptr<SegmentInfos> oldInfos = SegmentInfos::readCommit(
            directoryOrig, commit->getSegmentsFileName());
        segmentInfos->replace(oldInfos);
        changed();

        if (infoStream->isEnabled(L"IW")) {
          infoStream->message(L"IW", L"init: loaded commit \"" +
                                         commit->getSegmentsFileName() + L"\"");
        }
      }

      rollbackSegments = segmentInfos->createBackupSegmentInfos();
    }

    commitUserData = (unordered_map<>(segmentInfos->getUserData()))->entrySet();

    pendingNumDocs->set(segmentInfos->totalMaxDoc());

    // start with previous field numbers, but new FieldInfos
    // NOTE: this is correct even for an NRT reader because we'll pull
    // FieldInfos even for the un-committed segments:
    globalFieldNumberMap = getFieldNumberMap();

    validateIndexSort();

    config->getFlushPolicy()->init(config);
    bufferedUpdatesStream = make_shared<BufferedUpdatesStream>(infoStream);
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    docWriter = make_shared<DocumentsWriter>(
        flushNotifications, segmentInfos->getIndexCreatedVersionMajor(),
        pendingNumDocs, enableTestPoints, shared_from_this()::newSegmentName,
        config, directoryOrig, directory, globalFieldNumberMap);
    readerPool = make_shared<ReaderPool>(
        directory, directoryOrig, segmentInfos, globalFieldNumberMap,
        bufferedUpdatesStream::getCompletedDelGen, infoStream,
        conf->getSoftDeletesField(), reader);
    if (config->getReaderPooling()) {
      readerPool->enableReaderPooling();
    }
    // Default deleter (for backwards compatibility) is
    // KeepOnlyLastCommitDeleter:

    // Sync'd is silly here, but IFD asserts we sync'd on the IW instance:
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    synchronized(shared_from_this())
    {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      deleter = make_shared<IndexFileDeleter>(
          files, directoryOrig, directory, config->getIndexDeletionPolicy(),
          segmentInfos, infoStream, shared_from_this(), initialIndexExists,
          reader != nullptr);

      // We incRef all files when we return an NRT reader from IW, so all files
      // must exist even in the NRT case:
      assert(create || filesExist(segmentInfos));
    }

    if (deleter->startingCommitDeleted) {
      // Deletion policy deleted the "head" commit point.
      // We have to mark ourself as changed so that if we
      // are closed w/o any further changes we write a new
      // segments_N file.
      changed();
    }

    if (reader != nullptr) {
      // We always assume we are carrying over incoming changes when opening
      // from reader:
      segmentInfos->changed();
      changed();
    }

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"init: create=" +
                                     StringHelper::toString(create) +
                                     L" reader=" + reader);
      messageState();
    }

    success = true;

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(
            L"IW", L"init: hit exception on init; releasing write lock");
      }
      IOUtils::closeWhileHandlingException({writeLock});
      writeLock.reset();
    }
  }
}

void IndexWriter::validateIndexSort() 
{
  shared_ptr<Sort> indexSort = config->getIndexSort();
  if (indexSort != nullptr) {
    for (auto info : segmentInfos) {
      shared_ptr<Sort> segmentIndexSort = info->info->getIndexSort();
      if (segmentIndexSort != nullptr &&
          indexSort->equals(segmentIndexSort) == false) {
        throw invalid_argument(L"cannot change previous indexSort=" +
                               segmentIndexSort + L" (from segment=" + info +
                               L") to new indexSort=" + indexSort);
      } else if (segmentIndexSort == nullptr &&
                 info->info->getVersion()->onOrAfter(Version::LUCENE_6_5_0)) {
        // Flushed segments are not sorted if they were built with a version
        // prior to 6.5.0
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw make_shared<CorruptIndexException>(
            L"segment not sorted with indexSort=" + segmentIndexSort,
            info->info->toString());
      }
    }
  }
}

shared_ptr<FieldInfos>
IndexWriter::readFieldInfos(shared_ptr<SegmentCommitInfo> si) 
{
  shared_ptr<Codec> codec = si->info->getCodec();
  shared_ptr<FieldInfosFormat> reader = codec->fieldInfosFormat();

  if (si->hasFieldUpdates()) {
    // there are updates, we read latest (always outside of CFS)
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring segmentSuffix =
        Long::toString(si->getFieldInfosGen(), Character::MAX_RADIX);
    return reader->read(si->info->dir, si->info, segmentSuffix,
                        IOContext::READONCE);
  } else if (si->info->getUseCompoundFile()) {
    // cfs
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory cfs =
    // codec.compoundFormat().getCompoundReader(si.info.dir, si.info,
    // org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::Directory cfs =
          codec->compoundFormat()->getCompoundReader(
              si->info->dir, si->info,
              org::apache::lucene::store::IOContext::DEFAULT);
      return reader->read(cfs, si->info, L"", IOContext::READONCE);
    }
  } else {
    // no cfs
    return reader->read(si->info->dir, si->info, L"", IOContext::READONCE);
  }
}

shared_ptr<FieldNumbers> IndexWriter::getFieldNumberMap() 
{
  shared_ptr<FieldNumbers> *const map_obj =
      make_shared<FieldNumbers>(config->softDeletesField);

  for (auto info : segmentInfos) {
    shared_ptr<FieldInfos> fis = readFieldInfos(info);
    for (auto fi : fis) {
      map_obj->addOrGet(fi->name, fi->number, fi->getDocValuesType(),
                    fi->getPointDimensionCount(), fi->getPointNumBytes(),
                    fi->isSoftDeletesField());
    }
  }

  return map_obj;
}

shared_ptr<LiveIndexWriterConfig> IndexWriter::getConfig()
{
  ensureOpen(false);
  return config;
}

void IndexWriter::messageState()
{
  if (infoStream->isEnabled(L"IW") && didMessageState == false) {
    didMessageState = true;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    infoStream->message(L"IW", L"\ndir=" + directoryOrig + L"\n" + L"index=" +
                                   segString() + L"\n" + L"version=" +
                                   Version::LATEST->toString() + L"\n" +
                                   config->toString());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<StringBuilder> *const unmapInfo = make_shared<StringBuilder>(
        Boolean::toString(MMapDirectory::UNMAP_SUPPORTED));
    if (!MMapDirectory::UNMAP_SUPPORTED) {
      unmapInfo->append(L" (")
          ->append(MMapDirectory::UNMAP_NOT_SUPPORTED_REASON)
          ->append(L")");
    }
    infoStream->message(L"IW", L"MMapDirectory.UNMAP_SUPPORTED=" + unmapInfo);
  }
}

void IndexWriter::shutdown() 
{
  if (pendingCommit->size() > 0) {
    throw make_shared<IllegalStateException>(
        L"cannot close: prepareCommit was already called with no corresponding "
        L"call to commit");
  }
  // Ensure that only one thread actually gets to do the
  // closing
  if (shouldClose(true)) {
    try {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"now flush at close");
      }

      flush(true, true);
      waitForMerges();
      commitInternal(config->getMergePolicy());
      rollbackInternal(); // ie close, since we just committed
    } catch (const runtime_error &t) {
      // Be certain to close the index on any exception
      try {
        rollbackInternal();
      } catch (const runtime_error &t1) {
        t.addSuppressed(t1);
      }
      throw t;
    }
  }
}

IndexWriter::~IndexWriter()
{
  if (config->getCommitOnClose()) {
    shutdown();
  } else {
    rollback();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::shouldClose(bool waitForClose)
{
  while (true) {
    if (closed == false) {
      if (closing == false) {
        // We get to close
        closing = true;
        return true;
      } else if (waitForClose == false) {
        return false;
      } else {
        // Another thread is presently trying to close;
        // wait until it finishes one way (closes
        // successfully) or another (fails to close)
        doWait();
      }
    } else {
      return false;
    }
  }
}

shared_ptr<Directory> IndexWriter::getDirectory()
{
  // return the original directory the user supplied, unwrapped.
  return directoryOrig;
}

shared_ptr<InfoStream> IndexWriter::getInfoStream() { return infoStream; }

shared_ptr<Analyzer> IndexWriter::getAnalyzer()
{
  ensureOpen();
  return analyzer;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int IndexWriter::maxDoc()
{
  ensureOpen();
  return docWriter->getNumDocs() + segmentInfos->totalMaxDoc();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::advanceSegmentInfosVersion(int64_t newVersion)
{
  ensureOpen();
  if (segmentInfos->getVersion() < newVersion) {
    segmentInfos->setVersion(newVersion);
  }
  changed();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int IndexWriter::numDocs()
{
  ensureOpen();
  int count = docWriter->getNumDocs();
  for (auto info : segmentInfos) {
    count += info->info->maxDoc() - numDeletedDocs(info);
  }
  return count;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::hasDeletions()
{
  ensureOpen();
  if (bufferedUpdatesStream->any()) {
    return true;
  }
  if (docWriter->anyDeletions()) {
    return true;
  }
  if (readerPool->anyDeletions()) {
    return true;
  }
  for (auto info : segmentInfos) {
    if (info->hasDeletions()) {
      return true;
    }
  }
  return false;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long addDocument(Iterable<? extends
// IndexableField> doc) throws java.io.IOException
int64_t IndexWriter::addDocument(deque<T1> doc) 
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: return updateDocument((DocumentsWriterDeleteQueue.Node<?>)
  // null, doc);
      return updateDocument(std::static_pointer_cast<DocumentsWriterDeleteQueue::Node<?>>(nullptr), doc);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long addDocuments(Iterable<? extends
// Iterable<? extends IndexableField>> docs) throws java.io.IOException
int64_t IndexWriter::addDocuments(deque<T1> docs) 
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: return updateDocuments((DocumentsWriterDeleteQueue.Node<?>)
  // null, docs);
      return updateDocuments(std::static_pointer_cast<DocumentsWriterDeleteQueue::Node<?>>(nullptr), docs);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long updateDocuments(Term delTerm,
// Iterable<? extends Iterable<? extends IndexableField>> docs) throws
// java.io.IOException
int64_t IndexWriter::updateDocuments(shared_ptr<Term> delTerm,
                                       deque<T1> docs) 
{
  return updateDocuments(delTerm == nullptr
                             ? nullptr
                             : DocumentsWriterDeleteQueue::newNode(delTerm),
                         docs);
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private long updateDocuments(final
// DocumentsWriterDeleteQueue.Node<?> delNode, Iterable<? extends Iterable<?
// extends IndexableField>> docs) throws java.io.IOException
int64_t IndexWriter::updateDocuments(
    shared_ptr<DocumentsWriterDeleteQueue::Node<T1>> delNode,
    deque<T2> docs) 
{
  ensureOpen();
  bool success = false;
  try {
    int64_t seqNo = docWriter->updateDocuments(docs, analyzer, delNode);
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }
    success = true;
    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"updateDocuments");
    throw tragedy;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"hit exception updating document");
      }
      maybeCloseOnTragicEvent();
    }
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long softUpdateDocuments(Term term,
// Iterable<? extends Iterable<? extends IndexableField>> docs,
// org.apache.lucene.document.Field... softDeletes) throws java.io.IOException
int64_t
IndexWriter::softUpdateDocuments(shared_ptr<Term> term, deque<T1> docs,
                                 deque<Field> &softDeletes) 
{
  if (term == nullptr) {
    throw invalid_argument(L"term must not be null");
  }
  if (softDeletes == nullptr || softDeletes->length == 0) {
    throw invalid_argument(L"at least one soft delete must be present");
  }
  return updateDocuments(DocumentsWriterDeleteQueue::newNode(
                             buildDocValuesUpdate(term, softDeletes)),
                         docs);
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t IndexWriter::tryDeleteDocument(shared_ptr<IndexReader> readerIn,
                                         int docID) 
{
  // NOTE: DON'T use docID inside the closure
  return tryModifyDocument(readerIn, docID, [&](leafDocId, rld) {
    if (rld::delete (leafDocId)) {
      if (isFullyDeleted(rld)) {
        dropDeletedSegment(rld::info);
        checkpoint();
      }
      changed();
    }
  });
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
IndexWriter::tryUpdateDocValue(shared_ptr<IndexReader> readerIn, int docID,
                               deque<Field> &fields) 
{
  // NOTE: DON'T use docID inside the closure
  std::deque<std::shared_ptr<DocValuesUpdate>> dvUpdates =
      buildDocValuesUpdate(nullptr, fields);
  return tryModifyDocument(readerIn, docID, [&](leafDocId, rld) {
    int64_t nextGen = bufferedUpdatesStream->getNextGen();
    try {
      unordered_map<wstring, std::shared_ptr<DocValuesFieldUpdates>>
          fieldUpdatesMap =
              unordered_map<wstring, std::shared_ptr<DocValuesFieldUpdates>>();
      for (auto update : dvUpdates) {
        shared_ptr<DocValuesFieldUpdates> docValuesFieldUpdates =
            fieldUpdatesMap.computeIfAbsent(update->field, [&](any k) {
              switch (update->type) {
              case org::apache::lucene::index::DocValuesType::NUMERIC:
                return make_shared<NumericDocValuesFieldUpdates>(
                    nextGen, k, rld::info::info::maxDoc());
              case org::apache::lucene::index::DocValuesType::BINARY:
                return make_shared<BinaryDocValuesFieldUpdates>(
                    nextGen, k, rld::info::info::maxDoc());
              default:
                throw make_shared<AssertionError>(L"type: " + update->type +
                                                  L" is not supported");
              }
            });
        if (update->hasValue()) {
          switch (update->type) {
          case org::apache::lucene::index::DocValuesType::NUMERIC:
            docValuesFieldUpdates->add(
                leafDocId,
                (std::static_pointer_cast<NumericDocValuesUpdate>(update))
                    ->getValue());
            break;
          case org::apache::lucene::index::DocValuesType::BINARY:
            docValuesFieldUpdates->add(
                leafDocId,
                (std::static_pointer_cast<BinaryDocValuesUpdate>(update))
                    ->getValue());
            break;
          default:
            throw make_shared<AssertionError>(L"type: " + update->type +
                                              L" is not supported");
          }
        } else {
          docValuesFieldUpdates->reset(leafDocId);
        }
      }
      for (auto updates : fieldUpdatesMap) {
        updates->second.finish();
        rld::addDVUpdate(updates->second);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      bufferedUpdatesStream->finishedSegment(nextGen);
    }
    changed();
  });
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t IndexWriter::tryModifyDocument(shared_ptr<IndexReader> readerIn,
                                         int docID,
                                         DocModifier toApply) 
{
  shared_ptr<LeafReader> *const reader;
  if (std::dynamic_pointer_cast<LeafReader>(readerIn) != nullptr) {
    // Reader is already atomic: use the incoming docID:
    reader = std::static_pointer_cast<LeafReader>(readerIn);
  } else {
    // Composite reader: lookup sub-reader and re-base docID:
    deque<std::shared_ptr<LeafReaderContext>> leaves = readerIn->leaves();
    int subIndex = ReaderUtil::subIndex(docID, leaves);
    reader = leaves[subIndex]->reader();
    docID -= leaves[subIndex]->docBase;
    assert(docID >= 0);
    assert(docID < reader->maxDoc());
  }

  if (!(std::dynamic_pointer_cast<SegmentReader>(reader) != nullptr)) {
    throw invalid_argument(L"the reader must be a SegmentReader or composite "
                           L"reader containing only SegmentReaders");
  }

  shared_ptr<SegmentCommitInfo> *const info =
      (std::static_pointer_cast<SegmentReader>(reader))
          ->getOriginalSegmentInfo();

  // TODO: this is a slow linear search, but, number of
  // segments should be contained unless something is
  // seriously wrong w/ the index, so it should be a minor
  // cost:

  if (segmentInfos->find(info) != -1) {
    shared_ptr<ReadersAndUpdates> rld = getPooledInstance(info, false);
    if (rld != nullptr) {
      {
        lock_guard<mutex> lock(bufferedUpdatesStream);
        toApply(docID, rld);
        return docWriter->deleteQueue->getNextSequenceNumber();
      }
    }
  }
  return -1;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::dropDeletedSegment(shared_ptr<SegmentCommitInfo> info) throw(
    IOException)
{
  // If a merge has already registered for this
  // segment, we leave it in the readerPool; the
  // merge will skip merging it and will then drop
  // it once it's done:
  if (find(mergingSegments.begin(), mergingSegments.end(), info) !=
      mergingSegments.end() == false) {
    // it's possible that we invoke this method more than once for the same SCI
    // we must only remove the docs once!
    // C++ TODO: The Java deque 'remove(Object)' method is not converted:
    bool dropPendingDocs = segmentInfos->remove(info);
    try {
      // this is sneaky - we might hit an exception while dropping a reader but
      // then we have already removed the segment for the segmentInfo and we
      // lost the pendingDocs update due to that. therefore we execute the
      // adjustPendingNumDocs in a finally block to account for that.
      dropPendingDocs |= readerPool->drop(info);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (dropPendingDocs) {
        adjustPendingNumDocs(-info->info->maxDoc());
      }
    }
  }
}

int64_t IndexWriter::deleteDocuments(deque<Term> &terms) 
{
  ensureOpen();
  try {
    int64_t seqNo = docWriter->deleteTerms({terms});
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }
    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"deleteDocuments(Term..)");
    throw tragedy;
  }
}

int64_t
IndexWriter::deleteDocuments(deque<Query> &queries) 
{
  ensureOpen();

  // LUCENE-6379: Specialize MatchAllDocsQuery
  for (shared_ptr<Query> query : queries) {
    if (query->getClass() == MatchAllDocsQuery::typeid) {
      return deleteAll();
    }
  }

  try {
    int64_t seqNo = docWriter->deleteQueries({queries});
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }

    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"deleteDocuments(Query..)");
    throw tragedy;
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long updateDocument(Term term, Iterable<?
// extends IndexableField> doc) throws java.io.IOException
int64_t IndexWriter::updateDocument(shared_ptr<Term> term,
                                      deque<T1> doc) 
{
  return updateDocument(
      term == nullptr ? nullptr : DocumentsWriterDeleteQueue::newNode(term),
      doc);
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private long updateDocument(final
// DocumentsWriterDeleteQueue.Node<?> delNode, Iterable<? extends IndexableField>
// doc) throws java.io.IOException
int64_t IndexWriter::updateDocument(
    shared_ptr<DocumentsWriterDeleteQueue::Node<T1>> delNode,
    deque<T2> doc) 
{
  ensureOpen();
  bool success = false;
  try {
    int64_t seqNo = docWriter->updateDocument(doc, analyzer, delNode);
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }
    success = true;
    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"updateDocument");
    throw tragedy;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"hit exception updating document");
      }
    }
    maybeCloseOnTragicEvent();
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long softUpdateDocument(Term term,
// Iterable<? extends IndexableField> doc, org.apache.lucene.document.Field...
// softDeletes) throws java.io.IOException
int64_t
IndexWriter::softUpdateDocument(shared_ptr<Term> term, deque<T1> doc,
                                deque<Field> &softDeletes) 
{
  if (term == nullptr) {
    throw invalid_argument(L"term must not be null");
  }
  if (softDeletes == nullptr || softDeletes->length == 0) {
    throw invalid_argument(L"at least one soft delete must be present");
  }
  return updateDocument(DocumentsWriterDeleteQueue::newNode(
                            buildDocValuesUpdate(term, softDeletes)),
                        doc);
}

int64_t IndexWriter::updateNumericDocValue(shared_ptr<Term> term,
                                             const wstring &field,
                                             int64_t value) 
{
  ensureOpen();
  if (!globalFieldNumberMap->contains(field, DocValuesType::NUMERIC)) {
    throw invalid_argument(
        L"can only update existing numeric-docvalues fields!");
  }
  if (config->getIndexSortFields()->contains(field)) {
    throw invalid_argument(
        L"cannot update docvalues field involved in the index sort, field=" +
        field + L", sort=" + config->getIndexSort());
  }
  try {
    int64_t seqNo = docWriter->updateDocValues(
        {make_shared<NumericDocValuesUpdate>(term, field, value)});
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }
    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"updateNumericDocValue");
    throw tragedy;
  }
}

int64_t
IndexWriter::updateBinaryDocValue(shared_ptr<Term> term, const wstring &field,
                                  shared_ptr<BytesRef> value) 
{
  ensureOpen();
  if (value == nullptr) {
    throw invalid_argument(L"cannot update a field to a null value: " + field);
  }
  if (!globalFieldNumberMap->contains(field, DocValuesType::BINARY)) {
    throw invalid_argument(
        L"can only update existing binary-docvalues fields!");
  }
  try {
    int64_t seqNo = docWriter->updateDocValues(
        {make_shared<BinaryDocValuesUpdate>(term, field, value)});
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }
    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"updateBinaryDocValue");
    throw tragedy;
  }
}

int64_t
IndexWriter::updateDocValues(shared_ptr<Term> term,
                             deque<Field> &updates) 
{
  ensureOpen();
  std::deque<std::shared_ptr<DocValuesUpdate>> dvUpdates =
      buildDocValuesUpdate(term, updates);
  try {
    int64_t seqNo = docWriter->updateDocValues(dvUpdates);
    if (seqNo < 0) {
      seqNo = -seqNo;
      processEvents(true);
    }
    return seqNo;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"updateDocValues");
    throw tragedy;
  }
}

std::deque<std::shared_ptr<DocValuesUpdate>>
IndexWriter::buildDocValuesUpdate(shared_ptr<Term> term,
                                  std::deque<std::shared_ptr<Field>> &updates)
{
  std::deque<std::shared_ptr<DocValuesUpdate>> dvUpdates(updates.size());
  for (int i = 0; i < updates.size(); i++) {
    shared_ptr<Field> *const f = updates[i];
    constexpr DocValuesType dvType = f->fieldType()->docValuesType();
    if (dvType == nullptr) {
      throw make_shared<NullPointerException>(
          L"DocValuesType must not be null (field: \"" + f->name() + L"\")");
    }
    if (dvType == DocValuesType::NONE) {
      throw invalid_argument(
          L"can only update NUMERIC or BINARY fields! field=" + f->name());
    }
    if (globalFieldNumberMap->contains(f->name(), dvType) == false) {
      // if this field doesn't exists we try to add it. if it exists and the DV
      // type doesn't match we get a consistent error message as if you try to
      // do that during an indexing operation.
      globalFieldNumberMap->addOrGet(f->name(), -1, dvType, 0, 0,
                                     f->name() == config->softDeletesField);
      assert((globalFieldNumberMap->contains(f->name(), dvType)));
    }
    if (config->getIndexSortFields()->contains(f->name())) {
      throw invalid_argument(
          L"cannot update docvalues field involved in the index sort, field=" +
          f->name() + L", sort=" + config->getIndexSort());
    }

    switch (dvType) {
    case org::apache::lucene::index::DocValuesType::NUMERIC: {
      optional<int64_t> value =
          static_cast<optional<int64_t>>(f->numericValue());
      dvUpdates[i] =
          make_shared<NumericDocValuesUpdate>(term, f->name(), value);
      break;
    }
    case org::apache::lucene::index::DocValuesType::BINARY:
      dvUpdates[i] =
          make_shared<BinaryDocValuesUpdate>(term, f->name(), f->binaryValue());
      break;
    default:
      throw invalid_argument(
          L"can only update NUMERIC or BINARY fields: field=" + f->name() +
          L", type=" + dvType);
    }
  }
  return dvUpdates;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int IndexWriter::getSegmentCount() { return segmentInfos->size(); }

// C++ WARNING: The following method was originally marked 'synchronized':
int IndexWriter::getNumBufferedDocuments() { return docWriter->getNumDocs(); }

// C++ WARNING: The following method was originally marked 'synchronized':
int IndexWriter::maxDoc(int i)
{
  if (i >= 0 && i < segmentInfos->size()) {
    return segmentInfos->info(i)->info->maxDoc();
  } else {
    return -1;
  }
}

int IndexWriter::getFlushCount() { return flushCount->get(); }

int IndexWriter::getFlushDeletesCount() { return flushDeletesCount->get(); }

shared_ptr<Set<wstring>> IndexWriter::getFieldNames()
{
  return globalFieldNumberMap->getFieldNames(); // FieldNumbers#getFieldNames()
                                                // returns an unmodifiableSet
}

wstring IndexWriter::newSegmentName()
{
  // Cannot synchronize on IndexWriter because that causes
  // deadlock
  {
    lock_guard<mutex> lock(segmentInfos);
    // Important to increment changeCount so that the
    // segmentInfos is written on close.  Otherwise we
    // could close, re-open and re-return the same segment
    // name that was previously returned which can cause
    // problems at least with ConcurrentMergeScheduler.
    changeCount->incrementAndGet();
    segmentInfos->changed();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return L"_" + Long::toString(segmentInfos->counter++, Character::MAX_RADIX);
  }
}

void IndexWriter::forceMerge(int maxNumSegments) 
{
  forceMerge(maxNumSegments, true);
}

void IndexWriter::forceMerge(int maxNumSegments, bool doWait) 
{
  ensureOpen();

  if (maxNumSegments < 1) {
    throw invalid_argument(L"maxNumSegments must be >= 1; got " +
                           to_wstring(maxNumSegments));
  }

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"forceMerge: index now " + segString());
    infoStream->message(L"IW", L"now flush at forceMerge");
  }
  flush(true, true);
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    resetMergeExceptions();
    segmentsToMerge.clear();
    for (auto info : segmentInfos) {
      assert(info != nullptr);
      segmentsToMerge.emplace(info, Boolean::TRUE);
    }
    mergeMaxNumSegments = maxNumSegments;

    // Now mark all pending & running merges for forced
    // merge:
    for (auto merge : pendingMerges) {
      merge->maxNumSegments = maxNumSegments;
      if (merge->info != nullptr) {
        // TODO: explain why this is sometimes still null
        segmentsToMerge.emplace(merge->info, Boolean::TRUE);
      }
    }

    for (auto merge : runningMerges) {
      merge->maxNumSegments = maxNumSegments;
      if (merge->info != nullptr) {
        // TODO: explain why this is sometimes still null
        segmentsToMerge.emplace(merge->info, Boolean::TRUE);
      }
    }
  }

  maybeMerge(config->getMergePolicy(), MergeTrigger::EXPLICIT, maxNumSegments);

  if (doWait) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      while (true) {

        if (tragedy->get() != nullptr) {
          throw make_shared<IllegalStateException>(
              L"this writer hit an unrecoverable error; cannot complete "
              L"forceMerge",
              tragedy->get());
        }

        if (mergeExceptions.size() > 0) {
          // Forward any exceptions in background merge
          // threads to the current thread:
          constexpr int size = mergeExceptions.size();
          for (int i = 0; i < size; i++) {
            shared_ptr<MergePolicy::OneMerge> *const merge = mergeExceptions[i];
            if (merge->maxNumSegments != UNBOUNDED_MAX_MERGE_SEGMENTS) {
              throw make_shared<IOException>(
                  L"background merge hit exception: " + merge->segString(),
                  merge->getException());
            }
          }
        }

        if (maxNumSegmentsMergesPending()) {
          this->doWait();
        } else {
          break;
        }
      }
    }

    // If close is called while we are still
    // running, throw an exception so the calling
    // thread will know merging did not
    // complete
    ensureOpen();
  }
  // NOTE: in the ConcurrentMergeScheduler case, when
  // doWait is false, we can return immediately while
  // background threads accomplish the merging
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::maxNumSegmentsMergesPending()
{
  for (auto merge : pendingMerges) {
    if (merge->maxNumSegments != UNBOUNDED_MAX_MERGE_SEGMENTS) {
      return true;
    }
  }

  for (auto merge : runningMerges) {
    if (merge->maxNumSegments != UNBOUNDED_MAX_MERGE_SEGMENTS) {
      return true;
    }
  }

  return false;
}

void IndexWriter::forceMergeDeletes(bool doWait) 
{
  ensureOpen();

  flush(true, true);

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"forceMergeDeletes: index now " + segString());
  }

  shared_ptr<MergePolicy> *const mergePolicy = config->getMergePolicy();
  shared_ptr<MergePolicy::MergeSpecification> spec;
  bool newMergesFound = false;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    spec =
        mergePolicy->findForcedDeletesMerges(segmentInfos, shared_from_this());
    newMergesFound = spec != nullptr;
    if (newMergesFound) {
      constexpr int numMerges = spec->merges.size();
      for (int i = 0; i < numMerges; i++) {
        registerMerge(spec->merges[i]);
      }
    }
  }

  mergeScheduler->merge(shared_from_this(), MergeTrigger::EXPLICIT,
                        newMergesFound);

  if (spec != nullptr && doWait) {
    constexpr int numMerges = spec->merges.size();
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      bool running = true;
      while (running) {

        if (tragedy->get() != nullptr) {
          throw make_shared<IllegalStateException>(
              L"this writer hit an unrecoverable error; cannot complete "
              L"forceMergeDeletes",
              tragedy->get());
        }

        // Check each merge that MergePolicy asked us to
        // do, to see if any of them are still running and
        // if any of them have hit an exception.
        running = false;
        for (int i = 0; i < numMerges; i++) {
          shared_ptr<MergePolicy::OneMerge> *const merge = spec->merges[i];
          if (find(pendingMerges.begin(), pendingMerges.end(), merge) !=
                  pendingMerges.end() ||
              runningMerges->contains(merge)) {
            running = true;
          }
          runtime_error t = merge->getException();
          if (t != nullptr) {
            throw make_shared<IOException>(
                L"background merge hit exception: " + merge->segString(), t);
          }
        }

        // If any of our merges are still running, wait:
        if (running) {
          this->doWait();
        }
      }
    }
  }

  // NOTE: in the ConcurrentMergeScheduler case, when
  // doWait is false, we can return immediately while
  // background threads accomplish the merging
}

void IndexWriter::forceMergeDeletes() 
{
  forceMergeDeletes(true);
}

void IndexWriter::maybeMerge() 
{
  maybeMerge(config->getMergePolicy(), MergeTrigger::EXPLICIT,
             UNBOUNDED_MAX_MERGE_SEGMENTS);
}

void IndexWriter::maybeMerge(shared_ptr<MergePolicy> mergePolicy,
                             MergeTrigger trigger,
                             int maxNumSegments) 
{
  ensureOpen(false);
  bool newMergesFound =
      updatePendingMerges(mergePolicy, trigger, maxNumSegments);
  mergeScheduler->merge(shared_from_this(), trigger, newMergesFound);
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::updatePendingMerges(shared_ptr<MergePolicy> mergePolicy,
                                      MergeTrigger trigger,
                                      int maxNumSegments) 
{

  // In case infoStream was disabled on init, but then enabled at some
  // point, try again to log the config here:
  messageState();

  assert(maxNumSegments == UNBOUNDED_MAX_MERGE_SEGMENTS || maxNumSegments > 0);
  assert(trigger != nullptr);
  if (stopMerges) {
    return false;
  }

  // Do not start new merges if disaster struck
  if (tragedy->get() != nullptr) {
    return false;
  }
  bool newMergesFound = false;
  shared_ptr<MergePolicy::MergeSpecification> *const spec;
  if (maxNumSegments != UNBOUNDED_MAX_MERGE_SEGMENTS) {
    assert((trigger == MergeTrigger::EXPLICIT ||
                trigger == MergeTrigger::MERGE_FINISHED,
            L"Expected EXPLICT or MERGE_FINISHED as trigger even with "
            L"maxNumSegments set but was: " +
                trigger.name()));

    spec = mergePolicy->findForcedMerges(
        segmentInfos, maxNumSegments,
        segmentsToMerge, shared_from_this());
    newMergesFound = spec != nullptr;
    if (newMergesFound) {
      constexpr int numMerges = spec->merges.size();
      for (int i = 0; i < numMerges; i++) {
        shared_ptr<MergePolicy::OneMerge> *const merge = spec->merges[i];
        merge->maxNumSegments = maxNumSegments;
      }
    }
  } else {
    spec = mergePolicy->findMerges(trigger, segmentInfos, shared_from_this());
  }
  newMergesFound = spec != nullptr;
  if (newMergesFound) {
    constexpr int numMerges = spec->merges.size();
    for (int i = 0; i < numMerges; i++) {
      registerMerge(spec->merges[i]);
    }
  }
  return newMergesFound;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>>
IndexWriter::getMergingSegments()
{
  return Collections::unmodifiableSet(mergingSegments);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<MergePolicy::OneMerge> IndexWriter::getNextMerge()
{
  if (pendingMerges.empty()) {
    return nullptr;
  } else {
    // Advance the merge from pending to running
    shared_ptr<MergePolicy::OneMerge> merge = pendingMerges.pop_front();
    runningMerges->add(merge);
    return merge;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::hasPendingMerges() { return pendingMerges.size() != 0; }

void IndexWriter::rollback() 
{
  // don't call ensureOpen here: this acts like "close()" in closeable.

  // Ensure that only one thread actually gets to do the
  // closing, and make sure no commit is also in progress:
  if (shouldClose(true)) {
    rollbackInternal();
  }
}

void IndexWriter::rollbackInternal() 
{
  {
    // Make sure no commit is running, else e.g. we can close while another
    // thread is still fsync'ing:
    lock_guard<mutex> lock(commitLock);
    rollbackInternalNoCommit();

    assert((pendingNumDocs->get() == segmentInfos->totalMaxDoc(),
            L"pendingNumDocs " + pendingNumDocs->get() + L" != " +
                to_wstring(segmentInfos->totalMaxDoc()) + L" totalMaxDoc"));
  }
}

void IndexWriter::rollbackInternalNoCommit() 
{
  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"rollback");
  }

  try {
    abortMerges();
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"rollback: done finish merges");
    }

    // Must pre-close in case it increments changeCount so that we can then
    // set it to false before calling rollbackInternal
    delete mergeScheduler;

    delete docWriter; // mark it as closed first to prevent subsequent indexing
                      // actions/flushes
    assert((!Thread::holdsLock(shared_from_this()),
            L"IndexWriter lock should never be hold when aborting"));
    docWriter->abort(); // don't sync on IW here
    docWriter->flushControl
        ->waitForFlush();         // wait for all concurrently running flushes
    publishFlushedSegments(true); // empty the flush ticket queue otherwise we
                                  // might not have cleaned up all resources
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {

      if (pendingCommit->size() > 0) {
        pendingCommit->rollbackCommit(directory);
        try {
          deleter->decRef(pendingCommit);
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          pendingCommit.reset();
          notifyAll();
        }
      }
      constexpr int totalMaxDoc = segmentInfos->totalMaxDoc();
      // Keep the same segmentInfos instance but replace all
      // of its SegmentInfo instances so IFD below will remove
      // any segments we flushed since the last commit:
      segmentInfos->rollbackSegmentInfos(rollbackSegments);
      int rollbackMaxDoc = segmentInfos->totalMaxDoc();
      // now we need to adjust this back to the rolled back SI but don't set it
      // to the absolute value otherwise we might hide internal bugsf
      adjustPendingNumDocs(-(totalMaxDoc - rollbackMaxDoc));
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW",
                            L"rollback: infos=" + segString(segmentInfos));
      }

      testPoint(L"rollback before checkpoint");

      // Ask deleter to locate unreferenced files & remove
      // them ... only when we are not experiencing a tragedy, else
      // these methods throw ACE:
      if (tragedy->get() == nullptr) {
        deleter->checkpoint(segmentInfos, false);
        deleter->refresh();
        delete deleter;
      }

      lastCommitChangeCount = changeCount->get();
      // Don't bother saving any changes in our segmentInfos
      delete readerPool;
      // Must set closed while inside same sync block where we call
      // deleter.refresh, else concurrent threads may try to sneak a flush in,
      // after we leave this sync block and before we enter the sync block in
      // the finally clause below that sets closed:
      closed = true;

      IOUtils::close({writeLock}); // release write lock
      writeLock.reset();
      closed = true;
      closing = false;
      // So any "concurrently closing" threads wake up and see that the close
      // has now completed:
      notifyAll();
    }
  } catch (const runtime_error &throwable) {
    try {
      // Must not hold IW's lock while closing
      // mergeScheduler: this can lead to deadlock,
      // e.g. TestIW.testThreadInterruptDeadlock
      IOUtils::closeWhileHandlingException({mergeScheduler});
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        // we tried to be nice about it: do the minimum
        // don't leak a segments_N file if there is a pending commit
        if (pendingCommit->size() > 0) {
          try {
            pendingCommit->rollbackCommit(directory);
            deleter->decRef(pendingCommit);
          } catch (const runtime_error &t) {
            throwable.addSuppressed(t);
          }
          pendingCommit.reset();
        }

        // close all the closeables we can (but important is readerPool and
        // writeLock to prevent leaks)
        IOUtils::closeWhileHandlingException({readerPool, deleter, writeLock});
        writeLock.reset();
        closed = true;
        closing = false;

        // So any "concurrently closing" threads wake up and see that the close
        // has now completed:
        notifyAll();
      }
    } catch (const runtime_error &t) {
      throwable.addSuppressed(t);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (std::dynamic_pointer_cast<VirtualMachineError>(throwable) !=
          nullptr) {
        try {
          tragicEvent(throwable, L"rollbackInternal");
        } catch (const runtime_error &t1) {
          throwable.addSuppressed(t1);
        }
      }
    }
    throw throwable;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") public long deleteAll() throws
// java.io.IOException
int64_t IndexWriter::deleteAll() 
{
  ensureOpen();
  // Remove any buffered docs
  bool success = false;
  /* hold the full flush lock to prevent concurrency commits / NRT reopens to
   * get in our way and do unnecessary work. -- if we don't lock this here we
   * might get in trouble if */
  /*
   * We first abort and trash everything we have in-memory
   * and keep the thread-states locked, the lockAndAbortAll operation
   * also guarantees "point in time semantics" ie. the checkpoint that we need
   * in terms of logical happens-before relationship in the DW. So we do abort
   * all in memory structures We also drop global field numbering before during
   * abort to make sure it's just like a fresh index.
   */
  try {
    {
      lock_guard<mutex> lock(fullFlushLock);
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer =
      // docWriter.lockAndAbortAll())
      {
        java::io::Closeable finalizer = docWriter->lockAndAbortAll();
        processEvents(false);
        // C++ TODO: Multithread locking on 'this' is not converted to native
        // C++:
        synchronized(shared_from_this())
        {
          try {
            // Abort any running merges
            abortMerges();
            // Let merges run again
            stopMerges = false;
            adjustPendingNumDocs(-segmentInfos->totalMaxDoc());
            // Remove all segments
            segmentInfos->clear();
            // Ask deleter to locate unreferenced files & remove them:
            deleter->checkpoint(segmentInfos, false);

            /* don't refresh the deleter here since there might
             * be concurrent indexing requests coming in opening
             * files on the directory after we called DW#abort()
             * if we do so these indexing requests might hit FNF exceptions.
             * We will remove the files incrementally as we go...
             */
            // Don't bother saving any changes in our segmentInfos
            readerPool->dropAll();
            // Mark that the index has changed
            changeCount->incrementAndGet();
            segmentInfos->changed();
            globalFieldNumberMap->clear();
            success = true;
            int64_t seqNo = docWriter->deleteQueue->getNextSequenceNumber();
            docWriter->setLastSeqNo(seqNo);
            return seqNo;
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            if (success == false) {
              if (infoStream->isEnabled(L"IW")) {
                infoStream->message(L"IW", L"hit exception during deleteAll");
              }
            }
          }
        }
      }
    }
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"deleteAll");
    throw tragedy;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::abortMerges()
{

  stopMerges = true;

  // Abort all pending & running merges:
  for (auto merge : pendingMerges) {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"now abort pending merge " +
                                     segString(merge->segments));
    }
    merge->setAborted();
    mergeFinish(merge);
  }
  pendingMerges.clear();

  for (auto merge : runningMerges) {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"now abort running merge " +
                                     segString(merge->segments));
    }
    merge->setAborted();
  }

  // We wait here to make all merges stop.  It should not
  // take very long because they periodically check if
  // they are aborted.
  while (runningMerges->size() != 0) {

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"now wait for " + runningMerges->size() +
                                     L" running merge/s to abort");
    }

    doWait();
  }

  notifyAll();
  assert(0 == mergingSegments.size());

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"all running merges have aborted");
  }
}

void IndexWriter::waitForMerges() 
{

  // Give merge scheduler last chance to run, in case
  // any pending merges are waiting. We can't hold IW's lock
  // when going into merge because it can lead to deadlock.
  mergeScheduler->merge(shared_from_this(), MergeTrigger::CLOSING, false);

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    ensureOpen(false);
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"waitForMerges");
    }

    while (pendingMerges.size() > 0 || runningMerges->size() > 0) {
      doWait();
    }

    // sanity check
    assert(0 == mergingSegments.size());

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"waitForMerges done");
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::checkpoint() 
{
  changed();
  deleter->checkpoint(segmentInfos, false);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::checkpointNoSIS() 
{
  changeCount->incrementAndGet();
  deleter->checkpoint(segmentInfos, false);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::changed()
{
  changeCount->incrementAndGet();
  segmentInfos->changed();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
IndexWriter::publishFrozenUpdates(shared_ptr<FrozenBufferedUpdates> packet)
{
  assert(packet != nullptr && packet->any());
  int64_t nextGen = bufferedUpdatesStream->push(packet);
  // Do this as an event so it applies higher in the stack when we are not
  // holding DocumentsWriterFlushQueue.purgeLock:
  eventQueue.push_back([&](any w) {
    try {
      packet->apply(w);
    } catch (const runtime_error &t) {
      try {
        w::onTragicEvent(t, L"applyUpdatesPacket");
      } catch (const runtime_error &t1) {
        t.addSuppressed(t1);
      }
      throw t;
    }
    w::flushDeletesCount::incrementAndGet();
  });
  return nextGen;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::publishFlushedSegment(
    shared_ptr<SegmentCommitInfo> newSegment, shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<FrozenBufferedUpdates> packet,
    shared_ptr<FrozenBufferedUpdates> globalPacket,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  bool published = false;
  try {
    // Lock order IW -> BDS
    ensureOpen(false);

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"publishFlushedSegment " + newSegment);
    }

    if (globalPacket != nullptr && globalPacket->any()) {
      publishFrozenUpdates(globalPacket);
    }

    // Publishing the segment must be sync'd on IW -> BDS to make the sure
    // that no merge prunes away the seg. private delete packet
    constexpr int64_t nextGen;
    if (packet != nullptr && packet->any()) {
      nextGen = publishFrozenUpdates(packet);
    } else {
      // Since we don't have a delete packet to apply we can get a new
      // generation right away
      nextGen = bufferedUpdatesStream->getNextGen();
      // No deletes/updates here, so marked finished immediately:
      bufferedUpdatesStream->finishedSegment(nextGen);
    }
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"publish sets newSegment delGen=" +
                                     to_wstring(nextGen) + L" seg=" +
                                     segString(newSegment));
    }
    newSegment->setBufferedDeletesGen(nextGen);
    segmentInfos->push_back(newSegment);
    published = true;
    checkpoint();
    if (packet != nullptr && packet->any() && sortMap != nullptr) {
      // TODO: not great we do this heavyish op while holding IW's monitor lock,
      // but it only applies if you are using sorted indices and updating doc
      // values:
      shared_ptr<ReadersAndUpdates> rld = getPooledInstance(newSegment, true);
      rld->sortMap = sortMap;
      // DON't release this ReadersAndUpdates we need to stick with that sortMap
    }
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(
        config->softDeletesField); // will return null if no soft deletes are
                                   // present
    // this is a corner case where documents delete them-self with soft deletes.
    // This is used to build delete tombstones etc. in this case we haven't seen
    // any updates to the DV in this fresh flushed segment. if we have seen
    // updates the update code checks if the segment is fully deleted.
    bool hasInitialSoftDeleted =
        (fieldInfo != nullptr && fieldInfo->getDocValuesGen() == -1 &&
         fieldInfo->getDocValuesType() != DocValuesType::NONE);
    constexpr bool isFullyHardDeleted =
        newSegment->getDelCount() == newSegment->info->maxDoc();
    // we either have a fully hard-deleted segment or one or more docs are
    // soft-deleted. In both cases we need to go and check if they are fully
    // deleted. This has the nice side-effect that we now have accurate numbers
    // for the soft delete right after we flushed to disk.
    if (hasInitialSoftDeleted || isFullyHardDeleted) {
      // this operation is only really executed if needed an if soft-deletes are
      // not configured it only be executed if we deleted all docs in this newly
      // flushed segment.
      shared_ptr<ReadersAndUpdates> rld = getPooledInstance(newSegment, true);
      try {
        if (isFullyDeleted(rld)) {
          dropDeletedSegment(newSegment);
          checkpoint();
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        release(rld);
      }
    }

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (published == false) {
      adjustPendingNumDocs(-newSegment->info->maxDoc());
    }
    flushCount->incrementAndGet();
    doAfterFlush();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::resetMergeExceptions()
{
  mergeExceptions = deque<>();
  mergeGen++;
}

void IndexWriter::noDupDirs(deque<Directory> &dirs)
{
  unordered_set<std::shared_ptr<Directory>> dups =
      unordered_set<std::shared_ptr<Directory>>();
  for (int i = 0; i < dirs->length; i++) {
    if (find(dups.begin(), dups.end(), dirs[i]) != dups.end()) {
      throw invalid_argument(L"Directory " + dirs[i] +
                             L" appears more than once");
    }
    if (dirs[i] == directoryOrig) {
      throw invalid_argument(L"Cannot add directory to itself");
    }
    dups.insert(dirs[i]);
  }
}

deque<std::shared_ptr<Lock>>
IndexWriter::acquireWriteLocks(deque<Directory> &dirs) 
{
  deque<std::shared_ptr<Lock>> locks =
      deque<std::shared_ptr<Lock>>(dirs->length);
  for (int i = 0; i < dirs->length; i++) {
    bool success = false;
    try {
      shared_ptr<Lock> lock = dirs[i].obtainLock(WRITE_LOCK_NAME);
      locks.push_back(lock);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success == false) {
        // Release all previously acquired locks:
        // TODO: addSuppressed? it could be many...
        IOUtils::closeWhileHandlingException(locks);
      }
    }
  }
  return locks;
}

int64_t IndexWriter::addIndexes(deque<Directory> &dirs) 
{
  ensureOpen();

  noDupDirs({dirs});

  deque<std::shared_ptr<Lock>> locks = acquireWriteLocks({dirs});

  shared_ptr<Sort> indexSort = config->getIndexSort();

  bool successTop = false;

  int64_t seqNo;

  try {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"flush at addIndexes(Directory...)");
    }

    flush(false, true);

    deque<std::shared_ptr<SegmentCommitInfo>> infos =
        deque<std::shared_ptr<SegmentCommitInfo>>();

    // long so we can detect int overflow:
    int64_t totalMaxDoc = 0;
    deque<std::shared_ptr<SegmentInfos>> commits =
        deque<std::shared_ptr<SegmentInfos>>(dirs->length);
    for (shared_ptr<Directory> dir : dirs) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"addIndexes: process directory " + dir);
      }
      shared_ptr<SegmentInfos> sis =
          SegmentInfos::readLatestCommit(dir); // read infos from dir
      if (segmentInfos->getIndexCreatedVersionMajor() !=
          sis->getIndexCreatedVersionMajor()) {
        throw invalid_argument(
            wstring(L"Cannot use addIndexes(Directory) with indexes that have "
                    L"been created ") +
            L"by a different Lucene version. The current index was generated "
            L"by Lucene " +
            to_wstring(segmentInfos->getIndexCreatedVersionMajor()) +
            L" while one of the directories contains an index that was "
            L"generated with Lucene " +
            to_wstring(sis->getIndexCreatedVersionMajor()));
      }
      totalMaxDoc += sis->totalMaxDoc();
      commits.push_back(sis);
    }

    // Best-effort up front check:
    testReserveDocs(totalMaxDoc);

    bool success = false;
    try {
      for (auto sis : commits) {
        for (auto info : sis) {
          assert((!find(infos.begin(), infos.end(), info) != infos.end(),
                  L"dup info dir=" + info->info->dir + L" name=" +
                      info->info->name));

          shared_ptr<Sort> segmentIndexSort = info->info->getIndexSort();

          if (indexSort != nullptr && segmentIndexSort != nullptr &&
              indexSort->equals(segmentIndexSort) == false) {
            // TODO: we could make this smarter, e.g. if the incoming indexSort
            // is congruent with our sort ("starts with") then it's OK
            throw invalid_argument(L"cannot change index sort from " +
                                   segmentIndexSort + L" to " + indexSort);
          }

          wstring newSegName = newSegmentName();

          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(L"IW",
                                L"addIndexes: process segment origName=" +
                                    info->info->name + L" newName=" +
                                    newSegName + L" info=" + info);
          }

          shared_ptr<IOContext> context =
              make_shared<IOContext>(make_shared<FlushInfo>(
                  info->info->maxDoc(), info->sizeInBytes()));

          shared_ptr<FieldInfos> fis = readFieldInfos(info);
          for (auto fi : fis) {
            // This will throw exceptions if any of the incoming fields have an
            // illegal schema change:
            globalFieldNumberMap->addOrGet(
                fi->name, fi->number, fi->getDocValuesType(),
                fi->getPointDimensionCount(), fi->getPointNumBytes(),
                fi->isSoftDeletesField());
          }
          infos.push_back(copySegmentAsIs(info, newSegName, context));
        }
      }
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        for (auto sipc : infos) {
          // Safe: these files must exist
          deleteNewFiles(sipc->files());
        }
      }
    }

    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      success = false;
      try {
        ensureOpen();

        // Now reserve the docs, just before we update SIS:
        reserveDocs(totalMaxDoc);

        seqNo = docWriter->deleteQueue->getNextSequenceNumber();

        success = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (!success) {
          for (auto sipc : infos) {
            // Safe: these files must exist
            deleteNewFiles(sipc->files());
          }
        }
      }
      segmentInfos->insert(segmentInfos->end(), infos.begin(), infos.end());
      checkpoint();
    }

    successTop = true;

  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"addIndexes(Directory...)");
    throw tragedy;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (successTop) {
      IOUtils::close(locks);
    } else {
      IOUtils::closeWhileHandlingException(locks);
    }
  }
  maybeMerge();

  return seqNo;
}

void IndexWriter::validateMergeReader(shared_ptr<CodecReader> leaf)
{
  shared_ptr<LeafMetaData> segmentMeta = leaf->getMetaData();
  if (segmentInfos->getIndexCreatedVersionMajor() !=
      segmentMeta->getCreatedVersionMajor()) {
    throw invalid_argument(
        L"Cannot merge a segment that has been created with major version " +
        to_wstring(segmentMeta->getCreatedVersionMajor()) +
        L" into this index which has been created by major version " +
        to_wstring(segmentInfos->getIndexCreatedVersionMajor()));
  }

  if (segmentInfos->getIndexCreatedVersionMajor() >= 7 &&
      segmentMeta->getMinVersion() == nullptr) {
    throw make_shared<IllegalStateException>(
        L"Indexes created on or after Lucene 7 must record the created version "
        L"major, but " +
        leaf + L" hides it");
  }

  shared_ptr<Sort> leafIndexSort = segmentMeta->getSort();
  if (config->getIndexSort() != nullptr && leafIndexSort != nullptr &&
      config->getIndexSort()->equals(leafIndexSort) == false) {
    throw invalid_argument(L"cannot change index sort from " + leafIndexSort +
                           L" to " + config->getIndexSort());
  }
}

int64_t
IndexWriter::addIndexes(deque<CodecReader> &readers) 
{
  ensureOpen();

  // long so we can detect int overflow:
  int64_t numDocs = 0;
  int64_t seqNo;
  try {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"flush at addIndexes(CodecReader...)");
    }
    flush(false, true);

    wstring mergedName = newSegmentName();
    int numSoftDeleted = 0;
    for (shared_ptr<CodecReader> leaf : readers) {
      numDocs += leaf->numDocs();
      validateMergeReader(leaf);
      if (softDeletesEnabled) {
        shared_ptr<Bits> liveDocs = leaf->getLiveDocs();
        numSoftDeleted += PendingSoftDeletes::countSoftDeletes(
            DocValuesFieldExistsQuery::getDocValuesDocIdSetIterator(
                config->getSoftDeletesField(), leaf),
            liveDocs);
      }
    }

    // Best-effort up front check:
    testReserveDocs(numDocs);

    shared_ptr<IOContext> *const context = make_shared<IOContext>(
        make_shared<MergeInfo>(Math::toIntExact(numDocs), -1, false,
                               UNBOUNDED_MAX_MERGE_SEGMENTS));

    // TODO: somehow we should fix this merge so it's
    // abortable so that IW.close(false) is able to stop it
    shared_ptr<TrackingDirectoryWrapper> trackingDir =
        make_shared<TrackingDirectoryWrapper>(directory);

    // We set the min version to null for now, it will be set later by
    // SegmentMerger
    shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
        directoryOrig, Version::LATEST, nullptr, mergedName, -1, false, codec,
        Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
        config->getIndexSort());

    shared_ptr<SegmentMerger> merger =
        make_shared<SegmentMerger>(Arrays::asList(readers), info, infoStream,
                                   trackingDir, globalFieldNumberMap, context);

    if (!merger->shouldMerge()) {
      return docWriter->deleteQueue->getNextSequenceNumber();
    }

    merger->merge(); // merge 'em
    shared_ptr<SegmentCommitInfo> infoPerCommit =
        make_shared<SegmentCommitInfo>(info, 0, numSoftDeleted, -1LL, -1LL,
                                       -1LL);

    info->setFiles(unordered_set<>(trackingDir->getCreatedFiles()));
    trackingDir->clearCreatedFiles();

    setDiagnostics(info, SOURCE_ADDINDEXES_READERS);

    shared_ptr<MergePolicy> *const mergePolicy = config->getMergePolicy();
    bool useCompoundFile;
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    { // Guard segmentInfos
      if (stopMerges) {
        // Safe: these files must exist
        deleteNewFiles(infoPerCommit->files());

        return docWriter->deleteQueue->getNextSequenceNumber();
      }
      ensureOpen();
      useCompoundFile = mergePolicy->useCompoundFile(
          segmentInfos, infoPerCommit, shared_from_this());
    }

    // Now create the compound file if needed
    if (useCompoundFile) {
      shared_ptr<deque<wstring>> filesToDelete = infoPerCommit->files();
      shared_ptr<TrackingDirectoryWrapper> trackingCFSDir =
          make_shared<TrackingDirectoryWrapper>(directory);
      // TODO: unlike merge, on exception we arent sniping any trash cfs files
      // here? createCompoundFile tries to cleanup, but it might not always be
      // able to...
      try {
        createCompoundFile(infoStream, trackingCFSDir, info, context,
                           shared_from_this()::deleteNewFiles);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // delete new non cfs files directly: they were never
        // registered with IFD
        deleteNewFiles(filesToDelete);
      }
      info->setUseCompoundFile(true);
    }

    // Have codec write SegmentInfo.  Must do this after
    // creating CFS so that 1) .si isn't slurped into CFS,
    // and 2) .si reflects useCompoundFile=true change
    // above:
    codec->segmentInfoFormat()->write(trackingDir, info, context);

    info->addFiles(trackingDir->getCreatedFiles());

    // Register the new segment
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      if (stopMerges) {
        // Safe: these files must exist
        deleteNewFiles(infoPerCommit->files());

        return docWriter->deleteQueue->getNextSequenceNumber();
      }
      ensureOpen();

      // Now reserve the docs, just before we update SIS:
      reserveDocs(numDocs);

      segmentInfos->push_back(infoPerCommit);
      seqNo = docWriter->deleteQueue->getNextSequenceNumber();
      checkpoint();
    }
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"addIndexes(CodecReader...)");
    throw tragedy;
  }
  maybeMerge();

  return seqNo;
}

shared_ptr<SegmentCommitInfo>
IndexWriter::copySegmentAsIs(shared_ptr<SegmentCommitInfo> info,
                             const wstring &segName,
                             shared_ptr<IOContext> context) 
{

  // Same SI as before but we change directory and name
  shared_ptr<SegmentInfo> newInfo = make_shared<SegmentInfo>(
      directoryOrig, info->info->getVersion(), info->info->getMinVersion(),
      segName, info->info->maxDoc(), info->info->getUseCompoundFile(),
      info->info->getCodec(), info->info->getDiagnostics(), info->info->getId(),
      info->info->getAttributes(), info->info->getIndexSort());
  shared_ptr<SegmentCommitInfo> newInfoPerCommit =
      make_shared<SegmentCommitInfo>(
          newInfo, info->getDelCount(), info->getSoftDelCount(),
          info->getDelGen(), info->getFieldInfosGen(), info->getDocValuesGen());

  newInfo->setFiles(info->info->files());
  newInfoPerCommit->setFieldInfosFiles(info->getFieldInfosFiles());
  newInfoPerCommit->setDocValuesUpdatesFiles(info->getDocValuesUpdatesFiles());

  bool success = false;

  shared_ptr<Set<wstring>> copiedFiles = unordered_set<wstring>();
  try {
    // Copy the segment's files
    for (auto file : info->files()) {
      const wstring newFileName = newInfo->namedForThisSegment(file);
      directory->copyFrom(info->info->dir, file, newFileName, context);
      copiedFiles->add(newFileName);
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // Safe: these files must exist
      deleteNewFiles(copiedFiles);
    }
  }

  assert((copiedFiles->equals(newInfoPerCommit->files()),
          L"copiedFiles=" + copiedFiles + L" vs " + newInfoPerCommit->files()));

  return newInfoPerCommit;
}

void IndexWriter::doAfterFlush()  {}

void IndexWriter::doBeforeFlush()  {}

int64_t IndexWriter::prepareCommit() 
{
  ensureOpen();
  pendingSeqNo = prepareCommitInternal();
  // we must do this outside of the commitLock else we can deadlock:
  if (maybeMerge_->getAndSet(false)) {
    maybeMerge(config->getMergePolicy(), MergeTrigger::FULL_FLUSH,
               UNBOUNDED_MAX_MERGE_SEGMENTS);
  }
  return pendingSeqNo;
}

bool IndexWriter::flushNextBuffer() 
{
  try {
    if (docWriter->flushOneDWPT()) {
      processEvents(true);
      return true; // we wrote a segment
    }
    return false;
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"flushNextBuffer");
    throw tragedy;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    maybeCloseOnTragicEvent();
  }
}

int64_t IndexWriter::prepareCommitInternal() 
{
  startCommitTime = System::nanoTime();
  {
    lock_guard<mutex> lock(commitLock);
    ensureOpen(false);
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"prepareCommit: flush");
      infoStream->message(L"IW", L"  index before flush " + segString());
    }

    if (tragedy->get() != nullptr) {
      throw make_shared<IllegalStateException>(
          L"this writer hit an unrecoverable error; cannot commit",
          tragedy->get());
    }

    if (pendingCommit->size() > 0) {
      throw make_shared<IllegalStateException>(
          L"prepareCommit was already called with no corresponding call to "
          L"commit");
    }

    doBeforeFlush();
    testPoint(L"startDoFlush");
    shared_ptr<SegmentInfos> toCommit;
    bool anyChanges = false;
    int64_t seqNo;

    // This is copied from doFlush, except it's modified to
    // clone & incRef the flushed SegmentInfos inside the
    // sync block:

    try {

      {
        lock_guard<mutex> lock(fullFlushLock);
        bool flushSuccess = false;
        bool success = false;
        try {
          seqNo = docWriter->flushAllThreads();
          if (seqNo < 0) {
            anyChanges = true;
            seqNo = -seqNo;
          }
          if (anyChanges == false) {
            // prevent double increment since docWriter#doFlush increments the
            // flushcount if we flushed anything.
            flushCount->incrementAndGet();
          }
          publishFlushedSegments(true);
          // cannot pass triggerMerges=true here else it can lead to deadlock:
          processEvents(false);

          flushSuccess = true;

          applyAllDeletesAndUpdates();
          // C++ TODO: Multithread locking on 'this' is not converted to native
          // C++:
          synchronized(shared_from_this())
          {
            writeReaderPool(true);
            if (changeCount->get() != lastCommitChangeCount) {
              // There are changes to commit, so we will write a new segments_N
              // in startCommit. The act of committing is itself an NRT-visible
              // change (an NRT reader that was just opened before this should
              // see it on reopen) so we increment changeCount and segments
              // version so a future NRT reopen will see the change:
              changeCount->incrementAndGet();
              segmentInfos->changed();
            }

            if (commitUserData.size() > 0) {
              unordered_map<wstring, wstring> userData =
                  unordered_map<wstring, wstring>();
              for (auto ent : commitUserData) {
                userData.emplace(ent.first, ent.second);
              }
              segmentInfos->setUserData(userData, false);
            }

            // Must clone the segmentInfos while we still
            // hold fullFlushLock and while sync'd so that
            // no partial changes (eg a delete w/o
            // corresponding add from an updateDocument) can
            // sneak into the commit point:
            toCommit = segmentInfos->clone();

            pendingCommitChangeCount = changeCount->get();

            // This protects the segmentInfos we are now going
            // to commit.  This is important in case, eg, while
            // we are trying to sync all referenced files, a
            // merge completes which would otherwise have
            // removed the files we are now syncing.
            filesToCommit = toCommit->files(false);
            deleter->incRef(filesToCommit);
          }
          success = true;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          if (!success) {
            if (infoStream->isEnabled(L"IW")) {
              infoStream->message(L"IW", L"hit exception during prepareCommit");
            }
          }
          assert(holdsFullFlushLock());
          // Done: finish the full flush!
          docWriter->finishFullFlush(flushSuccess);
          doAfterFlush();
        }
      }
    } catch (const VirtualMachineError &tragedy) {
      tragicEvent(tragedy, L"prepareCommit");
      throw tragedy;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      maybeCloseOnTragicEvent();
    }

    try {
      if (anyChanges) {
        maybeMerge_->set(true);
      }
      startCommit(toCommit);
      if (pendingCommit->empty()) {
        return -1;
      } else {
        return seqNo;
      }
    } catch (const runtime_error &t) {
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        if (filesToCommit != nullptr) {
          try {
            deleter->decRef(filesToCommit);
          } catch (const runtime_error &t1) {
            t.addSuppressed(t1);
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            filesToCommit.reset();
          }
        }
      }
      throw t;
    }
  }
}

void IndexWriter::writeReaderPool(bool writeDeletes) 
{
  assert(Thread::holdsLock(shared_from_this()));
  if (writeDeletes) {
    if (readerPool->commit(segmentInfos)) {
      checkpointNoSIS();
    }
  } else { // only write the docValues
    if (readerPool->writeAllDocValuesUpdates()) {
      checkpoint();
    }
  }
  // now do some best effort to check if a segment is fully deleted
  deque<std::shared_ptr<SegmentCommitInfo>> toDrop =
      deque<std::shared_ptr<SegmentCommitInfo>>(); // don't modify segmentInfos
                                                    // in-place
  for (auto info : segmentInfos) {
    shared_ptr<ReadersAndUpdates> readersAndUpdates =
        readerPool->get(info, false);
    if (readersAndUpdates != nullptr) {
      if (isFullyDeleted(readersAndUpdates)) {
        toDrop.push_back(info);
      }
    }
  }
  for (auto info : toDrop) {
    dropDeletedSegment(info);
  }
  if (toDrop.empty() == false) {
    checkpoint();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::setLiveCommitData(
    deque<unordered_map::Entry<wstring, wstring>> &commitUserData)
{
  setLiveCommitData(commitUserData, true);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::setLiveCommitData(
    deque<unordered_map::Entry<wstring, wstring>> &commitUserData,
    bool doIncrementVersion)
{
  this->commitUserData = commitUserData;
  if (doIncrementVersion) {
    segmentInfos->changed();
  }
  changeCount->incrementAndGet();
}

// C++ WARNING: The following method was originally marked 'synchronized':
deque<unordered_map::Entry<wstring, wstring>> IndexWriter::getLiveCommitData()
{
  return commitUserData;
}

int64_t IndexWriter::commit() 
{
  ensureOpen();
  return commitInternal(config->getMergePolicy());
}

bool IndexWriter::hasUncommittedChanges()
{
  return changeCount->get() != lastCommitChangeCount ||
         docWriter->anyChanges() || bufferedUpdatesStream->any();
}

int64_t IndexWriter::commitInternal(
    shared_ptr<MergePolicy> mergePolicy) 
{

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"commit: start");
  }

  int64_t seqNo;

  {
    lock_guard<mutex> lock(commitLock);
    ensureOpen(false);

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"commit: enter lock");
    }

    if (pendingCommit->empty()) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"commit: now prepare");
      }
      seqNo = prepareCommitInternal();
    } else {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"commit: already prepared");
      }
      seqNo = pendingSeqNo;
    }

    finishCommit();
  }

  // we must do this outside of the commitLock else we can deadlock:
  if (maybeMerge_->getAndSet(false)) {
    maybeMerge(mergePolicy, MergeTrigger::FULL_FLUSH,
               UNBOUNDED_MAX_MERGE_SEGMENTS);
  }

  return seqNo;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") private final void finishCommit()
// throws java.io.IOException
void IndexWriter::finishCommit() 
{

  bool commitCompleted = false;
  wstring committedSegmentsFileName = L"";

  try {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      ensureOpen(false);

      if (tragedy->get() != nullptr) {
        throw make_shared<IllegalStateException>(
            L"this writer hit an unrecoverable error; cannot complete commit",
            tragedy->get());
      }

      if (pendingCommit->size() > 0) {
        shared_ptr<deque<wstring>> *const commitFiles =
            this->filesToCommit;
        {

          java::io::Closeable finalizer = [&]() {
            deleter->decRef(commitFiles);
          };
          try {

            if (infoStream->isEnabled(L"IW")) {
              infoStream->message(L"IW", L"commit: pendingCommit != null");
            }

            committedSegmentsFileName = pendingCommit->finishCommit(directory);

            // we committed, if anything goes wrong after this, we are screwed
            // and it's a tragedy:
            commitCompleted = true;

            if (infoStream->isEnabled(L"IW")) {
              infoStream->message(L"IW",
                                  L"commit: done writing segments file \"" +
                                      committedSegmentsFileName + L"\"");
            }

            // NOTE: don't use this.checkpoint() here, because
            // we do not want to increment changeCount:
            deleter->checkpoint(pendingCommit, true);

            // Carry over generation to our master SegmentInfos:
            segmentInfos->updateGeneration(pendingCommit);

            lastCommitChangeCount = pendingCommitChangeCount;
            rollbackSegments = pendingCommit->createBackupSegmentInfos();

          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            notifyAll();
            pendingCommit.reset();
            this->filesToCommit.reset();
          }
        }
      } else {
        assert(filesToCommit == nullptr);
        if (infoStream->isEnabled(L"IW")) {
          infoStream->message(L"IW", L"commit: pendingCommit == null; skip");
        }
      }
    }
  } catch (const runtime_error &t) {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW",
                          L"hit exception during finishCommit: " + t.what());
    }
    if (commitCompleted) {
      tragicEvent(t, L"finishCommit");
    }
    throw t;
  }

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(
        L"IW",
        wstring::format(Locale::ROOT, L"commit: took %.1f msec",
                        (System::nanoTime() - startCommitTime) / 1000000.0));
    infoStream->message(L"IW", L"commit: done");
  }
}

bool IndexWriter::holdsFullFlushLock()
{
  return Thread::holdsLock(fullFlushLock);
}

void IndexWriter::flush()  { flush(true, true); }

void IndexWriter::flush(bool triggerMerge,
                        bool applyAllDeletes) 
{

  // NOTE: this method cannot be sync'd because
  // maybeMerge() in turn calls mergeScheduler.merge which
  // in turn can take a long time to run and we don't want
  // to hold the lock for that.  In the case of
  // ConcurrentMergeScheduler this can lead to deadlock
  // when it stalls due to too many running merges.

  // We can be called during close, when closing==true, so we must pass false to
  // ensureOpen:
  ensureOpen(false);
  if (doFlush(applyAllDeletes) && triggerMerge) {
    maybeMerge(config->getMergePolicy(), MergeTrigger::FULL_FLUSH,
               UNBOUNDED_MAX_MERGE_SEGMENTS);
  }
}

bool IndexWriter::doFlush(bool applyAllDeletes) 
{
  if (tragedy->get() != nullptr) {
    throw make_shared<IllegalStateException>(
        L"this writer hit an unrecoverable error; cannot flush",
        tragedy->get());
  }

  doBeforeFlush();
  testPoint(L"startDoFlush");
  bool success = false;
  try {

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"  start flush: applyAllDeletes=" +
                                     StringHelper::toString(applyAllDeletes));
      infoStream->message(L"IW", L"  index before flush " + segString());
    }
    bool anyChanges = false;

    {
      lock_guard<mutex> lock(fullFlushLock);
      bool flushSuccess = false;
      try {
        int64_t seqNo = docWriter->flushAllThreads();
        if (seqNo < 0) {
          seqNo = -seqNo;
          anyChanges = true;
        } else {
          anyChanges = false;
        }
        if (!anyChanges) {
          // flushCount is incremented in flushAllThreads
          flushCount->incrementAndGet();
        }
        publishFlushedSegments(true);
        flushSuccess = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        assert(holdsFullFlushLock());
        docWriter->finishFullFlush(flushSuccess);
        processEvents(false);
      }
    }

    if (applyAllDeletes) {
      applyAllDeletesAndUpdates();
    }

    anyChanges |= maybeMerge_->getAndSet(false);

    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      writeReaderPool(applyAllDeletes);
      doAfterFlush();
      success = true;
      return anyChanges;
    }
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"doFlush");
    throw tragedy;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"hit exception during flush");
      }
      maybeCloseOnTragicEvent();
    }
  }
}

void IndexWriter::applyAllDeletesAndUpdates() 
{
  assert(Thread::holdsLock(shared_from_this()) == false);
  flushDeletesCount->incrementAndGet();
  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(
        L"IW",
        L"now apply all deletes for all segments buffered updates bytesUsed=" +
            to_wstring(bufferedUpdatesStream->ramBytesUsed()) +
            L" reader pool bytesUsed=" +
            to_wstring(readerPool->ramBytesUsed()));
  }
  bufferedUpdatesStream->waitApplyAll(shared_from_this());
}

shared_ptr<DocumentsWriter> IndexWriter::getDocsWriter() { return docWriter; }

// C++ WARNING: The following method was originally marked 'synchronized':
int IndexWriter::numRamDocs()
{
  ensureOpen();
  return docWriter->getNumDocs();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::ensureValidMerge(shared_ptr<MergePolicy::OneMerge> merge)
{
  for (auto info : merge->segments) {
    if (!find(segmentInfos->begin(), segmentInfos->end(), info) !=
        segmentInfos->end()) {
      throw make_shared<MergePolicy::MergeException>(
          L"MergePolicy selected a segment (" + info->info->name +
              L") that is not in the current index " + segString(),
          directoryOrig);
    }
  }
}

void IndexWriter::skipDeletedDoc(
    std::deque<std::shared_ptr<DocValuesFieldUpdates::Iterator>> &updatesIters,
    int deletedDoc)
{
  for (auto iter : updatesIters) {
    if (iter->docID() == deletedDoc) {
      iter->nextDoc();
    }
    // when entering the method, all iterators must already be beyond the
    // deleted document, or right on it, in which case we advance them over
    // and they must be beyond it now.
    assert((iter->docID() > deletedDoc,
            L"updateDoc=" + to_wstring(iter->docID()) + L" deletedDoc=" +
                to_wstring(deletedDoc)));
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<ReadersAndUpdates> IndexWriter::commitMergedDeletesAndUpdates(
    shared_ptr<MergePolicy::OneMerge> merge,
    shared_ptr<MergeState> mergeState) 
{

  mergeFinishedGen->incrementAndGet();

  testPoint(L"startCommitMergeDeletes");

  const deque<std::shared_ptr<SegmentCommitInfo>> &sourceSegments =
      merge->segments;

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW",
                        L"commitMergeDeletes " + segString(merge->segments));
  }

  // Carefully merge deletes that occurred after we
  // started merging:
  int64_t minGen = numeric_limits<int64_t>::max();

  // Lazy init (only when we find a delete or update to carry over):
  shared_ptr<ReadersAndUpdates> *const mergedDeletesAndUpdates =
      getPooledInstance(merge->info, true);
  int numDeletesBefore = mergedDeletesAndUpdates->getDelCount();
  // field -> delGen -> dv field updates
  unordered_map<
      wstring, unordered_map<int64_t, std::shared_ptr<DocValuesFieldUpdates>>>
      mappedDVUpdates = unordered_map<
          wstring,
          unordered_map<int64_t, std::shared_ptr<DocValuesFieldUpdates>>>();

  bool anyDVUpdates = false;

  assert(sourceSegments.size() == mergeState->docMaps.size());
  for (int i = 0; i < sourceSegments.size(); i++) {
    shared_ptr<SegmentCommitInfo> info = sourceSegments[i];
    minGen = min(info->getBufferedDeletesGen(), minGen);
    constexpr int maxDoc = info->info->maxDoc();
    shared_ptr<ReadersAndUpdates> *const rld = getPooledInstance(info, false);
    // We hold a ref, from when we opened the readers during mergeInit, so it
    // better still be in the pool:
    assert((rld != nullptr, L"seg=" + info->info->name));

    shared_ptr<MergeState::DocMap> segDocMap = mergeState->docMaps[i];
    shared_ptr<MergeState::DocMap> segLeafDocMap = mergeState->leafDocMaps[i];

    carryOverHardDeletes(mergedDeletesAndUpdates, maxDoc,
                         mergeState->liveDocs[i], merge->hardLiveDocs[i],
                         rld->getHardLiveDocs(), segDocMap, segLeafDocMap);

    // Now carry over all doc values updates that were resolved while we were
    // merging, remapping the docIDs to the newly merged docIDs. We only carry
    // over packets that finished resolving; if any are still running
    // (concurrently) they will detect that our merge completed and re-resolve
    // against the newly merged segment:
    unordered_map<wstring, deque<std::shared_ptr<DocValuesFieldUpdates>>>
        mergingDVUpdates = rld->getMergingDVUpdates();
    for (auto ent : mergingDVUpdates) {

      wstring field = ent.first;

      unordered_map<int64_t, std::shared_ptr<DocValuesFieldUpdates>>
          mappedField = mappedDVUpdates[field];
      if (mappedField.empty()) {
        mappedField = unordered_map<>();
        mappedDVUpdates.emplace(field, mappedField);
      }

      for (shared_ptr<DocValuesFieldUpdates> updates : ent.second) {

        if (bufferedUpdatesStream->stillRunning(updates->delGen)) {
          continue;
        }

        // sanity check:
        assert(field == updates->field);

        shared_ptr<DocValuesFieldUpdates> mappedUpdates =
            mappedField[updates->delGen];
        if (mappedUpdates == nullptr) {
          switch (updates->type) {
          case org::apache::lucene::index::DocValuesType::NUMERIC:
            mappedUpdates = make_shared<NumericDocValuesFieldUpdates>(
                updates->delGen, updates->field, merge->info->info->maxDoc());
            break;
          case org::apache::lucene::index::DocValuesType::BINARY:
            mappedUpdates = make_shared<BinaryDocValuesFieldUpdates>(
                updates->delGen, updates->field, merge->info->info->maxDoc());
            break;
          default:
            throw make_shared<AssertionError>();
          }
          mappedField.emplace(updates->delGen, mappedUpdates);
        }

        shared_ptr<DocValuesFieldUpdates::Iterator> it = updates->begin();
        int doc;
        while ((doc = it->nextDoc()) != NO_MORE_DOCS) {
          int mappedDoc = segDocMap->get(segLeafDocMap->get(doc));
          if (mappedDoc != -1) {
            if (it->hasValue()) {
              // not deleted
              mappedUpdates->add(mappedDoc, it);
            } else {
              mappedUpdates->reset(mappedDoc);
            }
            anyDVUpdates = true;
          }
        }
      }
    }
  }

  if (anyDVUpdates) {
    // Persist the merged DV updates onto the RAU for the merged segment:
    for (auto d : mappedDVUpdates) {
      for (shared_ptr<DocValuesFieldUpdates> updates : d.second::values()) {
        updates->finish();
        mergedDeletesAndUpdates->addDVUpdate(updates);
      }
    }
  }

  if (infoStream->isEnabled(L"IW")) {
    if (mergedDeletesAndUpdates == nullptr) {
      infoStream->message(
          L"IW", L"no new deletes or field updates since merge started");
    } else {
      wstring msg = mergedDeletesAndUpdates->getDelCount() -
                    to_wstring(numDeletesBefore) + L" new deletes";
      if (anyDVUpdates) {
        msg += L" and " +
               to_wstring(mergedDeletesAndUpdates->getNumDVUpdates()) +
               L" new field updates";
        msg +=
            L" (" + mergedDeletesAndUpdates->ramBytesUsed->get() + L") bytes";
      }
      msg += L" since merge started";
      infoStream->message(L"IW", msg);
    }
  }

  merge->info->setBufferedDeletesGen(minGen);

  return mergedDeletesAndUpdates;
}

void IndexWriter::carryOverHardDeletes(
    shared_ptr<ReadersAndUpdates> mergedReadersAndUpdates, int maxDoc,
    shared_ptr<Bits> mergeLiveDocs, shared_ptr<Bits> prevHardLiveDocs,
    shared_ptr<Bits> currentHardLiveDocs,
    shared_ptr<MergeState::DocMap> segDocMap,
    shared_ptr<MergeState::DocMap> segLeafDocMap) 
{

  assert(mergeLiveDocs == nullptr || mergeLiveDocs->length() == maxDoc);
  // if we mix soft and hard deletes we need to make sure that we only carry
  // over deletes that were not deleted before. Otherwise the segDocMap doesn't
  // contain a mapping. yet this is also required if any MergePolicy modifies
  // the liveDocs since this is what the segDocMap is build on.
  const function<bool(int)> carryOverDelete =
      mergeLiveDocs == nullptr || mergeLiveDocs == prevHardLiveDocs
      ? [&](any docId) {
          return currentHardLiveDocs->get(docId) == false : [&](any docId) {
            return mergeLiveDocs->get(docId) &&
                   currentHardLiveDocs->get(docId) == false;
          };
        };
  if (prevHardLiveDocs != nullptr) {
    // If we had deletions on starting the merge we must
    // still have deletions now:
    assert(currentHardLiveDocs != nullptr);
    assert(mergeLiveDocs != nullptr);
    assert(prevHardLiveDocs->length() == maxDoc);
    assert(currentHardLiveDocs->length() == maxDoc);

    // There were deletes on this segment when the merge
    // started.  The merge has collapsed away those
    // deletes, but, if new deletes were flushed since
    // the merge started, we must now carefully keep any
    // newly flushed deletes but mapping them to the new
    // docIDs.

    // Since we copy-on-write, if any new deletes were
    // applied after merging has started, we can just
    // check if the before/after liveDocs have changed.
    // If so, we must carefully merge the liveDocs one
    // doc at a time:
    if (currentHardLiveDocs != prevHardLiveDocs) {
      // This means this segment received new deletes
      // since we started the merge, so we
      // must merge them:
      for (int j = 0; j < maxDoc; j++) {
        if (prevHardLiveDocs->get(j) == false) {
          // if the document was deleted before, it better still be deleted!
          assert(currentHardLiveDocs->get(j) == false);
        } else if (carryOverDelete(j)) {
          // the document was deleted while we were merging:
          mergedReadersAndUpdates->delete (
              segDocMap->get(segLeafDocMap->get(j)));
        }
      }
    }
  } else if (currentHardLiveDocs != nullptr) {
    assert(currentHardLiveDocs->length() == maxDoc);
    // This segment had no deletes before but now it
    // does:
    for (int j = 0; j < maxDoc; j++) {
      if (carryOverDelete(j)) {
        mergedReadersAndUpdates->delete (segDocMap->get(segLeafDocMap->get(j)));
      }
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") synchronized private bool
// commitMerge(MergePolicy.OneMerge merge, MergeState mergeState) throws
// java.io.IOException C++ WARNING: The following method was originally marked
// 'synchronized':
bool IndexWriter::commitMerge(
    shared_ptr<MergePolicy::OneMerge> merge,
    shared_ptr<MergeState> mergeState) 
{

  testPoint(L"startCommitMerge");

  if (tragedy->get() != nullptr) {
    throw make_shared<IllegalStateException>(
        L"this writer hit an unrecoverable error; cannot complete merge",
        tragedy->get());
  }

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"commitMerge: " + segString(merge->segments) +
                                   L" index=" + segString());
  }

  assert(merge->registerDone);

  // If merge was explicitly aborted, or, if rollback() or
  // rollbackTransaction() had been called since our merge
  // started (which results in an unqualified
  // deleter.refresh() call that will remove any index
  // file that current segments does not reference), we
  // abort this merge
  if (merge->isAborted()) {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"commitMerge: skip: it was aborted");
    }
    // In case we opened and pooled a reader for this
    // segment, drop it now.  This ensures that we close
    // the reader before trying to delete any of its
    // files.  This is not a very big deal, since this
    // reader will never be used by any NRT reader, and
    // another thread is currently running close(false)
    // so it will be dropped shortly anyway, but not
    // doing this  makes  MockDirWrapper angry in
    // TestNRTThreads (LUCENE-5434):
    readerPool->drop(merge->info);
    // Safe: these files must exist:
    deleteNewFiles(merge->info->files());
    return false;
  }

  shared_ptr<ReadersAndUpdates> *const mergedUpdates =
      merge->info->info->maxDoc() == 0
          ? nullptr
          : commitMergedDeletesAndUpdates(merge, mergeState);

  // If the doc store we are using has been closed and
  // is in now compound format (but wasn't when we
  // started), then we will switch to the compound
  // format as well:

  assert((!find(segmentInfos->begin(), segmentInfos->end(), merge->info) !=
          segmentInfos->end()));

  constexpr bool allDeleted =
      merge->segments.empty() || merge->info->info->maxDoc() == 0 ||
      (mergedUpdates != nullptr && isFullyDeleted(mergedUpdates));

  if (infoStream->isEnabled(L"IW")) {
    if (allDeleted) {
      infoStream->message(L"IW", L"merged segment " + merge->info +
                                     L" is 100% deleted; skipping insert");
    }
  }

  constexpr bool dropSegment = allDeleted;

  // If we merged no segments then we better be dropping
  // the new segment:
  assert(merge->segments.size() > 0 || dropSegment);

  assert(merge->info->info->maxDoc() != 0 || dropSegment);

  if (mergedUpdates != nullptr) {
    bool success = false;
    try {
      if (dropSegment) {
        mergedUpdates->dropChanges();
      }
      // Pass false for assertInfoLive because the merged
      // segment is not yet live (only below do we commit it
      // to the segmentInfos):
      release(mergedUpdates, false);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        mergedUpdates->dropChanges();
        readerPool->drop(merge->info);
      }
    }
  }

  // Must do this after readerPool.release, in case an
  // exception is hit e.g. writing the live docs for the
  // merge segment, in which case we need to abort the
  // merge:
  segmentInfos->applyMergeChanges(merge, dropSegment);

  // Now deduct the deleted docs that we just reclaimed from this
  // merge:
  int delDocCount;
  if (dropSegment) {
    // if we drop the segment we have to reduce the pendingNumDocs by
    // merge.totalMaxDocs since we never drop the docs when we apply deletes if
    // the segment is currently merged.
    delDocCount = merge->totalMaxDoc;
  } else {
    delDocCount = merge->totalMaxDoc - merge->info->info->maxDoc();
  }
  assert(delDocCount >= 0);
  adjustPendingNumDocs(-delDocCount);

  if (dropSegment) {
    assert((!find(segmentInfos->begin(), segmentInfos->end(), merge->info) !=
            segmentInfos->end()));
    readerPool->drop(merge->info);
    // Safe: these files must exist
    deleteNewFiles(merge->info->files());
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer =
  // this::checkpoint)
  {
    java::io::Closeable finalizer = shared_from_this()::checkpoint;
    // Must close before checkpoint, otherwise IFD won't be
    // able to delete the held-open files from the merge
    // readers:
    closeMergeReaders(merge, false);
  }

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"after commitMerge: " + segString());
  }

  if (merge->maxNumSegments != UNBOUNDED_MAX_MERGE_SEGMENTS && !dropSegment) {
    // cascade the forceMerge:
    if (segmentsToMerge.find(merge->info) == segmentsToMerge.end()) {
      segmentsToMerge.emplace(merge->info, Boolean::FALSE);
    }
  }

  return true;
}

void IndexWriter::handleMergeException(
    runtime_error t, shared_ptr<MergePolicy::OneMerge> merge) 
{

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"handleMergeException: merge=" +
                                   segString(merge->segments) + L" exc=" + t);
  }

  // Set the exception on the merge, so if
  // forceMerge is waiting on us it sees the root
  // cause exception:
  merge->setException(t);
  addMergeException(merge);

  if (std::dynamic_pointer_cast<MergePolicy::MergeAbortedException>(t) !=
      nullptr) {
    // We can ignore this exception (it happens when
    // deleteAll or rollback is called), unless the
    // merge involves segments from external directories,
    // in which case we must throw it so, for example, the
    // rollbackTransaction code in addIndexes* is
    // executed.
    if (merge->isExternal) { // TODO can we simplify this and just throw all the
                             // time? this would simplify this a lot
      throw std::static_pointer_cast<MergePolicy::MergeAbortedException>(t);
    }
  } else {
    assert(t != nullptr);
    throw IOUtils::rethrowAlways(t);
  }
}

void IndexWriter::merge(shared_ptr<MergePolicy::OneMerge> merge) throw(
    IOException)
{

  bool success = false;

  constexpr int64_t t0 = System::currentTimeMillis();

  shared_ptr<MergePolicy> *const mergePolicy = config->getMergePolicy();
  try {
    try {
      try {
        mergeInit(merge);

        if (infoStream->isEnabled(L"IW")) {
          infoStream->message(L"IW", L"now merge\n  merge=" +
                                         segString(merge->segments) +
                                         L"\n  index=" + segString());
        }

        mergeMiddle(merge, mergePolicy);
        mergeSuccess(merge);
        success = true;
      } catch (const runtime_error &t) {
        handleMergeException(t, merge);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {

        mergeFinish(merge);

        if (success == false) {
          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(L"IW", L"hit exception during merge");
          }
        } else if (!merge->isAborted() &&
                   (merge->maxNumSegments != UNBOUNDED_MAX_MERGE_SEGMENTS ||
                    (!closed && !closing))) {
          // This merge (and, generally, any change to the
          // segments) may now enable new merges, so we call
          // merge policy & update pending merges.
          updatePendingMerges(mergePolicy, MergeTrigger::MERGE_FINISHED,
                              merge->maxNumSegments);
        }
      }
    }
  } catch (const runtime_error &t) {
    // Important that tragicEvent is called after mergeFinish, else we hang
    // waiting for our merge thread to be removed from runningMerges:
    tragicEvent(t, L"merge");
    throw t;
  }

  if (merge->info != nullptr && merge->isAborted() == false) {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(
          L"IW", L"merge time " + (System::currentTimeMillis() - t0) +
                     L" msec for " + to_wstring(merge->info->info->maxDoc()) +
                     L" docs");
    }
  }
}

void IndexWriter::mergeSuccess(shared_ptr<MergePolicy::OneMerge> merge) {}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::registerMerge(shared_ptr<MergePolicy::OneMerge> merge) throw(
    IOException)
{

  if (merge->registerDone) {
    return true;
  }
  assert(merge->segments.size() > 0);

  if (stopMerges) {
    merge->setAborted();
    throw make_shared<MergePolicy::MergeAbortedException>(
        L"merge is aborted: " + segString(merge->segments));
  }

  bool isExternal = false;
  for (auto info : merge->segments) {
    if (find(mergingSegments.begin(), mergingSegments.end(), info) !=
        mergingSegments.end()) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"reject merge " +
                                       segString(merge->segments) +
                                       L": segment " + segString(info) +
                                       L" is already marked for merge");
      }
      return false;
    }
    if (!find(segmentInfos->begin(), segmentInfos->end(), info) !=
        segmentInfos->end()) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"reject merge " +
                                       segString(merge->segments) +
                                       L": segment " + segString(info) +
                                       L" does not exist in live infos");
      }
      return false;
    }
    if (info->info->dir != directoryOrig) {
      isExternal = true;
    }
    if (segmentsToMerge.find(info) != segmentsToMerge.end()) {
      merge->maxNumSegments = mergeMaxNumSegments;
    }
  }

  ensureValidMerge(merge);

  pendingMerges.push_back(merge);

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"add merge to pendingMerges: " +
                                   segString(merge->segments) + L" [total " +
                                   pendingMerges.size() + L" pending]");
  }

  merge->mergeGen = mergeGen;
  merge->isExternal = isExternal;

  // OK it does not conflict; now record that this merge
  // is running (while synchronized) to avoid race
  // condition where two conflicting merges from different
  // threads, start
  if (infoStream->isEnabled(L"IW")) {
    shared_ptr<StringBuilder> builder =
        make_shared<StringBuilder>(L"registerMerge merging= [");
    for (auto info : mergingSegments) {
      builder->append(info->info->name)->append(L", ");
    }
    builder->append(L"]");
    // don't call mergingSegments.toString() could lead to
    // ConcurrentModException since merge updates the segments FieldInfos
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", builder->toString());
    }
  }
  for (auto info : merge->segments) {
    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"registerMerge info=" + segString(info));
    }
    mergingSegments.insert(info);
  }

  assert(merge->estimatedMergeBytes == 0);
  assert(merge->totalMergeBytes == 0);
  for (auto info : merge->segments) {
    if (info->info->maxDoc() > 0) {
      constexpr int delCount = numDeletedDocs(info);
      assert(delCount <= info->info->maxDoc());
      constexpr double delRatio =
          (static_cast<double>(delCount)) / info->info->maxDoc();
      merge->estimatedMergeBytes += info->sizeInBytes() * (1.0 - delRatio);
      merge->totalMergeBytes += info->sizeInBytes();
    }
  }

  // Merge is now registered
  merge->registerDone = true;

  return true;
}

void IndexWriter::mergeInit(shared_ptr<MergePolicy::OneMerge> merge) throw(
    IOException)
{
  assert(Thread::holdsLock(shared_from_this()) == false);
  // Make sure any deletes that must be resolved before we commit the merge are
  // complete:
  bufferedUpdatesStream->waitApplyForMerge(merge->segments, shared_from_this());

  bool success = false;
  try {
    _mergeInit(merge);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"hit exception in mergeInit");
      }
      mergeFinish(merge);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::_mergeInit(shared_ptr<MergePolicy::OneMerge> merge) throw(
    IOException)
{

  testPoint(L"startMergeInit");

  assert(merge->registerDone);
  assert(merge->maxNumSegments == UNBOUNDED_MAX_MERGE_SEGMENTS ||
         merge->maxNumSegments > 0);

  if (tragedy->get() != nullptr) {
    throw make_shared<IllegalStateException>(
        L"this writer hit an unrecoverable error; cannot merge",
        tragedy->get());
  }

  if (merge->info != nullptr) {
    // mergeInit already done
    return;
  }

  merge->mergeInit();

  if (merge->isAborted()) {
    return;
  }

  // TODO: in the non-pool'd case this is somewhat
  // wasteful, because we open these readers, close them,
  // and then open them again for merging.  Maybe  we
  // could pre-pool them somehow in that case...

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"now apply deletes for " +
                                   merge->segments.size() +
                                   L" merging segments");
  }

  // Must move the pending doc values updates to disk now, else the newly merged
  // segment will not see them:
  // TODO: we could fix merging to pull the merged DV iterator so we don't have
  // to move these updates to disk first, i.e. just carry them in memory:
  if (readerPool->writeDocValuesUpdatesForMerge(merge->segments)) {
    checkpoint();
  }

  // Bind a new segment name here so even with
  // ConcurrentMergePolicy we keep deterministic segment
  // names.
  const wstring mergeSegmentName = newSegmentName();
  // We set the min version to null for now, it will be set later by
  // SegmentMerger
  shared_ptr<SegmentInfo> si = make_shared<SegmentInfo>(
      directoryOrig, Version::LATEST, nullptr, mergeSegmentName, -1, false,
      codec, Collections::emptyMap(), StringHelper::randomId(),
      unordered_map<>(), config->getIndexSort());
  unordered_map<wstring, wstring> details = unordered_map<wstring, wstring>();
  details.emplace(L"mergeMaxNumSegments",
                  L"" + to_wstring(merge->maxNumSegments));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  details.emplace(L"mergeFactor", Integer::toString(merge->segments.size()));
  setDiagnostics(si, SOURCE_MERGE, details);
  merge->setMergeInfo(
      make_shared<SegmentCommitInfo>(si, 0, 0, -1LL, -1LL, -1LL));

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"merge seg=" + merge->info->info->name + L" " +
                                   segString(merge->segments));
  }
}

void IndexWriter::setDiagnostics(shared_ptr<SegmentInfo> info,
                                 const wstring &source)
{
  setDiagnostics(info, source, nullptr);
}

void IndexWriter::setDiagnostics(shared_ptr<SegmentInfo> info,
                                 const wstring &source,
                                 unordered_map<wstring, wstring> &details)
{
  unordered_map<wstring, wstring> diagnostics =
      unordered_map<wstring, wstring>();
  diagnostics.emplace(L"source", source);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  diagnostics.emplace(L"lucene.version", Version::LATEST->toString());
  diagnostics.emplace(L"os", Constants::OS_NAME);
  diagnostics.emplace(L"os.arch", Constants::OS_ARCH);
  diagnostics.emplace(L"os.version", Constants::OS_VERSION);
  diagnostics.emplace(L"java.version", Constants::JAVA_VERSION);
  diagnostics.emplace(L"java.vendor", Constants::JAVA_VENDOR);
  // On IBM J9 JVM this is better than java.version which is just 1.7.0 (no
  // update level):
  diagnostics.emplace(
      L"java.runtime.version",
      System::getProperty(L"java.runtime.version", L"undefined"));
  // Hotspot version, e.g. 2.8 for J9:
  diagnostics.emplace(L"java.vm.version",
                      System::getProperty(L"java.vm.version", L"undefined"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  diagnostics.emplace(L"timestamp", Long::toString((Date()).getTime()));
  if (details.size() > 0) {
    diagnostics.putAll(details);
  }
  info->setDiagnostics(diagnostics);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::mergeFinish(shared_ptr<MergePolicy::OneMerge> merge)
{

  // forceMerge, addIndexes or waitForMerges may be waiting
  // on merges to finish.
  notifyAll();

  // It's possible we are called twice, eg if there was an
  // exception inside mergeInit
  if (merge->registerDone) {
    const deque<std::shared_ptr<SegmentCommitInfo>> &sourceSegments =
        merge->segments;
    for (auto info : sourceSegments) {
      mergingSegments.remove(info);
    }
    merge->registerDone = false;
  }

  runningMerges->remove(merge);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") private synchronized void
// closeMergeReaders(MergePolicy.OneMerge merge, bool suppressExceptions)
// throws java.io.IOException C++ WARNING: The following method was originally
// marked 'synchronized':
void IndexWriter::closeMergeReaders(shared_ptr<MergePolicy::OneMerge> merge,
                                    bool suppressExceptions) 
{
  constexpr bool drop = suppressExceptions == false;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer =
  // merge::mergeFinished)
  {
    java::io::Closeable finalizer = merge::mergeFinished;
    try {
      IOUtils::applyToAll(merge->readers, [&](any sr) {
        shared_ptr<ReadersAndUpdates> *const rld =
            getPooledInstance(sr::getOriginalSegmentInfo(), false);
        assert(rld != nullptr);
        if (drop) {
          rld->dropChanges();
        } else {
          rld->dropMergingUpdates();
        }
        rld->release(sr);
        release(rld);
        if (drop) {
          readerPool->drop(rld->info);
        }
      });
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      Collections::fill(merge->readers, nullptr);
    }
  }
}

int IndexWriter::mergeMiddle(
    shared_ptr<MergePolicy::OneMerge> merge,
    shared_ptr<MergePolicy> mergePolicy) 
{
  merge->checkAborted();

  shared_ptr<Directory> mergeDirectory =
      config->getMergeScheduler()->wrapForMerge(merge, directory);
  deque<std::shared_ptr<SegmentCommitInfo>> &sourceSegments = merge->segments;

  shared_ptr<IOContext> context =
      make_shared<IOContext>(merge->getStoreMergeInfo());

  shared_ptr<TrackingDirectoryWrapper> *const dirWrapper =
      make_shared<TrackingDirectoryWrapper>(mergeDirectory);

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"merging " + segString(merge->segments));
  }

  merge->readers = deque<>(sourceSegments.size());
  merge->hardLiveDocs = deque<>(sourceSegments.size());

  // This is try/finally to make sure merger's readers are
  // closed:
  bool success = false;
  try {
    int segUpto = 0;
    while (segUpto < sourceSegments.size()) {

      shared_ptr<SegmentCommitInfo> *const info = sourceSegments[segUpto];

      // Hold onto the "live" reader; we will use this to
      // commit merged deletes
      shared_ptr<ReadersAndUpdates> *const rld = getPooledInstance(info, true);
      rld->setIsMerging();

      shared_ptr<ReadersAndUpdates::MergeReader> mr =
          rld->getReaderForMerge(context);
      shared_ptr<SegmentReader> reader = mr->reader;

      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW",
                            L"seg=" + segString(info) + L" reader=" + reader);
      }

      merge->hardLiveDocs.push_back(mr->hardLiveDocs);
      merge->readers.push_back(reader);
      segUpto++;
    }

    // Let the merge wrap readers
    deque<std::shared_ptr<CodecReader>> mergeReaders =
        deque<std::shared_ptr<CodecReader>>();
    int numSoftDeleted = 0;
    for (auto reader : merge->readers) {
      shared_ptr<CodecReader> wrappedReader = merge->wrapForMerge(reader);
      validateMergeReader(wrappedReader);
      mergeReaders.push_back(wrappedReader);
      if (softDeletesEnabled) {
        if (reader != wrappedReader) { // if we don't have a wrapped reader we
                                       // won't preserve any soft-deletes
          shared_ptr<Bits> liveDocs = wrappedReader->getLiveDocs();
          numSoftDeleted += PendingSoftDeletes::countSoftDeletes(
              DocValuesFieldExistsQuery::getDocValuesDocIdSetIterator(
                  config->getSoftDeletesField(), wrappedReader),
              liveDocs);
        }
      }
    }
    shared_ptr<SegmentMerger> *const merger =
        make_shared<SegmentMerger>(mergeReaders, merge->info->info, infoStream,
                                   dirWrapper, globalFieldNumberMap, context);
    merge->info->setSoftDelCount(numSoftDeleted);
    merge->checkAborted();

    merge->mergeStartNS = System::nanoTime();

    // This is where all the work happens:
    if (merger->shouldMerge()) {
      merger->merge();
    }

    shared_ptr<MergeState> mergeState = merger->mergeState;
    assert(mergeState->segmentInfo == merge->info->info);
    merge->info->info->setFiles(unordered_set<>(dirWrapper->getCreatedFiles()));

    if (infoStream->isEnabled(L"IW")) {
      if (merger->shouldMerge()) {
        wstring pauseInfo =
            merge->getMergeProgress()
                ->getPauseTimes()
                .entrySet()
                .stream()
                .filter([&](e) { return e::getValue() > 0; })
                .map_obj([&](e) {
                  wstring::format(java::util::Locale::ROOT, L"%.1f sec %s",
                                  e::getValue() / 1000000000.0,
                                  e::getKey().name()->toLowerCase(
                                      java::util::Locale::ROOT));
                })
                .collect(Collectors::joining(L", "));
        if (!pauseInfo.isEmpty()) {
          pauseInfo = L" (" + pauseInfo + L")";
        }

        int64_t t1 = System::nanoTime();
        double sec = (t1 - merge->mergeStartNS) / 1000000000.0;
        double segmentMB = (merge->info->sizeInBytes() / 1024.0 / 1024.0);
        infoStream->message(
            L"IW",
            L"merge codec=" + codec + L" maxDoc=" +
                to_wstring(merge->info->info->maxDoc()) +
                L"; merged segment has " +
                (mergeState->mergeFieldInfos->hasVectors() ? L"vectors"
                                                           : L"no vectors") +
                L"; " +
                (mergeState->mergeFieldInfos->hasNorms() ? L"norms"
                                                         : L"no norms") +
                L"; " +
                (mergeState->mergeFieldInfos->hasDocValues()
                     ? L"docValues"
                     : L"no docValues") +
                L"; " +
                (mergeState->mergeFieldInfos->hasProx() ? L"prox"
                                                        : L"no prox") +
                L"; " +
                (mergeState->mergeFieldInfos->hasProx() ? L"freqs"
                                                        : L"no freqs") +
                L"; " +
                (mergeState->mergeFieldInfos->hasPointValues() ? L"points"
                                                               : L"no points") +
                L"; " +
                wstring::format(
                    Locale::ROOT,
                    L"%.1f sec%s to merge segment [%.2f MB, %.2f MB/sec]", sec,
                    pauseInfo, segmentMB, segmentMB / sec));
      } else {
        infoStream->message(L"IW", L"skip merging fully deleted segments");
      }
    }

    if (merger->shouldMerge() == false) {
      // Merge would produce a 0-doc segment, so we do nothing except commit the
      // merge to remove all the 0-doc segments that we "merged":
      assert(merge->info->info->maxDoc() == 0);
      commitMerge(merge, mergeState);
      return 0;
    }

    assert(merge->info->info->maxDoc() > 0);

    // Very important to do this before opening the reader
    // because codec must know if prox was written for
    // this segment:
    bool useCompoundFile;
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    { // Guard segmentInfos
      useCompoundFile = mergePolicy->useCompoundFile(segmentInfos, merge->info,
                                                     shared_from_this());
    }

    if (useCompoundFile) {
      success = false;

      shared_ptr<deque<wstring>> filesToRemove = merge->info->files();
      shared_ptr<TrackingDirectoryWrapper> trackingCFSDir =
          make_shared<TrackingDirectoryWrapper>(mergeDirectory);
      try {
        createCompoundFile(infoStream, trackingCFSDir, merge->info->info,
                           context, shared_from_this()::deleteNewFiles);
        success = true;
      } catch (const runtime_error &t) {
        // C++ TODO: Multithread locking on 'this' is not converted to native
        // C++:
        synchronized(shared_from_this())
        {
          if (merge->isAborted()) {
            // This can happen if rollback is called while we were building
            // our CFS -- fall through to logic below to remove the non-CFS
            // merged files:
            if (infoStream->isEnabled(L"IW")) {
              infoStream->message(L"IW", L"hit merge abort exception creating "
                                         L"compound file during merge");
            }
            return 0;
          } else {
            handleMergeException(t, merge);
          }
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (success == false) {
          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(
                L"IW", L"hit exception creating compound file during merge");
          }
          // Safe: these files must exist
          deleteNewFiles(merge->info->files());
        }
      }

      // So that, if we hit exc in deleteNewFiles (next)
      // or in commitMerge (later), we close the
      // per-segment readers in the finally clause below:
      success = false;

      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {

        // delete new non cfs files directly: they were never
        // registered with IFD
        deleteNewFiles(filesToRemove);

        if (merge->isAborted()) {
          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(L"IW", L"abort merge after building CFS");
          }
          // Safe: these files must exist
          deleteNewFiles(merge->info->files());
          return 0;
        }
      }

      merge->info->info->setUseCompoundFile(true);
    } else {
      // So that, if we hit exc in commitMerge (later),
      // we close the per-segment readers in the finally
      // clause below:
      success = false;
    }

    // Have codec write SegmentInfo.  Must do this after
    // creating CFS so that 1) .si isn't slurped into CFS,
    // and 2) .si reflects useCompoundFile=true change
    // above:
    bool success2 = false;
    try {
      codec->segmentInfoFormat()->write(directory, merge->info->info, context);
      success2 = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success2) {
        // Safe: these files must exist
        deleteNewFiles(merge->info->files());
      }
    }

    // TODO: ideally we would freeze merge.info here!!
    // because any changes after writing the .si will be
    // lost...

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(
          L"IW",
          wstring::format(Locale::ROOT,
                          L"merged segment size=%.3f MB vs estimate=%.3f MB",
                          merge->info->sizeInBytes() / 1024.0 / 1024.0,
                          merge->estimatedMergeBytes / 1024 / 1024.0));
    }

    constexpr IndexReaderWarmer mergedSegmentWarmer =
        config->getMergedSegmentWarmer();
    if (readerPool->isReaderPoolingEnabled() &&
        mergedSegmentWarmer != nullptr) {
      shared_ptr<ReadersAndUpdates> *const rld =
          getPooledInstance(merge->info, true);
      shared_ptr<SegmentReader> *const sr = rld->getReader(IOContext::READ);
      try {
        mergedSegmentWarmer(sr);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // C++ TODO: Multithread locking on 'this' is not converted to native
        // C++:
        synchronized(shared_from_this())
        {
          rld->release(sr);
          release(rld);
        }
      }
    }

    if (!commitMerge(merge, mergeState)) {
      // commitMerge will return false if this merge was
      // aborted
      return 0;
    }

    success = true;

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // Readers are already closed in commitMerge if we didn't hit
    // an exc:
    if (success == false) {
      closeMergeReaders(merge, true);
    }
  }

  return merge->info->info->maxDoc();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::addMergeException(shared_ptr<MergePolicy::OneMerge> merge)
{
  assert(merge->getException() != nullptr);
  if (!find(mergeExceptions.begin(), mergeExceptions.end(), merge) !=
          mergeExceptions.end() &&
      mergeGen == merge->mergeGen) {
    mergeExceptions.push_back(merge);
  }
}

int IndexWriter::getBufferedDeleteTermsSize()
{
  return docWriter->getBufferedDeleteTermsSize();
}

int IndexWriter::getNumBufferedDeleteTerms()
{
  return docWriter->getNumBufferedDeleteTerms();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SegmentCommitInfo> IndexWriter::newestSegment()
{
  return segmentInfos->size() > 0 ? segmentInfos->info(segmentInfos->size() - 1)
                                  : nullptr;
}

// C++ WARNING: The following method was originally marked 'synchronized':
wstring IndexWriter::segString() { return segString(segmentInfos); }

// C++ WARNING: The following method was originally marked 'synchronized':
wstring
IndexWriter::segString(deque<std::shared_ptr<SegmentCommitInfo>> &infos)
{
  return StreamSupport::stream(infos.spliterator(), false)
      .map_obj(shared_from_this()::segString)
      .collect(Collectors::joining(L" "));
}

// C++ WARNING: The following method was originally marked 'synchronized':
wstring IndexWriter::segString(shared_ptr<SegmentCommitInfo> info)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return info->toString(numDeletedDocs(info) -
                        info->getDelCount(softDeletesEnabled));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::doWait()
{
  // NOTE: the callers of this method should in theory
  // be able to do simply wait(), but, as a defense
  // against thread timing hazards where notifyAll()
  // fails to be called, we wait for at most 1 second
  // and then return so caller can check if wait
  // conditions are satisfied:
  try {
    wait(1000);
  } catch (const InterruptedException &ie) {
    throw make_shared<ThreadInterruptedException>(ie);
  }
}

bool IndexWriter::filesExist(shared_ptr<SegmentInfos> toSync) 
{

  shared_ptr<deque<wstring>> files = toSync->files(false);
  for (auto fileName : files) {
    // If this trips it means we are missing a call to
    // .checkpoint somewhere, because by the time we
    // are called, deleter should know about every
    // file referenced by the current head
    // segmentInfos:
    assert((deleter->exists(fileName),
            L"IndexFileDeleter doesn't know about file " + fileName));
  }
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SegmentInfos> IndexWriter::toLiveInfos(shared_ptr<SegmentInfos> sis)
{
  shared_ptr<SegmentInfos> *const newSIS =
      make_shared<SegmentInfos>(sis->getIndexCreatedVersionMajor());
  const unordered_map<std::shared_ptr<SegmentCommitInfo>,
                      std::shared_ptr<SegmentCommitInfo>>
      liveSIS = unordered_map<std::shared_ptr<SegmentCommitInfo>,
                              std::shared_ptr<SegmentCommitInfo>>();
  for (auto info : segmentInfos) {
    liveSIS.emplace(info, info);
  }
  for (auto info : sis) {
    shared_ptr<SegmentCommitInfo> liveInfo = liveSIS[info];
    if (liveInfo != nullptr) {
      info = liveInfo;
    }
    newSIS->push_back(info);
  }

  return newSIS;
}

void IndexWriter::startCommit(shared_ptr<SegmentInfos> toSync) throw(
    IOException)
{

  testPoint(L"startStartCommit");
  assert(pendingCommit->empty());

  if (tragedy->get() != nullptr) {
    throw make_shared<IllegalStateException>(
        L"this writer hit an unrecoverable error; cannot commit",
        tragedy->get());
  }

  try {

    if (infoStream->isEnabled(L"IW")) {
      infoStream->message(L"IW", L"startCommit(): start");
    }

    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {

      if (lastCommitChangeCount > changeCount->get()) {
        throw make_shared<IllegalStateException>(
            L"lastCommitChangeCount=" + to_wstring(lastCommitChangeCount) +
            L",changeCount=" + changeCount);
      }

      if (pendingCommitChangeCount == lastCommitChangeCount) {
        if (infoStream->isEnabled(L"IW")) {
          infoStream->message(L"IW",
                              L"  skip startCommit(): no changes pending");
        }
        try {
          deleter->decRef(filesToCommit);
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          filesToCommit.reset();
        }
        return;
      }

      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"startCommit index=" +
                                       segString(toLiveInfos(toSync)) +
                                       L" changeCount=" + changeCount);
      }

      assert(filesExist(toSync));
    }

    testPoint(L"midStartCommit");

    bool pendingCommitSet = false;

    try {

      testPoint(L"midStartCommit2");

      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {

        assert(pendingCommit->empty());

        assert(segmentInfos->getGeneration() == toSync->getGeneration());

        // Exception here means nothing is prepared
        // (this method unwinds everything it did on
        // an exception)
        toSync->prepareCommit(directory);
        if (infoStream->isEnabled(L"IW")) {
          infoStream->message(L"IW",
                              L"startCommit: wrote pending segments file \"" +
                                  IndexFileNames::fileNameFromGeneration(
                                      IndexFileNames::PENDING_SEGMENTS, L"",
                                      toSync->getGeneration()) +
                                  L"\"");
        }

        pendingCommitSet = true;
        pendingCommit = toSync;
      }

      // This call can take a long time -- 10s of seconds
      // or more.  We do it without syncing on this:
      bool success = false;
      shared_ptr<deque<wstring>> *const filesToSync;
      try {
        filesToSync = toSync->files(false);
        directory->sync(filesToSync);
        success = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (!success) {
          pendingCommitSet = false;
          pendingCommit.reset();
          toSync->rollbackCommit(directory);
        }
      }

      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"done all syncs: " + filesToSync);
      }

      testPoint(L"midStartCommitSuccess");
    } catch (const runtime_error &t) {
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        if (!pendingCommitSet) {
          if (infoStream->isEnabled(L"IW")) {
            infoStream->message(L"IW",
                                L"hit exception committing segments file");
          }
          try {
            // Hit exception
            deleter->decRef(filesToCommit);
          } catch (const runtime_error &t1) {
            t.addSuppressed(t1);
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            filesToCommit.reset();
          }
        }
      }
      throw t;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        // Have our master segmentInfos record the
        // generations we just prepared.  We do this
        // on error or success so we don't
        // double-write a segments_N file.
        segmentInfos->updateGeneration(toSync);
      }
    }
  } catch (const VirtualMachineError &tragedy) {
    tragicEvent(tragedy, L"startCommit");
    throw tragedy;
  }
  testPoint(L"finishStartCommit");
}

void IndexWriter::onTragicEvent(runtime_error tragedy, const wstring &location)
{
  // This is not supposed to be tragic: IW is supposed to catch this and
  // ignore, because it means we asked the merge to abort:
  assert(std::dynamic_pointer_cast<MergePolicy::MergeAbortedException>(
             tragedy) != nullptr == false);
  // How can it be a tragedy when nothing happened?
  assert(tragedy != nullptr);
  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"hit tragic " +
                                   tragedy.getClass().getSimpleName() +
                                   L" inside " + location);
  }
  this->tragedy->compareAndSet(nullptr, tragedy); // only set it once
}

void IndexWriter::tragicEvent(runtime_error tragedy,
                              const wstring &location) 
{
  try {
    onTragicEvent(tragedy, location);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    maybeCloseOnTragicEvent();
  }
}

void IndexWriter::maybeCloseOnTragicEvent() 
{
  // We cannot hold IW's lock here else it can lead to deadlock:
  assert(Thread::holdsLock(shared_from_this()) == false);
  assert(Thread::holdsLock(fullFlushLock) == false);
  // if we are already closed (e.g. called by rollback), this will be a no-op.
  if (this->tragedy->get() != nullptr && shouldClose(false)) {
    rollbackInternal();
  }
}

runtime_error IndexWriter::getTragicException() { return tragedy->get(); }

bool IndexWriter::isOpen() { return closing == false && closed == false; }

void IndexWriter::testPoint(const wstring &message)
{
  if (enableTestPoints) {
    assert(infoStream->isEnabled(L"TP")); // don't enable unless you need them.
    infoStream->message(L"TP", message);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::nrtIsCurrent(shared_ptr<SegmentInfos> infos)
{
  ensureOpen();
  bool isCurrent = infos->getVersion() == segmentInfos->getVersion() &&
                   docWriter->anyChanges() == false &&
                   bufferedUpdatesStream->any() == false &&
                   readerPool->anyDocValuesChanges() == false;
  if (infoStream->isEnabled(L"IW")) {
    if (isCurrent == false) {
      infoStream->message(
          L"IW", L"nrtIsCurrent: infoVersion matches: " +
                     StringHelper::toString(infos->getVersion() ==
                                            segmentInfos->getVersion()) +
                     L"; DW changes: " +
                     StringHelper::toString(docWriter->anyChanges()) +
                     L"; BD changes: " +
                     StringHelper::toString(bufferedUpdatesStream->any()));
    }
  }
  return isCurrent;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool IndexWriter::isClosed() { return closed; }

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::deleteUnusedFiles() 
{
  // TODO: should we remove this method now that it's the Directory's job to
  // retry deletions?  Except, for the super expert IDP use case it's still
  // needed?
  ensureOpen(false);
  deleter->revisitPolicy();
}

void IndexWriter::createCompoundFile(
    shared_ptr<InfoStream> infoStream,
    shared_ptr<TrackingDirectoryWrapper> directory,
    shared_ptr<SegmentInfo> info, shared_ptr<IOContext> context,
    IOUtils::IOConsumer<deque<wstring>> deleteFiles) 
{

  // maybe this check is not needed, but why take the risk?
  if (!directory->getCreatedFiles()->isEmpty()) {
    throw make_shared<IllegalStateException>(
        L"pass a clean trackingdir for CFS creation");
  }

  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"create compound file");
  }
  // Now merge all added files
  bool success = false;
  try {
    info->getCodec()->compoundFormat()->write(directory, info, context);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // Safe: these files must exist
      deleteFiles(directory->getCreatedFiles());
    }
  }

  // Replace all previous files with the CFS/CFE files:
  info->setFiles(unordered_set<>(directory->getCreatedFiles()));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::deleteNewFiles(shared_ptr<deque<wstring>> files) throw(
    IOException)
{
  deleter->deleteNewFiles(files);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::flushFailed(shared_ptr<SegmentInfo> info) 
{
  // TODO: this really should be a tragic
  shared_ptr<deque<wstring>> files;
  try {
    files = info->files();
  } catch (const IllegalStateException &ise) {
    // OK
    files.reset();
  }
  if (files != nullptr) {
    deleter->deleteNewFiles(files);
  }
}

void IndexWriter::publishFlushedSegments(bool forced) 
{
  docWriter->purgeFlushTickets(forced, [&](any ticket) {
    shared_ptr<DocumentsWriterPerThread::FlushedSegment> newSegment =
        ticket::getFlushedSegment();
    shared_ptr<FrozenBufferedUpdates> bufferedUpdates =
        ticket::getFrozenUpdates();
    ticket::markPublished();
    if (newSegment == nullptr) {
      if (bufferedUpdates != nullptr && bufferedUpdates->any()) {
        publishFrozenUpdates(bufferedUpdates);
        if (infoStream->isEnabled(L"IW")) {
          infoStream->message(L"IW", L"flush: push buffered updates: " +
                                         bufferedUpdates);
        }
      }
    } else {
      assert(newSegment->segmentInfo != nullptr);
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW",
                            L"publishFlushedSegment seg-private updates=" +
                                newSegment->segmentUpdates);
      }
      if (newSegment->segmentUpdates != nullptr &&
          infoStream->isEnabled(L"DW")) {
        infoStream->message(L"IW",
                            L"flush: push buffered seg private updates: " +
                                newSegment->segmentUpdates);
      }
      publishFlushedSegment(newSegment->segmentInfo, newSegment->fieldInfos,
                            newSegment->segmentUpdates, bufferedUpdates,
                            newSegment->sortMap);
    }
  });
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::incRefDeleter(shared_ptr<SegmentInfos> segmentInfos) throw(
    IOException)
{
  ensureOpen();
  deleter->incRef(segmentInfos, false);
  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"incRefDeleter for NRT reader version=" +
                                   to_wstring(segmentInfos->getVersion()) +
                                   L" segments=" + segString(segmentInfos));
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void IndexWriter::decRefDeleter(shared_ptr<SegmentInfos> segmentInfos) throw(
    IOException)
{
  ensureOpen();
  deleter->decRef(segmentInfos);
  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"decRefDeleter for NRT reader version=" +
                                   to_wstring(segmentInfos->getVersion()) +
                                   L" segments=" + segString(segmentInfos));
  }
}

void IndexWriter::processEvents(bool triggerMerge) 
{
  if (tragedy->get() == nullptr) {
    Event event_;
    while ((event_ = eventQueue.pop_front()) != nullptr) {
      event_(shared_from_this());
    }
  }
  if (triggerMerge) {
    maybeMerge(getConfig()->getMergePolicy(), MergeTrigger::SEGMENT_FLUSH,
               UNBOUNDED_MAX_MERGE_SEGMENTS);
  }
}

void IndexWriter::reserveDocs(int64_t addedNumDocs)
{
  assert(addedNumDocs >= 0);
  if (adjustPendingNumDocs(addedNumDocs) > actualMaxDocs) {
    // Reserve failed: put the docs back and throw exc:
    adjustPendingNumDocs(-addedNumDocs);
    tooManyDocs(addedNumDocs);
  }
}

void IndexWriter::testReserveDocs(int64_t addedNumDocs)
{
  assert(addedNumDocs >= 0);
  if (pendingNumDocs->get() + addedNumDocs > actualMaxDocs) {
    tooManyDocs(addedNumDocs);
  }
}

void IndexWriter::tooManyDocs(int64_t addedNumDocs)
{
  assert(addedNumDocs >= 0);
  throw invalid_argument(L"number of documents in the index cannot exceed " +
                         to_wstring(actualMaxDocs) +
                         L" (current document count is " +
                         pendingNumDocs->get() + L"; added numDocs is " +
                         to_wstring(addedNumDocs) + L")");
}

int64_t IndexWriter::getMaxCompletedSequenceNumber()
{
  ensureOpen();
  return docWriter->getMaxCompletedSequenceNumber();
}

int64_t IndexWriter::adjustPendingNumDocs(int64_t numDocs)
{
  int64_t count = pendingNumDocs->addAndGet(numDocs);
  assert((count >= 0, L"pendingNumDocs is negative: " + to_wstring(count)));
  return count;
}

bool IndexWriter::isFullyDeleted(
    shared_ptr<ReadersAndUpdates> readersAndUpdates) 
{
  if (readersAndUpdates->isFullyDeleted()) {
    assert(Thread::holdsLock(shared_from_this()));
    return readersAndUpdates->keepFullyDeletedSegment(
               config->getMergePolicy()) == false;
  }
  return false;
}

int IndexWriter::numDeletesToMerge(shared_ptr<SegmentCommitInfo> info) throw(
    IOException)
{
  ensureOpen(false);
  validate(info);
  shared_ptr<MergePolicy> mergePolicy = config->getMergePolicy();
  shared_ptr<ReadersAndUpdates> *const rld = getPooledInstance(info, false);
  int numDeletesToMerge;
  if (rld != nullptr) {
    numDeletesToMerge = rld->numDeletesToMerge(mergePolicy);
  } else {
    // if we don't have a  pooled instance lets just return the hard deletes,
    // this is safe!
    numDeletesToMerge = info->getDelCount();
  }
  assert((numDeletesToMerge <= info->info->maxDoc(),
          L"numDeletesToMerge: " + to_wstring(numDeletesToMerge) +
              L" > maxDoc: " + to_wstring(info->info->maxDoc())));
  return numDeletesToMerge;
}

void IndexWriter::release(
    shared_ptr<ReadersAndUpdates> readersAndUpdates) 
{
  release(readersAndUpdates, true);
}

void IndexWriter::release(shared_ptr<ReadersAndUpdates> readersAndUpdates,
                          bool assertLiveInfo) 
{
  assert(Thread::holdsLock(shared_from_this()));
  if (readerPool->release(readersAndUpdates, assertLiveInfo)) {
    // if we write anything here we have to hold the lock otherwise IDF will
    // delete files underneath us
    assert(Thread::holdsLock(shared_from_this()));
    checkpointNoSIS();
  }
}

shared_ptr<ReadersAndUpdates>
IndexWriter::getPooledInstance(shared_ptr<SegmentCommitInfo> info, bool create)
{
  ensureOpen(false);
  return readerPool->get(info, create);
}

void IndexWriter::finished(shared_ptr<FrozenBufferedUpdates> packet)
{
  bufferedUpdatesStream->finished(packet);
}

int IndexWriter::getPendingUpdatesCount()
{
  return bufferedUpdatesStream->getPendingUpdatesCount();
}

bool IndexWriter::isEnableTestPoints() { return false; }

void IndexWriter::validate(shared_ptr<SegmentCommitInfo> info)
{
  if (info->info->dir != directoryOrig) {
    throw invalid_argument(
        L"SegmentCommitInfo must be from the same directory");
  }
}
} // namespace org::apache::lucene::index