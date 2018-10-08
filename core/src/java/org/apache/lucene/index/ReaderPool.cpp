using namespace std;

#include "ReaderPool.h"

namespace org::apache::lucene::index
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

ReaderPool::ReaderPool(
    shared_ptr<Directory> directory, shared_ptr<Directory> originalDirectory,
    shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<FieldInfos::FieldNumbers> fieldNumbers,
    function<int64_t()> &completedDelGenSupplier,
    shared_ptr<InfoStream> infoStream, const wstring &softDeletesField,
    shared_ptr<StandardDirectoryReader> reader) 
    : directory(directory), originalDirectory(originalDirectory),
      fieldNumbers(fieldNumbers),
      completedDelGenSupplier(completedDelGenSupplier), infoStream(infoStream),
      segmentInfos(segmentInfos), softDeletesField(softDeletesField)
{
  if (reader != nullptr) {
    // Pre-enroll all segment readers into the reader pool; this is necessary so
    // any in-memory NRT live docs are correctly carried over, and so NRT
    // readers pulled from this IW share the same segment reader:
    deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
    assert(segmentInfos->size() == leaves.size());
    for (int i = 0; i < leaves.size(); i++) {
      shared_ptr<LeafReaderContext> leaf = leaves[i];
      shared_ptr<SegmentReader> segReader =
          std::static_pointer_cast<SegmentReader>(leaf->reader());
      shared_ptr<SegmentReader> newReader = make_shared<SegmentReader>(
          segmentInfos->info(i), segReader, segReader->getLiveDocs(),
          segReader->numDocs());
      readerMap.emplace(
          newReader->getOriginalSegmentInfo(),
          make_shared<ReadersAndUpdates>(
              segmentInfos->getIndexCreatedVersionMajor(), newReader,
              newPendingDeletes(newReader,
                                newReader->getOriginalSegmentInfo())));
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReaderPool::assertInfoIsLive(shared_ptr<SegmentCommitInfo> info)
{
  int idx = segmentInfos->find(info);
  assert((idx != -1, L"info=" + info + L" isn't live"));
  assert((segmentInfos->info(idx) == info,
          L"info=" + info + L" doesn't match live info in segmentInfos"));
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReaderPool::drop(shared_ptr<SegmentCommitInfo> info) 
{
  shared_ptr<ReadersAndUpdates> *const rld = readerMap[info];
  if (rld != nullptr) {
    assert(info == rld->info);
    readerMap.erase(info);
    rld->dropReaders();
    return true;
  }
  return false;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t ReaderPool::ramBytesUsed()
{
  int64_t bytes = 0;
  for (auto rld : readerMap) {
    bytes += rld->second.ramBytesUsed->get();
  }
  return bytes;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReaderPool::anyDeletions()
{
  for (auto rld : readerMap) {
    if (rld->second.getDelCount() > 0) {
      return true;
    }
  }
  return false;
}

void ReaderPool::enableReaderPooling() { poolReaders = true; }

bool ReaderPool::isReaderPoolingEnabled() { return poolReaders; }

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReaderPool::release(shared_ptr<ReadersAndUpdates> rld,
                         bool assertInfoLive) 
{
  bool changed = false;
  // Matches incRef in get:
  rld->decRef();

  if (rld->refCount() == 0) {
    // This happens if the segment was just merged away,
    // while a buffered deletes packet was still applying deletes/updates to it.
    assert((readerMap.find(rld->info) != readerMap.end() == false,
            L"seg=" + rld->info +
                L" has refCount 0 but still unexpectedly exists in the reader "
                L"pool"));
  } else {

    // Pool still holds a ref:
    assert((rld->refCount() > 0, L"refCount=" + to_wstring(rld->refCount()) +
                                     L" reader=" + rld->info));

    if (poolReaders == false && rld->refCount() == 1 &&
        readerMap.find(rld->info) != readerMap.end()) {
      // This is the last ref to this RLD, and we're not
      // pooling, so remove it:
      if (rld->writeLiveDocs(directory)) {
        // Make sure we only write del docs for a live segment:
        assert(assertInfoLive == false || assertInfoIsLive(rld->info));
        // Must checkpoint because we just
        // created new _X_N.del and field updates files;
        // don't call IW.checkpoint because that also
        // increments SIS.version, which we do not want to
        // do here: it was done previously (after we
        // invoked BDS.applyDeletes), whereas here all we
        // did was move the state to disk:
        changed = true;
      }
      if (rld->writeFieldUpdates(directory, fieldNumbers,
                                 completedDelGenSupplier->getAsLong(),
                                 infoStream)) {
        changed = true;
      }
      if (rld->getNumDVUpdates() == 0) {
        rld->dropReaders();
        readerMap.erase(rld->info);
      } else {
        // We are forced to pool this segment until its deletes fully apply (no
        // delGen gaps)
      }
    }
  }
  return changed;
}

// C++ WARNING: The following method was originally marked 'synchronized':
ReaderPool::~ReaderPool()
{
  if (closed->compareAndSet(false, true)) {
    dropAll();
  }
}

bool ReaderPool::writeAllDocValuesUpdates() 
{
  shared_ptr<deque<std::shared_ptr<ReadersAndUpdates>>> copy;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    // this needs to be protected by the reader pool lock otherwise we hit
    // ConcurrentModificationException
    copy = unordered_set<>(readerMap.values());
  }
  bool any = false;
  for (auto rld : copy) {
    any |= rld->writeFieldUpdates(directory, fieldNumbers,
                                  completedDelGenSupplier->getAsLong(),
                                  infoStream);
  }
  return any;
}

bool ReaderPool::writeDocValuesUpdatesForMerge(
    deque<std::shared_ptr<SegmentCommitInfo>> &infos) 
{
  bool any = false;
  for (auto info : infos) {
    shared_ptr<ReadersAndUpdates> rld = get(info, false);
    if (rld != nullptr) {
      any |= rld->writeFieldUpdates(directory, fieldNumbers,
                                    completedDelGenSupplier->getAsLong(),
                                    infoStream);
      rld->setIsMerging();
    }
  }
  return any;
}

// C++ WARNING: The following method was originally marked 'synchronized':
deque<std::shared_ptr<ReadersAndUpdates>> ReaderPool::getReadersByRam()
{
  // C++ TODO: Local classes are not converted by Java to C++ Converter:
  //      class RamRecordingHolder
  //    {
  //      final ReadersAndUpdates updates;
  //      final long ramBytesUsed;
  //      RamRecordingHolder(ReadersAndUpdates updates)
  //      {
  //        this.updates = updates;
  //        this.ramBytesUsed = updates.ramBytesUsed.get();
  //      }
  //    }
  const deque<std::shared_ptr<RamRecordingHolder>> readersByRam;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (readerMap.empty()) {
      return Collections::emptyList();
    }
    readersByRam =
        deque<std::shared_ptr<RamRecordingHolder>>(readerMap.size());
    for (auto rld : readerMap) {
      // we have to record the ram usage once and then sort
      // since the ram usage can change concurrently and that will confuse the
      // sort or hit an assertion the we can acquire here is not enough we would
      // need to lock all ReadersAndUpdates to make sure it doesn't change
      readersByRam.push_back(make_shared<RamRecordingHolder>(rld->second));
    }
  }
  // Sort this outside of the lock by largest ramBytesUsed:
  CollectionUtil::introSort(readersByRam, [&](a, b) {
    Long::compare(b::ramBytesUsed, a::ramBytesUsed);
  });
  return Collections::unmodifiableList(readersByRam.stream()
                                           .map_obj([&](any h) { h::updates; })
                                           .collect(Collectors::toList()));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReaderPool::dropAll() 
{
  runtime_error priorE = nullptr;
  constexpr unordered_map<std::shared_ptr<SegmentCommitInfo>,
                          std::shared_ptr<ReadersAndUpdates>>::const_iterator
      it = readerMap.begin();
  while (it != readerMap.end()) {
    shared_ptr<ReadersAndUpdates> *const rld = it->second;

    // Important to remove as-we-go, not with .clear()
    // in the end, in case we hit an exception;
    // otherwise we could over-decref if close() is
    // called again:
    it.remove();

    // NOTE: it is allowed that these decRefs do not
    // actually close the SRs; this happens when a
    // near real-time reader is kept open after the
    // IndexWriter instance is closed:
    try {
      rld->dropReaders();
    } catch (const runtime_error &t) {
      priorE = IOUtils::useOrSuppress(priorE, t);
    }
    it++;
  }
  assert(readerMap.empty());
  if (priorE != nullptr) {
    throw IOUtils::rethrowAlways(priorE);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReaderPool::commit(shared_ptr<SegmentInfos> infos) 
{
  bool atLeastOneChange = false;
  for (auto info : infos) {
    shared_ptr<ReadersAndUpdates> *const rld = readerMap[info];
    if (rld != nullptr) {
      assert(rld->info == info);
      bool changed = rld->writeLiveDocs(directory);
      changed |= rld->writeFieldUpdates(directory, fieldNumbers,
                                        completedDelGenSupplier->getAsLong(),
                                        infoStream);

      if (changed) {
        // Make sure we only write del docs for a live segment:
        assert(assertInfoIsLive(info));

        // Must checkpoint because we just
        // created new _X_N.del and field updates files;
        // don't call IW.checkpoint because that also
        // increments SIS.version, which we do not want to
        // do here: it was done previously (after we
        // invoked BDS.applyDeletes), whereas here all we
        // did was move the state to disk:
        atLeastOneChange = true;
      }
    }
  }
  return atLeastOneChange;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReaderPool::anyDocValuesChanges()
{
  for (auto rld : readerMap) {
    // NOTE: we don't check for pending deletes because deletes carry over in
    // RAM to NRT readers
    if (rld->second.getNumDVUpdates() != 0) {
      return true;
    }
  }
  return false;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<ReadersAndUpdates>
ReaderPool::get(shared_ptr<SegmentCommitInfo> info, bool create)
{
  assert((info->info->dir == originalDirectory,
          L"info.dir=" + info->info->dir + L" vs " + originalDirectory));
  if (closed->get()) {
    assert((readerMap.empty(), L"Reader map_obj is not empty: " + readerMap));
    throw make_shared<AlreadyClosedException>(L"ReaderPool is already closed");
  }

  shared_ptr<ReadersAndUpdates> rld = readerMap[info];
  if (rld == nullptr) {
    if (create == false) {
      return nullptr;
    }
    rld = make_shared<ReadersAndUpdates>(
        segmentInfos->getIndexCreatedVersionMajor(), info,
        newPendingDeletes(info));
    // Steal initial reference:
    readerMap.emplace(info, rld);
  } else {
    assert((rld->info == info,
            L"rld.info=" + rld->info + L" info=" + info + L" isLive?=" +
                StringHelper::toString(assertInfoIsLive(rld->info)) + L" vs " +
                StringHelper::toString(assertInfoIsLive(info))));
  }

  if (create) {
    // Return ref to caller:
    rld->incRef();
  }

  assert(noDups());

  return rld;
}

shared_ptr<PendingDeletes>
ReaderPool::newPendingDeletes(shared_ptr<SegmentCommitInfo> info)
{
  return softDeletesField == L""
             ? make_shared<PendingDeletes>(info)
             : make_shared<PendingSoftDeletes>(softDeletesField, info);
}

shared_ptr<PendingDeletes>
ReaderPool::newPendingDeletes(shared_ptr<SegmentReader> reader,
                              shared_ptr<SegmentCommitInfo> info)
{
  return softDeletesField == L""
             ? make_shared<PendingDeletes>(reader, info)
             : make_shared<PendingSoftDeletes>(softDeletesField, reader, info);
}

bool ReaderPool::noDups()
{
  shared_ptr<Set<wstring>> seen = unordered_set<wstring>();
  for (auto info : readerMap) {
    assert(!seen->contains(info->first.info.name));
    seen->add(info->first.info.name);
  }
  return true;
}
} // namespace org::apache::lucene::index