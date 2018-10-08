using namespace std;

#include "SegmentInfos.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;
shared_ptr<java::io::PrintStream> SegmentInfos::infoStream = nullptr;

SegmentInfos::SegmentInfos(int indexCreatedVersionMajor)
    : indexCreatedVersionMajor(indexCreatedVersionMajor)
{
  if (indexCreatedVersionMajor > Version::LATEST->major) {
    throw invalid_argument(L"indexCreatedVersionMajor is in the future: " +
                           to_wstring(indexCreatedVersionMajor));
  }
  if (indexCreatedVersionMajor < 6) {
    throw invalid_argument(L"indexCreatedVersionMajor must be >= 6, got: " +
                           to_wstring(indexCreatedVersionMajor));
  }
}

shared_ptr<SegmentCommitInfo> SegmentInfos::info(int i) { return segments[i]; }

int64_t SegmentInfos::getLastCommitGeneration(std::deque<wstring> &files)
{
  int64_t max = -1;
  for (auto file : files) {
    if (file.startsWith(IndexFileNames::SEGMENTS) &&
        !file.equals(IndexFileNames::OLD_SEGMENTS_GEN)) {
      int64_t gen = generationFromSegmentsFileName(file);
      if (gen > max) {
        max = gen;
      }
    }
  }
  return max;
}

int64_t SegmentInfos::getLastCommitGeneration(
    shared_ptr<Directory> directory) 
{
  return getLastCommitGeneration(directory->listAll());
}

wstring SegmentInfos::getLastCommitSegmentsFileName(std::deque<wstring> &files)
{
  return IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS, L"",
                                                getLastCommitGeneration(files));
}

wstring SegmentInfos::getLastCommitSegmentsFileName(
    shared_ptr<Directory> directory) 
{
  return IndexFileNames::fileNameFromGeneration(
      IndexFileNames::SEGMENTS, L"", getLastCommitGeneration(directory));
}

wstring SegmentInfos::getSegmentsFileName()
{
  return IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS, L"",
                                                lastGeneration);
}

int64_t SegmentInfos::generationFromSegmentsFileName(const wstring &fileName)
{
  if (fileName == IndexFileNames::SEGMENTS) {
    return 0;
  } else if (StringHelper::startsWith(fileName, IndexFileNames::SEGMENTS)) {
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: return
    // Long.parseLong(fileName.substring(1+IndexFileNames.SEGMENTS.length()),
    // Character.MAX_RADIX);
    return int64_t ::valueOf(
        fileName.substr(1 + IndexFileNames::SEGMENTS.length()),
        Character::MAX_RADIX);
  } else {
    throw invalid_argument(L"fileName \"" + fileName +
                           L"\" is not a segments file");
  }
}

int64_t SegmentInfos::getNextPendingGeneration()
{
  if (generation == -1) {
    return 1;
  } else {
    return generation + 1;
  }
}

std::deque<char> SegmentInfos::getId() { return id.clone(); }

shared_ptr<SegmentInfos>
SegmentInfos::readCommit(shared_ptr<Directory> directory,
                         const wstring &segmentFileName) 
{

  int64_t generation = generationFromSegmentsFileName(segmentFileName);
  // System.out.println(Thread.currentThread() + ": SegmentInfos.readCommit " +
  // segmentFileName);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = directory.openChecksumInput(segmentFileName,
  // org.apache.lucene.store.IOContext.READ))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        directory->openChecksumInput(
            segmentFileName, org::apache::lucene::store::IOContext::READ);
    try {
      return readCommit(directory, input, generation);
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (EOFException | NoSuchFileException | FileNotFoundException e) {
      throw make_shared<CorruptIndexException>(
          L"Unexpected file read error while reading index.", input, e);
    }
  }
}

shared_ptr<SegmentInfos>
SegmentInfos::readCommit(shared_ptr<Directory> directory,
                         shared_ptr<ChecksumIndexInput> input,
                         int64_t generation) 
{

  // NOTE: as long as we want to throw indexformattooold (vs
  // corruptindexexception), we need to read the magic ourselves.
  int magic = input->readInt();
  if (magic != CodecUtil::CODEC_MAGIC) {
    throw make_shared<IndexFormatTooOldException>(
        input, magic, CodecUtil::CODEC_MAGIC, CodecUtil::CODEC_MAGIC);
  }
  int format = CodecUtil::checkHeaderNoMagic(input, L"segments", VERSION_53,
                                             VERSION_CURRENT);
  std::deque<char> id(StringHelper::ID_LENGTH);
  input->readBytes(id, 0, id.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  CodecUtil::checkIndexHeaderSuffix(
      input, Long::toString(generation, Character::MAX_RADIX));

  shared_ptr<Version> luceneVersion = Version::fromBits(
      input->readVInt(), input->readVInt(), input->readVInt());
  if (luceneVersion->onOrAfter(Version::LUCENE_6_0_0) == false) {
    // TODO: should we check indexCreatedVersion instead?
    throw make_shared<IndexFormatTooOldException>(
        input, L"this index is too old (version: " + luceneVersion + L")");
  }

  int indexCreatedVersion = 6;
  if (format >= VERSION_70) {
    indexCreatedVersion = input->readVInt();
  }

  shared_ptr<SegmentInfos> infos =
      make_shared<SegmentInfos>(indexCreatedVersion);
  infos->id = id;
  infos->generation = generation;
  infos->lastGeneration = generation;
  infos->luceneVersion = luceneVersion;

  infos->version = input->readLong();
  // System.out.println("READ sis version=" + infos.version);
  if (format > VERSION_70) {
    infos->counter = input->readVLong();
  } else {
    infos->counter = input->readInt();
  }
  int numSegments = input->readInt();
  if (numSegments < 0) {
    throw make_shared<CorruptIndexException>(
        L"invalid segment count: " + to_wstring(numSegments), input);
  }

  if (numSegments > 0) {
    infos->minSegmentLuceneVersion = Version::fromBits(
        input->readVInt(), input->readVInt(), input->readVInt());
  } else {
    // else leave as null: no segments
  }

  int64_t totalDocs = 0;
  for (int seg = 0; seg < numSegments; seg++) {
    wstring segName = input->readString();
    if (format < VERSION_70) {
      char hasID = input->readByte();
      if (hasID == 0) {
        throw make_shared<IndexFormatTooOldException>(
            input, L"Segment is from Lucene 4.x");
      } else if (hasID != 1) {
        throw make_shared<CorruptIndexException>(
            L"invalid hasID byte, got: " + to_wstring(hasID), input);
      }
    }
    std::deque<char> segmentID(StringHelper::ID_LENGTH);
    input->readBytes(segmentID, 0, segmentID.size());
    shared_ptr<Codec> codec = readCodec(input);
    shared_ptr<SegmentInfo> info = codec->segmentInfoFormat()->read(
        directory, segName, segmentID, IOContext::READ);
    info->setCodec(codec);
    totalDocs += info->maxDoc();
    int64_t delGen = input->readLong();
    int delCount = input->readInt();
    if (delCount < 0 || delCount > info->maxDoc()) {
      throw make_shared<CorruptIndexException>(
          L"invalid deletion count: " + to_wstring(delCount) + L" vs maxDoc=" +
              to_wstring(info->maxDoc()),
          input);
    }
    int64_t fieldInfosGen = input->readLong();
    int64_t dvGen = input->readLong();
    int softDelCount = format > VERSION_72 ? input->readInt() : 0;
    if (softDelCount < 0 || softDelCount > info->maxDoc()) {
      throw make_shared<CorruptIndexException>(
          L"invalid deletion count: " + to_wstring(softDelCount) +
              L" vs maxDoc=" + to_wstring(info->maxDoc()),
          input);
    }
    if (softDelCount + delCount > info->maxDoc()) {
      throw make_shared<CorruptIndexException>(
          L"invalid deletion count: " + to_wstring(softDelCount) +
              to_wstring(delCount) + L" vs maxDoc=" +
              to_wstring(info->maxDoc()),
          input);
    }
    shared_ptr<SegmentCommitInfo> siPerCommit = make_shared<SegmentCommitInfo>(
        info, delCount, softDelCount, delGen, fieldInfosGen, dvGen);
    siPerCommit->setFieldInfosFiles(input->readSetOfStrings());
    const unordered_map<int, Set<wstring>> dvUpdateFiles;
    constexpr int numDVFields = input->readInt();
    if (numDVFields == 0) {
      dvUpdateFiles = Collections::emptyMap();
    } else {
      unordered_map<int, Set<wstring>> obj =
          unordered_map<int, Set<wstring>>(numDVFields);
      for (int i = 0; i < numDVFields; i++) {
        obj.emplace(input->readInt(), input->readSetOfStrings());
      }
      dvUpdateFiles = obj;
    }
    siPerCommit->setDocValuesUpdatesFiles(dvUpdateFiles);
    infos->push_back(siPerCommit);

    shared_ptr<Version> segmentVersion = info->getVersion();

    if (segmentVersion->onOrAfter(infos->minSegmentLuceneVersion) == false) {
      throw make_shared<CorruptIndexException>(
          L"segments file recorded minSegmentLuceneVersion=" +
              infos->minSegmentLuceneVersion + L" but segment=" + info +
              L" has older version=" + segmentVersion,
          input);
    }

    if (infos->indexCreatedVersionMajor >= 7 &&
        segmentVersion->major < infos->indexCreatedVersionMajor) {
      throw make_shared<CorruptIndexException>(
          L"segments file recorded indexCreatedVersionMajor=" +
              to_wstring(infos->indexCreatedVersionMajor) + L" but segment=" +
              info + L" has older version=" + segmentVersion,
          input);
    }

    if (infos->indexCreatedVersionMajor >= 7 &&
        info->getMinVersion() == nullptr) {
      throw make_shared<CorruptIndexException>(
          L"segments infos must record minVersion with "
          L"indexCreatedVersionMajor=" +
              to_wstring(infos->indexCreatedVersionMajor),
          input);
    }
  }

  infos->userData = input->readMapOfStrings();

  CodecUtil::checkFooter(input);

  // LUCENE-6299: check we are in bounds
  if (totalDocs > IndexWriter::getActualMaxDocs()) {
    throw make_shared<CorruptIndexException>(
        L"Too many documents: an index cannot exceed " +
            to_wstring(IndexWriter::getActualMaxDocs()) +
            L" but readers have total maxDoc=" + to_wstring(totalDocs),
        input);
  }

  return infos;
}

shared_ptr<Codec>
SegmentInfos::readCodec(shared_ptr<DataInput> input) 
{
  const wstring name = input->readString();
  try {
    return Codec::forName(name);
  } catch (const invalid_argument &e) {
    // maybe it's an old default codec that moved
    if (StringHelper::startsWith(name, L"Lucene")) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Could not load codec
      // '" + name + "'.  Did you forget to add lucene-backward-codecs.jar?", e);
      throw invalid_argument(
          L"Could not load codec '" + name +
          L"'.  Did you forget to add lucene-backward-codecs.jar?");
    }
    throw e;
  }
}

shared_ptr<SegmentInfos> SegmentInfos::readLatestCommit(
    shared_ptr<Directory> directory) 
{
  return make_shared<FindSegmentsFileAnonymousInnerClass>(directory).run();
}

SegmentInfos::FindSegmentsFileAnonymousInnerClass::
    FindSegmentsFileAnonymousInnerClass(shared_ptr<Directory> directory)
    : FindSegmentsFile<SegmentInfos>(directory)
{
  this->directory = directory;
}

shared_ptr<SegmentInfos>
SegmentInfos::FindSegmentsFileAnonymousInnerClass::doBody(
    const wstring &segmentFileName) 
{
  return readCommit(directory, segmentFileName);
}

void SegmentInfos::write(shared_ptr<Directory> directory) 
{

  int64_t nextGeneration = getNextPendingGeneration();
  wstring segmentFileName = IndexFileNames::fileNameFromGeneration(
      IndexFileNames::PENDING_SEGMENTS, L"", nextGeneration);

  // Always advance the generation on write:
  generation = nextGeneration;

  shared_ptr<IndexOutput> segnOutput = nullptr;
  bool success = false;

  try {
    segnOutput = directory->createOutput(segmentFileName, IOContext::DEFAULT);
    write(directory, segnOutput);
    delete segnOutput;
    directory->sync(Collections::singleton(segmentFileName));
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      pendingCommit = true;
    } else {
      // We hit an exception above; try to close the file
      // but suppress any exception:
      IOUtils::closeWhileHandlingException({segnOutput});
      // Try not to leave a truncated segments_N file in
      // the index:
      IOUtils::deleteFilesIgnoringExceptions(directory, {segmentFileName});
    }
  }
}

void SegmentInfos::write(shared_ptr<Directory> directory,
                         shared_ptr<IndexOutput> out) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  CodecUtil::writeIndexHeader(out, L"segments", VERSION_CURRENT,
                              StringHelper::randomId(),
                              Long::toString(generation, Character::MAX_RADIX));
  out->writeVInt(Version::LATEST->major);
  out->writeVInt(Version::LATEST->minor);
  out->writeVInt(Version::LATEST->bugfix);
  // System.out.println(Thread.currentThread().getName() + ": now write " +
  // out.getName() + " with version=" + version);

  out->writeVInt(indexCreatedVersionMajor);

  out->writeLong(version);
  out->writeVLong(counter); // write counter
  out->writeInt(size());

  if (size() > 0) {

    shared_ptr<Version> minSegmentVersion = nullptr;

    // We do a separate loop up front so we can write the minSegmentVersion
    // before any SegmentInfo; this makes it cleaner to throw
    // IndexFormatTooOldExc at read time:
    for (auto siPerCommit : shared_from_this()) {
      shared_ptr<Version> segmentVersion = siPerCommit->info->getVersion();
      if (minSegmentVersion == nullptr ||
          segmentVersion->onOrAfter(minSegmentVersion) == false) {
        minSegmentVersion = segmentVersion;
      }
    }

    out->writeVInt(minSegmentVersion->major);
    out->writeVInt(minSegmentVersion->minor);
    out->writeVInt(minSegmentVersion->bugfix);
  }

  // write infos
  for (auto siPerCommit : shared_from_this()) {
    shared_ptr<SegmentInfo> si = siPerCommit->info;
    if (indexCreatedVersionMajor >= 7 && si->minVersion == nullptr) {
      throw make_shared<IllegalStateException>(
          L"Segments must record minVersion if they have been created on or "
          L"after Lucene 7: " +
          si);
    }
    out->writeString(si->name);
    std::deque<char> segmentID = si->getId();
    if (segmentID.size() != StringHelper::ID_LENGTH) {
      throw make_shared<IllegalStateException>(
          L"cannot write segment: invalid id segment=" + si->name + L"id=" +
          StringHelper::idToString(segmentID));
    }
    out->writeBytes(segmentID, segmentID.size());
    out->writeString(si->getCodec()->getName());
    out->writeLong(siPerCommit->getDelGen());
    int delCount = siPerCommit->getDelCount();
    if (delCount < 0 || delCount > si->maxDoc()) {
      throw make_shared<IllegalStateException>(
          L"cannot write segment: invalid maxDoc segment=" + si->name +
          L" maxDoc=" + to_wstring(si->maxDoc()) + L" delCount=" +
          to_wstring(delCount));
    }
    out->writeInt(delCount);
    out->writeLong(siPerCommit->getFieldInfosGen());
    out->writeLong(siPerCommit->getDocValuesGen());
    int softDelCount = siPerCommit->getSoftDelCount();
    if (softDelCount < 0 || softDelCount > si->maxDoc()) {
      throw make_shared<IllegalStateException>(
          L"cannot write segment: invalid maxDoc segment=" + si->name +
          L" maxDoc=" + to_wstring(si->maxDoc()) + L" softDelCount=" +
          to_wstring(softDelCount));
    }
    out->writeInt(softDelCount);
    out->writeSetOfStrings(siPerCommit->getFieldInfosFiles());
    const unordered_map<int, Set<wstring>> dvUpdatesFiles =
        siPerCommit->getDocValuesUpdatesFiles();
    out->writeInt(dvUpdatesFiles.size());
    for (auto e : dvUpdatesFiles) {
      out->writeInt(e.first);
      out->writeSetOfStrings(e.second);
    }
  }
  out->writeMapOfStrings(userData);
  CodecUtil::writeFooter(out);
}

shared_ptr<SegmentInfos> SegmentInfos::clone()
{
  try {
    shared_ptr<SegmentInfos> *const sis =
        std::static_pointer_cast<SegmentInfos>(__super::clone());
    // deep clone, first recreate all collections:
    sis->segments = deque<>(size());
    for (auto info : shared_from_this()) {
      assert(info->info->getCodec() != nullptr);
      // dont directly access segments, use add method!!!
      sis->push_back(info->clone());
    }
    sis->userData = unordered_map<>(userData);
    return sis;
  } catch (const CloneNotSupportedException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("should not happen", e);
    throw runtime_error(L"should not happen");
  }
}

int64_t SegmentInfos::getVersion() { return version; }

int64_t SegmentInfos::getGeneration() { return generation; }

int64_t SegmentInfos::getLastGeneration() { return lastGeneration; }

void SegmentInfos::setInfoStream(shared_ptr<PrintStream> infoStream)
{
  SegmentInfos::infoStream = infoStream;
}

shared_ptr<PrintStream> SegmentInfos::getInfoStream() { return infoStream; }

void SegmentInfos::message(const wstring &message)
{
  infoStream->println(L"SIS [" + Thread::currentThread().getName() + L"]: " +
                      message);
}

void SegmentInfos::updateGeneration(shared_ptr<SegmentInfos> other)
{
  lastGeneration = other->lastGeneration;
  generation = other->generation;
}

void SegmentInfos::updateGenerationVersionAndCounter(
    shared_ptr<SegmentInfos> other)
{
  updateGeneration(other);
  this->version = other->version;
  this->counter = other->counter;
}

void SegmentInfos::setNextWriteGeneration(int64_t generation)
{
  if (generation < this->generation) {
    throw make_shared<IllegalStateException>(
        L"cannot decrease generation to " + to_wstring(generation) +
        L" from current generation " + to_wstring(this->generation));
  }
  this->generation = generation;
}

void SegmentInfos::rollbackCommit(shared_ptr<Directory> dir)
{
  if (pendingCommit) {
    pendingCommit = false;

    // we try to clean up our pending_segments_N

    // Must carefully compute fileName from "generation"
    // since lastGeneration isn't incremented:
    const wstring pending = IndexFileNames::fileNameFromGeneration(
        IndexFileNames::PENDING_SEGMENTS, L"", generation);

    // Suppress so we keep throwing the original exception
    // in our caller
    IOUtils::deleteFilesIgnoringExceptions(dir, {pending});
  }
}

void SegmentInfos::prepareCommit(shared_ptr<Directory> dir) 
{
  if (pendingCommit) {
    throw make_shared<IllegalStateException>(
        L"prepareCommit was already called");
  }
  dir->syncMetaData();
  write(dir);
}

shared_ptr<deque<wstring>>
SegmentInfos::files(bool includeSegmentsFile) 
{
  unordered_set<wstring> files = unordered_set<wstring>();
  if (includeSegmentsFile) {
    const wstring segmentFileName = getSegmentsFileName();
    if (segmentFileName != L"") {
      files.insert(segmentFileName);
    }
  }
  constexpr int size = this->size();
  for (int i = 0; i < size; i++) {
    shared_ptr<SegmentCommitInfo> *const info = this->info(i);
    files.addAll(info->files());
  }

  return files;
}

wstring SegmentInfos::finishCommit(shared_ptr<Directory> dir) 
{
  if (pendingCommit == false) {
    throw make_shared<IllegalStateException>(L"prepareCommit was not called");
  }
  bool success = false;
  const wstring dest;
  try {
    const wstring src = IndexFileNames::fileNameFromGeneration(
        IndexFileNames::PENDING_SEGMENTS, L"", generation);
    dest = IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS, L"",
                                                  generation);
    dir->rename(src, dest);
    dir->syncMetaData();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // deletes pending_segments_N:
      rollbackCommit(dir);
    }
  }

  pendingCommit = false;
  lastGeneration = generation;
  return dest;
}

void SegmentInfos::commit(shared_ptr<Directory> dir) 
{
  prepareCommit(dir);
  finishCommit(dir);
}

wstring SegmentInfos::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(getSegmentsFileName())->append(L": ");
  constexpr int count = size();
  for (int i = 0; i < count; i++) {
    if (i > 0) {
      buffer->append(L' ');
    }
    shared_ptr<SegmentCommitInfo> *const info = this->info(i);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(info->toString(0));
  }
  return buffer->toString();
}

unordered_map<wstring, wstring> SegmentInfos::getUserData() { return userData; }

void SegmentInfos::setUserData(unordered_map<wstring, wstring> &data,
                               bool doIncrementVersion)
{
  if (data.empty()) {
    userData = Collections::emptyMap<wstring, wstring>();
  } else {
    userData = data;
  }
  if (doIncrementVersion) {
    changed();
  }
}

void SegmentInfos::replace(shared_ptr<SegmentInfos> other)
{
  rollbackSegmentInfos(other->asList());
  lastGeneration = other->lastGeneration;
}

int SegmentInfos::totalMaxDoc()
{
  int64_t count = 0;
  for (auto info : shared_from_this()) {
    count += info->info->maxDoc();
  }
  // we should never hit this, checks should happen elsewhere...
  assert(count <= IndexWriter::getActualMaxDocs());
  return Math::toIntExact(count);
}

void SegmentInfos::changed()
{
  version++;
  // System.out.println(Thread.currentThread().getName() + ": SIS.change to
  // version=" + version); new Throwable().printStackTrace(System.out);
}

void SegmentInfos::setVersion(int64_t newVersion)
{
  if (newVersion < version) {
    throw invalid_argument(L"newVersion (=" + to_wstring(newVersion) +
                           L") cannot be less than current version (=" +
                           to_wstring(version) + L")");
  }
  // System.out.println(Thread.currentThread().getName() + ": SIS.setVersion
  // change from " + version + " to " + newVersion);
  version = newVersion;
}

void SegmentInfos::applyMergeChanges(shared_ptr<MergePolicy::OneMerge> merge,
                                     bool dropSegment)
{
  if (indexCreatedVersionMajor >= 7 &&
      merge->info->info->minVersion == nullptr) {
    throw invalid_argument(L"All segments must record the minVersion for "
                           L"indices created on or after Lucene 7");
  }

  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const mergedAway =
      unordered_set<std::shared_ptr<SegmentCommitInfo>>(merge->segments);
  bool inserted = false;
  int newSegIdx = 0;
  for (int segIdx = 0, cnt = segments.size(); segIdx < cnt; segIdx++) {
    assert(segIdx >= newSegIdx);
    shared_ptr<SegmentCommitInfo> *const info = segments[segIdx];
    if (mergedAway->contains(info)) {
      if (!inserted && !dropSegment) {
        segments[segIdx] = merge->info;
        inserted = true;
        newSegIdx++;
      }
    } else {
      segments[newSegIdx] = info;
      newSegIdx++;
    }
  }

  // the rest of the segments in deque are duplicates, so don't remove from map_obj,
  // only deque!
  segments.subList(newSegIdx, segments.size())->clear();

  // Either we found place to insert segment, or, we did
  // not, but only because all segments we merged becamee
  // deleted while we are merging, in which case it should
  // be the case that the new segment is also all deleted,
  // we insert it at the beginning if it should not be dropped:
  if (!inserted && !dropSegment) {
    segments.push_back(0, merge->info);
  }
}

deque<std::shared_ptr<SegmentCommitInfo>>
SegmentInfos::createBackupSegmentInfos()
{
  const deque<std::shared_ptr<SegmentCommitInfo>> deque =
      deque<std::shared_ptr<SegmentCommitInfo>>(size());
  for (auto info : shared_from_this()) {
    assert(info->info->getCodec() != nullptr);
    deque.push_back(info->clone());
  }
  return deque;
}

void SegmentInfos::rollbackSegmentInfos(
    deque<std::shared_ptr<SegmentCommitInfo>> &infos)
{
  this->clear();
  this->insert(this->end(), infos.begin(), infos.end());
}

shared_ptr<Iterator<std::shared_ptr<SegmentCommitInfo>>>
SegmentInfos::iterator()
{
  return asList().begin();
}

deque<std::shared_ptr<SegmentCommitInfo>> SegmentInfos::asList()
{
  return Collections::unmodifiableList(segments);
}

int SegmentInfos::size() { return segments.size(); }

void SegmentInfos::add(shared_ptr<SegmentCommitInfo> si)
{
  if (indexCreatedVersionMajor >= 7 && si->info->minVersion == nullptr) {
    throw invalid_argument(L"All segments must record the minVersion for "
                           L"indices created on or after Lucene 7");
  }

  segments.push_back(si);
}

void SegmentInfos::addAll(deque<std::shared_ptr<SegmentCommitInfo>> &sis)
{
  for (auto si : sis) {
    this->push_back(si);
  }
}

void SegmentInfos::clear() { segments.clear(); }

bool SegmentInfos::remove(shared_ptr<SegmentCommitInfo> si)
{
  // C++ TODO: The Java deque 'remove(Object)' method is not converted:
  return segments.remove(si);
}

void SegmentInfos::remove(int index)
{
  segments.erase(segments.begin() + index);
}

bool SegmentInfos::contains(shared_ptr<SegmentCommitInfo> si)
{
  return find(segments.begin(), segments.end(), si) != segments.end();
}

int SegmentInfos::indexOf(shared_ptr<SegmentCommitInfo> si)
{
  return segments.find(si);
}

shared_ptr<Version> SegmentInfos::getCommitLuceneVersion()
{
  return luceneVersion;
}

shared_ptr<Version> SegmentInfos::getMinSegmentLuceneVersion()
{
  return minSegmentLuceneVersion;
}

int SegmentInfos::getIndexCreatedVersionMajor()
{
  return indexCreatedVersionMajor;
}
} // namespace org::apache::lucene::index