using namespace std;

#include "StandardDirectoryReader.h"

namespace org::apache::lucene::index
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IOUtils = org::apache::lucene::util::IOUtils;

StandardDirectoryReader::StandardDirectoryReader(
    shared_ptr<Directory> directory,
    std::deque<std::shared_ptr<LeafReader>> &readers,
    shared_ptr<IndexWriter> writer, shared_ptr<SegmentInfos> sis,
    bool applyAllDeletes, bool writeAllDeletes) 
    : DirectoryReader(directory, readers), writer(writer), segmentInfos(sis),
      applyAllDeletes(applyAllDeletes), writeAllDeletes(writeAllDeletes)
{
}

shared_ptr<DirectoryReader>
StandardDirectoryReader::open(shared_ptr<Directory> directory,
                              shared_ptr<IndexCommit> commit) 
{
  return make_shared<FindSegmentsFileAnonymousInnerClass>(directory).run(
      commit);
}

StandardDirectoryReader::FindSegmentsFileAnonymousInnerClass::
    FindSegmentsFileAnonymousInnerClass(shared_ptr<Directory> directory)
    : SegmentInfos::FindSegmentsFile<DirectoryReader>(directory)
{
  this->directory = directory;
}

shared_ptr<DirectoryReader>
StandardDirectoryReader::FindSegmentsFileAnonymousInnerClass::doBody(
    const wstring &segmentFileName) 
{
  shared_ptr<SegmentInfos> sis =
      SegmentInfos::readCommit(directory, segmentFileName);
  std::deque<std::shared_ptr<SegmentReader>> readers(sis->size());
  bool success = false;
  try {
    for (int i = sis->size() - 1; i >= 0; i--) {
      readers[i] = make_shared<SegmentReader>(
          sis->info(i), sis->getIndexCreatedVersionMajor(), IOContext::READ);
    }

    // This may throw CorruptIndexException if there are too many docs, so
    // it must be inside try clause so we close readers in that case:
    shared_ptr<DirectoryReader> reader = make_shared<StandardDirectoryReader>(
        directory, readers, nullptr, sis, false, false);
    success = true;

    return reader;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      IOUtils::closeWhileHandlingException(readers);
    }
  }
}

shared_ptr<DirectoryReader> StandardDirectoryReader::open(
    shared_ptr<IndexWriter> writer, shared_ptr<SegmentInfos> infos,
    bool applyAllDeletes, bool writeAllDeletes) 
{
  // IndexWriter synchronizes externally before calling
  // us, which ensures infos will not change; so there's
  // no need to process segments in reverse order
  constexpr int numSegments = infos->size();

  const deque<std::shared_ptr<SegmentReader>> readers =
      deque<std::shared_ptr<SegmentReader>>(numSegments);
  shared_ptr<Directory> *const dir = writer->getDirectory();

  shared_ptr<SegmentInfos> *const segmentInfos = infos->clone();
  int infosUpto = 0;
  try {
    for (int i = 0; i < numSegments; i++) {
      // NOTE: important that we use infos not
      // segmentInfos here, so that we are passing the
      // actual instance of SegmentInfoPerCommit in
      // IndexWriter's segmentInfos:
      shared_ptr<SegmentCommitInfo> *const info = infos->info(i);
      assert(info->info->dir == dir);
      shared_ptr<ReadersAndUpdates> *const rld =
          writer->getPooledInstance(info, true);
      try {
        shared_ptr<SegmentReader> *const reader =
            rld->getReadOnlyClone(IOContext::READ);
        if (reader->numDocs() > 0 ||
            writer->getConfig()->mergePolicy->keepFullyDeletedSegment(
                [&]() { reader; })) {
          // Steal the ref:
          readers.push_back(reader);
          infosUpto++;
        } else {
          reader->decRef();
          segmentInfos->erase(segmentInfos->begin() + infosUpto);
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        writer->release(rld);
      }
    }

    writer->incRefDeleter(segmentInfos);

    shared_ptr<StandardDirectoryReader> result =
        make_shared<StandardDirectoryReader>(
            dir,
            readers.toArray(
                std::deque<std::shared_ptr<SegmentReader>>(readers.size())),
            writer, segmentInfos, applyAllDeletes, writeAllDeletes);
    return result;
  } catch (const runtime_error &t) {
    try {
      IOUtils::applyToAll(readers, SegmentReader::decRef);
    } catch (const runtime_error &t1) {
      t.addSuppressed(t1);
    }
    throw t;
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static DirectoryReader
// open(org.apache.lucene.store.Directory directory, SegmentInfos infos,
// java.util.List<? extends LeafReader> oldReaders) throws java.io.IOException
shared_ptr<DirectoryReader>
StandardDirectoryReader::open(shared_ptr<Directory> directory,
                              shared_ptr<SegmentInfos> infos,
                              deque<T1> oldReaders) 
{

  // we put the old SegmentReaders in a map_obj, that allows us
  // to lookup a reader using its segment name
  const unordered_map<wstring, int> segmentReaders =
      (oldReaders.empty() ? Collections::emptyMap()
                          : unordered_map<wstring, int>(oldReaders.size()));

  if (oldReaders.size() > 0) {
    // create a Map SegmentName->SegmentReader
    for (int i = 0, c = oldReaders.size(); i < c; i++) {
      shared_ptr<SegmentReader> *const sr =
          std::static_pointer_cast<SegmentReader>(oldReaders[i]);
      segmentReaders.emplace(sr->getSegmentName(), static_cast<Integer>(i));
    }
  }

  std::deque<std::shared_ptr<SegmentReader>> newReaders(infos->size());
  for (int i = infos->size() - 1; i >= 0; i--) {
    shared_ptr<SegmentCommitInfo> commitInfo = infos->info(i);

    // find SegmentReader for this segment
    optional<int> oldReaderIndex = segmentReaders[commitInfo->info->name];
    shared_ptr<SegmentReader> oldReader;
    if (!oldReaderIndex) {
      // this is a new segment, no old SegmentReader can be reused
      oldReader.reset();
    } else {
      // there is an old reader for this segment - we'll try to reopen it
      oldReader = std::static_pointer_cast<SegmentReader>(
          oldReaders[oldReaderIndex.value()]);
    }

    // Make a best effort to detect when the app illegally "rm -rf" their
    // index while a reader was open, and then called openIfChanged:
    if (oldReader != nullptr &&
        Arrays::equals(commitInfo->info->getId(),
                       oldReader->getSegmentInfo()->info->getId()) == false) {
      throw make_shared<IllegalStateException>(
          L"same segment " + commitInfo->info->name +
          L" has invalid doc count change; likely you are re-opening a reader "
          L"after illegally removing index files yourself and building a new "
          L"index in their place.  Use IndexWriter.deleteAll or open a new "
          L"IndexWriter using OpenMode.CREATE instead");
    }

    bool success = false;
    try {
      shared_ptr<SegmentReader> newReader;
      if (oldReader == nullptr ||
          commitInfo->info->getUseCompoundFile() !=
              oldReader->getSegmentInfo()->info->getUseCompoundFile()) {

        // this is a new reader; in case we hit an exception we can decRef it
        // safely
        newReader = make_shared<SegmentReader>(
            commitInfo, infos->getIndexCreatedVersionMajor(), IOContext::READ);
        newReaders[i] = newReader;
      } else {
        if (oldReader->isNRT) {
          // We must load liveDocs/DV updates from disk:
          newReaders[i] = make_shared<SegmentReader>(commitInfo, oldReader);
        } else {

          if (oldReader->getSegmentInfo()->getDelGen() ==
                  commitInfo->getDelGen() &&
              oldReader->getSegmentInfo()->getFieldInfosGen() ==
                  commitInfo->getFieldInfosGen()) {
            // No change; this reader will be shared between
            // the old and the new one, so we must incRef
            // it:
            oldReader->incRef();
            newReaders[i] = oldReader;
          } else {
            // Steal the ref returned by SegmentReader ctor:
            assert(commitInfo->info->dir ==
                   oldReader->getSegmentInfo()->info->dir);

            if (oldReader->getSegmentInfo()->getDelGen() ==
                commitInfo->getDelGen()) {
              // only DV updates
              newReaders[i] = make_shared<SegmentReader>(
                  commitInfo, oldReader, oldReader->getLiveDocs(),
                  oldReader->numDocs(), false); // this is not an NRT reader!
            } else {
              // both DV and liveDocs have changed
              newReaders[i] = make_shared<SegmentReader>(commitInfo, oldReader);
            }
          }
        }
      }
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        decRefWhileHandlingException(newReaders);
      }
    }
  }
  return make_shared<StandardDirectoryReader>(directory, newReaders, nullptr,
                                              infos, false, false);
}

void StandardDirectoryReader::decRefWhileHandlingException(
    std::deque<std::shared_ptr<SegmentReader>> &readers)
{
  for (auto reader : readers) {
    if (reader != nullptr) {
      try {
        reader->decRef();
      } catch (const runtime_error &t) {
        // Ignore so we keep throwing original exception
      }
    }
  }
}

wstring StandardDirectoryReader::toString()
{
  shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>();
  buffer->append(getClass().getSimpleName());
  buffer->append(L'(');
  const wstring segmentsFile = segmentInfos->getSegmentsFileName();
  if (segmentsFile != L"") {
    buffer->append(segmentsFile)
        ->append(L":")
        ->append(segmentInfos->getVersion());
  }
  if (writer != nullptr) {
    buffer->append(L":nrt");
  }
  for (auto r : getSequentialSubReaders()) {
    buffer->append(L' ');
    buffer->append(r);
  }
  buffer->append(L')');
  return buffer->toString();
}

shared_ptr<DirectoryReader>
StandardDirectoryReader::doOpenIfChanged() 
{
  return doOpenIfChanged(std::static_pointer_cast<IndexCommit>(nullptr));
}

shared_ptr<DirectoryReader> StandardDirectoryReader::doOpenIfChanged(
    shared_ptr<IndexCommit> commit) 
{
  ensureOpen();

  // If we were obtained by writer.getReader(), re-ask the
  // writer to get a new reader.
  if (writer != nullptr) {
    return doOpenFromWriter(commit);
  } else {
    return doOpenNoWriter(commit);
  }
}

shared_ptr<DirectoryReader> StandardDirectoryReader::doOpenIfChanged(
    shared_ptr<IndexWriter> writer, bool applyAllDeletes) 
{
  ensureOpen();
  if (writer == this->writer && applyAllDeletes == this->applyAllDeletes) {
    return doOpenFromWriter(nullptr);
  } else {
    return writer->getReader(applyAllDeletes, writeAllDeletes);
  }
}

shared_ptr<DirectoryReader> StandardDirectoryReader::doOpenFromWriter(
    shared_ptr<IndexCommit> commit) 
{
  if (commit != nullptr) {
    return doOpenFromCommit(commit);
  }

  if (writer->nrtIsCurrent(segmentInfos)) {
    return nullptr;
  }

  shared_ptr<DirectoryReader> reader =
      writer->getReader(applyAllDeletes, writeAllDeletes);

  // If in fact no changes took place, return null:
  if (reader->getVersion() == segmentInfos->getVersion()) {
    reader->decRef();
    return nullptr;
  }

  return reader;
}

shared_ptr<DirectoryReader> StandardDirectoryReader::doOpenNoWriter(
    shared_ptr<IndexCommit> commit) 
{

  if (commit == nullptr) {
    if (isCurrent()) {
      return nullptr;
    }
  } else {
    if (directory_ != commit->getDirectory()) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(
          L"the specified commit does not match the specified Directory");
    }
    if (segmentInfos->size() > 0 &&
        commit->getSegmentsFileName() == segmentInfos->getSegmentsFileName()) {
      return nullptr;
    }
  }

  return doOpenFromCommit(commit);
}

shared_ptr<DirectoryReader> StandardDirectoryReader::doOpenFromCommit(
    shared_ptr<IndexCommit> commit) 
{
  return make_shared<FindSegmentsFileAnonymousInnerClass2>(shared_from_this(),
                                                           directory_)
      .run(commit);
}

StandardDirectoryReader::FindSegmentsFileAnonymousInnerClass2::
    FindSegmentsFileAnonymousInnerClass2(
        shared_ptr<StandardDirectoryReader> outerInstance,
        shared_ptr<Directory> directory)
    : SegmentInfos::FindSegmentsFile<DirectoryReader>(directory)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DirectoryReader>
StandardDirectoryReader::FindSegmentsFileAnonymousInnerClass2::doBody(
    const wstring &segmentFileName) 
{
  shared_ptr<SegmentInfos> *const infos =
      SegmentInfos::readCommit(outerInstance->directory_, segmentFileName);
  return outerInstance->doOpenIfChanged(infos);
}

shared_ptr<DirectoryReader> StandardDirectoryReader::doOpenIfChanged(
    shared_ptr<SegmentInfos> infos) 
{
  return StandardDirectoryReader::open(directory_, infos,
                                       getSequentialSubReaders());
}

int64_t StandardDirectoryReader::getVersion()
{
  ensureOpen();
  return segmentInfos->getVersion();
}

shared_ptr<SegmentInfos> StandardDirectoryReader::getSegmentInfos()
{
  return segmentInfos;
}

bool StandardDirectoryReader::isCurrent() 
{
  ensureOpen();
  if (writer == nullptr || writer->isClosed()) {
    // Fully read the segments file: this ensures that it's
    // completely written so that if
    // IndexWriter.prepareCommit has been called (but not
    // yet commit), then the reader will still see itself as
    // current:
    shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(directory_);

    // we loaded SegmentInfos from the directory
    return sis->getVersion() == segmentInfos->getVersion();
  } else {
    return writer->nrtIsCurrent(segmentInfos);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings("try") protected void doClose()
// throws java.io.IOException
void StandardDirectoryReader::doClose() 
{
  shared_ptr<Closeable> decRefDeleter = [&]() {
    if (writer != nullptr) {
      try {
        writer->decRefDeleter(segmentInfos);
      } catch (const AlreadyClosedException &ex) {
        // This is OK, it just means our original writer was
        // closed before we were, and this may leave some
        // un-referenced files in the index, which is
        // harmless.  The next time IW is opened on the
        // index, it will delete them.
      }
    }
  };
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer = decRefDeleter)
  {
    java::io::Closeable finalizer = decRefDeleter;
    // try to close each reader, even if an exception is thrown
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.List<? extends LeafReader>
    // sequentialSubReaders = getSequentialSubReaders();
    const deque < ? extends LeafReader > sequentialSubReaders =
                         getSequentialSubReaders();
    IOUtils::applyToAll(sequentialSubReaders, LeafReader::decRef);
  }
}

shared_ptr<IndexCommit>
StandardDirectoryReader::getIndexCommit() 
{
  ensureOpen();
  return make_shared<ReaderCommit>(shared_from_this(), segmentInfos,
                                   directory_);
}

StandardDirectoryReader::ReaderCommit::ReaderCommit(
    shared_ptr<StandardDirectoryReader> reader, shared_ptr<SegmentInfos> infos,
    shared_ptr<Directory> dir) 
    : userData(infos->getUserData()), segmentCount(infos->size()),
      reader(reader)
{
  segmentsFileName = infos->getSegmentsFileName();
  this->dir = dir;
  files = Collections::unmodifiableCollection(infos->files(true));
  generation = infos->getGeneration();

  // NOTE: we intentionally do not incRef this!  Else we'd need to make
  // IndexCommit Closeable...
}

wstring StandardDirectoryReader::ReaderCommit::toString()
{
  return L"StandardDirectoryReader.ReaderCommit(" + segmentsFileName +
         L" files=" + files + L")";
}

int StandardDirectoryReader::ReaderCommit::getSegmentCount()
{
  return segmentCount;
}

wstring StandardDirectoryReader::ReaderCommit::getSegmentsFileName()
{
  return segmentsFileName;
}

shared_ptr<deque<wstring>>
StandardDirectoryReader::ReaderCommit::getFileNames()
{
  return files;
}

shared_ptr<Directory> StandardDirectoryReader::ReaderCommit::getDirectory()
{
  return dir;
}

int64_t StandardDirectoryReader::ReaderCommit::getGeneration()
{
  return generation;
}

bool StandardDirectoryReader::ReaderCommit::isDeleted() { return false; }

unordered_map<wstring, wstring>
StandardDirectoryReader::ReaderCommit::getUserData()
{
  return userData;
}

void StandardDirectoryReader::ReaderCommit::delete_()
{
  throw make_shared<UnsupportedOperationException>(
      L"This IndexCommit does not support deletions");
}

shared_ptr<StandardDirectoryReader>
StandardDirectoryReader::ReaderCommit::getReader()
{
  return reader;
}

StandardDirectoryReader::CacheHelperAnonymousInnerClass::
    CacheHelperAnonymousInnerClass()
{
}

shared_ptr<CacheKey>
StandardDirectoryReader::CacheHelperAnonymousInnerClass::getKey()
{
  return cacheKey;
}

void StandardDirectoryReader::CacheHelperAnonymousInnerClass::addClosedListener(
    shared_ptr<ClosedListener> listener)
{
  ensureOpen();
  outerInstance->readerClosedListeners->add(listener);
}

void StandardDirectoryReader::notifyReaderClosedListeners() 
{
  {
    lock_guard<mutex> lock(readerClosedListeners);
    IOUtils::applyToAll(readerClosedListeners,
                        [&](any l) { l::onClose(cacheHelper::getKey()); });
  }
}

shared_ptr<CacheHelper> StandardDirectoryReader::getReaderCacheHelper()
{
  return cacheHelper;
}
} // namespace org::apache::lucene::index