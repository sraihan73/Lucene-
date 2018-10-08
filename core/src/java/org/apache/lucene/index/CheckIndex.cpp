using namespace std;

#include "CheckIndex.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Document = org::apache::lucene::document::Document;
using DocumentStoredFieldVisitor =
    org::apache::lucene::document::DocumentStoredFieldVisitor;
using DocValuesStatus =
    org::apache::lucene::index::CheckIndex::Status::DocValuesStatus;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using Lock = org::apache::lucene::store::Lock;
using Accountables = org::apache::lucene::util::Accountables;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CommandLineUtil = org::apache::lucene::util::CommandLineUtil;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using StringHelper = org::apache::lucene::util::StringHelper;
using org::apache::lucene::util::SuppressForbidden;
using Version = org::apache::lucene::util::Version;
using Automata = org::apache::lucene::util::automaton::Automata;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

CheckIndex::Status::Status() {}

CheckIndex::Status::SegmentInfoStatus::SegmentInfoStatus() {}

CheckIndex::Status::LiveDocStatus::LiveDocStatus() {}

CheckIndex::Status::FieldInfoStatus::FieldInfoStatus() {}

CheckIndex::Status::FieldNormStatus::FieldNormStatus() {}

CheckIndex::Status::TermIndexStatus::TermIndexStatus() {}

CheckIndex::Status::StoredFieldStatus::StoredFieldStatus() {}

CheckIndex::Status::TermVectorStatus::TermVectorStatus() {}

shared_ptr<internal> CheckIndex::Status::DocValuesStatus::DocValuesStatus() {}

CheckIndex::Status::PointsStatus::PointsStatus() {}

CheckIndex::Status::IndexSortStatus::IndexSortStatus() {}

CheckIndex::CheckIndex(shared_ptr<Directory> dir) 
    : CheckIndex(dir, dir->obtainLock(IndexWriter::WRITE_LOCK_NAME))
{
}

CheckIndex::CheckIndex(shared_ptr<Directory> dir, shared_ptr<Lock> writeLock)
{
  this->dir = dir;
  this->writeLock = writeLock;
  this->infoStream.reset();
}

void CheckIndex::ensureOpen()
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(L"this instance is closed");
  }
}

CheckIndex::~CheckIndex()
{
  closed = true;
  IOUtils::close({writeLock});
}

void CheckIndex::setCrossCheckTermVectors(bool v) { crossCheckTermVectors = v; }

bool CheckIndex::getCrossCheckTermVectors() { return crossCheckTermVectors; }

void CheckIndex::setFailFast(bool v) { failFast = v; }

bool CheckIndex::getFailFast() { return failFast; }

bool CheckIndex::getChecksumsOnly() { return checksumsOnly; }

void CheckIndex::setChecksumsOnly(bool v) { checksumsOnly = v; }

void CheckIndex::setInfoStream(shared_ptr<PrintStream> out, bool verbose)
{
  infoStream = out;
  this->verbose = verbose;
}

void CheckIndex::setInfoStream(shared_ptr<PrintStream> out)
{
  setInfoStream(out, false);
}

void CheckIndex::msg(shared_ptr<PrintStream> out, const wstring &msg)
{
  if (out != nullptr) {
    out->println(msg);
  }
}

shared_ptr<Status> CheckIndex::checkIndex() 
{
  return checkIndex(nullptr);
}

shared_ptr<Status>
CheckIndex::checkIndex(deque<wstring> &onlySegments) 
{
  ensureOpen();
  int64_t startNS = System::nanoTime();
  shared_ptr<NumberFormat> nf = NumberFormat::getInstance(Locale::ROOT);
  shared_ptr<SegmentInfos> sis;
  shared_ptr<Status> result = make_shared<Status>();
  result->dir = dir;
  std::deque<wstring> files = dir->listAll();
  wstring lastSegmentsFile = SegmentInfos::getLastCommitSegmentsFileName(files);
  if (lastSegmentsFile == L"") {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<IndexNotFoundException>(L"no segments* file found in " +
                                              dir + L": files: " +
                                              Arrays->toString(files));
  }
  try {
    // Do not use SegmentInfos.read(Directory) since the spooky
    // retrying it does is not necessary here (we hold the write lock):
    sis = SegmentInfos::readCommit(dir, lastSegmentsFile);
  } catch (const runtime_error &t) {
    if (failFast) {
      throw IOUtils::rethrowAlways(t);
    }
    msg(infoStream, L"ERROR: could not read any segments file in directory");
    result->missingSegments = true;
    if (infoStream != nullptr) {
      t.printStackTrace(infoStream);
    }
    return result;
  }

  // find the oldest and newest segment versions
  shared_ptr<Version> oldest = nullptr;
  shared_ptr<Version> newest = nullptr;
  wstring oldSegs = L"";
  for (auto si : sis) {
    shared_ptr<Version> version = si->info->getVersion();
    if (version == nullptr) {
      // pre-3.1 segment
      oldSegs = L"pre-3.1";
    } else {
      if (oldest == nullptr || version->onOrAfter(oldest) == false) {
        oldest = version;
      }
      if (newest == nullptr || version->onOrAfter(newest)) {
        newest = version;
      }
    }
  }

  constexpr int numSegments = sis->size();
  const wstring segmentsFileName = sis->getSegmentsFileName();
  // note: we only read the format byte (required preamble) here!
  shared_ptr<IndexInput> input = nullptr;
  try {
    input = dir->openInput(segmentsFileName, IOContext::READONCE);
  } catch (const runtime_error &t) {
    if (failFast) {
      throw IOUtils::rethrowAlways(t);
    }
    msg(infoStream, L"ERROR: could not open segments file in directory");
    if (infoStream != nullptr) {
      t.printStackTrace(infoStream);
    }
    result->cantOpenSegments = true;
    return result;
  }
  try {
    /*int format =*/
    input->readInt();
  } catch (const runtime_error &t) {
    if (failFast) {
      throw IOUtils::rethrowAlways(t);
    }
    msg(infoStream, L"ERROR: could not read segment file version in directory");
    if (infoStream != nullptr) {
      t.printStackTrace(infoStream);
    }
    result->missingSegmentVersion = true;
    return result;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (input != nullptr) {
      delete input;
    }
  }

  result->segmentsFileName = segmentsFileName;
  result->numSegments = numSegments;
  result->userData = sis->getUserData();
  wstring userDataString;
  if (sis->getUserData().size() > 0) {
    userDataString = L" userData=" + sis->getUserData();
  } else {
    userDataString = L"";
  }

  wstring versionString = L"";
  if (oldSegs != L"") {
    if (newest != nullptr) {
      versionString = L"versions=[" + oldSegs + L" .. " + newest + L"]";
    } else {
      versionString = L"version=" + oldSegs;
    }
  } else if (newest != nullptr) { // implies oldest != null
    versionString = oldest->equals(newest)
                        ? (L"version=" + oldest)
                        : (L"versions=[" + oldest + L" .. " + newest + L"]");
  }

  msg(infoStream, L"Segments file=" + segmentsFileName + L" numSegments=" +
                      to_wstring(numSegments) + L" " + versionString + L" id=" +
                      StringHelper::idToString(sis->getId()) + userDataString);

  if (onlySegments.size() > 0) {
    result->partial_ = true;
    if (infoStream != nullptr) {
      infoStream->print(L"\nChecking only these segments:");
      for (auto s : onlySegments) {
        infoStream->print(L" " + s);
      }
    }
    result->segmentsChecked.insert(result->segmentsChecked.end(),
                                   onlySegments.begin(), onlySegments.end());
    msg(infoStream, L":");
  }

  result->newSegments = sis->clone();
  result->newSegments->clear();
  result->maxSegmentName = -1;

  for (int i = 0; i < numSegments; i++) {
    shared_ptr<SegmentCommitInfo> *const info = sis->info(i);
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: long segmentName =
    // Long.parseLong(info.info.name.substring(1), Character.MAX_RADIX);
    int64_t segmentName =
        int64_t ::valueOf(info->info->name.substr(1), Character::MAX_RADIX);
    if (segmentName > result->maxSegmentName) {
      result->maxSegmentName = segmentName;
    }
    if (onlySegments.size() > 0 &&
        !find(onlySegments.begin(), onlySegments.end(), info->info->name) !=
            onlySegments.end()) {
      continue;
    }
    shared_ptr<Status::SegmentInfoStatus> segInfoStat =
        make_shared<Status::SegmentInfoStatus>();
    result->segmentInfos.push_back(segInfoStat);
    msg(infoStream, L"  " + to_wstring(1 + i) + L" of " +
                        to_wstring(numSegments) + L": name=" +
                        info->info->name + L" maxDoc=" +
                        to_wstring(info->info->maxDoc()));
    segInfoStat->name = info->info->name;
    segInfoStat->maxDoc = info->info->maxDoc();

    shared_ptr<Version> *const version = info->info->getVersion();
    if (info->info->maxDoc() <= 0) {
      throw runtime_error(L"illegal number of documents: maxDoc=" +
                          to_wstring(info->info->maxDoc()));
    }

    int toLoseDocCount = info->info->maxDoc();

    shared_ptr<SegmentReader> reader = nullptr;
    shared_ptr<Sort> previousIndexSort = nullptr;

    try {
      msg(infoStream,
          L"    version=" + (version == nullptr ? L"3.0" : version));
      msg(infoStream,
          L"    id=" + StringHelper::idToString(info->info->getId()));
      shared_ptr<Codec> *const codec = info->info->getCodec();
      msg(infoStream, L"    codec=" + codec);
      segInfoStat->codec = codec;
      msg(infoStream, L"    compound=" + StringHelper::toString(
                                             info->info->getUseCompoundFile()));
      segInfoStat->compound = info->info->getUseCompoundFile();
      msg(infoStream, L"    numFiles=" + info->files()->size());
      shared_ptr<Sort> indexSort = info->info->getIndexSort();
      if (indexSort != nullptr) {
        msg(infoStream, L"    sort=" + indexSort);
        if (previousIndexSort != nullptr) {
          if (previousIndexSort->equals(indexSort) == false) {
            throw runtime_error(L"index sort changed from " +
                                previousIndexSort + L" to " + indexSort);
          }
        } else {
          previousIndexSort = indexSort;
        }
      }
      segInfoStat->numFiles = info->files()->size();
      segInfoStat->sizeMB = info->sizeInBytes() / (1024.0 * 1024.0);
      msg(infoStream, L"    size (MB)=" + nf->format(segInfoStat->sizeMB));
      unordered_map<wstring, wstring> diagnostics =
          info->info->getDiagnostics();
      segInfoStat->diagnostics = diagnostics;
      if (diagnostics.size() > 0) {
        msg(infoStream, L"    diagnostics = " + diagnostics);
      }

      if (!info->hasDeletions()) {
        msg(infoStream, L"    no deletions");
        segInfoStat->hasDeletions = false;
      } else {
        msg(infoStream, L"    has deletions [delGen=" +
                            to_wstring(info->getDelGen()) + L"]");
        segInfoStat->hasDeletions = true;
        segInfoStat->deletionsGen = info->getDelGen();
      }

      int64_t startOpenReaderNS = System::nanoTime();
      if (infoStream != nullptr) {
        infoStream->print(L"    test: open reader.........");
      }
      reader = make_shared<SegmentReader>(
          info, sis->getIndexCreatedVersionMajor(), IOContext::DEFAULT);
      msg(infoStream,
          wstring::format(Locale::ROOT, L"OK [took %.3f sec]",
                          nsToSec(System::nanoTime() - startOpenReaderNS)));

      segInfoStat->openReaderPassed = true;

      int64_t startIntegrityNS = System::nanoTime();
      if (infoStream != nullptr) {
        infoStream->print(L"    test: check integrity.....");
      }
      reader->checkIntegrity();
      msg(infoStream,
          wstring::format(Locale::ROOT, L"OK [took %.3f sec]",
                          nsToSec(System::nanoTime() - startIntegrityNS)));

      if (reader->maxDoc() != info->info->maxDoc()) {
        throw runtime_error(
            L"SegmentReader.maxDoc() " + to_wstring(reader->maxDoc()) +
            L" != SegmentInfo.maxDoc " + to_wstring(info->info->maxDoc()));
      }

      constexpr int numDocs = reader->numDocs();
      toLoseDocCount = numDocs;

      if (reader->hasDeletions()) {
        if (reader->numDocs() != info->info->maxDoc() - info->getDelCount()) {
          throw runtime_error(
              L"delete count mismatch: info=" +
              to_wstring(info->info->maxDoc() - info->getDelCount()) +
              L" vs reader=" + to_wstring(reader->numDocs()));
        }
        if ((info->info->maxDoc() - reader->numDocs()) > reader->maxDoc()) {
          throw runtime_error(
              L"too many deleted docs: maxDoc()=" +
              to_wstring(reader->maxDoc()) + L" vs del count=" +
              to_wstring(info->info->maxDoc() - reader->numDocs()));
        }
        if (info->info->maxDoc() - reader->numDocs() != info->getDelCount()) {
          throw runtime_error(
              L"delete count mismatch: info=" +
              to_wstring(info->getDelCount()) + L" vs reader=" +
              to_wstring(info->info->maxDoc() - reader->numDocs()));
        }
      } else {
        if (info->getDelCount() != 0) {
          throw runtime_error(
              L"delete count mismatch: info=" +
              to_wstring(info->getDelCount()) + L" vs reader=" +
              to_wstring(info->info->maxDoc() - reader->numDocs()));
        }
      }

      if (checksumsOnly == false) {
        // Test Livedocs
        segInfoStat->liveDocStatus = testLiveDocs(reader, infoStream, failFast);

        // Test Fieldinfos
        segInfoStat->fieldInfoStatus =
            testFieldInfos(reader, infoStream, failFast);

        // Test Field Norms
        segInfoStat->fieldNormStatus =
            testFieldNorms(reader, infoStream, failFast);

        // Test the Term Index
        segInfoStat->termIndexStatus =
            testPostings(reader, infoStream, verbose, failFast, version);

        // Test Stored Fields
        segInfoStat->storedFieldStatus =
            testStoredFields(reader, infoStream, failFast);

        // Test Term Vectors
        segInfoStat->termVectorStatus =
            testTermVectors(reader, infoStream, verbose, crossCheckTermVectors,
                            failFast, version);

        // Test Docvalues
        segInfoStat->docValuesStatus =
            testDocValues(reader, infoStream, failFast);

        // Test PointValues
        segInfoStat->pointsStatus = testPoints(reader, infoStream, failFast);

        // Test index sort
        segInfoStat->indexSortStatus =
            testSort(reader, indexSort, infoStream, failFast);

        // Rethrow the first exception we encountered
        //  This will cause stats for failed segments to be incremented properly
        if (segInfoStat->liveDocStatus->error != nullptr) {
          throw runtime_error(L"Live docs test failed");
        } else if (segInfoStat->fieldInfoStatus->error != nullptr) {
          throw runtime_error(L"Field Info test failed");
        } else if (segInfoStat->fieldNormStatus->error != nullptr) {
          throw runtime_error(L"Field Norm test failed");
        } else if (segInfoStat->termIndexStatus->error != nullptr) {
          throw runtime_error(L"Term Index test failed");
        } else if (segInfoStat->storedFieldStatus->error != nullptr) {
          throw runtime_error(L"Stored Field test failed");
        } else if (segInfoStat->termVectorStatus->error != nullptr) {
          throw runtime_error(L"Term Vector test failed");
        } else if (segInfoStat->docValuesStatus->error != nullptr) {
          throw runtime_error(L"DocValues test failed");
        } else if (segInfoStat->pointsStatus->error != nullptr) {
          throw runtime_error(L"Points test failed");
        }
      }
      const wstring softDeletesField =
          reader->getFieldInfos()->getSoftDeletesField();
      if (softDeletesField != L"") {
        checkSoftDeletes(softDeletesField, info, reader, infoStream, failFast);
      }
      msg(infoStream, L"");

      if (verbose) {
        msg(infoStream, L"detailed segment RAM usage: ");
        // C++ TODO: There is no native C++ equivalent to 'toString':
        msg(infoStream, Accountables::toString(reader));
      }

    } catch (const runtime_error &t) {
      if (failFast) {
        throw IOUtils::rethrowAlways(t);
      }
      msg(infoStream, L"FAILED");
      wstring comment;
      comment = L"exorciseIndex() would remove reference to this segment";
      msg(infoStream, L"    WARNING: " + comment + L"; full exception:");
      if (infoStream != nullptr) {
        t.printStackTrace(infoStream);
      }
      msg(infoStream, L"");
      result->totLoseDocCount += toLoseDocCount;
      result->numBadSegments++;
      continue;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (reader != nullptr) {
        delete reader;
      }
    }

    // Keeper
    result->newSegments->push_back(info->clone());
  }

  if (0 == result->numBadSegments) {
    result->clean = true;
  } else {
    msg(infoStream, L"WARNING: " + to_wstring(result->numBadSegments) +
                        L" broken segments (containing " +
                        to_wstring(result->totLoseDocCount) +
                        L" documents) detected");
  }

  if (!(result->validCounter = (result->maxSegmentName < sis->counter))) {
    result->clean = false;
    result->newSegments->counter = result->maxSegmentName + 1;
    msg(infoStream, L"ERROR: Next segment name counter " +
                        to_wstring(sis->counter) +
                        L" is not greater than max segment name " +
                        to_wstring(result->maxSegmentName));
  }

  if (result->clean) {
    msg(infoStream, L"No problems were detected with this index.\n");
  }

  msg(infoStream, wstring::format(Locale::ROOT, L"Took %.3f sec total.",
                                  nsToSec(System::nanoTime() - startNS)));

  return result;
}

shared_ptr<Status::IndexSortStatus>
CheckIndex::testSort(shared_ptr<CodecReader> reader, shared_ptr<Sort> sort,
                     shared_ptr<PrintStream> infoStream,
                     bool failFast) 
{
  // This segment claims its documents are sorted according to the incoming sort
  // ... let's make sure:

  int64_t startNS = System::nanoTime();

  shared_ptr<Status::IndexSortStatus> status =
      make_shared<Status::IndexSortStatus>();

  if (sort != nullptr) {
    if (infoStream != nullptr) {
      infoStream->print(L"    test: index sort..........");
    }

    std::deque<std::shared_ptr<SortField>> fields = sort->getSort();
    const std::deque<int> reverseMul = std::deque<int>(fields.size());
    std::deque<std::shared_ptr<LeafFieldComparator>> comparators(
        fields.size());

    shared_ptr<LeafReaderContext> readerContext =
        make_shared<LeafReaderContext>(reader);

    for (int i = 0; i < fields.size(); i++) {
      reverseMul[i] = fields[i]->getReverse() ? -1 : 1;
      comparators[i] =
          fields[i]->getComparator(1, i).getLeafComparator(readerContext);
    }

    int maxDoc = reader->maxDoc();

    try {

      for (int docID = 1; docID < maxDoc; docID++) {

        int cmp = 0;

        for (int i = 0; i < comparators.size(); i++) {
          // TODO: would be better if copy() didnt cause a term lookup in
          // TermOrdVal & co, the segments are always the same here...
          comparators[i]->copy(0, docID - 1);
          comparators[i]->setBottom(0);
          cmp = reverseMul[i] * comparators[i]->compareBottom(docID);
          if (cmp != 0) {
            break;
          }
        }

        if (cmp > 0) {
          throw runtime_error(L"segment has indexSort=" + sort +
                              L" but docID=" + to_wstring(docID - 1) +
                              L" sorts after docID=" + to_wstring(docID));
        }
      }
      msg(infoStream, wstring::format(Locale::ROOT, L"OK [took %.3f sec]",
                                      nsToSec(System::nanoTime() - startNS)));
    } catch (const runtime_error &e) {
      if (failFast) {
        throw IOUtils::rethrowAlways(e);
      }
      msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
      status->error = e;
      if (infoStream != nullptr) {
        e.printStackTrace(infoStream);
      }
    }
  }

  return status;
}

shared_ptr<Status::LiveDocStatus>
CheckIndex::testLiveDocs(shared_ptr<CodecReader> reader,
                         shared_ptr<PrintStream> infoStream,
                         bool failFast) 
{
  int64_t startNS = System::nanoTime();
  shared_ptr<Status::LiveDocStatus> *const status =
      make_shared<Status::LiveDocStatus>();

  try {
    if (infoStream != nullptr) {
      infoStream->print(L"    test: check live docs.....");
    }
    constexpr int numDocs = reader->numDocs();
    if (reader->hasDeletions()) {
      shared_ptr<Bits> liveDocs = reader->getLiveDocs();
      if (liveDocs == nullptr) {
        throw runtime_error(
            L"segment should have deletions, but liveDocs is null");
      } else {
        int numLive = 0;
        for (int j = 0; j < liveDocs->length(); j++) {
          if (liveDocs->get(j)) {
            numLive++;
          }
        }
        if (numLive != numDocs) {
          throw runtime_error(L"liveDocs count mismatch: info=" +
                              to_wstring(numDocs) + L", vs bits=" +
                              to_wstring(numLive));
        }
      }

      status->numDeleted = reader->numDeletedDocs();
      msg(infoStream,
          wstring::format(Locale::ROOT, L"OK [%d deleted docs] [took %.3f sec]",
                          status->numDeleted,
                          nsToSec(System::nanoTime() - startNS)));
    } else {
      shared_ptr<Bits> liveDocs = reader->getLiveDocs();
      if (liveDocs != nullptr) {
        // it's ok for it to be non-null here, as long as none are set right?
        for (int j = 0; j < liveDocs->length(); j++) {
          if (!liveDocs->get(j)) {
            throw runtime_error(
                L"liveDocs mismatch: info says no deletions but doc " +
                to_wstring(j) + L" is deleted.");
          }
        }
      }
      msg(infoStream, wstring::format(Locale::ROOT, L"OK [took %.3f sec]",
                                      (nsToSec(System::nanoTime() - startNS))));
    }

  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

shared_ptr<Status::FieldInfoStatus>
CheckIndex::testFieldInfos(shared_ptr<CodecReader> reader,
                           shared_ptr<PrintStream> infoStream,
                           bool failFast) 
{
  int64_t startNS = System::nanoTime();
  shared_ptr<Status::FieldInfoStatus> *const status =
      make_shared<Status::FieldInfoStatus>();

  try {
    // Test Field Infos
    if (infoStream != nullptr) {
      infoStream->print(L"    test: field infos.........");
    }
    shared_ptr<FieldInfos> fieldInfos = reader->getFieldInfos();
    for (auto f : fieldInfos) {
      f->checkConsistency();
    }
    msg(infoStream,
        wstring::format(Locale::ROOT, L"OK [%d fields] [took %.3f sec]",
                        fieldInfos->size(),
                        nsToSec(System::nanoTime() - startNS)));
    status->totFields = fieldInfos->size();
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

shared_ptr<Status::FieldNormStatus>
CheckIndex::testFieldNorms(shared_ptr<CodecReader> reader,
                           shared_ptr<PrintStream> infoStream,
                           bool failFast) 
{
  int64_t startNS = System::nanoTime();
  shared_ptr<Status::FieldNormStatus> *const status =
      make_shared<Status::FieldNormStatus>();

  try {
    // Test Field Norms
    if (infoStream != nullptr) {
      infoStream->print(L"    test: field norms.........");
    }
    shared_ptr<NormsProducer> normsReader = reader->getNormsReader();
    if (normsReader != nullptr) {
      normsReader = normsReader->getMergeInstance();
    }
    for (auto info : reader->getFieldInfos()) {
      if (info->hasNorms()) {
        checkNumericDocValues(info->name, normsReader->getNorms(info));
        ++status->totFields;
      }
    }

    msg(infoStream,
        wstring::format(Locale::ROOT, L"OK [%d fields] [took %.3f sec]",
                        status->totFields,
                        nsToSec(System::nanoTime() - startNS)));
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

int64_t CheckIndex::getDocsFromTermRange(const wstring &field, int maxDoc,
                                           shared_ptr<TermsEnum> termsEnum,
                                           shared_ptr<FixedBitSet> docsSeen,
                                           shared_ptr<BytesRef> minTerm,
                                           shared_ptr<BytesRef> maxTerm,
                                           bool isIntersect) 
{
  docsSeen->clear(0, docsSeen->length());

  int64_t termCount = 0;
  shared_ptr<PostingsEnum> postingsEnum = nullptr;
  shared_ptr<BytesRefBuilder> lastTerm = nullptr;
  while (true) {
    shared_ptr<BytesRef> term;

    // Kinda messy: for intersect, we must first next(), but for "normal", we
    // are already on our first term:
    if (isIntersect || termCount != 0) {
      term = termsEnum->next();
    } else {
      term = termsEnum->term();
    }

    if (term == nullptr) {
      if (isIntersect == false) {
        throw runtime_error(L"didn't see max term field=" + field + L" term=" +
                            maxTerm);
      }
      // System.out.println("      terms=" + termCount);
      return termCount;
    }

    assert(term->isValid());

    if (lastTerm == nullptr) {
      lastTerm = make_shared<BytesRefBuilder>();
      lastTerm->copyBytes(term);
    } else {
      if (lastTerm->get()->compareTo(term) >= 0) {
        throw runtime_error(L"terms out of order: lastTerm=" + lastTerm->get() +
                            L" term=" + term);
      }
      lastTerm->copyBytes(term);
    }

    // System.out.println("    term=" + term);

    // Caller already ensured terms enum positioned >= minTerm:
    if (term->compareTo(minTerm) < 0) {
      throw runtime_error(L"saw term before min term field=" + field +
                          L" term=" + minTerm);
    }

    if (isIntersect == false) {
      int cmp = term->compareTo(maxTerm);
      if (cmp == 0) {
        // Done!
        // System.out.println("      terms=" + termCount);
        return termCount;
      } else if (cmp > 0) {
        throw runtime_error(L"didn't see end term field=" + field + L" term=" +
                            maxTerm);
      }
    }

    postingsEnum = termsEnum->postings(postingsEnum, 0);

    int lastDoc = -1;
    while (true) {
      int doc = postingsEnum->nextDoc();
      if (doc == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      if (doc <= lastDoc) {
        throw runtime_error(L"term " + term + L": doc " + to_wstring(doc) +
                            L" <= lastDoc " + to_wstring(lastDoc));
      }
      if (doc >= maxDoc) {
        throw runtime_error(L"term " + term + L": doc " + to_wstring(doc) +
                            L" >= maxDoc " + to_wstring(maxDoc));
      }

      // System.out.println("      doc=" + doc);
      docsSeen->set(doc);

      lastDoc = doc;
    }

    termCount++;
  }
}

bool CheckIndex::checkSingleTermRange(
    const wstring &field, int maxDoc, shared_ptr<Terms> terms,
    shared_ptr<BytesRef> minTerm, shared_ptr<BytesRef> maxTerm,
    shared_ptr<FixedBitSet> normalDocs,
    shared_ptr<FixedBitSet> intersectDocs) 
{
  // System.out.println("    check minTerm=" + minTerm.utf8ToString() + "
  // maxTerm=" + maxTerm.utf8ToString());
  assert(minTerm->compareTo(maxTerm) <= 0);

  shared_ptr<TermsEnum> termsEnum = terms->begin();
  TermsEnum::SeekStatus status = termsEnum->seekCeil(minTerm);
  if (status != TermsEnum::SeekStatus::FOUND) {
    throw runtime_error(L"failed to seek to existing term field=" + field +
                        L" term=" + minTerm);
  }

  // Do "dumb" iteration to visit all terms in the range:
  int64_t normalTermCount = getDocsFromTermRange(
      field, maxDoc, termsEnum, normalDocs, minTerm, maxTerm, false);

  // Now do the same operation using intersect:
  int64_t intersectTermCount = getDocsFromTermRange(
      field, maxDoc,
      terms->intersect(
          make_shared<CompiledAutomaton>(
              Automata::makeBinaryInterval(minTerm, true, maxTerm, false), true,
              false, numeric_limits<int>::max(), true),
          nullptr),
      intersectDocs, minTerm, maxTerm, true);

  if (intersectTermCount > normalTermCount) {
    throw runtime_error(L"intersect returned too many terms: field=" + field +
                        L" intersectTermCount=" +
                        to_wstring(intersectTermCount) + L" normalTermCount=" +
                        to_wstring(normalTermCount));
  }

  if (normalDocs->equals(intersectDocs) == false) {
    throw runtime_error(
        L"intersect visited different docs than straight terms enum: " +
        to_wstring(normalDocs->cardinality()) + L" for straight enum, vs " +
        to_wstring(intersectDocs->cardinality()) + L" for intersect, minTerm=" +
        minTerm + L" maxTerm=" + maxTerm);
  }
  // System.out.println("      docs=" + normalTermCount);
  // System.out.println("    " + intersectTermCount + " vs " + normalTermCount);
  return intersectTermCount != normalTermCount;
}

void CheckIndex::checkTermRanges(const wstring &field, int maxDoc,
                                 shared_ptr<Terms> terms,
                                 int64_t numTerms) 
{

  // We'll target this many terms in our interval for the current level:
  double currentInterval = numTerms;

  shared_ptr<FixedBitSet> normalDocs = make_shared<FixedBitSet>(maxDoc);
  shared_ptr<FixedBitSet> intersectDocs = make_shared<FixedBitSet>(maxDoc);

  // System.out.println("CI.checkTermRanges field=" + field + " numTerms=" +
  // numTerms);

  while (currentInterval >= 10.0) {
    // System.out.println("  cycle interval=" + currentInterval);

    // We iterate this terms enum to locate min/max term for each
    // sliding/overlapping interval we test at the current level:
    shared_ptr<TermsEnum> termsEnum = terms->begin();

    int64_t termCount = 0;

    shared_ptr<Deque<std::shared_ptr<BytesRef>>> termBounds =
        deque<std::shared_ptr<BytesRef>>();

    int64_t lastTermAdded = numeric_limits<int64_t>::min();

    shared_ptr<BytesRefBuilder> lastTerm = nullptr;

    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      // System.out.println("  top: term=" + term.utf8ToString());
      if (termCount >= lastTermAdded + currentInterval / 4) {
        termBounds->add(BytesRef::deepCopyOf(term));
        lastTermAdded = termCount;
        if (termBounds->size() == 5) {
          shared_ptr<BytesRef> minTerm = termBounds->removeFirst();
          shared_ptr<BytesRef> maxTerm = termBounds->getLast();
          checkSingleTermRange(field, maxDoc, terms, minTerm, maxTerm,
                               normalDocs, intersectDocs);
        }
      }
      termCount++;

      if (lastTerm == nullptr) {
        lastTerm = make_shared<BytesRefBuilder>();
        lastTerm->copyBytes(term);
      } else {
        if (lastTerm->get()->compareTo(term) >= 0) {
          throw runtime_error(L"terms out of order: lastTerm=" +
                              lastTerm->get() + L" term=" + term);
        }
        lastTerm->copyBytes(term);
      }
    }
    // System.out.println("    count=" + termCount);

    if (lastTerm != nullptr && termBounds->isEmpty() == false) {
      shared_ptr<BytesRef> minTerm = termBounds->removeFirst();
      shared_ptr<BytesRef> maxTerm = lastTerm->get();
      checkSingleTermRange(field, maxDoc, terms, minTerm, maxTerm, normalDocs,
                           intersectDocs);
    }

    currentInterval *= .75;
  }
}

shared_ptr<Status::TermIndexStatus>
CheckIndex::checkFields(shared_ptr<Fields> fields, shared_ptr<Bits> liveDocs,
                        int maxDoc, shared_ptr<FieldInfos> fieldInfos,
                        bool doPrint, bool isVectors,
                        shared_ptr<PrintStream> infoStream, bool verbose,
                        shared_ptr<Version> version) 
{
  // TODO: we should probably return our own stats thing...?!
  int64_t startNS;
  if (doPrint) {
    startNS = System::nanoTime();
  } else {
    startNS = 0;
  }

  shared_ptr<Status::TermIndexStatus> *const status =
      make_shared<Status::TermIndexStatus>();
  int computedFieldCount = 0;

  shared_ptr<PostingsEnum> postings = nullptr;

  wstring lastField = L"";
  for (auto field : fields) {

    // MultiFieldsEnum relies upon this order...
    if (lastField != L"" && field.compareTo(lastField) <= 0) {
      throw runtime_error(L"fields out of order: lastField=" + lastField +
                          L" field=" + field);
    }
    lastField = field;

    // check that the field is in fieldinfos, and is indexed.
    // TODO: add a separate test to check this for different reader impls
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(field);
    if (fieldInfo == nullptr) {
      throw runtime_error(
          L"fieldsEnum inconsistent with fieldInfos, no fieldInfos for: " +
          field);
    }
    if (fieldInfo->getIndexOptions() == IndexOptions::NONE) {
      throw runtime_error(
          L"fieldsEnum inconsistent with fieldInfos, isIndexed == false for: " +
          field);
    }

    // TODO: really the codec should not return a field
    // from FieldsEnum if it has no Terms... but we do
    // this today:
    // assert fields.terms(field) != null;
    computedFieldCount++;

    shared_ptr<Terms> *const terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }

    constexpr bool hasFreqs = terms->hasFreqs();
    constexpr bool hasPositions = terms->hasPositions();
    constexpr bool hasPayloads = terms->hasPayloads();
    constexpr bool hasOffsets = terms->hasOffsets();

    shared_ptr<BytesRef> maxTerm;
    shared_ptr<BytesRef> minTerm;
    if (isVectors) {
      // Term vectors impls can be very slow for getMax
      maxTerm.reset();
      minTerm.reset();
    } else {
      shared_ptr<BytesRef> bb = terms->getMin();
      if (bb != nullptr) {
        assert(bb->isValid());
        minTerm = BytesRef::deepCopyOf(bb);
      } else {
        minTerm.reset();
      }

      bb = terms->getMax();
      if (bb != nullptr) {
        assert(bb->isValid());
        maxTerm = BytesRef::deepCopyOf(bb);
        if (minTerm == nullptr) {
          throw runtime_error(L"field \"" + field +
                              L"\" has null minTerm but non-null maxTerm");
        }
      } else {
        maxTerm.reset();
        if (minTerm != nullptr) {
          throw runtime_error(L"field \"" + field +
                              L"\" has non-null minTerm but null maxTerm");
        }
      }
    }

    // term vectors cannot omit TF:
    constexpr bool expectedHasFreqs =
        (isVectors || fieldInfo->getIndexOptions().compareTo(
                          IndexOptions::DOCS_AND_FREQS) >= 0);

    if (hasFreqs != expectedHasFreqs) {
      throw runtime_error(L"field \"" + field + L"\" should have hasFreqs=" +
                          StringHelper::toString(expectedHasFreqs) +
                          L" but got " + StringHelper::toString(hasFreqs));
    }

    if (hasFreqs == false) {
      if (terms->getSumTotalTermFreq() != -1) {
        throw runtime_error(
            L"field \"" + field +
            L"\" hasFreqs is false, but Terms.getSumTotalTermFreq()=" +
            to_wstring(terms->getSumTotalTermFreq()) + L" (should be -1)");
      }
    }

    if (!isVectors) {
      constexpr bool expectedHasPositions =
          fieldInfo->getIndexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
      if (hasPositions != expectedHasPositions) {
        throw runtime_error(
            L"field \"" + field + L"\" should have hasPositions=" +
            StringHelper::toString(expectedHasPositions) + L" but got " +
            StringHelper::toString(hasPositions));
      }

      constexpr bool expectedHasPayloads = fieldInfo->hasPayloads();
      if (hasPayloads != expectedHasPayloads) {
        throw runtime_error(L"field \"" + field +
                            L"\" should have hasPayloads=" +
                            StringHelper::toString(expectedHasPayloads) +
                            L" but got " + StringHelper::toString(hasPayloads));
      }

      constexpr bool expectedHasOffsets =
          fieldInfo->getIndexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
      if (hasOffsets != expectedHasOffsets) {
        throw runtime_error(L"field \"" + field +
                            L"\" should have hasOffsets=" +
                            StringHelper::toString(expectedHasOffsets) +
                            L" but got " + StringHelper::toString(hasOffsets));
      }
    }

    shared_ptr<TermsEnum> *const termsEnum = terms->begin();

    bool hasOrd = true;
    constexpr int64_t termCountStart =
        status->delTermCount + status->termCount;

    shared_ptr<BytesRefBuilder> lastTerm = nullptr;

    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    shared_ptr<FixedBitSet> visitedDocs = make_shared<FixedBitSet>(maxDoc);
    while (true) {

      shared_ptr<BytesRef> *const term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      // System.out.println("CI: field=" + field + " check term=" + term + "
      // docFreq=" + termsEnum.docFreq());

      assert(term->isValid());

      // make sure terms arrive in order according to
      // the comp
      if (lastTerm == nullptr) {
        lastTerm = make_shared<BytesRefBuilder>();
        lastTerm->copyBytes(term);
      } else {
        if (lastTerm->get()->compareTo(term) >= 0) {
          throw runtime_error(L"terms out of order: lastTerm=" +
                              lastTerm->get() + L" term=" + term);
        }
        lastTerm->copyBytes(term);
      }

      if (isVectors == false) {
        if (minTerm == nullptr) {
          // We checked this above:
          assert(maxTerm == nullptr);
          throw runtime_error(L"field=\"" + field + L"\": invalid term: term=" +
                              term + L", minTerm=" + minTerm);
        }

        if (term->compareTo(minTerm) < 0) {
          throw runtime_error(L"field=\"" + field + L"\": invalid term: term=" +
                              term + L", minTerm=" + minTerm);
        }

        if (term->compareTo(maxTerm) > 0) {
          throw runtime_error(L"field=\"" + field + L"\": invalid term: term=" +
                              term + L", maxTerm=" + maxTerm);
        }
      }

      constexpr int docFreq = termsEnum->docFreq();
      if (docFreq <= 0) {
        throw runtime_error(L"docfreq: " + to_wstring(docFreq) +
                            L" is out of bounds");
      }
      sumDocFreq += docFreq;

      postings = termsEnum->postings(postings, PostingsEnum::ALL);

      if (hasFreqs == false) {
        if (termsEnum->totalTermFreq() != -1) {
          throw runtime_error(
              L"field \"" + field +
              L"\" hasFreqs is false, but TermsEnum.totalTermFreq()=" +
              to_wstring(termsEnum->totalTermFreq()) + L" (should be -1)");
        }
      }

      if (hasOrd) {
        int64_t ord = -1;
        try {
          ord = termsEnum->ord();
        } catch (const UnsupportedOperationException &uoe) {
          hasOrd = false;
        }

        if (hasOrd) {
          constexpr int64_t ordExpected =
              status->delTermCount + status->termCount - termCountStart;
          if (ord != ordExpected) {
            throw runtime_error(L"ord mismatch: TermsEnum has ord=" +
                                to_wstring(ord) + L" vs actual=" +
                                to_wstring(ordExpected));
          }
        }
      }

      int lastDoc = -1;
      int docCount = 0;
      bool hasNonDeletedDocs = false;
      int64_t totalTermFreq = 0;
      while (true) {
        constexpr int doc = postings->nextDoc();
        if (doc == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        visitedDocs->set(doc);
        int freq = -1;
        if (hasFreqs) {
          freq = postings->freq();
          if (freq <= 0) {
            throw runtime_error(L"term " + term + L": doc " + to_wstring(doc) +
                                L": freq " + to_wstring(freq) +
                                L" is out of bounds");
          }
          totalTermFreq += freq;
        } else {
          // When a field didn't index freq, it must
          // consistently "lie" and pretend that freq was
          // 1:
          if (postings->freq() != 1) {
            throw runtime_error(L"term " + term + L": doc " + to_wstring(doc) +
                                L": freq " + to_wstring(freq) +
                                L" != 1 when Terms.hasFreqs() is false");
          }
        }
        if (liveDocs == nullptr || liveDocs->get(doc)) {
          hasNonDeletedDocs = true;
          status->totFreq++;
          if (freq >= 0) {
            status->totPos += freq;
          }
        }
        docCount++;

        if (doc <= lastDoc) {
          throw runtime_error(L"term " + term + L": doc " + to_wstring(doc) +
                              L" <= lastDoc " + to_wstring(lastDoc));
        }
        if (doc >= maxDoc) {
          throw runtime_error(L"term " + term + L": doc " + to_wstring(doc) +
                              L" >= maxDoc " + to_wstring(maxDoc));
        }

        lastDoc = doc;

        int lastPos = -1;
        int lastOffset = 0;
        if (hasPositions) {
          for (int j = 0; j < freq; j++) {
            constexpr int pos = postings->nextPosition();

            if (pos < 0) {
              throw runtime_error(L"term " + term + L": doc " +
                                  to_wstring(doc) + L": pos " +
                                  to_wstring(pos) + L" is out of bounds");
            }
            if (pos > IndexWriter::MAX_POSITION) {
              throw runtime_error(
                  L"term " + term + L": doc " + to_wstring(doc) + L": pos " +
                  to_wstring(pos) + L" > IndexWriter.MAX_POSITION=" +
                  to_wstring(IndexWriter::MAX_POSITION));
            }
            if (pos < lastPos) {
              throw runtime_error(
                  L"term " + term + L": doc " + to_wstring(doc) + L": pos " +
                  to_wstring(pos) + L" < lastPos " + to_wstring(lastPos));
            }
            lastPos = pos;
            shared_ptr<BytesRef> payload = postings->getPayload();
            if (payload != nullptr) {
              assert(payload->isValid());
            }
            if (payload != nullptr && payload->length < 1) {
              throw runtime_error(
                  L"term " + term + L": doc " + to_wstring(doc) + L": pos " +
                  to_wstring(pos) + L" payload length is out of bounds " +
                  to_wstring(payload->length));
            }
            if (hasOffsets) {
              int startOffset = postings->startOffset();
              int endOffset = postings->endOffset();
              // In Lucene 7 we fixed IndexWriter to also enforce term deque
              // offsets
              if (isVectors == false ||
                  version->onOrAfter(Version::LUCENE_7_0_0)) {
                if (startOffset < 0) {
                  throw runtime_error(
                      L"term " + term + L": doc " + to_wstring(doc) +
                      L": pos " + to_wstring(pos) + L": startOffset " +
                      to_wstring(startOffset) + L" is out of bounds");
                }
                if (startOffset < lastOffset) {
                  throw runtime_error(
                      L"term " + term + L": doc " + to_wstring(doc) +
                      L": pos " + to_wstring(pos) + L": startOffset " +
                      to_wstring(startOffset) + L" < lastStartOffset " +
                      to_wstring(lastOffset) +
                      L"; consider using the FixBrokenOffsets tool in Lucene's "
                      L"backward-codecs module to correct your index");
                }
                if (endOffset < 0) {
                  throw runtime_error(
                      L"term " + term + L": doc " + to_wstring(doc) +
                      L": pos " + to_wstring(pos) + L": endOffset " +
                      to_wstring(endOffset) + L" is out of bounds");
                }
                if (endOffset < startOffset) {
                  throw runtime_error(
                      L"term " + term + L": doc " + to_wstring(doc) +
                      L": pos " + to_wstring(pos) + L": endOffset " +
                      to_wstring(endOffset) + L" < startOffset " +
                      to_wstring(startOffset));
                }
              }
              lastOffset = startOffset;
            }
          }
        }
      }

      if (hasNonDeletedDocs) {
        status->termCount++;
      } else {
        status->delTermCount++;
      }

      constexpr int64_t totalTermFreq2 = termsEnum->totalTermFreq();
      constexpr bool hasTotalTermFreq = hasFreqs && totalTermFreq2 != -1;

      if (docCount != docFreq) {
        throw runtime_error(
            L"term " + term + L" docFreq=" + to_wstring(docFreq) +
            L" != tot docs w/o deletions " + to_wstring(docCount));
      }
      if (hasTotalTermFreq) {
        if (totalTermFreq2 <= 0) {
          throw runtime_error(L"totalTermFreq: " + to_wstring(totalTermFreq2) +
                              L" is out of bounds");
        }
        sumTotalTermFreq += totalTermFreq;
        if (totalTermFreq != totalTermFreq2) {
          throw runtime_error(L"term " + term + L" totalTermFreq=" +
                              to_wstring(totalTermFreq2) +
                              L" != recomputed totalTermFreq=" +
                              to_wstring(totalTermFreq));
        }
      }

      // Test skipping
      if (hasPositions) {
        for (int idx = 0; idx < 7; idx++) {
          constexpr int skipDocID = static_cast<int>(
              ((idx + 1) * static_cast<int64_t>(maxDoc)) / 8);
          postings = termsEnum->postings(postings, PostingsEnum::ALL);
          constexpr int docID = postings->advance(skipDocID);
          if (docID == DocIdSetIterator::NO_MORE_DOCS) {
            break;
          } else {
            if (docID < skipDocID) {
              throw runtime_error(L"term " + term + L": advance(docID=" +
                                  to_wstring(skipDocID) + L") returned docID=" +
                                  to_wstring(docID));
            }
            constexpr int freq = postings->freq();
            if (freq <= 0) {
              throw runtime_error(L"termFreq " + to_wstring(freq) +
                                  L" is out of bounds");
            }
            int lastPosition = -1;
            int lastOffset = 0;
            for (int posUpto = 0; posUpto < freq; posUpto++) {
              constexpr int pos = postings->nextPosition();

              if (pos < 0) {
                throw runtime_error(L"position " + to_wstring(pos) +
                                    L" is out of bounds");
              }
              if (pos < lastPosition) {
                throw runtime_error(L"position " + to_wstring(pos) +
                                    L" is < lastPosition " +
                                    to_wstring(lastPosition));
              }
              lastPosition = pos;
              if (hasOffsets) {
                int startOffset = postings->startOffset();
                int endOffset = postings->endOffset();
                // NOTE: we cannot enforce any bounds whatsoever on vectors...
                // they were a free-for-all before? but for offsets in the
                // postings lists these checks are fine: they were always
                // enforced by IndexWriter
                if (!isVectors) {
                  if (startOffset < 0) {
                    throw runtime_error(
                        L"term " + term + L": doc " + to_wstring(docID) +
                        L": pos " + to_wstring(pos) + L": startOffset " +
                        to_wstring(startOffset) + L" is out of bounds");
                  }
                  if (startOffset < lastOffset) {
                    throw runtime_error(
                        L"term " + term + L": doc " + to_wstring(docID) +
                        L": pos " + to_wstring(pos) + L": startOffset " +
                        to_wstring(startOffset) + L" < lastStartOffset " +
                        to_wstring(lastOffset));
                  }
                  if (endOffset < 0) {
                    throw runtime_error(
                        L"term " + term + L": doc " + to_wstring(docID) +
                        L": pos " + to_wstring(pos) + L": endOffset " +
                        to_wstring(endOffset) + L" is out of bounds");
                  }
                  if (endOffset < startOffset) {
                    throw runtime_error(
                        L"term " + term + L": doc " + to_wstring(docID) +
                        L": pos " + to_wstring(pos) + L": endOffset " +
                        to_wstring(endOffset) + L" < startOffset " +
                        to_wstring(startOffset));
                  }
                }
                lastOffset = startOffset;
              }
            }

            constexpr int nextDocID = postings->nextDoc();
            if (nextDocID == DocIdSetIterator::NO_MORE_DOCS) {
              break;
            }
            if (nextDocID <= docID) {
              throw runtime_error(L"term " + term + L": advance(docID=" +
                                  to_wstring(skipDocID) +
                                  L"), then .next() returned docID=" +
                                  to_wstring(nextDocID) + L" vs prev docID=" +
                                  to_wstring(docID));
            }
          }

          if (isVectors) {
            // Only 1 doc in the postings for term vectors, so we only test 1
            // advance:
            break;
          }
        }
      } else {
        for (int idx = 0; idx < 7; idx++) {
          constexpr int skipDocID = static_cast<int>(
              ((idx + 1) * static_cast<int64_t>(maxDoc)) / 8);
          postings = termsEnum->postings(postings, PostingsEnum::NONE);
          constexpr int docID = postings->advance(skipDocID);
          if (docID == DocIdSetIterator::NO_MORE_DOCS) {
            break;
          } else {
            if (docID < skipDocID) {
              throw runtime_error(L"term " + term + L": advance(docID=" +
                                  to_wstring(skipDocID) + L") returned docID=" +
                                  to_wstring(docID));
            }
            constexpr int nextDocID = postings->nextDoc();
            if (nextDocID == DocIdSetIterator::NO_MORE_DOCS) {
              break;
            }
            if (nextDocID <= docID) {
              throw runtime_error(L"term " + term + L": advance(docID=" +
                                  to_wstring(skipDocID) +
                                  L"), then .next() returned docID=" +
                                  to_wstring(nextDocID) + L" vs prev docID=" +
                                  to_wstring(docID));
            }
          }
          if (isVectors) {
            // Only 1 doc in the postings for term vectors, so we only test 1
            // advance:
            break;
          }
        }
      }
    }

    if (minTerm != nullptr && status->termCount + status->delTermCount == 0) {
      throw runtime_error(L"field=\"" + field +
                          L"\": minTerm is non-null yet we saw no terms: " +
                          minTerm);
    }

    shared_ptr<Terms> *const fieldTerms = fields->terms(field);
    if (fieldTerms == nullptr) {
      // Unusual: the FieldsEnum returned a field but
      // the Terms for that field is null; this should
      // only happen if it's a ghost field (field with
      // no terms, eg there used to be terms but all
      // docs got deleted and then merged away):

    } else {

      int64_t fieldTermCount =
          (status->delTermCount + status->termCount) - termCountStart;

      // LUCENE-5879: this is just too slow for now:
      if (false && hasFreqs == false) {
        // For DOCS_ONLY fields we recursively test term ranges:
        checkTermRanges(field, maxDoc, fieldTerms, fieldTermCount);
      }

      constexpr any stats = fieldTerms->getStats();
      assert(stats != nullptr);
      if (status->blockTreeStats.empty()) {
        status->blockTreeStats = unordered_map<>();
      }
      status->blockTreeStats.emplace(field, stats);

      if (sumTotalTermFreq != 0) {
        constexpr int64_t v = fields->terms(field)->getSumTotalTermFreq();
        if (v != -1 && sumTotalTermFreq != v) {
          throw runtime_error(L"sumTotalTermFreq for field " + field + L"=" +
                              to_wstring(v) +
                              L" != recomputed sumTotalTermFreq=" +
                              to_wstring(sumTotalTermFreq));
        }
      }

      if (sumDocFreq != 0) {
        constexpr int64_t v = fields->terms(field)->getSumDocFreq();
        if (v != -1 && sumDocFreq != v) {
          throw runtime_error(L"sumDocFreq for field " + field + L"=" +
                              to_wstring(v) + L" != recomputed sumDocFreq=" +
                              to_wstring(sumDocFreq));
        }
      }

      constexpr int v = fieldTerms->getDocCount();
      if (v != -1 && visitedDocs->cardinality() != v) {
        throw runtime_error(L"docCount for field " + field + L"=" +
                            to_wstring(v) + L" != recomputed docCount=" +
                            to_wstring(visitedDocs->cardinality()));
      }

      // Test seek to last term:
      if (lastTerm != nullptr) {
        if (termsEnum->seekCeil(lastTerm->get()) !=
            TermsEnum::SeekStatus::FOUND) {
          throw runtime_error(L"seek to last term " + lastTerm->get() +
                              L" failed");
        }
        if (termsEnum->term()->equals(lastTerm->get()) == false) {
          throw runtime_error(L"seek to last term " + lastTerm->get() +
                              L" returned FOUND but seeked to the wrong term " +
                              termsEnum->term());
        }

        int expectedDocFreq = termsEnum->docFreq();
        shared_ptr<PostingsEnum> d =
            termsEnum->postings(nullptr, PostingsEnum::NONE);
        int docFreq = 0;
        while (d->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
          docFreq++;
        }
        if (docFreq != expectedDocFreq) {
          throw runtime_error(L"docFreq for last term " + lastTerm->get() +
                              L"=" + to_wstring(expectedDocFreq) +
                              L" != recomputed docFreq=" + to_wstring(docFreq));
        }
      }

      // check unique term count
      int64_t termCount = -1;

      if (fieldTermCount > 0) {
        termCount = fields->terms(field)->size();

        if (termCount != -1 && termCount != fieldTermCount) {
          throw runtime_error(L"termCount mismatch " + to_wstring(termCount) +
                              L" vs " + to_wstring(fieldTermCount));
        }
      }

      // Test seeking by ord
      if (hasOrd && status->termCount - termCountStart > 0) {
        int seekCount = static_cast<int>(min(10000LL, termCount));
        if (seekCount > 0) {
          std::deque<std::shared_ptr<BytesRef>> seekTerms(seekCount);

          // Seek by ord
          for (int i = seekCount - 1; i >= 0; i--) {
            int64_t ord = i * (termCount / seekCount);
            termsEnum->seekExact(ord);
            int64_t actualOrd = termsEnum->ord();
            if (actualOrd != ord) {
              throw runtime_error(L"seek to ord " + to_wstring(ord) +
                                  L" returned ord " + to_wstring(actualOrd));
            }
            seekTerms[i] = BytesRef::deepCopyOf(termsEnum->term());
          }

          // Seek by term
          for (int i = seekCount - 1; i >= 0; i--) {
            if (termsEnum->seekCeil(seekTerms[i]) !=
                TermsEnum::SeekStatus::FOUND) {
              throw runtime_error(L"seek to existing term " + seekTerms[i] +
                                  L" failed");
            }
            if (termsEnum->term()->equals(seekTerms[i]) == false) {
              throw runtime_error(
                  L"seek to existing term " + seekTerms[i] +
                  L" returned FOUND but seeked to the wrong term " +
                  termsEnum->term());
            }

            postings = termsEnum->postings(postings, PostingsEnum::NONE);
            if (postings == nullptr) {
              throw runtime_error(L"null DocsEnum from to existing term " +
                                  seekTerms[i]);
            }
          }
        }
      }
    }
  }

  int fieldCount = fields->size();

  if (fieldCount != -1) {
    if (fieldCount < 0) {
      throw runtime_error(L"invalid fieldCount: " + to_wstring(fieldCount));
    }
    if (fieldCount != computedFieldCount) {
      throw runtime_error(L"fieldCount mismatch " + to_wstring(fieldCount) +
                          L" vs recomputed field count " +
                          to_wstring(computedFieldCount));
    }
  }

  if (doPrint) {
    msg(infoStream,
        wstring::format(
            Locale::ROOT,
            L"OK [%d terms; %d terms/docs pairs; %d tokens] [took %.3f sec]",
            status->termCount, status->totFreq, status->totPos,
            nsToSec(System::nanoTime() - startNS)));
  }

  if (verbose && status->blockTreeStats.size() > 0 && infoStream != nullptr &&
      status->termCount > 0) {
    for (auto ent : status->blockTreeStats) {
      infoStream->println(L"      field \"" + ent.first + L"\":");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      infoStream->println(L"      " +
                          ent.second->toString()->replace(L"\n", L"\n      "));
    }
  }

  return status;
}

shared_ptr<Status::TermIndexStatus>
CheckIndex::testPostings(shared_ptr<CodecReader> reader,
                         shared_ptr<PrintStream> infoStream,
                         shared_ptr<Version> version) 
{
  return testPostings(reader, infoStream, false, false, version);
}

shared_ptr<Status::TermIndexStatus> CheckIndex::testPostings(
    shared_ptr<CodecReader> reader, shared_ptr<PrintStream> infoStream,
    bool verbose, bool failFast, shared_ptr<Version> version) 
{

  // TODO: we should go and verify term vectors match, if
  // crossCheckTermVectors is on...

  shared_ptr<Status::TermIndexStatus> status;
  constexpr int maxDoc = reader->maxDoc();

  try {
    if (infoStream != nullptr) {
      infoStream->print(L"    test: terms, freq, prox...");
    }

    shared_ptr<Fields> *const fields =
        reader->getPostingsReader()->getMergeInstance();
    shared_ptr<FieldInfos> *const fieldInfos = reader->getFieldInfos();
    status = checkFields(fields, reader->getLiveDocs(), maxDoc, fieldInfos,
                         true, false, infoStream, verbose, version);
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR: " + e);
    status = make_shared<Status::TermIndexStatus>();
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

shared_ptr<Status::PointsStatus>
CheckIndex::testPoints(shared_ptr<CodecReader> reader,
                       shared_ptr<PrintStream> infoStream,
                       bool failFast) 
{
  if (infoStream != nullptr) {
    infoStream->print(L"    test: points..............");
  }
  int64_t startNS = System::nanoTime();
  shared_ptr<FieldInfos> fieldInfos = reader->getFieldInfos();
  shared_ptr<Status::PointsStatus> status = make_shared<Status::PointsStatus>();
  try {

    if (fieldInfos->hasPointValues()) {
      shared_ptr<PointsReader> pointsReader = reader->getPointsReader();
      if (pointsReader == nullptr) {
        throw runtime_error(L"there are fields with points, but "
                            L"reader.getPointsReader() is null");
      }
      for (auto fieldInfo : fieldInfos) {
        if (fieldInfo->getPointDimensionCount() > 0) {
          shared_ptr<PointValues> values =
              pointsReader->getValues(fieldInfo->name);
          if (values == nullptr) {
            continue;
          }

          status->totalValueFields++;

          int64_t size = values->size();
          int docCount = values->getDocCount();

          constexpr int64_t crossCost = values->estimatePointCount(
              make_shared<ConstantRelationIntersectVisitor>(
                  Relation::CELL_CROSSES_QUERY));
          if (crossCost < size / 2) {
            throw runtime_error(L"estimatePointCount should return >= size/2 "
                                L"when all cells match");
          }
          constexpr int64_t insideCost = values->estimatePointCount(
              make_shared<ConstantRelationIntersectVisitor>(
                  Relation::CELL_INSIDE_QUERY));
          if (insideCost < size) {
            throw runtime_error(L"estimatePointCount should return >= size "
                                L"when all cells fully match");
          }
          constexpr int64_t outsideCost = values->estimatePointCount(
              make_shared<ConstantRelationIntersectVisitor>(
                  Relation::CELL_OUTSIDE_QUERY));
          if (outsideCost != 0) {
            throw runtime_error(
                L"estimatePointCount should return 0 when no cells match");
          }

          shared_ptr<VerifyPointsVisitor> visitor =
              make_shared<VerifyPointsVisitor>(fieldInfo->name,
                                               reader->maxDoc(), values);
          values->intersect(visitor);

          if (visitor->getPointCountSeen() != size) {
            throw runtime_error(L"point values for field \"" + fieldInfo->name +
                                L"\" claims to have size=" + to_wstring(size) +
                                L" points, but in fact has " +
                                to_wstring(visitor->getPointCountSeen()));
          }

          if (visitor->getDocCountSeen() != docCount) {
            throw runtime_error(L"point values for field \"" + fieldInfo->name +
                                L"\" claims to have docCount=" +
                                to_wstring(docCount) + L" but in fact has " +
                                to_wstring(visitor->getDocCountSeen()));
          }

          status->totalValuePoints += visitor->getPointCountSeen();
        }
      }
    }

    msg(infoStream,
        wstring::format(Locale::ROOT,
                        L"OK [%d fields, %d points] [took %.3f sec]",
                        status->totalValueFields, status->totalValuePoints,
                        nsToSec(System::nanoTime() - startNS)));

  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR: " + e);
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

CheckIndex::VerifyPointsVisitor::VerifyPointsVisitor(
    const wstring &fieldName, int maxDoc,
    shared_ptr<PointValues> values) 
    : maxDoc(maxDoc), docsSeen(make_shared<FixedBitSet>(maxDoc)),
      lastMinPackedValue(std::deque<char>(packedBytesCount)),
      lastMaxPackedValue(std::deque<char>(packedBytesCount)),
      lastPackedValue(std::deque<char>(packedBytesCount)),
      globalMinPackedValue(values->getMinPackedValue()),
      globalMaxPackedValue(values->getMaxPackedValue()),
      packedBytesCount(numDims * bytesPerDim),
      numDims(values->getNumDimensions()),
      bytesPerDim(values->getBytesPerDimension()), fieldName(fieldName)
{

  if (values->getDocCount() > values->size()) {
    throw runtime_error(
        L"point values for field \"" + fieldName + L"\" claims to have size=" +
        to_wstring(values->size()) + L" points and inconsistent docCount=" +
        to_wstring(values->getDocCount()));
  }

  if (values->getDocCount() > maxDoc) {
    throw runtime_error(
        L"point values for field \"" + fieldName +
        L"\" claims to have docCount=" + to_wstring(values->getDocCount()) +
        L" but that's greater than maxDoc=" + to_wstring(maxDoc));
  }

  if (globalMinPackedValue.empty()) {
    if (values->size() != 0) {
      throw runtime_error(L"getMinPackedValue is null points for field \"" +
                          fieldName + L"\" yet size=" +
                          to_wstring(values->size()));
    }
  } else if (globalMinPackedValue.size() != packedBytesCount) {
    throw runtime_error(L"getMinPackedValue for field \"" + fieldName +
                        L"\" return length=" + globalMinPackedValue.size() +
                        L" array, but should be " +
                        to_wstring(packedBytesCount));
  }
  if (globalMaxPackedValue.empty()) {
    if (values->size() != 0) {
      throw runtime_error(L"getMaxPackedValue is null points for field \"" +
                          fieldName + L"\" yet size=" +
                          to_wstring(values->size()));
    }
  } else if (globalMaxPackedValue.size() != packedBytesCount) {
    throw runtime_error(L"getMaxPackedValue for field \"" + fieldName +
                        L"\" return length=" + globalMaxPackedValue.size() +
                        L" array, but should be " +
                        to_wstring(packedBytesCount));
  }
}

int64_t CheckIndex::VerifyPointsVisitor::getPointCountSeen()
{
  return pointCountSeen;
}

int64_t CheckIndex::VerifyPointsVisitor::getDocCountSeen()
{
  return docsSeen->cardinality();
}

void CheckIndex::VerifyPointsVisitor::visit(int docID)
{
  throw runtime_error(
      L"codec called IntersectVisitor.visit without a packed value for docID=" +
      to_wstring(docID));
}

void CheckIndex::VerifyPointsVisitor::visit(int docID,
                                            std::deque<char> &packedValue)
{
  checkPackedValue(L"packed value", packedValue, docID);
  pointCountSeen++;
  docsSeen->set(docID);

  for (int dim = 0; dim < numDims; dim++) {
    int offset = bytesPerDim * dim;

    // Compare to last cell:
    if (StringHelper::compare(bytesPerDim, packedValue, offset,
                              lastMinPackedValue, offset) < 0) {
      // This doc's point, in this dimension, is lower than the minimum value of
      // the last cell checked:
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(
          L"packed points value " + Arrays->toString(packedValue) +
          L" for field=\"" + fieldName + L"\", docID=" + to_wstring(docID) +
          L" is out-of-bounds of the last cell min=" +
          Arrays->toString(lastMinPackedValue) + L" max=" +
          Arrays->toString(lastMaxPackedValue) + L" dim=" + to_wstring(dim));
    }

    if (StringHelper::compare(bytesPerDim, packedValue, offset,
                              lastMaxPackedValue, offset) > 0) {
      // This doc's point, in this dimension, is greater than the maximum value
      // of the last cell checked:
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(
          L"packed points value " + Arrays->toString(packedValue) +
          L" for field=\"" + fieldName + L"\", docID=" + to_wstring(docID) +
          L" is out-of-bounds of the last cell min=" +
          Arrays->toString(lastMinPackedValue) + L" max=" +
          Arrays->toString(lastMaxPackedValue) + L" dim=" + to_wstring(dim));
    }
  }

  // In the 1D case, PointValues must make a single in-order sweep through all
  // values, and tie-break by increasing docID:
  if (numDims == 1) {
    int cmp =
        StringHelper::compare(bytesPerDim, lastPackedValue, 0, packedValue, 0);
    if (cmp > 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(
          L"packed points value " + Arrays->toString(packedValue) +
          L" for field=\"" + fieldName + L"\", for docID=" + to_wstring(docID) +
          L" is out-of-order vs the previous document's value " +
          Arrays->toString(lastPackedValue));
    } else if (cmp == 0) {
      if (docID < lastDocID) {
        throw runtime_error(
            L"packed points value is the same, but docID=" + to_wstring(docID) +
            L" is out of order vs previous docID=" + to_wstring(lastDocID) +
            L", field=\"" + fieldName + L"\"");
      }
    }
    System::arraycopy(packedValue, 0, lastPackedValue, 0, bytesPerDim);
    lastDocID = docID;
  }
}

PointValues::Relation
CheckIndex::VerifyPointsVisitor::compare(std::deque<char> &minPackedValue,
                                         std::deque<char> &maxPackedValue)
{
  checkPackedValue(L"min packed value", minPackedValue, -1);
  System::arraycopy(minPackedValue, 0, lastMinPackedValue, 0, packedBytesCount);
  checkPackedValue(L"max packed value", maxPackedValue, -1);
  System::arraycopy(maxPackedValue, 0, lastMaxPackedValue, 0, packedBytesCount);

  for (int dim = 0; dim < numDims; dim++) {
    int offset = bytesPerDim * dim;

    if (StringHelper::compare(bytesPerDim, minPackedValue, offset,
                              maxPackedValue, offset) > 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(L"packed points cell minPackedValue " +
                          Arrays->toString(minPackedValue) +
                          L" is out-of-bounds of the cell's maxPackedValue " +
                          Arrays->toString(maxPackedValue) + L" dim=" +
                          to_wstring(dim) + L" field=\"" + fieldName + L"\"");
    }

    // Make sure this cell is not outside of the global min/max:
    if (StringHelper::compare(bytesPerDim, minPackedValue, offset,
                              globalMinPackedValue, offset) < 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(L"packed points cell minPackedValue " +
                          Arrays->toString(minPackedValue) +
                          L" is out-of-bounds of the global minimum " +
                          Arrays->toString(globalMinPackedValue) + L" dim=" +
                          to_wstring(dim) + L" field=\"" + fieldName + L"\"");
    }

    if (StringHelper::compare(bytesPerDim, maxPackedValue, offset,
                              globalMinPackedValue, offset) < 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(L"packed points cell maxPackedValue " +
                          Arrays->toString(maxPackedValue) +
                          L" is out-of-bounds of the global minimum " +
                          Arrays->toString(globalMinPackedValue) + L" dim=" +
                          to_wstring(dim) + L" field=\"" + fieldName + L"\"");
    }

    if (StringHelper::compare(bytesPerDim, minPackedValue, offset,
                              globalMaxPackedValue, offset) > 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(L"packed points cell minPackedValue " +
                          Arrays->toString(minPackedValue) +
                          L" is out-of-bounds of the global maximum " +
                          Arrays->toString(globalMaxPackedValue) + L" dim=" +
                          to_wstring(dim) + L" field=\"" + fieldName + L"\"");
    }
    if (StringHelper::compare(bytesPerDim, maxPackedValue, offset,
                              globalMaxPackedValue, offset) > 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(L"packed points cell maxPackedValue " +
                          Arrays->toString(maxPackedValue) +
                          L" is out-of-bounds of the global maximum " +
                          Arrays->toString(globalMaxPackedValue) + L" dim=" +
                          to_wstring(dim) + L" field=\"" + fieldName + L"\"");
    }
  }

  // We always pretend the query shape is so complex that it crosses every cell,
  // so that packedValue is passed for every document
  return PointValues::Relation::CELL_CROSSES_QUERY;
}

void CheckIndex::VerifyPointsVisitor::checkPackedValue(
    const wstring &desc, std::deque<char> &packedValue, int docID)
{
  if (packedValue.empty()) {
    throw runtime_error(desc + L" is null for docID=" + to_wstring(docID) +
                        L" field=\"" + fieldName + L"\"");
  }

  if (packedValue.size() != packedBytesCount) {
    throw runtime_error(desc + L" has incorrect length=" + packedValue.size() +
                        L" vs expected=" + to_wstring(packedBytesCount) +
                        L" for docID=" + to_wstring(docID) + L" field=\"" +
                        fieldName + L"\"");
  }
}

CheckIndex::ConstantRelationIntersectVisitor::ConstantRelationIntersectVisitor(
    Relation relation)
    : relation(relation)
{
}

void CheckIndex::ConstantRelationIntersectVisitor::visit(int docID) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

void CheckIndex::ConstantRelationIntersectVisitor::visit(
    int docID, std::deque<char> &packedValue) 
{
  throw make_shared<UnsupportedOperationException>();
}

Relation CheckIndex::ConstantRelationIntersectVisitor::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return relation;
}

shared_ptr<Status::StoredFieldStatus>
CheckIndex::testStoredFields(shared_ptr<CodecReader> reader,
                             shared_ptr<PrintStream> infoStream,
                             bool failFast) 
{
  int64_t startNS = System::nanoTime();
  shared_ptr<Status::StoredFieldStatus> *const status =
      make_shared<Status::StoredFieldStatus>();

  try {
    if (infoStream != nullptr) {
      infoStream->print(L"    test: stored fields.......");
    }

    // Scan stored fields for all documents
    shared_ptr<Bits> *const liveDocs = reader->getLiveDocs();
    shared_ptr<StoredFieldsReader> storedFields =
        reader->getFieldsReader()->getMergeInstance();
    for (int j = 0; j < reader->maxDoc(); ++j) {
      // Intentionally pull even deleted documents to
      // make sure they too are not corrupt:
      shared_ptr<DocumentStoredFieldVisitor> visitor =
          make_shared<DocumentStoredFieldVisitor>();
      storedFields->visitDocument(j, visitor);
      shared_ptr<Document> doc = visitor->getDocument();
      if (liveDocs == nullptr || liveDocs->get(j)) {
        status->docCount++;
        status->totFields += doc->getFields().size();
      }
    }

    // Validate docCount
    if (status->docCount != reader->numDocs()) {
      throw runtime_error(L"docCount=" + to_wstring(status->docCount) +
                          L" but saw " + to_wstring(status->docCount) +
                          L" undeleted docs");
    }

    msg(infoStream, wstring::format(Locale::ROOT,
                                    L"OK [%d total field count; avg %.1f "
                                    L"fields per doc] [took %.3f sec]",
                                    status->totFields,
                                    ((static_cast<float>(status->totFields)) /
                                     status->docCount),
                                    nsToSec(System::nanoTime() - startNS)));
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

shared_ptr<Status::DocValuesStatus>
CheckIndex::testDocValues(shared_ptr<CodecReader> reader,
                          shared_ptr<PrintStream> infoStream,
                          bool failFast) 
{
  int64_t startNS = System::nanoTime();
  shared_ptr<Status::DocValuesStatus> *const status =
      make_shared<Status::DocValuesStatus>();
  try {
    if (infoStream != nullptr) {
      infoStream->print(L"    test: docvalues...........");
    }
    shared_ptr<DocValuesProducer> dvReader = reader->getDocValuesReader();
    if (dvReader != nullptr) {
      dvReader = dvReader->getMergeInstance();
    }
    for (auto fieldInfo : reader->getFieldInfos()) {
      if (fieldInfo->getDocValuesType() != DocValuesType::NONE) {
        status->totalValueFields++;
        checkDocValues(fieldInfo, dvReader, reader->maxDoc(), infoStream,
                       status);
      }
    }

    msg(infoStream,
        wstring::format(
            Locale::ROOT,
            L"OK [%d docvalues fields; %d BINARY; %d NUMERIC; %d SORTED; %d "
            L"SORTED_NUMERIC; %d SORTED_SET] [took %.3f sec]",
            status->totalValueFields, status->totalBinaryFields,
            status->totalNumericFields, status->totalSortedFields,
            status->totalSortedNumericFields, status->totalSortedSetFields,
            nsToSec(System::nanoTime() - startNS)));
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }
  return status;
}

void CheckIndex::checkDVIterator(
    shared_ptr<FieldInfo> fi, int maxDoc,
    DocValuesIteratorSupplier producer) 
{
  wstring field = fi->name;

  // Check advance
  shared_ptr<DocValuesIterator> it1 = producer(fi);
  shared_ptr<DocValuesIterator> it2 = producer(fi);
  int i = 0;
  for (int doc = it1->nextDoc();; doc = it1->nextDoc()) {

    if (i++ % 10 == 1) {
      int doc2 = it2->advance(doc - 1);
      if (doc2 < doc - 1) {
        throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                            to_wstring(doc - 1) + L" went backwords (got: " +
                            to_wstring(doc2) + L")");
      }
      if (doc2 == doc - 1) {
        doc2 = it2->nextDoc();
      }
      if (doc2 != doc) {
        throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                            to_wstring(doc) +
                            L" was not found through advance() (got: " +
                            to_wstring(doc2) + L")");
      }
      if (it2->docID() != doc) {
        throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                            to_wstring(doc) + L" reports wrong doc ID (got: " +
                            to_wstring(it2->docID()) + L")");
      }
    }

    if (doc == DocIdSetIterator::NO_MORE_DOCS) {
      break;
    }
  }

  // Check advanceExact
  it1 = producer(fi);
  it2 = producer(fi);
  i = 0;
  int lastDoc = -1;
  for (int doc = it1->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = it1->nextDoc()) {

    if (i++ % 13 == 1) {
      bool found = it2->advanceExact(doc - 1);
      if ((doc - 1 == lastDoc) != found) {
        throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                            to_wstring(doc - 1) +
                            L" disagrees about whether document exists (got: " +
                            StringHelper::toString(found) + L")");
      }
      if (it2->docID() != doc - 1) {
        throw runtime_error(
            L"dv iterator field=" + field + L": doc=" + to_wstring(doc - 1) +
            L" reports wrong doc ID (got: " + to_wstring(it2->docID()) + L")");
      }

      bool found2 = it2->advanceExact(doc - 1);
      if (found != found2) {
        throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                            to_wstring(doc - 1) +
                            L" has unstable advanceExact");
      }

      if (i % 2 == 0) {
        int doc2 = it2->nextDoc();
        if (doc != doc2) {
          throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                              to_wstring(doc) +
                              L" was not found through advance() (got: " +
                              to_wstring(doc2) + L")");
        }
        if (it2->docID() != doc) {
          throw runtime_error(L"dv iterator field=" + field + L": doc=" +
                              to_wstring(doc) +
                              L" reports wrong doc ID (got: " +
                              to_wstring(it2->docID()) + L")");
        }
      }
    }

    lastDoc = doc;
  }
}

void CheckIndex::checkBinaryDocValues(
    const wstring &fieldName, int maxDoc,
    shared_ptr<BinaryDocValues> bdv) 
{
  int doc;
  if (bdv->docID() != -1) {
    throw runtime_error(L"binary dv iterator for field: " + fieldName +
                        L" should start at docID=-1, but got " +
                        to_wstring(bdv->docID()));
  }
  // TODO: we could add stats to DVs, e.g. total doc count w/ a value for this
  // field
  while ((doc = bdv->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    shared_ptr<BytesRef> value = bdv->binaryValue();
    value->isValid();
  }
}

void CheckIndex::checkSortedDocValues(
    const wstring &fieldName, int maxDoc,
    shared_ptr<SortedDocValues> dv) 
{
  if (dv->docID() != -1) {
    throw runtime_error(L"sorted dv iterator for field: " + fieldName +
                        L" should start at docID=-1, but got " +
                        to_wstring(dv->docID()));
  }
  constexpr int maxOrd = dv->getValueCount() - 1;
  shared_ptr<FixedBitSet> seenOrds =
      make_shared<FixedBitSet>(dv->getValueCount());
  int maxOrd2 = -1;
  int docID;
  while ((docID = dv->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    int ord = dv->ordValue();
    if (ord == -1) {
      throw runtime_error(L"dv for field: " + fieldName + L" has -1 ord");
    } else if (ord < -1 || ord > maxOrd) {
      throw runtime_error(L"ord out of bounds: " + to_wstring(ord));
    } else {
      maxOrd2 = max(maxOrd2, ord);
      seenOrds->set(ord);
    }
  }
  if (maxOrd != maxOrd2) {
    throw runtime_error(L"dv for field: " + fieldName +
                        L" reports wrong maxOrd=" + to_wstring(maxOrd) +
                        L" but this is not the case: " + to_wstring(maxOrd2));
  }
  if (seenOrds->cardinality() != dv->getValueCount()) {
    throw runtime_error(L"dv for field: " + fieldName +
                        L" has holes in its ords, valueCount=" +
                        to_wstring(dv->getValueCount()) + L" but only used: " +
                        to_wstring(seenOrds->cardinality()));
  }
  shared_ptr<BytesRef> lastValue = nullptr;
  for (int i = 0; i <= maxOrd; i++) {
    shared_ptr<BytesRef> *const term = dv->lookupOrd(i);
    term->isValid();
    if (lastValue != nullptr) {
      if (term->compareTo(lastValue) <= 0) {
        throw runtime_error(L"dv for field: " + fieldName +
                            L" has ords out of order: " + lastValue + L" >=" +
                            term);
      }
    }
    lastValue = BytesRef::deepCopyOf(term);
  }
}

void CheckIndex::checkSortedSetDocValues(
    const wstring &fieldName, int maxDoc,
    shared_ptr<SortedSetDocValues> dv) 
{
  constexpr int64_t maxOrd = dv->getValueCount() - 1;
  shared_ptr<LongBitSet> seenOrds =
      make_shared<LongBitSet>(dv->getValueCount());
  int64_t maxOrd2 = -1;
  int docID;
  while ((docID = dv->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    int64_t lastOrd = -1;
    int64_t ord;
    int ordCount = 0;
    while ((ord = dv->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
      if (ord <= lastOrd) {
        throw runtime_error(L"ords out of order: " + to_wstring(ord) + L" <= " +
                            to_wstring(lastOrd) + L" for doc: " +
                            to_wstring(docID));
      }
      if (ord < 0 || ord > maxOrd) {
        throw runtime_error(L"ord out of bounds: " + to_wstring(ord));
      }
      lastOrd = ord;
      maxOrd2 = max(maxOrd2, ord);
      seenOrds->set(ord);
      ordCount++;
    }
    if (ordCount == 0) {
      throw runtime_error(L"dv for field: " + fieldName + L" returned docID=" +
                          to_wstring(docID) + L" yet has no ordinals");
    }
  }
  if (maxOrd != maxOrd2) {
    throw runtime_error(L"dv for field: " + fieldName +
                        L" reports wrong maxOrd=" + to_wstring(maxOrd) +
                        L" but this is not the case: " + to_wstring(maxOrd2));
  }
  if (seenOrds->cardinality() != dv->getValueCount()) {
    throw runtime_error(L"dv for field: " + fieldName +
                        L" has holes in its ords, valueCount=" +
                        to_wstring(dv->getValueCount()) + L" but only used: " +
                        to_wstring(seenOrds->cardinality()));
  }

  shared_ptr<BytesRef> lastValue = nullptr;
  for (int64_t i = 0; i <= maxOrd; i++) {
    shared_ptr<BytesRef> *const term = dv->lookupOrd(i);
    assert(term->isValid());
    if (lastValue != nullptr) {
      if (term->compareTo(lastValue) <= 0) {
        throw runtime_error(L"dv for field: " + fieldName +
                            L" has ords out of order: " + lastValue + L" >=" +
                            term);
      }
    }
    lastValue = BytesRef::deepCopyOf(term);
  }
}

void CheckIndex::checkSortedNumericDocValues(
    const wstring &fieldName, int maxDoc,
    shared_ptr<SortedNumericDocValues> ndv) 
{
  if (ndv->docID() != -1) {
    throw runtime_error(L"dv iterator for field: " + fieldName +
                        L" should start at docID=-1, but got " +
                        to_wstring(ndv->docID()));
  }
  while (true) {
    int docID = ndv->nextDoc();
    if (docID == DocIdSetIterator::NO_MORE_DOCS) {
      break;
    }
    int count = ndv->docValueCount();
    if (count == 0) {
      throw runtime_error(L"sorted numeric dv for field: " + fieldName +
                          L" returned docValueCount=0 for docID=" +
                          to_wstring(docID));
    }
    int64_t previous = numeric_limits<int64_t>::min();
    for (int j = 0; j < count; j++) {
      int64_t value = ndv->nextValue();
      if (value < previous) {
        throw runtime_error(L"values out of order: " + to_wstring(value) +
                            L" < " + to_wstring(previous) + L" for doc: " +
                            to_wstring(docID));
      }
      previous = value;
    }
  }
}

void CheckIndex::checkNumericDocValues(
    const wstring &fieldName,
    shared_ptr<NumericDocValues> ndv) 
{
  int doc;
  if (ndv->docID() != -1) {
    throw runtime_error(L"dv iterator for field: " + fieldName +
                        L" should start at docID=-1, but got " +
                        to_wstring(ndv->docID()));
  }
  // TODO: we could add stats to DVs, e.g. total doc count w/ a value for this
  // field
  while ((doc = ndv->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    ndv->longValue();
  }
}

void CheckIndex::checkDocValues(
    shared_ptr<FieldInfo> fi, shared_ptr<DocValuesProducer> dvReader,
    int maxDoc, shared_ptr<PrintStream> infoStream,
    shared_ptr<DocValuesStatus> status) 
{
  switch (fi->getDocValuesType()) {
  case SORTED:
    status->totalSortedFields++;
    checkDVIterator(fi, maxDoc, dvReader::getSorted);
    checkBinaryDocValues(fi->name, maxDoc, dvReader->getSorted(fi));
    checkSortedDocValues(fi->name, maxDoc, dvReader->getSorted(fi));
    break;
  case SORTED_NUMERIC:
    status->totalSortedNumericFields++;
    checkDVIterator(fi, maxDoc, dvReader::getSortedNumeric);
    checkSortedNumericDocValues(fi->name, maxDoc,
                                dvReader->getSortedNumeric(fi));
    break;
  case SORTED_SET:
    status->totalSortedSetFields++;
    checkDVIterator(fi, maxDoc, dvReader::getSortedSet);
    checkSortedSetDocValues(fi->name, maxDoc, dvReader->getSortedSet(fi));
    break;
  case BINARY:
    status->totalBinaryFields++;
    checkDVIterator(fi, maxDoc, dvReader::getBinary);
    checkBinaryDocValues(fi->name, maxDoc, dvReader->getBinary(fi));
    break;
  case NUMERIC:
    status->totalNumericFields++;
    checkDVIterator(fi, maxDoc, dvReader::getNumeric);
    checkNumericDocValues(fi->name, dvReader->getNumeric(fi));
    break;
  default:
    throw make_shared<AssertionError>();
  }
}

shared_ptr<Status::TermVectorStatus>
CheckIndex::testTermVectors(shared_ptr<CodecReader> reader,
                            shared_ptr<PrintStream> infoStream,
                            shared_ptr<Version> version) 
{
  return testTermVectors(reader, infoStream, false, false, false, version);
}

shared_ptr<Status::TermVectorStatus>
CheckIndex::testTermVectors(shared_ptr<CodecReader> reader,
                            shared_ptr<PrintStream> infoStream, bool verbose,
                            bool crossCheckTermVectors, bool failFast,
                            shared_ptr<Version> version) 
{
  int64_t startNS = System::nanoTime();
  shared_ptr<Status::TermVectorStatus> *const status =
      make_shared<Status::TermVectorStatus>();
  shared_ptr<FieldInfos> *const fieldInfos = reader->getFieldInfos();

  try {
    if (infoStream != nullptr) {
      infoStream->print(L"    test: term vectors........");
    }

    shared_ptr<PostingsEnum> postings = nullptr;

    // Only used if crossCheckTermVectors is true:
    shared_ptr<PostingsEnum> postingsDocs = nullptr;

    shared_ptr<Bits> *const liveDocs = reader->getLiveDocs();

    shared_ptr<Fields> *const postingsFields;
    // TODO: testTermsIndex
    if (crossCheckTermVectors) {
      postingsFields = reader->getPostingsReader()->getMergeInstance();
    } else {
      postingsFields.reset();
    }

    shared_ptr<TermVectorsReader> vectorsReader =
        reader->getTermVectorsReader();

    if (vectorsReader != nullptr) {
      vectorsReader = vectorsReader->getMergeInstance();
      for (int j = 0; j < reader->maxDoc(); ++j) {
        // Intentionally pull/visit (but don't count in
        // stats) deleted documents to make sure they too
        // are not corrupt:
        shared_ptr<Fields> tfv = vectorsReader->get(j);

        // TODO: can we make a IS(FIR) that searches just
        // this term deque... to pass for searcher?

        if (tfv->size() > 0) {
          // First run with no deletions:
          checkFields(tfv, nullptr, 1, fieldInfos, false, true, infoStream,
                      verbose, version);

          // Only agg stats if the doc is live:
          constexpr bool doStats = liveDocs == nullptr || liveDocs->get(j);

          if (doStats) {
            status->docCount++;
          }

          for (auto field : tfv) {
            if (doStats) {
              status->totVectors++;
            }

            // Make sure FieldInfo thinks this field is deque'd:
            shared_ptr<FieldInfo> *const fieldInfo =
                fieldInfos->fieldInfo(field);
            if (!fieldInfo->hasVectors()) {
              throw runtime_error(L"docID=" + to_wstring(j) +
                                  L" has term vectors for field=" + field +
                                  L" but FieldInfo has storeTermVector=false");
            }

            if (crossCheckTermVectors) {
              shared_ptr<Terms> terms = tfv->terms(field);
              shared_ptr<TermsEnum> termsEnum = terms->begin();
              constexpr bool postingsHasFreq =
                  fieldInfo->getIndexOptions().compareTo(
                      IndexOptions::DOCS_AND_FREQS) >= 0;
              constexpr bool postingsHasPayload = fieldInfo->hasPayloads();
              constexpr bool vectorsHasPayload = terms->hasPayloads();

              shared_ptr<Terms> postingsTerms = postingsFields->terms(field);
              if (postingsTerms == nullptr) {
                throw runtime_error(L"deque field=" + field +
                                    L" does not exist in postings; doc=" +
                                    to_wstring(j));
              }
              shared_ptr<TermsEnum> postingsTermsEnum = postingsTerms->begin();

              constexpr bool hasProx =
                  terms->hasOffsets() || terms->hasPositions();
              shared_ptr<BytesRef> term = nullptr;
              while ((term = termsEnum->next()) != nullptr) {

                // This is the term vectors:
                postings = termsEnum->postings(postings, PostingsEnum::ALL);
                assert(postings != nullptr);

                if (!postingsTermsEnum->seekExact(term)) {
                  throw runtime_error(
                      L"deque term=" + term + L" field=" + field +
                      L" does not exist in postings; doc=" + to_wstring(j));
                }

                // This is the inverted index ("real" postings):
                postingsDocs = postingsTermsEnum->postings(postingsDocs,
                                                           PostingsEnum::ALL);
                assert(postingsDocs != nullptr);

                constexpr int advanceDoc = postingsDocs->advance(j);
                if (advanceDoc != j) {
                  throw runtime_error(L"deque term=" + term + L" field=" +
                                      field + L": doc=" + to_wstring(j) +
                                      L" was not found in postings (got: " +
                                      to_wstring(advanceDoc) + L")");
                }

                constexpr int doc = postings->nextDoc();

                if (doc != 0) {
                  throw runtime_error(L"deque for doc " + to_wstring(j) +
                                      L" didn't return docID=0: got docID=" +
                                      to_wstring(doc));
                }

                if (postingsHasFreq) {
                  constexpr int tf = postings->freq();
                  if (postingsHasFreq && postingsDocs->freq() != tf) {
                    throw runtime_error(L"deque term=" + term + L" field=" +
                                        field + L" doc=" + to_wstring(j) +
                                        L": freq=" + to_wstring(tf) +
                                        L" differs from postings freq=" +
                                        to_wstring(postingsDocs->freq()));
                  }

                  // Term vectors has prox?
                  if (hasProx) {
                    for (int i = 0; i < tf; i++) {
                      int pos = postings->nextPosition();
                      if (postingsTerms->hasPositions()) {
                        int postingsPos = postingsDocs->nextPosition();
                        if (terms->hasPositions() && pos != postingsPos) {
                          throw runtime_error(
                              L"deque term=" + term + L" field=" + field +
                              L" doc=" + to_wstring(j) + L": pos=" +
                              to_wstring(pos) + L" differs from postings pos=" +
                              to_wstring(postingsPos));
                        }
                      }

                      // Call the methods to at least make
                      // sure they don't throw exc:
                      constexpr int startOffset = postings->startOffset();
                      constexpr int endOffset = postings->endOffset();
                      // TODO: these are too anal...?
                      /*
                      if (endOffset < startOffset) {
                      throw new RuntimeException("deque startOffset=" +
                      startOffset + " is > endOffset=" + endOffset);
                      }
                      if (startOffset < lastStartOffset) {
                      throw new RuntimeException("deque startOffset=" +
                      startOffset + " is < prior startOffset=" +
                      lastStartOffset);
                      }
                      lastStartOffset = startOffset;
                       */

                      if (startOffset != -1 && endOffset != -1 &&
                          postingsTerms->hasOffsets()) {
                        int postingsStartOffset = postingsDocs->startOffset();
                        int postingsEndOffset = postingsDocs->endOffset();
                        if (startOffset != postingsStartOffset) {
                          throw runtime_error(
                              L"deque term=" + term + L" field=" + field +
                              L" doc=" + to_wstring(j) + L": startOffset=" +
                              to_wstring(startOffset) +
                              L" differs from postings startOffset=" +
                              to_wstring(postingsStartOffset));
                        }
                        if (endOffset != postingsEndOffset) {
                          throw runtime_error(
                              L"deque term=" + term + L" field=" + field +
                              L" doc=" + to_wstring(j) + L": endOffset=" +
                              to_wstring(endOffset) +
                              L" differs from postings endOffset=" +
                              to_wstring(postingsEndOffset));
                        }
                      }

                      shared_ptr<BytesRef> payload = postings->getPayload();

                      if (payload != nullptr) {
                        assert(vectorsHasPayload);
                      }

                      if (postingsHasPayload && vectorsHasPayload) {

                        if (payload == nullptr) {
                          // we have payloads, but not at this position.
                          // postings has payloads too, it should not have one
                          // at this position
                          if (postingsDocs->getPayload() != nullptr) {
                            throw runtime_error(
                                L"deque term=" + term + L" field=" + field +
                                L" doc=" + to_wstring(j) +
                                L" has no payload but postings does: " +
                                postingsDocs->getPayload());
                          }
                        } else {
                          // we have payloads, and one at this position
                          // postings should also have one at this position,
                          // with the same bytes.
                          if (postingsDocs->getPayload() == nullptr) {
                            throw runtime_error(
                                L"deque term=" + term + L" field=" + field +
                                L" doc=" + to_wstring(j) + L" has payload=" +
                                payload + L" but postings does not.");
                          }
                          shared_ptr<BytesRef> postingsPayload =
                              postingsDocs->getPayload();
                          if (!payload->equals(postingsPayload)) {
                            throw runtime_error(
                                L"deque term=" + term + L" field=" + field +
                                L" doc=" + to_wstring(j) + L" has payload=" +
                                payload +
                                L" but differs from postings payload=" +
                                postingsPayload);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    float vectorAvg =
        status->docCount == 0
            ? 0
            : status->totVectors / static_cast<float>(status->docCount);
    msg(infoStream,
        wstring::format(Locale::ROOT,
                        L"OK [%d total term deque count; avg %.1f term/freq "
                        L"deque fields per doc] [took %.3f sec]",
                        status->totVectors, vectorAvg,
                        nsToSec(System::nanoTime() - startNS)));
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    status->error = e;
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }

  return status;
}

void CheckIndex::exorciseIndex(shared_ptr<Status> result) 
{
  ensureOpen();
  if (result->partial_) {
    throw invalid_argument(L"can only exorcise an index that was fully checked "
                           L"(this status checked a subset of segments)");
  }
  result->newSegments->changed();
  result->newSegments->commit(result->dir);
}

bool CheckIndex::assertsOn = false;

bool CheckIndex::testAsserts()
{
  assertsOn_ = true;
  return true;
}

bool CheckIndex::assertsOn()
{
  assert(testAsserts());
  return assertsOn_;
}

void CheckIndex::main(std::deque<wstring> &args) throw(IOException,
                                                        InterruptedException)
{
  int exitCode = doMain(args);
  exit(exitCode);
}

CheckIndex::Options::Options() {}

wstring CheckIndex::Options::getDirImpl() { return dirImpl; }

wstring CheckIndex::Options::getIndexPath() { return indexPath; }

void CheckIndex::Options::setOut(shared_ptr<PrintStream> out)
{
  this->out = out;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") private static int doMain(std::wstring args[]) throws java.io.IOException,
// InterruptedException
int CheckIndex::doMain(std::deque<wstring> &args) throw(IOException,
                                                         InterruptedException)
{
  shared_ptr<Options> opts;
  try {
    opts = parseOptions(args);
  } catch (const invalid_argument &e) {
    wcout << e.what() << endl;
    return 1;
  }

  if (!assertsOn()) {
    wcout << L"\nNOTE: testing will be more thorough if you run java with "
             L"'-ea:org.apache.lucene...', so assertions are enabled"
          << endl;
  }

  wcout << L"\nOpening index @ " << opts->indexPath << L"\n" << endl;
  shared_ptr<Directory> directory = nullptr;
  shared_ptr<Path> path = Paths->get(opts->indexPath);
  try {
    if (opts->dirImpl == L"") {
      directory = FSDirectory::open(path);
    } else {
      directory = CommandLineUtil::newFSDirectory(opts->dirImpl, path);
    }
  } catch (const runtime_error &t) {
    wcout << L"ERROR: could not open directory \"" << opts->indexPath
          << L"\"; exiting" << endl;
    t.printStackTrace(System::out);
    return 1;
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // directory; CheckIndex checker = new CheckIndex(dir))
  {
    org::apache::lucene::store::Directory dir = directory;
    CheckIndex checker = CheckIndex(dir);
    opts->out = System::out;
    return checker->doCheck(opts);
  }
}

shared_ptr<Options> CheckIndex::parseOptions(std::deque<wstring> &args)
{
  shared_ptr<Options> opts = make_shared<Options>();

  int i = 0;
  while (i < args.size()) {
    wstring arg = args[i];
    if (L"-fast" == arg) {
      opts->doChecksumsOnly = true;
    } else if (L"-exorcise" == arg) {
      opts->doExorcise = true;
    } else if (L"-crossCheckTermVectors" == arg) {
      opts->doCrossCheckTermVectors = true;
    } else if (arg == L"-verbose") {
      opts->verbose = true;
    } else if (arg == L"-segment") {
      if (i == args.size() - 1) {
        throw invalid_argument(L"ERROR: missing name for -segment option");
      }
      i++;
      opts->onlySegments.push_back(args[i]);
    } else if (L"-dir-impl" == arg) {
      if (i == args.size() - 1) {
        throw invalid_argument(L"ERROR: missing value for -dir-impl option");
      }
      i++;
      opts->dirImpl = args[i];
    } else {
      if (opts->indexPath != L"") {
        throw invalid_argument(L"ERROR: unexpected extra argument '" + args[i] +
                               L"'");
      }
      opts->indexPath = args[i];
    }
    i++;
  }

  if (opts->indexPath == L"") {
    throw invalid_argument(
        wstring(L"\nERROR: index path not specified") +
        L"\nUsage: java org.apache.lucene.index.CheckIndex pathToIndex "
        L"[-exorcise] [-crossCheckTermVectors] [-segment X] [-segment Y] "
        L"[-dir-impl X]\n" +
        L"\n" +
        L"  -exorcise: actually write a new segments_N file, removing any "
        L"problematic segments\n" +
        L"  -fast: just verify file checksums, omitting logical integrity "
        L"checks\n" +
        L"  -crossCheckTermVectors: verifies that term vectors match postings; "
        L"THIS IS VERY SLOW!\n" +
        L"  -codec X: when exorcising, codec to write the new segments_N file "
        L"with\n" +
        L"  -verbose: print additional details\n" +
        L"  -segment X: only check the specified segments.  This can be "
        L"specified multiple\n" +
        L"              times, to check more than one segment, eg '-segment _2 "
        L"-segment _a'.\n" +
        L"              You can't use this with the -exorcise option\n" +
        L"  -dir-impl X: use a specific " +
        FSDirectory::typeid->getSimpleName() + L" implementation. " +
        L"If no package is specified the " +
        FSDirectory::typeid->getPackage().getName() +
        L" package will be used.\n" + L"\n" +
        L"**WARNING**: -exorcise *LOSES DATA*. This should only be used on an "
        L"emergency basis as it will cause\n" +
        L"documents (perhaps many) to be permanently removed from the index.  "
        L"Always make\n" +
        L"a backup copy of your index before running this!  Do not run this "
        L"tool on an index\n" +
        L"that is actively being written to.  You have been warned!\n" + L"\n" +
        L"Run without -exorcise, this tool will open the index, report version "
        L"information\n" +
        L"and report any exceptions it hits and what action it would take if "
        L"-exorcise were\n" +
        L"specified.  With -exorcise, this tool will remove any segments that "
        L"have issues and\n" +
        L"write a new segments_N file.  This means all documents contained in "
        L"the affected\n" +
        L"segments will be removed.\n" + L"\n" +
        L"This tool exits with exit code 1 if the index cannot be opened or "
        L"has any\n" +
        L"corruption, else 0.\n");
  }

  if (opts->onlySegments.empty()) {
    opts->onlySegments.clear();
  } else if (opts->doExorcise) {
    throw invalid_argument(
        L"ERROR: cannot specify both -exorcise and -segment");
  }

  if (opts->doChecksumsOnly && opts->doCrossCheckTermVectors) {
    throw invalid_argument(
        L"ERROR: cannot specify both -fast and -crossCheckTermVectors");
  }

  return opts;
}

int CheckIndex::doCheck(shared_ptr<Options> opts) throw(IOException,
                                                        InterruptedException)
{
  setCrossCheckTermVectors(opts->doCrossCheckTermVectors);
  setChecksumsOnly(opts->doChecksumsOnly);
  setInfoStream(opts->out, opts->verbose);

  shared_ptr<Status> result = checkIndex(opts->onlySegments);
  if (result->missingSegments) {
    return 1;
  }

  if (!result->clean) {
    if (!opts->doExorcise) {
      opts->out->println(
          L"WARNING: would write new segments file, and " +
          to_wstring(result->totLoseDocCount) +
          L" documents would be lost, if -exorcise were specified\n");
    } else {
      opts->out->println(L"WARNING: " + to_wstring(result->totLoseDocCount) +
                         L" documents will be lost\n");
      opts->out->println(L"NOTE: will write new segments file in 5 seconds; "
                         L"this will remove " +
                         to_wstring(result->totLoseDocCount) +
                         L" docs from the index. YOU WILL LOSE DATA. THIS IS "
                         L"YOUR LAST CHANCE TO CTRL+C!");
      for (int s = 0; s < 5; s++) {
        delay(1000);
        opts->out->println(L"  " + to_wstring(5 - s) + L"...");
      }
      opts->out->println(L"Writing...");
      exorciseIndex(result);
      opts->out->println(L"OK");
      opts->out->println(L"Wrote new segments file \"" +
                         result->newSegments->getSegmentsFileName() + L"\"");
    }
  }
  opts->out->println(L"");

  if (result->clean == true) {
    return 0;
  } else {
    return 1;
  }
}

void CheckIndex::checkSoftDeletes(const wstring &softDeletesField,
                                  shared_ptr<SegmentCommitInfo> info,
                                  shared_ptr<SegmentReader> reader,
                                  shared_ptr<PrintStream> infoStream,
                                  bool failFast) 
{
  if (infoStream != nullptr) {
    infoStream->print(L"    test: check soft deletes.....");
  }
  try {
    int softDeletes = PendingSoftDeletes::countSoftDeletes(
        DocValuesFieldExistsQuery::getDocValuesDocIdSetIterator(
            softDeletesField, reader),
        reader->getLiveDocs());
    if (softDeletes != info->getSoftDelCount()) {
      throw runtime_error(L"actual soft deletes: " + to_wstring(softDeletes) +
                          L" but expected: " +
                          to_wstring(info->getSoftDelCount()));
    }
  } catch (const runtime_error &e) {
    if (failFast) {
      throw IOUtils::rethrowAlways(e);
    }
    msg(infoStream, L"ERROR [" + wstring::valueOf(e.what()) + L"]");
    if (infoStream != nullptr) {
      e.printStackTrace(infoStream);
    }
  }
}

double CheckIndex::nsToSec(int64_t ns) { return ns / 1000000000.0; }
} // namespace org::apache::lucene::index