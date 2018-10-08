using namespace std;

#include "DocumentsWriterPerThread.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Codec = org::apache::lucene::codecs::Codec;
using DeleteSlice =
    org::apache::lucene::index::DocumentsWriterDeleteQueue::DeleteSlice;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using Allocator = org::apache::lucene::util::ByteBlockPool::Allocator;
using DirectTrackingAllocator =
    org::apache::lucene::util::ByteBlockPool::DirectTrackingAllocator;
using Counter = org::apache::lucene::util::Counter;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using InfoStream = org::apache::lucene::util::InfoStream;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_MASK;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

void DocumentsWriterPerThread::onAbortingException(runtime_error throwable)
{
  assert((abortingException == nullptr,
          L"aborting excpetion has already been set"));
  abortingException = throwable;
}

bool DocumentsWriterPerThread::hasHitAbortingException()
{
  return abortingException != nullptr;
}

bool DocumentsWriterPerThread::isAborted() { return aborted; }

const shared_ptr<IndexingChain> DocumentsWriterPerThread::defaultIndexingChain =
    make_shared<IndexingChainAnonymousInnerClass>();

DocumentsWriterPerThread::IndexingChainAnonymousInnerClass::
    IndexingChainAnonymousInnerClass()
{
}

shared_ptr<DocConsumer>
DocumentsWriterPerThread::IndexingChainAnonymousInnerClass::getChain(
    shared_ptr<DocumentsWriterPerThread>
        documentsWriterPerThread) 
{
  return make_shared<DefaultIndexingChain>(documentsWriterPerThread);
}

DocumentsWriterPerThread::DocState::DocState(
    shared_ptr<DocumentsWriterPerThread> docWriter,
    shared_ptr<InfoStream> infoStream)
    : docWriter(docWriter)
{
  this->infoStream = infoStream;
}

void DocumentsWriterPerThread::DocState::clear()
{
  // don't hold onto doc nor analyzer, in case it is
  // largish:
  doc.clear();
  analyzer.reset();
}

DocumentsWriterPerThread::FlushedSegment::FlushedSegment(
    shared_ptr<InfoStream> infoStream,
    shared_ptr<SegmentCommitInfo> segmentInfo,
    shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<BufferedUpdates> segmentUpdates,
    shared_ptr<FixedBitSet> liveDocs, int delCount,
    shared_ptr<Sorter::DocMap> sortMap) 
    : segmentInfo(segmentInfo), fieldInfos(fieldInfos),
      segmentUpdates(segmentUpdates != nullptr && segmentUpdates->any()
                         ? make_shared<FrozenBufferedUpdates>(
                               infoStream, segmentUpdates, segmentInfo)
                         : nullptr),
      liveDocs(liveDocs), sortMap(sortMap), delCount(delCount)
{
}

void DocumentsWriterPerThread::abort() 
{
  aborted = true;
  pendingNumDocs->addAndGet(-numDocsInRAM);
  try {
    if (infoStream->isEnabled(L"DWPT")) {
      infoStream->message(L"DWPT", L"now abort");
    }
    try {
      consumer->abort();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      pendingUpdates->clear();
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (infoStream->isEnabled(L"DWPT")) {
      infoStream->message(L"DWPT", L"done abort");
    }
  }
}

DocumentsWriterPerThread::DocumentsWriterPerThread(
    int indexVersionCreated, const wstring &segmentName,
    shared_ptr<Directory> directoryOrig, shared_ptr<Directory> directory,
    shared_ptr<LiveIndexWriterConfig> indexWriterConfig,
    shared_ptr<InfoStream> infoStream,
    shared_ptr<DocumentsWriterDeleteQueue> deleteQueue,
    shared_ptr<FieldInfos::Builder> fieldInfos,
    shared_ptr<AtomicLong> pendingNumDocs,
    bool enableTestPoints) 
    : codec(indexWriterConfig->getCodec()),
      directory(make_shared<TrackingDirectoryWrapper>(directory)),
      directoryOrig(directoryOrig),
      docState(make_shared<DocState>(shared_from_this(), infoStream)),
      consumer(
          indexWriterConfig->getIndexingChain()->getChain(shared_from_this())),
      bytesUsed(Counter::newCounter()),
      pendingUpdates(make_shared<BufferedUpdates>(segmentName)),
      segmentInfo(make_shared<SegmentInfo>(
          directoryOrig, Version::LATEST, Version::LATEST, segmentName, -1,
          false, codec, Collections::emptyMap(), StringHelper::randomId(),
          unordered_map<>(), indexWriterConfig->getIndexSort())),
      fieldInfos(fieldInfos), infoStream(infoStream), deleteQueue(deleteQueue),
      deleteSlice(deleteQueue->newSlice()),
      byteBlockAllocator(make_shared<DirectTrackingAllocator>(bytesUsed_)),
      intBlockAllocator(make_shared<IntBlockAllocator>(bytesUsed_)),
      pendingNumDocs(pendingNumDocs), indexWriterConfig(indexWriterConfig),
      enableTestPoints(enableTestPoints),
      indexVersionCreated(indexVersionCreated)
{
  this->docState->similarity = indexWriterConfig->getSimilarity();
  assert((numDocsInRAM == 0, L"num docs " + to_wstring(numDocsInRAM)));

  assert(numDocsInRAM == 0);
  if (INFO_VERBOSE && infoStream->isEnabled(L"DWPT")) {
    infoStream->message(L"DWPT", Thread::currentThread().getName() +
                                     L" init seg=" + segmentName +
                                     L" delQueue=" + deleteQueue);
  }
  // this should be the last call in the ctor
  // it really sucks that we need to pull this within the ctor and pass this ref
  // to the chain!
}

shared_ptr<FieldInfos::Builder> DocumentsWriterPerThread::getFieldInfosBuilder()
{
  return fieldInfos;
}

int DocumentsWriterPerThread::getIndexCreatedVersionMajor()
{
  return indexVersionCreated;
}

void DocumentsWriterPerThread::testPoint(const wstring &message)
{
  if (enableTestPoints) {
    assert(infoStream->isEnabled(L"TP")); // don't enable unless you need them.
    infoStream->message(L"TP", message);
  }
}

void DocumentsWriterPerThread::reserveOneDoc()
{
  if (pendingNumDocs->incrementAndGet() > IndexWriter::getActualMaxDocs()) {
    // Reserve failed: put the one doc back and throw exc:
    pendingNumDocs->decrementAndGet();
    throw invalid_argument(L"number of documents in the index cannot exceed " +
                           to_wstring(IndexWriter::getActualMaxDocs()));
  }
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long updateDocument(Iterable<? extends
// IndexableField> doc, org.apache.lucene.analysis.Analyzer analyzer,
// DocumentsWriterDeleteQueue.Node<?> deleteNode,
// DocumentsWriter.FlushNotifications flushNotifications) throws
// java.io.IOException
int64_t DocumentsWriterPerThread::updateDocument(
    deque<T1> doc, shared_ptr<Analyzer> analyzer,
    shared_ptr<DocumentsWriterDeleteQueue::Node<T2>> deleteNode,
    shared_ptr<DocumentsWriter::FlushNotifications>
        flushNotifications) 
{
  try {
    assert((hasHitAbortingException() == false,
            L"DWPT has hit aborting exception but is still indexing"));
    testPoint(L"DocumentsWriterPerThread addDocument start");
    assert(deleteQueue != nullptr);
    reserveOneDoc();
    docState->doc = doc;
    docState->analyzer = analyzer;
    docState->docID = numDocsInRAM;
    if (INFO_VERBOSE && infoStream->isEnabled(L"DWPT")) {
      infoStream->message(
          L"DWPT", Thread::currentThread().getName() + L" update delTerm=" +
                       deleteNode + L" docID=" + to_wstring(docState->docID) +
                       L" seg=" + segmentInfo->name);
    }
    // Even on exception, the document is still added (but marked
    // deleted), so we don't need to un-reserve at that point.
    // Aborting exceptions will actually "lose" more than one
    // document, so the counter will be "wrong" in that case, but
    // it's very hard to fix (we can't easily distinguish aborting
    // vs non-aborting exceptions):
    bool success = false;
    try {
      try {
        consumer->processDocument();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        docState->clear();
      }
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        // mark document as deleted
        deleteDocID(docState->docID);
        numDocsInRAM++;
      }
    }

    return finishDocument(deleteNode);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    maybeAbort(L"updateDocument", flushNotifications);
  }
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long updateDocuments(Iterable<? extends
// Iterable<? extends IndexableField>> docs, org.apache.lucene.analysis.Analyzer
// analyzer, DocumentsWriterDeleteQueue.Node<?> deleteNode,
// DocumentsWriter.FlushNotifications flushNotifications) throws
// java.io.IOException
int64_t DocumentsWriterPerThread::updateDocuments(
    deque<T1> docs, shared_ptr<Analyzer> analyzer,
    shared_ptr<DocumentsWriterDeleteQueue::Node<T2>> deleteNode,
    shared_ptr<DocumentsWriter::FlushNotifications>
        flushNotifications) 
{
  try {
    testPoint(L"DocumentsWriterPerThread addDocuments start");
    assert((hasHitAbortingException() == false,
            L"DWPT has hit aborting exception but is still indexing"));
    assert(deleteQueue != nullptr);
    docState->analyzer = analyzer;
    if (INFO_VERBOSE && infoStream->isEnabled(L"DWPT")) {
      infoStream->message(
          L"DWPT", Thread::currentThread().getName() + L" update delTerm=" +
                       deleteNode + L" docID=" + to_wstring(docState->docID) +
                       L" seg=" + segmentInfo->name);
    }
    int docCount = 0;
    bool allDocsIndexed = false;
    try {

      for (auto doc : docs) {
        // Even on exception, the document is still added (but marked
        // deleted), so we don't need to un-reserve at that point.
        // Aborting exceptions will actually "lose" more than one
        // document, so the counter will be "wrong" in that case, but
        // it's very hard to fix (we can't easily distinguish aborting
        // vs non-aborting exceptions):
        reserveOneDoc();
        docState->doc = doc;
        docState->docID = numDocsInRAM;
        docCount++;

        bool success = false;
        try {
          consumer->processDocument();
          success = true;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          if (!success) {
            // Incr here because finishDocument will not
            // be called (because an exc is being thrown):
            numDocsInRAM++;
          }
        }

        numDocsInRAM++;
      }
      allDocsIndexed = true;

      // Apply delTerm only after all indexing has
      // succeeded, but apply it only to docs prior to when
      // this batch started:
      int64_t seqNo;
      if (deleteNode != nullptr) {
        seqNo = deleteQueue->add(deleteNode, deleteSlice);
        assert((deleteSlice->isTail(deleteNode),
                L"expected the delete term as the tail item"));
        deleteSlice->apply(pendingUpdates, numDocsInRAM - docCount);
        return seqNo;
      } else {
        seqNo = deleteQueue->updateSlice(deleteSlice);
        if (seqNo < 0) {
          seqNo = -seqNo;
          deleteSlice->apply(pendingUpdates, numDocsInRAM - docCount);
        } else {
          deleteSlice->reset();
        }
      }

      return seqNo;

    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!allDocsIndexed && !aborted) {
        // the iterator threw an exception that is not aborting
        // go and mark all docs from this block as deleted
        int docID = numDocsInRAM - 1;
        constexpr int endDocID = docID - docCount;
        while (docID > endDocID) {
          deleteDocID(docID);
          docID--;
        }
      }
      docState->clear();
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    maybeAbort(L"updateDocuments", flushNotifications);
  }
}

template <typename T1>
int64_t DocumentsWriterPerThread::finishDocument(
    shared_ptr<DocumentsWriterDeleteQueue::Node<T1>> deleteNode)
{
  /*
   * here we actually finish the document in two steps 1. push the delete into
   * the queue and update our slice. 2. increment the DWPT private document
   * id.
   *
   * the updated slice we get from 1. holds all the deletes that have occurred
   * since we updated the slice the last time.
   */
  bool applySlice = numDocsInRAM != 0;
  int64_t seqNo;
  if (deleteNode != nullptr) {
    seqNo = deleteQueue->add(deleteNode, deleteSlice);
    assert((deleteSlice->isTail(deleteNode),
            L"expected the delete node as the tail"));
  } else {
    seqNo = deleteQueue->updateSlice(deleteSlice);

    if (seqNo < 0) {
      seqNo = -seqNo;
    } else {
      applySlice = false;
    }
  }

  if (applySlice) {
    deleteSlice->apply(pendingUpdates, numDocsInRAM);
  } else { // if we don't need to apply we must reset!
    deleteSlice->reset();
  }
  ++numDocsInRAM;

  return seqNo;
}

void DocumentsWriterPerThread::deleteDocID(int docIDUpto)
{
  pendingUpdates->addDocID(docIDUpto);
  // NOTE: we do not trigger flush here.  This is
  // potentially a RAM leak, if you have an app that tries
  // to add docs but every single doc always hits a
  // non-aborting exception.  Allowing a flush here gets
  // very messy because we are only invoked when handling
  // exceptions so to do this properly, while handling an
  // exception we'd have to go off and flush new deletes
  // which is risky (likely would hit some other
  // confounding exception).
}

int DocumentsWriterPerThread::getNumDocsInRAM()
{
  // public for FlushPolicy
  return numDocsInRAM;
}

shared_ptr<FrozenBufferedUpdates>
DocumentsWriterPerThread::prepareFlush() 
{
  assert(numDocsInRAM > 0);
  shared_ptr<FrozenBufferedUpdates> *const globalUpdates =
      deleteQueue->freezeGlobalBuffer(deleteSlice);
  /* deleteSlice can possibly be null if we have hit non-aborting exceptions
  during indexing and never succeeded adding a document. */
  if (deleteSlice != nullptr) {
    // apply all deletes before we flush and release the delete slice
    deleteSlice->apply(pendingUpdates, numDocsInRAM);
    assert(deleteSlice->isEmpty());
    deleteSlice->reset();
  }
  return globalUpdates;
}

shared_ptr<FlushedSegment>
DocumentsWriterPerThread::flush(shared_ptr<DocumentsWriter::FlushNotifications>
                                    flushNotifications) 
{
  assert(numDocsInRAM > 0);
  assert(
      (deleteSlice->isEmpty(), L"all deletes must be applied in prepareFlush"));
  segmentInfo->setMaxDoc(numDocsInRAM);
  shared_ptr<SegmentWriteState> *const flushState =
      make_shared<SegmentWriteState>(
          infoStream, directory, segmentInfo, fieldInfos->finish(),
          pendingUpdates,
          make_shared<IOContext>(
              make_shared<FlushInfo>(numDocsInRAM, bytesUsed())));
  constexpr double startMBUsed = bytesUsed() / 1024.0 / 1024.0;

  // Apply delete-by-docID now (delete-byDocID only
  // happens when an exception is hit processing that
  // doc, eg if analyzer has some problem w/ the text):
  if (pendingUpdates->deleteDocIDs.size() > 0) {
    flushState->liveDocs = make_shared<FixedBitSet>(numDocsInRAM);
    flushState->liveDocs->set(0, numDocsInRAM);
    for (auto delDocID : pendingUpdates->deleteDocIDs) {
      flushState->liveDocs->clear(delDocID);
    }
    flushState->delCountOnFlush = pendingUpdates->deleteDocIDs.size();
    pendingUpdates->bytesUsed->addAndGet(-pendingUpdates->deleteDocIDs.size() *
                                         BufferedUpdates::BYTES_PER_DEL_DOCID);
    pendingUpdates->deleteDocIDs.clear();
  }

  if (aborted) {
    if (infoStream->isEnabled(L"DWPT")) {
      infoStream->message(L"DWPT", L"flush: skip because aborting is set");
    }
    return nullptr;
  }

  int64_t t0 = System::nanoTime();

  if (infoStream->isEnabled(L"DWPT")) {
    infoStream->message(L"DWPT", L"flush postings as segment " +
                                     flushState->segmentInfo->name +
                                     L" numDocs=" + to_wstring(numDocsInRAM));
  }
  shared_ptr<Sorter::DocMap> *const sortMap;
  try {
    shared_ptr<DocIdSetIterator> softDeletedDocs;
    if (indexWriterConfig->getSoftDeletesField() != L"") {
      softDeletedDocs =
          consumer->getHasDocValues(indexWriterConfig->getSoftDeletesField());
    } else {
      softDeletedDocs.reset();
    }
    sortMap = consumer->flush(flushState);
    if (softDeletedDocs == nullptr) {
      flushState->softDelCountOnFlush = 0;
    } else {
      flushState->softDelCountOnFlush = PendingSoftDeletes::countSoftDeletes(
          softDeletedDocs, flushState->liveDocs);
      assert(flushState->segmentInfo->maxDoc() >=
             flushState->softDelCountOnFlush + flushState->delCountOnFlush);
    }
    // We clear this here because we already resolved them (private to this
    // segment) when writing postings:
    pendingUpdates->clearDeleteTerms();
    segmentInfo->setFiles(unordered_set<>(directory->getCreatedFiles()));

    shared_ptr<SegmentCommitInfo> *const segmentInfoPerCommit =
        make_shared<SegmentCommitInfo>(
            segmentInfo, 0, flushState->softDelCountOnFlush, -1LL, -1LL, -1LL);
    if (infoStream->isEnabled(L"DWPT")) {
      infoStream->message(L"DWPT",
                          L"new segment has " +
                              to_wstring(flushState->liveDocs == nullptr
                                             ? 0
                                             : flushState->delCountOnFlush) +
                              L" deleted docs");
      infoStream->message(L"DWPT",
                          L"new segment has " +
                              to_wstring(flushState->softDelCountOnFlush) +
                              L" soft-deleted docs");
      infoStream->message(
          L"DWPT",
          L"new segment has " +
              (flushState->fieldInfos->hasVectors() ? L"vectors"
                                                    : L"no vectors") +
              L"; " +
              (flushState->fieldInfos->hasNorms() ? L"norms" : L"no norms") +
              L"; " +
              (flushState->fieldInfos->hasDocValues() ? L"docValues"
                                                      : L"no docValues") +
              L"; " +
              (flushState->fieldInfos->hasProx() ? L"prox" : L"no prox") +
              L"; " +
              (flushState->fieldInfos->hasFreq() ? L"freqs" : L"no freqs"));
      infoStream->message(L"DWPT",
                          L"flushedFiles=" + segmentInfoPerCommit->files());
      infoStream->message(L"DWPT", L"flushed codec=" + codec);
    }

    shared_ptr<BufferedUpdates> *const segmentDeletes;
    if (pendingUpdates->deleteQueries.empty() &&
        pendingUpdates->numericUpdates.empty() &&
        pendingUpdates->binaryUpdates.empty()) {
      pendingUpdates->clear();
      segmentDeletes.reset();
    } else {
      segmentDeletes = pendingUpdates;
    }

    if (infoStream->isEnabled(L"DWPT")) {
      constexpr double newSegmentSize =
          segmentInfoPerCommit->sizeInBytes() / 1024.0 / 1024.0;
      infoStream->message(
          L"DWPT",
          L"flushed: segment=" + segmentInfo->name + L" ramUsed=" +
              nf->format(startMBUsed) + L" MB" + L" newFlushedSize=" +
              nf->format(newSegmentSize) + L" MB" + L" docs/MB=" +
              nf->format(flushState->segmentInfo->maxDoc() / newSegmentSize));
    }

    assert(segmentInfo != nullptr);

    shared_ptr<FlushedSegment> fs = make_shared<FlushedSegment>(
        infoStream, segmentInfoPerCommit, flushState->fieldInfos,
        segmentDeletes, flushState->liveDocs, flushState->delCountOnFlush,
        sortMap);
    sealFlushedSegment(fs, sortMap, flushNotifications);
    if (infoStream->isEnabled(L"DWPT")) {
      infoStream->message(L"DWPT", L"flush time " +
                                       ((System::nanoTime() - t0) / 1000000.0) +
                                       L" msec");
    }
    return fs;
  } catch (const runtime_error &t) {
    onAbortingException(t);
    throw t;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    maybeAbort(L"flush", flushNotifications);
  }
}

void DocumentsWriterPerThread::maybeAbort(
    const wstring &location, shared_ptr<DocumentsWriter::FlushNotifications>
                                 flushNotifications) 
{
  if (hasHitAbortingException() && aborted == false) {
    // if we are already aborted don't do anything here
    try {
      abort();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // whatever we do here we have to fire this tragic event up.
      flushNotifications->onTragicEvent(abortingException, location);
    }
  }
}

shared_ptr<Set<wstring>> DocumentsWriterPerThread::pendingFilesToDelete()
{
  return filesToDelete;
}

shared_ptr<FixedBitSet> DocumentsWriterPerThread::sortLiveDocs(
    shared_ptr<Bits> liveDocs,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  assert(liveDocs != nullptr && sortMap != nullptr);
  shared_ptr<FixedBitSet> sortedLiveDocs =
      make_shared<FixedBitSet>(liveDocs->length());
  sortedLiveDocs->set(0, liveDocs->length());
  for (int i = 0; i < liveDocs->length(); i++) {
    if (liveDocs->get(i) == false) {
      sortedLiveDocs->clear(sortMap->oldToNew(i));
    }
  }
  return sortedLiveDocs;
}

void DocumentsWriterPerThread::sealFlushedSegment(
    shared_ptr<FlushedSegment> flushedSegment,
    shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<DocumentsWriter::FlushNotifications>
        flushNotifications) 
{
  assert(flushedSegment != nullptr);
  shared_ptr<SegmentCommitInfo> newSegment = flushedSegment->segmentInfo;

  IndexWriter::setDiagnostics(newSegment->info, IndexWriter::SOURCE_FLUSH);

  shared_ptr<IOContext> context = make_shared<IOContext>(make_shared<FlushInfo>(
      newSegment->info->maxDoc(), newSegment->sizeInBytes()));

  bool success = false;
  try {

    if (indexWriterConfig->getUseCompoundFile()) {
      shared_ptr<Set<wstring>> originalFiles = newSegment->info->files();
      // TODO: like addIndexes, we are relying on createCompoundFile to
      // successfully cleanup...
      IndexWriter::createCompoundFile(
          infoStream, make_shared<TrackingDirectoryWrapper>(directory),
          newSegment->info, context, flushNotifications::deleteUnusedFiles);
      filesToDelete->addAll(originalFiles);
      newSegment->info->setUseCompoundFile(true);
    }

    // Have codec write SegmentInfo.  Must do this after
    // creating CFS so that 1) .si isn't slurped into CFS,
    // and 2) .si reflects useCompoundFile=true change
    // above:
    codec->segmentInfoFormat()->write(directory, newSegment->info, context);

    // TODO: ideally we would freeze newSegment here!!
    // because any changes after writing the .si will be
    // lost...

    // Must write deleted docs after the CFS so we don't
    // slurp the del file into CFS:
    if (flushedSegment->liveDocs != nullptr) {
      constexpr int delCount = flushedSegment->delCount;
      assert(delCount > 0);
      if (infoStream->isEnabled(L"DWPT")) {
        infoStream->message(
            L"DWPT", L"flush: write " + to_wstring(delCount) +
                         L" deletes gen=" +
                         to_wstring(flushedSegment->segmentInfo->getDelGen()));
      }

      // TODO: we should prune the segment if it's 100%
      // deleted... but merge will also catch it.

      // TODO: in the NRT case it'd be better to hand
      // this del deque over to the
      // shortly-to-be-opened SegmentReader and let it
      // carry the changes; there's no reason to use
      // filesystem as intermediary here.

      shared_ptr<SegmentCommitInfo> info = flushedSegment->segmentInfo;
      shared_ptr<Codec> codec = info->info->getCodec();
      shared_ptr<FixedBitSet> *const bits;
      if (sortMap == nullptr) {
        bits = flushedSegment->liveDocs;
      } else {
        bits = sortLiveDocs(flushedSegment->liveDocs, sortMap);
      }
      codec->liveDocsFormat()->writeLiveDocs(bits, directory, info, delCount,
                                             context);
      newSegment->setDelCount(delCount);
      newSegment->advanceDelGen();
    }

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      if (infoStream->isEnabled(L"DWPT")) {
        infoStream->message(
            L"DWPT",
            L"hit exception creating compound file for newly flushed segment " +
                newSegment->info->name);
      }
    }
  }
}

shared_ptr<SegmentInfo> DocumentsWriterPerThread::getSegmentInfo()
{
  return segmentInfo;
}

int64_t DocumentsWriterPerThread::bytesUsed()
{
  return bytesUsed_->get() + pendingUpdates->bytesUsed->get();
}

DocumentsWriterPerThread::IntBlockAllocator::IntBlockAllocator(
    shared_ptr<Counter> bytesUsed)
    : org::apache::lucene::util::IntBlockPool::Allocator(
          IntBlockPool::INT_BLOCK_SIZE),
      bytesUsed(bytesUsed)
{
}

std::deque<int> DocumentsWriterPerThread::IntBlockAllocator::getIntBlock()
{
  std::deque<int> b(IntBlockPool::INT_BLOCK_SIZE);
  bytesUsed->addAndGet(IntBlockPool::INT_BLOCK_SIZE * Integer::BYTES);
  return b;
}

void DocumentsWriterPerThread::IntBlockAllocator::recycleIntBlocks(
    std::deque<std::deque<int>> &blocks, int offset, int length)
{
  bytesUsed->addAndGet(
      -(length * (IntBlockPool::INT_BLOCK_SIZE * Integer::BYTES)));
}

wstring DocumentsWriterPerThread::toString()
{
  return L"DocumentsWriterPerThread [pendingDeletes=" + pendingUpdates +
         L", segment=" +
         (segmentInfo != nullptr ? segmentInfo->name : L"null") +
         L", aborted=" + StringHelper::toString(aborted) + L", numDocsInRAM=" +
         to_wstring(numDocsInRAM) + L", deleteQueue=" + deleteQueue + L"]";
}
} // namespace org::apache::lucene::index