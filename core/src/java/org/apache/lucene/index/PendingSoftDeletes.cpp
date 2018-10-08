using namespace std;

#include "PendingSoftDeletes.h"

namespace org::apache::lucene::index
{
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using IOUtils = org::apache::lucene::util::IOUtils;

PendingSoftDeletes::PendingSoftDeletes(const wstring &field,
                                       shared_ptr<SegmentCommitInfo> info)
    : PendingDeletes(info), field(field),
      hardDeletes(make_shared<PendingDeletes>(info))
{
}

PendingSoftDeletes::PendingSoftDeletes(const wstring &field,
                                       shared_ptr<SegmentReader> reader,
                                       shared_ptr<SegmentCommitInfo> info)
    : PendingDeletes(reader, info), field(field),
      hardDeletes(make_shared<PendingDeletes>(reader, info))
{
}

bool PendingSoftDeletes::delete_(int docID) 
{
  shared_ptr<FixedBitSet> mutableBits =
      getMutableBits(); // we need to fetch this first it might be a shared
                        // instance with hardDeletes
  if (hardDeletes->delete (docID)) {
    if (mutableBits->get(docID)) { // delete it here too!
      mutableBits->clear(docID);
      assert(hardDeletes->delete (docID) == false);
    } else {
      // if it was deleted subtract the delCount
      pendingDeleteCount--;
      assert(assertPendingDeletes());
    }
    return true;
  }
  return false;
}

int PendingSoftDeletes::numPendingDeletes()
{
  return PendingDeletes::numPendingDeletes() + hardDeletes->numPendingDeletes();
}

void PendingSoftDeletes::onNewReader(
    shared_ptr<CodecReader> reader,
    shared_ptr<SegmentCommitInfo> info) 
{
  PendingDeletes::onNewReader(reader, info);
  hardDeletes->onNewReader(reader, info);
  if (dvGeneration < info->getDocValuesGen()) { // only re-calculate this if we
                                                // haven't seen this generation
    shared_ptr<DocIdSetIterator> *const iterator =
        DocValuesFieldExistsQuery::getDocValuesDocIdSetIterator(field, reader);
    int newDelCount;
    if (iterator != nullptr) { // nothing is deleted we don't have a soft
                               // deletes field in this segment
      assert((info->info->maxDoc() > 0, L"maxDoc is 0"));
      newDelCount = applySoftDeletes(iterator, getMutableBits());
      assert((newDelCount >= 0,
              L" illegal pending delete count: " + to_wstring(newDelCount)));
    } else {
      newDelCount = 0;
    }
    assert((info->getSoftDelCount() == newDelCount,
            L"softDeleteCount doesn't match " +
                to_wstring(info->getSoftDelCount()) + L" != " +
                to_wstring(newDelCount)));
    dvGeneration = info->getDocValuesGen();
  }
  assert(
      (getDelCount() <= info->info->maxDoc(),
       to_wstring(getDelCount()) + L" > " + to_wstring(info->info->maxDoc())));
}

bool PendingSoftDeletes::writeLiveDocs(shared_ptr<Directory> dir) throw(
    IOException)
{
  // we need to set this here to make sure our stats in SCI are up-to-date
  // otherwise we might hit an assertion when the hard deletes are set since we
  // need to account for docs that used to be only soft-delete but now
  // hard-deleted
  this->info->setSoftDelCount(this->info->getSoftDelCount() +
                              pendingDeleteCount);
  PendingDeletes::dropChanges();
  // delegate the write to the hard deletes - it will only write if somebody
  // used it.
  if (hardDeletes->writeLiveDocs(dir)) {
    return true;
  }
  return false;
}

void PendingSoftDeletes::dropChanges()
{
  // don't reset anything here - this is called after a merge (successful or
  // not) to prevent rewriting the deleted docs to disk. we only pass it on and
  // reset the number of pending deletes
  hardDeletes->dropChanges();
}

int PendingSoftDeletes::applySoftDeletes(
    shared_ptr<DocIdSetIterator> iterator,
    shared_ptr<FixedBitSet> bits) 
{
  assert(iterator != nullptr);
  int newDeletes = 0;
  int docID;
  shared_ptr<DocValuesFieldUpdates::Iterator> hasValue =
      std::dynamic_pointer_cast<DocValuesFieldUpdates::Iterator>(iterator) !=
              nullptr
          ? std::static_pointer_cast<DocValuesFieldUpdates::Iterator>(iterator)
          : nullptr;
  while ((docID = iterator->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    if (hasValue == nullptr || hasValue->hasValue()) {
      if (bits->get(docID)) { // doc is live - clear it
        bits->clear(docID);
        newDeletes++;
        // now that we know we deleted it and we fully control the hard deletes
        // we can do correct accounting below.
      }
    } else {
      if (bits->get(docID) == false) {
        bits->set(docID);
        newDeletes--;
      }
    }
  }
  return newDeletes;
}

void PendingSoftDeletes::onDocValuesUpdate(
    shared_ptr<FieldInfo> info,
    shared_ptr<DocValuesFieldUpdates::Iterator> iterator) 
{
  if (this->field == info->name) {
    pendingDeleteCount += applySoftDeletes(iterator, getMutableBits());
    assert(assertPendingDeletes());
    assert((dvGeneration < info->getDocValuesGen(),
            L"we have seen this generation update already: " +
                to_wstring(dvGeneration) + L" vs. " +
                to_wstring(info->getDocValuesGen())));
    assert(
        (dvGeneration != -2, L"docValues generation is still uninitialized"));
    dvGeneration = info->getDocValuesGen();
    this->info->setSoftDelCount(this->info->getSoftDelCount() +
                                pendingDeleteCount);
    PendingDeletes::dropChanges();
  }
}

bool PendingSoftDeletes::assertPendingDeletes()
{
  assert((pendingDeleteCount + info->getSoftDelCount() >= 0,
          L" illegal pending delete count: " + to_wstring(pendingDeleteCount) +
              to_wstring(info->getSoftDelCount())));
  assert(info->info->maxDoc() >= getDelCount());
  return true;
}

wstring PendingSoftDeletes::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"PendingSoftDeletes(seg=")->append(info);
  sb->append(L" numPendingDeletes=")->append(pendingDeleteCount);
  sb->append(L" field=")->append(field);
  sb->append(L" dvGeneration=")->append(dvGeneration);
  sb->append(L" hardDeletes=")->append(hardDeletes);
  return sb->toString();
}

int PendingSoftDeletes::numDeletesToMerge(
    shared_ptr<MergePolicy> policy, IOSupplier<std::shared_ptr<CodecReader>>
                                        readerIOSupplier) 
{
  ensureInitialized(
      readerIOSupplier); // initialize to ensure we have accurate counts
  return PendingDeletes::numDeletesToMerge(policy, readerIOSupplier);
}

void PendingSoftDeletes::ensureInitialized(
    IOSupplier<std::shared_ptr<CodecReader>>
        readerIOSupplier) 
{
  if (dvGeneration == -2) {
    shared_ptr<FieldInfos> fieldInfos = readFieldInfos();
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(field);
    // we try to only open a reader if it's really necessary ie. indices that
    // are mainly append only might have big segments that don't even have any
    // docs in the soft deletes field. In such a case it's simply enough to look
    // at the FieldInfo for the field and check if the field has DocValues
    if (fieldInfo != nullptr &&
        fieldInfo->getDocValuesType() != DocValuesType::NONE) {
      // in order to get accurate numbers we need to have a least one reader see
      // here.
      onNewReader(readerIOSupplier(), info);
    } else {
      // we are safe here since we don't have any doc values for the soft-delete
      // field on disk no need to open a new reader
      dvGeneration = fieldInfo == nullptr ? -1 : fieldInfo->getDocValuesGen();
    }
  }
}

bool PendingSoftDeletes::isFullyDeleted(IOSupplier<std::shared_ptr<CodecReader>>
                                            readerIOSupplier) 
{
  ensureInitialized(
      readerIOSupplier); // initialize to ensure we have accurate counts - only
                         // needed in the soft-delete case
  return PendingDeletes::isFullyDeleted(readerIOSupplier);
}

shared_ptr<FieldInfos> PendingSoftDeletes::readFieldInfos() 
{
  shared_ptr<SegmentInfo> segInfo = info->info;
  shared_ptr<Directory> dir = segInfo->dir;
  if (info->hasFieldUpdates() == false) {
    // updates always outside of CFS
    shared_ptr<Closeable> toClose;
    if (segInfo->getUseCompoundFile()) {
      toClose = dir = segInfo->getCodec()->compoundFormat()->getCompoundReader(
          segInfo->dir, segInfo, IOContext::READONCE);
    } else {
      toClose.reset();
      dir = segInfo->dir;
    }
    try {
      return segInfo->getCodec()->fieldInfosFormat()->read(dir, segInfo, L"",
                                                           IOContext::READONCE);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::close({toClose});
    }
  } else {
    shared_ptr<FieldInfosFormat> fisFormat =
        segInfo->getCodec()->fieldInfosFormat();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring segmentSuffix =
        Long::toString(info->getFieldInfosGen(), Character::MAX_RADIX);
    return fisFormat->read(dir, segInfo, segmentSuffix, IOContext::READONCE);
  }
}

shared_ptr<Bits> PendingSoftDeletes::getHardLiveDocs()
{
  return hardDeletes->getLiveDocs();
}

int PendingSoftDeletes::countSoftDeletes(
    shared_ptr<DocIdSetIterator> softDeletedDocs,
    shared_ptr<Bits> hardDeletes) 
{
  int count = 0;
  if (softDeletedDocs != nullptr) {
    int doc;
    while ((doc = softDeletedDocs->nextDoc()) !=
           DocIdSetIterator::NO_MORE_DOCS) {
      if (hardDeletes == nullptr || hardDeletes->get(doc)) {
        count++;
      }
    }
  }
  return count;
}
} // namespace org::apache::lucene::index