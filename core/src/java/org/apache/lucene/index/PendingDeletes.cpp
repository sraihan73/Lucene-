using namespace std;

#include "PendingDeletes.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using IOUtils = org::apache::lucene::util::IOUtils;

PendingDeletes::PendingDeletes(shared_ptr<SegmentReader> reader,
                               shared_ptr<SegmentCommitInfo> info)
    : PendingDeletes(info, reader->getLiveDocs(), true)
{
  pendingDeleteCount = reader->numDeletedDocs() - info->getDelCount();
}

PendingDeletes::PendingDeletes(shared_ptr<SegmentCommitInfo> info)
    : PendingDeletes(info, nullptr, info->hasDeletions() == false)
{
  // if we don't have deletions we can mark it as initialized since we might
  // receive deletes on a segment without having a reader opened on it ie. after
  // a merge when we apply the deletes that IW received while merging. For
  // segments that were published we enforce a reader in the
  // BufferedUpdatesStream.SegmentState ctor
}

PendingDeletes::PendingDeletes(shared_ptr<SegmentCommitInfo> info,
                               shared_ptr<Bits> liveDocs,
                               bool liveDocsInitialized)
    : info(info)
{
  this->liveDocs = liveDocs;
  pendingDeleteCount = 0;
  this->liveDocsInitialized = liveDocsInitialized;
}

shared_ptr<FixedBitSet> PendingDeletes::getMutableBits()
{
  // if we pull mutable bits but we haven't been initialized something is
  // completely off. this means we receive deletes without having the bitset
  // that is on-disk ready to be cloned
  assert(
      (liveDocsInitialized, L"can't delete if liveDocs are not initialized"));
  if (writeableLiveDocs == nullptr) {
    // Copy on write: this means we've cloned a
    // SegmentReader sharing the current liveDocs
    // instance; must now make a private clone so we can
    // change it:
    if (liveDocs != nullptr) {
      writeableLiveDocs = FixedBitSet::copyOf(liveDocs);
    } else {
      writeableLiveDocs = make_shared<FixedBitSet>(info->info->maxDoc());
      writeableLiveDocs->set(0, info->info->maxDoc());
    }
    liveDocs = writeableLiveDocs->asReadOnlyBits();
  }
  return writeableLiveDocs;
}

bool PendingDeletes::delete_(int docID) 
{
  assert(info->info->maxDoc() > 0);
  shared_ptr<FixedBitSet> mutableBits = getMutableBits();
  assert(mutableBits != nullptr);
  assert((docID >= 0 && docID < mutableBits->length(),
          L"out of bounds: docid=" + to_wstring(docID) + L" liveDocsLength=" +
              to_wstring(mutableBits->length()) + L" seg=" + info->info->name +
              L" maxDoc=" + to_wstring(info->info->maxDoc())));
  constexpr bool didDelete = mutableBits->get(docID);
  if (didDelete) {
    mutableBits->clear(docID);
    pendingDeleteCount++;
  }
  return didDelete;
}

shared_ptr<Bits> PendingDeletes::getLiveDocs()
{
  // Prevent modifications to the returned live docs
  writeableLiveDocs.reset();
  return liveDocs;
}

shared_ptr<Bits> PendingDeletes::getHardLiveDocs() { return getLiveDocs(); }

int PendingDeletes::numPendingDeletes() { return pendingDeleteCount; }

void PendingDeletes::onNewReader(
    shared_ptr<CodecReader> reader,
    shared_ptr<SegmentCommitInfo> info) 
{
  if (liveDocsInitialized == false) {
    assert(writeableLiveDocs == nullptr);
    if (reader->hasDeletions()) {
      // we only initialize this once either in the ctor or here
      // if we use the live docs from a reader it has to be in a situation where
      // we don't have any existing live docs
      assert((pendingDeleteCount == 0,
              L"pendingDeleteCount: " + to_wstring(pendingDeleteCount)));
      liveDocs = reader->getLiveDocs();
      assert((liveDocs == nullptr ||
              assertCheckLiveDocs(liveDocs, info->info->maxDoc(),
                                  info->getDelCount())));
    }
    liveDocsInitialized = true;
  }
}

bool PendingDeletes::assertCheckLiveDocs(shared_ptr<Bits> bits,
                                         int expectedLength,
                                         int expectedDeleteCount)
{
  assert(bits->length() == expectedLength);
  int deletedCount = 0;
  for (int i = 0; i < bits->length(); i++) {
    if (bits->get(i) == false) {
      deletedCount++;
    }
  }
  assert((deletedCount == expectedDeleteCount,
          L"deleted: " + to_wstring(deletedCount) + L" != expected: " +
              to_wstring(expectedDeleteCount)));
  return true;
}

void PendingDeletes::dropChanges() { pendingDeleteCount = 0; }

wstring PendingDeletes::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"PendingDeletes(seg=")->append(info);
  sb->append(L" numPendingDeletes=")->append(pendingDeleteCount);
  sb->append(L" writeable=")->append(writeableLiveDocs != nullptr);
  return sb->toString();
}

bool PendingDeletes::writeLiveDocs(shared_ptr<Directory> dir) 
{
  if (pendingDeleteCount == 0) {
    return false;
  }

  shared_ptr<Bits> liveDocs = this->liveDocs;
  assert(liveDocs != nullptr);
  // We have new deletes
  assert(liveDocs->length() == info->info->maxDoc());

  // Do this so we can delete any created files on
  // exception; this saves all codecs from having to do
  // it:
  shared_ptr<TrackingDirectoryWrapper> trackingDir =
      make_shared<TrackingDirectoryWrapper>(dir);

  // We can write directly to the actual name (vs to a
  // .tmp & renaming it) because the file is not live
  // until segments file is written:
  bool success = false;
  try {
    shared_ptr<Codec> codec = info->info->getCodec();
    codec->liveDocsFormat()->writeLiveDocs(
        liveDocs, trackingDir, info, pendingDeleteCount, IOContext::DEFAULT);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // Advance only the nextWriteDelGen so that a 2nd
      // attempt to write will write to a new file
      info->advanceNextWriteDelGen();

      // Delete any partially created file(s):
      for (auto fileName : trackingDir->getCreatedFiles()) {
        IOUtils::deleteFilesIgnoringExceptions(dir, {fileName});
      }
    }
  }

  // If we hit an exc in the line above (eg disk full)
  // then info's delGen remains pointing to the previous
  // (successfully written) del docs:
  info->advanceDelGen();
  info->setDelCount(info->getDelCount() + pendingDeleteCount);
  dropChanges();
  return true;
}

bool PendingDeletes::isFullyDeleted(IOSupplier<std::shared_ptr<CodecReader>>
                                        readerIOSupplier) 
{
  return getDelCount() == info->info->maxDoc();
}

void PendingDeletes::onDocValuesUpdate(
    shared_ptr<FieldInfo> info,
    shared_ptr<DocValuesFieldUpdates::Iterator> iterator) 
{
}

int PendingDeletes::numDeletesToMerge(shared_ptr<MergePolicy> policy,
                                      IOSupplier<std::shared_ptr<CodecReader>>
                                          readerIOSupplier) 
{
  return policy->numDeletesToMerge(info, getDelCount(), readerIOSupplier);
}

bool PendingDeletes::needsRefresh(shared_ptr<CodecReader> reader)
{
  return reader->getLiveDocs() != getLiveDocs() ||
         reader->numDeletedDocs() != getDelCount();
}

int PendingDeletes::getDelCount()
{
  int delCount =
      info->getDelCount() + info->getSoftDelCount() + numPendingDeletes();
  return delCount;
}

int PendingDeletes::numDocs() { return info->info->maxDoc() - getDelCount(); }

bool PendingDeletes::verifyDocCounts(shared_ptr<CodecReader> reader)
{
  int count = 0;
  shared_ptr<Bits> liveDocs = getLiveDocs();
  if (liveDocs != nullptr) {
    for (int docID = 0; docID < info->info->maxDoc(); docID++) {
      if (liveDocs->get(docID)) {
        count++;
      }
    }
  } else {
    count = info->info->maxDoc();
  }
  assert((numDocs() == count,
          L"info.maxDoc=" + to_wstring(info->info->maxDoc()) +
              L" info.getDelCount()=" + to_wstring(info->getDelCount()) +));
  L" info.getSoftDelCount()=" + to_wstring(info->getSoftDelCount()) +
      L" pendingDeletes=" + toString() + L" count=" + to_wstring(count) +
      L" numDocs: " + to_wstring(numDocs());
  assert((reader->numDocs() == numDocs(),
          L"reader.numDocs() = " + to_wstring(reader->numDocs()) +
              L" numDocs() " + to_wstring(numDocs())));
  assert((reader->numDeletedDocs() <= info->info->maxDoc(),
          L"delCount=" + to_wstring(reader->numDeletedDocs()) +
              L" info.maxDoc=" + to_wstring(info->info->maxDoc()) +
              L" rld.pendingDeleteCount=" + to_wstring(numPendingDeletes()) +));
  L" info.getDelCount()=" + to_wstring(info->getDelCount());
  return true;
}
} // namespace org::apache::lucene::index