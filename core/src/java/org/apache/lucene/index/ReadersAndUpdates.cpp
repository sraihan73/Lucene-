using namespace std;

#include "ReadersAndUpdates.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using Directory = org::apache::lucene::store::Directory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

ReadersAndUpdates::ReadersAndUpdates(int indexCreatedVersionMajor,
                                     shared_ptr<SegmentCommitInfo> info,
                                     shared_ptr<PendingDeletes> pendingDeletes)
    : info(info), pendingDeletes(pendingDeletes),
      indexCreatedVersionMajor(indexCreatedVersionMajor)
{
}

ReadersAndUpdates::ReadersAndUpdates(
    int indexCreatedVersionMajor, shared_ptr<SegmentReader> reader,
    shared_ptr<PendingDeletes> pendingDeletes) 
    : ReadersAndUpdates(indexCreatedVersionMajor,
                        reader->getOriginalSegmentInfo(), pendingDeletes)
{
  this->reader = reader;
  pendingDeletes->onNewReader(reader, info);
}

void ReadersAndUpdates::incRef()
{
  constexpr int rc = refCount_->incrementAndGet();
  assert((rc > 1, L"seg=" + info));
}

void ReadersAndUpdates::decRef()
{
  constexpr int rc = refCount_->decrementAndGet();
  assert((rc >= 0, L"seg=" + info));
}

int ReadersAndUpdates::refCount()
{
  constexpr int rc = refCount_->get();
  assert(rc >= 0);
  return rc;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int ReadersAndUpdates::getDelCount() { return pendingDeletes->getDelCount(); }

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReadersAndUpdates::assertNoDupGen(
    deque<std::shared_ptr<DocValuesFieldUpdates>> &fieldUpdates,
    shared_ptr<DocValuesFieldUpdates> update)
{
  for (int i = 0; i < fieldUpdates.size(); i++) {
    shared_ptr<DocValuesFieldUpdates> oldUpdate = fieldUpdates[i];
    if (oldUpdate->delGen == update->delGen) {
      throw make_shared<AssertionError>(L"duplicate delGen=" +
                                        to_wstring(update->delGen) +
                                        L" for seg=" + info);
    }
  }
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::addDVUpdate(
    shared_ptr<DocValuesFieldUpdates> update) 
{
  if (update->getFinished() == false) {
    throw invalid_argument(L"call finish first");
  }
  deque<std::shared_ptr<DocValuesFieldUpdates>> fieldUpdates =
      pendingDVUpdates.computeIfAbsent(update->field,
                                       [&](any key) { deque<>(); });
  assert((assertNoDupGen(fieldUpdates, update)));

  ramBytesUsed->addAndGet(update->ramBytesUsed());

  fieldUpdates.push_back(update);

  if (isMerging_) {
    fieldUpdates = mergingDVUpdates[update->field];
    if (fieldUpdates.empty()) {
      fieldUpdates = deque<>();
      mergingDVUpdates.emplace(update->field, fieldUpdates);
    }
    fieldUpdates.push_back(update);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t ReadersAndUpdates::getNumDVUpdates()
{
  int64_t count = 0;
  for (auto updates : pendingDVUpdates) {
    count += updates.second->size();
  }
  return count;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SegmentReader>
ReadersAndUpdates::getReader(shared_ptr<IOContext> context) 
{
  if (reader == nullptr) {
    // We steal returned ref:
    reader =
        make_shared<SegmentReader>(info, indexCreatedVersionMajor, context);
    pendingDeletes->onNewReader(reader, info);
  }

  // Ref for caller
  reader->incRef();
  return reader;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::release(shared_ptr<SegmentReader> sr) 
{
  assert(info == sr->getOriginalSegmentInfo());
  sr->decRef();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReadersAndUpdates::delete_(int docID) 
{
  return pendingDeletes->delete (docID);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::dropReaders() 
{
  // TODO: can we somehow use IOUtils here...?  problem is
  // we are calling .decRef not .close)...
  if (reader != nullptr) {
    try {
      reader->decRef();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      reader.reset();
    }
  }

  decRef();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SegmentReader> ReadersAndUpdates::getReadOnlyClone(
    shared_ptr<IOContext> context) 
{
  if (reader == nullptr) {
    getReader(context)->decRef();
    assert(reader != nullptr);
  }
  // force new liveDocs
  shared_ptr<Bits> liveDocs = pendingDeletes->getLiveDocs();
  if (liveDocs != nullptr) {
    return make_shared<SegmentReader>(info, reader, liveDocs,
                                      pendingDeletes->numDocs());
  } else {
    // liveDocs == null and reader != null. That can only be if there are no
    // deletes
    assert(reader->getLiveDocs() == nullptr);
    reader->incRef();
    return reader;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int ReadersAndUpdates::numDeletesToMerge(shared_ptr<MergePolicy> policy) throw(
    IOException)
{
  return pendingDeletes->numDeletesToMerge(policy,
                                           shared_from_this()::getLatestReader);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<CodecReader> ReadersAndUpdates::getLatestReader() 
{
  if (this->reader == nullptr) {
    // get a reader and dec the ref right away we just make sure we have a
    // reader
    getReader(IOContext::READ)->decRef();
  }
  if (pendingDeletes->needsRefresh(reader)) {
    // we have a reader but its live-docs are out of sync. let's create a
    // temporary one that we never share
    swapNewReaderWithLatestLiveDocs();
  }
  return reader;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Bits> ReadersAndUpdates::getLiveDocs()
{
  return pendingDeletes->getLiveDocs();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Bits> ReadersAndUpdates::getHardLiveDocs()
{
  return pendingDeletes->getHardLiveDocs();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::dropChanges()
{
  // Discard (don't save) changes when we are dropping
  // the reader; this is used only on the sub-readers
  // after a successful merge.  If deletes had
  // accumulated on those sub-readers while the merge
  // is running, by now we have carried forward those
  // deletes onto the newly merged segment, so we can
  // discard them on the sub-readers:
  pendingDeletes->dropChanges();
  dropMergingUpdates();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReadersAndUpdates::writeLiveDocs(shared_ptr<Directory> dir) throw(
    IOException)
{
  return pendingDeletes->writeLiveDocs(dir);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::handleDVUpdates(
    shared_ptr<FieldInfos> infos, shared_ptr<Directory> dir,
    shared_ptr<DocValuesFormat> dvFormat, shared_ptr<SegmentReader> reader,
    unordered_map<int, Set<wstring>> &fieldFiles, int64_t maxDelGen,
    shared_ptr<InfoStream> infoStream) 
{
  for (auto ent : pendingDVUpdates) {
    const wstring field = ent.first;
    const deque<std::shared_ptr<DocValuesFieldUpdates>> updates = ent.second;
    DocValuesType type = updates[0]->type;
    assert((type == DocValuesType::NUMERIC || type == DocValuesType::BINARY,
            L"unsupported type: " + type));
    const deque<std::shared_ptr<DocValuesFieldUpdates>> updatesToApply =
        deque<std::shared_ptr<DocValuesFieldUpdates>>();
    int64_t bytes = 0;
    for (auto update : updates) {
      if (update->delGen <= maxDelGen) {
        // safe to apply this one
        bytes += update->ramBytesUsed();
        updatesToApply.push_back(update);
      }
    }
    if (updatesToApply.empty()) {
      // nothing to apply yet
      continue;
    }
    if (infoStream->isEnabled(L"BD")) {
      infoStream->message(
          L"BD", wstring::format(Locale::ROOT,
                                 L"now write %d pending numeric DV updates for "
                                 L"field=%s, seg=%s, bytes=%.3f MB",
                                 updatesToApply.size(), field, info,
                                 bytes / 1024.0 / 1024.0));
    }
    constexpr int64_t nextDocValuesGen = info->getNextDocValuesGen();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring segmentSuffix =
        Long::toString(nextDocValuesGen, Character::MAX_RADIX);
    shared_ptr<IOContext> *const updatesContext = make_shared<IOContext>(
        make_shared<FlushInfo>(info->info->maxDoc(), bytes));
    shared_ptr<FieldInfo> *const fieldInfo = infos->fieldInfo(field);
    assert(fieldInfo != nullptr);
    fieldInfo->setDocValuesGen(nextDocValuesGen);
    shared_ptr<FieldInfos> *const fieldInfos = make_shared<FieldInfos>(
        std::deque<std::shared_ptr<FieldInfo>>{fieldInfo});
    // separately also track which files were created for this gen
    shared_ptr<TrackingDirectoryWrapper> *const trackingDir =
        make_shared<TrackingDirectoryWrapper>(dir);
    shared_ptr<SegmentWriteState> *const state = make_shared<SegmentWriteState>(
        nullptr, trackingDir, info->info, fieldInfos, nullptr, updatesContext,
        segmentSuffix);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final
    // org.apache.lucene.codecs.DocValuesConsumer fieldsConsumer =
    // dvFormat.fieldsConsumer(state))
    {
      shared_ptr<org::apache::lucene::codecs::DocValuesConsumer>
          *const fieldsConsumer = dvFormat->fieldsConsumer(state);
      function<DocValuesFieldUpdates::Iterator *(FieldInfo *)> updateSupplier =
          [&](info) {
            if (info != fieldInfo) {
              throw invalid_argument(L"expected field info for field: " +
                                     fieldInfo->name + L" but got: " +
                                     info->name);
            }
            std::deque<std::shared_ptr<DocValuesFieldUpdates::Iterator>> subs(
                updatesToApply.size());
            for (int i = 0; i < subs.length; i++) {
              subs[i] = updatesToApply.get(i)->begin();
            }
            return DocValuesFieldUpdates::mergedIterator(subs);
          };
      pendingDeletes->onDocValuesUpdate(fieldInfo, updateSupplier(fieldInfo));
      if (type == DocValuesType::BINARY) {
        fieldsConsumer->addBinaryField(
            fieldInfo,
            make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                shared_from_this(), reader, field, fieldInfo, updateSupplier));
      } else {
        // write the numeric updates to a new gen'd docvalues file
        fieldsConsumer->addNumericField(
            fieldInfo,
            make_shared<EmptyDocValuesProducerAnonymousInnerClass2>(
                shared_from_this(), reader, field, fieldInfo, updateSupplier));
      }
    }
    info->advanceDocValuesGen();
    assert(fieldFiles.find(fieldInfo->number) == fieldFiles.end());
    fieldFiles.emplace(fieldInfo->number, trackingDir->getCreatedFiles());
  }
}

ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<ReadersAndUpdates> outerInstance,
        shared_ptr<org::apache::lucene::index::SegmentReader> reader,
        const wstring &field,
        shared_ptr<org::apache::lucene::index::FieldInfo> fieldInfo,
        function<DocValuesFieldUpdates::Iterator *(FieldInfo *)>
            &updateSupplier)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->field = field;
  this->fieldInfo = fieldInfo;
  this->updateSupplier = updateSupplier;
}

shared_ptr<BinaryDocValues>
ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::getBinary(
    shared_ptr<FieldInfo> fieldInfoIn) 
{
  shared_ptr<DocValuesFieldUpdates::Iterator> iterator =
      updateSupplier(fieldInfo);
  shared_ptr<MergedDocValues<std::shared_ptr<BinaryDocValues>>>
      *const mergedDocValues =
          make_shared<MergedDocValues<std::shared_ptr<BinaryDocValues>>>(
              reader->getBinaryDocValues(field),
              DocValuesFieldUpdates::Iterator::asBinaryDocValues(iterator),
              iterator);
  // Merge sort of the original doc values with updated doc values:
  return make_shared<BinaryDocValuesAnonymousInnerClass>(shared_from_this(),
                                                         mergedDocValues);
}

ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::BinaryDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::index::ReadersAndUpdates::
                       MergedDocValues<std::shared_ptr<BinaryDocValues>>>
            mergedDocValues)
{
  this->outerInstance = outerInstance;
  this->mergedDocValues = mergedDocValues;
}

shared_ptr<BytesRef>
ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::binaryValue() 
{
  return mergedDocValues->currentValuesSupplier->binaryValue();
}

bool ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::advanceExact(int target)
{
  return mergedDocValues->advanceExact(target);
}

int ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::docID()
{
  return mergedDocValues->docID();
}

int ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::nextDoc() 
{
  return mergedDocValues->nextDoc();
}

int ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::advance(int target)
{
  return mergedDocValues->advance(target);
}

int64_t ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::cost()
{
  return mergedDocValues->cost();
}

ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    EmptyDocValuesProducerAnonymousInnerClass2(
        shared_ptr<ReadersAndUpdates> outerInstance,
        shared_ptr<org::apache::lucene::index::SegmentReader> reader,
        const wstring &field,
        shared_ptr<org::apache::lucene::index::FieldInfo> fieldInfo,
        function<DocValuesFieldUpdates::Iterator *(FieldInfo *)>
            &updateSupplier)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->field = field;
  this->fieldInfo = fieldInfo;
  this->updateSupplier = updateSupplier;
}

shared_ptr<NumericDocValues>
ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::getNumeric(
    shared_ptr<FieldInfo> fieldInfoIn) 
{
  shared_ptr<DocValuesFieldUpdates::Iterator> iterator =
      updateSupplier(fieldInfo);
  shared_ptr<MergedDocValues<std::shared_ptr<NumericDocValues>>>
      *const mergedDocValues =
          make_shared<MergedDocValues<std::shared_ptr<NumericDocValues>>>(
              reader->getNumericDocValues(field),
              DocValuesFieldUpdates::Iterator::asNumericDocValues(iterator),
              iterator);
  // Merge sort of the original doc values with updated doc values:
  return make_shared<NumericDocValuesAnonymousInnerClass>(shared_from_this(),
                                                          mergedDocValues);
}

ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::NumericDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass2> outerInstance,
        shared_ptr<org::apache::lucene::index::ReadersAndUpdates::
                       MergedDocValues<std::shared_ptr<NumericDocValues>>>
            mergedDocValues)
{
  this->outerInstance = outerInstance;
  this->mergedDocValues = mergedDocValues;
}

int64_t ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::longValue() 
{
  return mergedDocValues->currentValuesSupplier->longValue();
}

bool ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::advanceExact(int target)
{
  return mergedDocValues->advanceExact(target);
}

int ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::docID()
{
  return mergedDocValues->docID();
}

int ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::nextDoc() 
{
  return mergedDocValues->nextDoc();
}

int ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::advance(int target)
{
  return mergedDocValues->advance(target);
}

int64_t ReadersAndUpdates::EmptyDocValuesProducerAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass::cost()
{
  return mergedDocValues->cost();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Set<wstring>> ReadersAndUpdates::writeFieldInfosGen(
    shared_ptr<FieldInfos> fieldInfos, shared_ptr<Directory> dir,
    shared_ptr<FieldInfosFormat> infosFormat) 
{
  constexpr int64_t nextFieldInfosGen = info->getNextFieldInfosGen();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  const wstring segmentSuffix =
      Long::toString(nextFieldInfosGen, Character::MAX_RADIX);
  // we write approximately that many bytes (based on Lucene46DVF):
  // HEADER + FOOTER: 40
  // 90 bytes per-field (over estimating long name and attributes map_obj)
  constexpr int64_t estInfosSize = 40 + 90 * fieldInfos->size();
  shared_ptr<IOContext> *const infosContext = make_shared<IOContext>(
      make_shared<FlushInfo>(info->info->maxDoc(), estInfosSize));
  // separately also track which files were created for this gen
  shared_ptr<TrackingDirectoryWrapper> *const trackingDir =
      make_shared<TrackingDirectoryWrapper>(dir);
  infosFormat->write(trackingDir, info->info, segmentSuffix, fieldInfos,
                     infosContext);
  info->advanceFieldInfosGen();
  return trackingDir->getCreatedFiles();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReadersAndUpdates::writeFieldUpdates(
    shared_ptr<Directory> dir,
    shared_ptr<FieldInfos::FieldNumbers> fieldNumbers, int64_t maxDelGen,
    shared_ptr<InfoStream> infoStream) 
{
  int64_t startTimeNS = System::nanoTime();
  const unordered_map<int, Set<wstring>> newDVFiles =
      unordered_map<int, Set<wstring>>();
  shared_ptr<Set<wstring>> fieldInfosFiles = nullptr;
  shared_ptr<FieldInfos> fieldInfos;
  bool any = false;
  for (auto updates : pendingDVUpdates) {
    // Sort by increasing delGen:
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(updates.second,
    // java.util.Comparator.comparingLong(a -> a.delGen));
    sort(updates.begin(), updates.end(),
         Comparator::comparingLong([&](any a) { a::delGen; }));
    for (shared_ptr<DocValuesFieldUpdates> update : updates.second) {
      if (update->delGen <= maxDelGen && update->any()) {
        any = true;
        break;
      }
    }
  }

  if (any == false) {
    // no updates
    return false;
  }

  // Do this so we can delete any created files on
  // exception; this saves all codecs from having to do it:
  shared_ptr<TrackingDirectoryWrapper> trackingDir =
      make_shared<TrackingDirectoryWrapper>(dir);

  bool success = false;
  try {
    shared_ptr<Codec> *const codec = info->info->getCodec();

    // reader could be null e.g. for a just merged segment (from
    // IndexWriter.commitMergedDeletes).
    shared_ptr<SegmentReader> *const reader;
    if (this->reader == nullptr) {
      reader = make_shared<SegmentReader>(info, indexCreatedVersionMajor,
                                          IOContext::READONCE);
      pendingDeletes->onNewReader(reader, info);
    } else {
      reader = this->reader;
    }

    try {
      // clone FieldInfos so that we can update their dvGen separately from
      // the reader's infos and write them to a new fieldInfos_gen file
      shared_ptr<FieldInfos::Builder> builder =
          make_shared<FieldInfos::Builder>(fieldNumbers);
      // cannot use builder.add(reader.getFieldInfos()) because it does not
      // clone FI.attributes as well FI.dvGen
      for (auto fi : reader->getFieldInfos()) {
        shared_ptr<FieldInfo> clone = builder->add(fi);
        // copy the stuff FieldInfos.Builder doesn't copy
        for (auto e : fi->attributes()) {
          clone->putAttribute(e.first, e.second);
        }
        clone->setDocValuesGen(fi->getDocValuesGen());
      }

      // create new fields with the right DV type
      for (auto updates : pendingDVUpdates) {
        shared_ptr<DocValuesFieldUpdates> update = updates.second->get(0);
        shared_ptr<FieldInfo> fieldInfo = builder->getOrAdd(update->field);
        fieldInfo->setDocValuesType(update->type);
      }

      fieldInfos = builder->finish();
      shared_ptr<DocValuesFormat> *const docValuesFormat =
          codec->docValuesFormat();

      handleDVUpdates(fieldInfos, trackingDir, docValuesFormat, reader,
                      newDVFiles, maxDelGen, infoStream);

      fieldInfosFiles = writeFieldInfosGen(fieldInfos, trackingDir,
                                           codec->fieldInfosFormat());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (reader != this->reader) {
        delete reader;
      }
    }

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      // Advance only the nextWriteFieldInfosGen and nextWriteDocValuesGen, so
      // that a 2nd attempt to write will write to a new file
      info->advanceNextWriteFieldInfosGen();
      info->advanceNextWriteDocValuesGen();

      // Delete any partially created file(s):
      for (auto fileName : trackingDir->getCreatedFiles()) {
        IOUtils::deleteFilesIgnoringExceptions(dir, {fileName});
      }
    }
  }

  // Prune the now-written DV updates:
  int64_t bytesFreed = 0;
  unordered_map<wstring,
                deque<std::shared_ptr<DocValuesFieldUpdates>>>::const_iterator
      it = pendingDVUpdates.begin();
  while (it != pendingDVUpdates.end()) {
    unordered_map::Entry<wstring,
                         deque<std::shared_ptr<DocValuesFieldUpdates>>>
        ent = *it;
    int upto = 0;
    deque<std::shared_ptr<DocValuesFieldUpdates>> updates = ent.getValue();
    for (auto update : updates) {
      if (update->delGen > maxDelGen) {
        // not yet applied
        updates[upto] = update;
        upto++;
      } else {
        bytesFreed += update->ramBytesUsed();
      }
    }
    if (upto == 0) {
      it.remove();
    } else {
      updates.subList(upto, updates.size())->clear();
    }
    it++;
  }

  int64_t bytes = ramBytesUsed->addAndGet(-bytesFreed);
  assert(bytes >= 0);

  // if there is a reader open, reopen it to reflect the updates
  if (reader != nullptr) {
    swapNewReaderWithLatestLiveDocs();
  }

  // writing field updates succeeded
  assert(fieldInfosFiles != nullptr);
  info->setFieldInfosFiles(fieldInfosFiles);

  // update the doc-values updates files. the files map_obj each field to its set
  // of files, hence we copy from the existing map_obj all fields w/ updates that
  // were not updated in this session, and add new mappings for fields that
  // were updated now.
  assert(newDVFiles.empty() == false);
  for (auto e : info->getDocValuesUpdatesFiles()) {
    if (newDVFiles.find(e.first) != newDVFiles.end() == false) {
      newDVFiles.emplace(e.first, e.second);
    }
  }
  info->setDocValuesUpdatesFiles(newDVFiles);

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD",
        wstring::format(
            Locale::ROOT,
            L"done write field updates for seg=%s; took %.3fs; new files: %s",
            info, (System::nanoTime() - startTimeNS) / 1000000000.0,
            newDVFiles));
  }
  return true;
}

shared_ptr<SegmentReader> ReadersAndUpdates::createNewReaderWithLatestLiveDocs(
    shared_ptr<SegmentReader> reader) 
{
  assert(reader != nullptr);
  assert((Thread::holdsLock(shared_from_this()),
          Thread::currentThread().getName()));
  shared_ptr<SegmentReader> newReader = make_shared<SegmentReader>(
      info, reader, pendingDeletes->getLiveDocs(), pendingDeletes->numDocs());
  bool success2 = false;
  try {
    pendingDeletes->onNewReader(newReader, info);
    reader->decRef();
    success2 = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success2 == false) {
      newReader->decRef();
    }
  }
  return newReader;
}

void ReadersAndUpdates::swapNewReaderWithLatestLiveDocs() 
{
  reader = createNewReaderWithLatestLiveDocs(reader);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::setIsMerging()
{
  // This ensures any newly resolved doc value updates while we are merging are
  // saved for re-applying after this segment is done merging:
  if (isMerging_ == false) {
    isMerging_ = true;
    assert(mergingDVUpdates.empty());
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReadersAndUpdates::isMerging() { return isMerging_; }

ReadersAndUpdates::MergeReader::MergeReader(shared_ptr<SegmentReader> reader,
                                            shared_ptr<Bits> hardLiveDocs)
    : reader(reader), hardLiveDocs(hardLiveDocs)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<MergeReader> ReadersAndUpdates::getReaderForMerge(
    shared_ptr<IOContext> context) 
{

  // We must carry over any still-pending DV updates because they were not
  // successfully written, e.g. because there was a hole in the delGens,
  // or they arrived after we wrote all DVs for merge but before we set
  // isMerging here:
  for (auto ent : pendingDVUpdates) {
    deque<std::shared_ptr<DocValuesFieldUpdates>> mergingUpdates =
        mergingDVUpdates[ent.first];
    if (mergingUpdates.empty()) {
      mergingUpdates = deque<>();
      mergingDVUpdates.emplace(ent.first, mergingUpdates);
    }
    mergingUpdates.insert(mergingUpdates.end(), ent.second::begin(),
                          ent.second::end());
  }

  shared_ptr<SegmentReader> reader = getReader(context);
  if (pendingDeletes->needsRefresh(reader)) {
    // beware of zombies:
    assert(pendingDeletes->getLiveDocs() != nullptr);
    reader = createNewReaderWithLatestLiveDocs(reader);
  }
  assert(pendingDeletes->verifyDocCounts(reader));
  return make_shared<MergeReader>(reader, pendingDeletes->getHardLiveDocs());
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReadersAndUpdates::dropMergingUpdates()
{
  mergingDVUpdates.clear();
  isMerging_ = false;
}

// C++ WARNING: The following method was originally marked 'synchronized':
unordered_map<wstring, deque<std::shared_ptr<DocValuesFieldUpdates>>>
ReadersAndUpdates::getMergingDVUpdates()
{
  // We must atomically (in single sync'd block) clear isMerging when we return
  // the DV updates otherwise we can lose updates:
  isMerging_ = false;
  return mergingDVUpdates;
}

wstring ReadersAndUpdates::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"ReadersAndLiveDocs(seg=")->append(info);
  sb->append(L" pendingDeletes=")->append(pendingDeletes);
  return sb->toString();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReadersAndUpdates::isFullyDeleted() 
{
  return pendingDeletes->isFullyDeleted(shared_from_this()::getLatestReader);
}

bool ReadersAndUpdates::keepFullyDeletedSegment(
    shared_ptr<MergePolicy> mergePolicy) 
{
  return mergePolicy->keepFullyDeletedSegment(
      shared_from_this()::getLatestReader);
}
} // namespace org::apache::lucene::index