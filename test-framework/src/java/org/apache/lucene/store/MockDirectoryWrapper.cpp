using namespace std;

#include "MockDirectoryWrapper.h"

namespace org::apache::lucene::store
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using NoDeletionPolicy = org::apache::lucene::index::NoDeletionPolicy;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThrottledIndexOutput = org::apache::lucene::util::ThrottledIndexOutput;

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::init()
{
  if (openFiles.empty()) {
    openFiles = unordered_map<>();
    openFilesDeleted = unordered_set<>();
  }

  if (createdFiles == nullptr) {
    createdFiles = unordered_set<>();
  }
  if (unSyncedFiles == nullptr) {
    unSyncedFiles = unordered_set<>();
  }
}

MockDirectoryWrapper::MockDirectoryWrapper(shared_ptr<Random> random,
                                           shared_ptr<Directory> delegate_)
    : BaseDirectoryWrapper(delegate_)
{
  // must make a private random since our methods are
  // called from different threads; else test failures may
  // not be reproducible from the original seed
  this->randomState = make_shared<Random>(random->nextInt());
  this->throttledOutput = make_shared<ThrottledIndexOutput>(
      ThrottledIndexOutput::mBitsToBytes(40 + randomState->nextInt(10)),
      1 + randomState->nextInt(5), nullptr);
  init();
}

int MockDirectoryWrapper::getInputCloneCount()
{
  return inputCloneCount->get();
}

void MockDirectoryWrapper::setVerboseClone(bool v) { verboseClone = v; }

void MockDirectoryWrapper::setTrackDiskUsage(bool v) { trackDiskUsage = v; }

void MockDirectoryWrapper::setAllowRandomFileNotFoundException(bool value)
{
  allowRandomFileNotFoundException = value;
}

void MockDirectoryWrapper::setAllowReadingFilesStillOpenForWrite(bool value)
{
  allowReadingFilesStillOpenForWrite = value;
}

void MockDirectoryWrapper::setThrottling(Throttling throttling)
{
  this->throttling = throttling;
}

void MockDirectoryWrapper::setUseSlowOpenClosers(bool v)
{
  useSlowOpenClosers = v;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::sync(shared_ptr<deque<wstring>> names) throw(
    IOException)
{
  maybeYield();
  maybeThrowDeterministicException();
  if (crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"cannot sync after crash");
  }
  // always pass thru fsync, directories rely on this.
  // 90% of time, we use DisableFsyncFS which omits the real calls.
  for (auto name : names) {
    // randomly fail with IOE on any file
    maybeThrowIOException(name);
    in_->sync(Collections::singleton(name));
    unSyncedFiles->remove(name);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::rename(const wstring &source,
                                  const wstring &dest) 
{
  maybeYield();
  maybeThrowDeterministicException();

  if (crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"cannot rename after crash");
  }

  if (openFiles.find(source) != openFiles.end() && assertNoDeleteOpenFile) {
    throw std::static_pointer_cast<AssertionError>(fillOpenTrace(
        make_shared<AssertionError>(L"MockDirectoryWrapper: source file \"" +
                                    source +
                                    L"\" is still open: cannot rename"),
        source, true));
  }

  if (openFiles.find(dest) != openFiles.end() && assertNoDeleteOpenFile) {
    throw std::static_pointer_cast<AssertionError>(fillOpenTrace(
        make_shared<AssertionError>(L"MockDirectoryWrapper: dest file \"" +
                                    dest + L"\" is still open: cannot rename"),
        dest, true));
  }

  bool success = false;
  try {
    in_->rename(source, dest);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      // we don't do this stuff with lucene's commit, but it's just for
      // completeness
      if (unSyncedFiles->contains(source)) {
        unSyncedFiles->remove(source);
        unSyncedFiles->add(dest);
      }
      openFilesDeleted->remove(source);
      createdFiles->remove(source);
      createdFiles->add(dest);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::syncMetaData() 
{
  maybeYield();
  maybeThrowDeterministicException();
  if (crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"cannot rename after crash");
  }
  in_->syncMetaData();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t MockDirectoryWrapper::sizeInBytes() 
{
  if (std::dynamic_pointer_cast<RAMDirectory>(in_) != nullptr) {
    return (std::static_pointer_cast<RAMDirectory>(in_))->ramBytesUsed();
  } else {
    // hack
    int64_t size = 0;
    for (auto file : in_->listAll()) {
      // hack 2: see TODO in ExtrasFS (ideally it would always return 0 byte
      // size for extras it creates, even though the size of non-regular files
      // is not defined)
      if (!file.startsWith(L"extra")) {
        size += in_->fileLength(file);
      }
    }
    return size;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::corruptUnknownFiles() 
{

  wcout << L"MDW: corrupt unknown files" << endl;
  shared_ptr<Set<wstring>> knownFiles = unordered_set<wstring>();
  for (auto fileName : listAll()) {
    if (fileName.startsWith(IndexFileNames::SEGMENTS)) {
      wcout << L"MDW: read " << fileName << L" to gather files it references"
            << endl;
      shared_ptr<SegmentInfos> infos;
      try {
        infos = SegmentInfos::readCommit(shared_from_this(), fileName);
      } catch (const IOException &ioe) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"MDW: exception reading segment infos " << fileName
              << L"; files: " << Arrays->toString(listAll()) << endl;
        throw ioe;
      }
      knownFiles->addAll(infos->files(true));
    }
  }

  shared_ptr<Set<wstring>> toCorrupt = unordered_set<wstring>();
  shared_ptr<Matcher> m = IndexFileNames::CODEC_FILE_PATTERN->matcher(L"");
  for (auto fileName : listAll()) {
    m->reset(fileName);
    if (knownFiles->contains(fileName) == false &&
        fileName.endsWith(L"write.lock") == false &&
        (m->matches() ||
         fileName.startsWith(IndexFileNames::PENDING_SEGMENTS))) {
      toCorrupt->add(fileName);
    }
  }

  corruptFiles(toCorrupt);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::corruptFiles(
    shared_ptr<deque<wstring>> files) 
{
  bool disabled = TestUtil::disableVirusChecker(in_);
  try {
    _corruptFiles(files);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (disabled) {
      TestUtil::enableVirusChecker(in_);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::_corruptFiles(
    shared_ptr<deque<wstring>> files) 
{

  // TODO: we should also mess with any recent file renames, file deletions, if
  // syncMetaData was not called!!

  // Must make a copy because we change the incoming unsyncedFiles
  // when we create temp files, delete, etc., below:
  const deque<wstring> filesToCorrupt = deque<wstring>(files);
  // sort the files otherwise we have reproducibility issues
  // across JVMs if the incoming collection is a hashSet etc.
  CollectionUtil::timSort(filesToCorrupt);
  for (auto name : filesToCorrupt) {
    int damage = randomState->nextInt(6);
    if (alwaysCorrupt && damage == 3) {
      damage = 4;
    }
    wstring action = L"";

    switch (damage) {

    case 0:
      action = L"deleted";
      deleteFile(name);
      break;

    case 1: {
      action = L"zeroed";
      // Zero out file entirely
      int64_t length;
      try {
        length = fileLength(name);
      } catch (const IOException &ioe) {
        // C++ TODO: This exception's constructor requires only one argument:
        // ORIGINAL LINE: throw new RuntimeException("hit unexpected IOException
        // while trying to corrupt file " + name, ioe);
        throw runtime_error(
            L"hit unexpected IOException while trying to corrupt file " + name);
      }

      // Delete original and write zeros back:
      deleteFile(name);

      std::deque<char> zeroes(256);
      int64_t upto = 0;
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (IndexOutput out =
      // in.createOutput(name,
      // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState)))
      {
        IndexOutput out = in_->createOutput(
            name, org::apache::lucene::util::LuceneTestCase::newIOContext(
                      randomState));
        try {
          while (upto < length) {
            constexpr int limit =
                static_cast<int>(min(length - upto, zeroes.size()));
            out->writeBytes(zeroes, 0, limit);
            upto += limit;
          }
        } catch (const IOException &ioe) {
          // C++ TODO: This exception's constructor requires only one argument:
          // ORIGINAL LINE: throw new RuntimeException("hit unexpected
          // IOException while trying to corrupt file " + name, ioe);
          throw runtime_error(
              L"hit unexpected IOException while trying to corrupt file " +
              name);
        }
      }
      break;
    }
    case 2: {
      action = L"partially truncated";
      // Partially Truncate the file:

      // First, make temp file and copy only half this
      // file over:
      wstring tempFileName = L"";
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (IndexOutput tempOut =
      // in.createTempOutput("name", "mdw_corrupt",
      // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState));
      // IndexInput ii = in.openInput(name,
      // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState)))
      {
        IndexOutput tempOut = in_->createTempOutput(
            L"name", L"mdw_corrupt",
            org::apache::lucene::util::LuceneTestCase::newIOContext(
                randomState));
        IndexInput ii = in_->openInput(
            name, org::apache::lucene::util::LuceneTestCase::newIOContext(
                      randomState));
        try {
          tempFileName = tempOut->getName();
          tempOut->copyBytes(ii, ii->length() / 2);
        } catch (const IOException &ioe) {
          // C++ TODO: This exception's constructor requires only one argument:
          // ORIGINAL LINE: throw new RuntimeException("hit unexpected
          // IOException while trying to corrupt file " + name, ioe);
          throw runtime_error(
              L"hit unexpected IOException while trying to corrupt file " +
              name);
        }
      }

      // Delete original and copy bytes back:
      deleteFile(name);

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (IndexOutput out =
      // in.createOutput(name,
      // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState));
      // IndexInput ii = in.openInput(tempFileName,
      // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState)))
      {
        IndexOutput out = in_->createOutput(
            name, org::apache::lucene::util::LuceneTestCase::newIOContext(
                      randomState));
        IndexInput ii = in_->openInput(
            tempFileName,
            org::apache::lucene::util::LuceneTestCase::newIOContext(
                randomState));
        try {
          out->copyBytes(ii, ii->length());
        } catch (const IOException &ioe) {
          // C++ TODO: This exception's constructor requires only one argument:
          // ORIGINAL LINE: throw new RuntimeException("hit unexpected
          // IOException while trying to corrupt file " + name, ioe);
          throw runtime_error(
              L"hit unexpected IOException while trying to corrupt file " +
              name);
        }
      }
      deleteFile(tempFileName);
    } break;

    case 3:
      // The file survived intact:
      action = L"didn't change";
      break;

    case 4:
      // Corrupt one bit randomly in the file:

      {

        wstring tempFileName = L"";
        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try (IndexOutput tempOut =
        // in.createTempOutput("name", "mdw_corrupt",
        // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState));
        // IndexInput ii = in.openInput(name,
        // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState)))
        {
          IndexOutput tempOut = in_->createTempOutput(
              L"name", L"mdw_corrupt",
              org::apache::lucene::util::LuceneTestCase::newIOContext(
                  randomState));
          IndexInput ii = in_->openInput(
              name, org::apache::lucene::util::LuceneTestCase::newIOContext(
                        randomState));
          try {
            tempFileName = tempOut->getName();
            if (ii->length() > 0) {
              // Copy first part unchanged:
              int64_t byteToCorrupt = static_cast<int64_t>(
                  randomState->nextDouble() * ii->length());
              if (byteToCorrupt > 0) {
                tempOut->copyBytes(ii, byteToCorrupt);
              }

              // Randomly flip one bit from this byte:
              char b = ii->readByte();
              int bitToFlip = randomState->nextInt(8);
              b = static_cast<char>(b ^ (1 << bitToFlip));
              tempOut->writeByte(b);

              action = L"flip bit " + to_wstring(bitToFlip) + L" of byte " +
                       to_wstring(byteToCorrupt) + L" out of " +
                       to_wstring(ii->length()) + L" bytes";

              // Copy last part unchanged:
              int64_t bytesLeft = ii->length() - byteToCorrupt - 1;
              if (bytesLeft > 0) {
                tempOut->copyBytes(ii, bytesLeft);
              }
            } else {
              action = L"didn't change";
            }
          } catch (const IOException &ioe) {
            // C++ TODO: This exception's constructor requires only one
            // argument: ORIGINAL LINE: throw new RuntimeException("hit
            // unexpected IOException while trying to corrupt file " + name,
            // ioe);
            throw runtime_error(
                L"hit unexpected IOException while trying to corrupt file " +
                name);
          }
        }

        // Delete original and copy bytes back:
        deleteFile(name);

        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try (IndexOutput out =
        // in.createOutput(name,
        // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState));
        // IndexInput ii = in.openInput(tempFileName,
        // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState)))
        {
          IndexOutput out = in_->createOutput(
              name, org::apache::lucene::util::LuceneTestCase::newIOContext(
                        randomState));
          IndexInput ii = in_->openInput(
              tempFileName,
              org::apache::lucene::util::LuceneTestCase::newIOContext(
                  randomState));
          try {
            out->copyBytes(ii, ii->length());
          } catch (const IOException &ioe) {
            // C++ TODO: This exception's constructor requires only one
            // argument: ORIGINAL LINE: throw new RuntimeException("hit
            // unexpected IOException while trying to corrupt file " + name,
            // ioe);
            throw runtime_error(
                L"hit unexpected IOException while trying to corrupt file " +
                name);
          }
        }

        deleteFile(tempFileName);
      }
      break;

    case 5:
      action = L"fully truncated";
      // Totally truncate the file to zero bytes
      deleteFile(name);

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (IndexOutput out =
      // in.createOutput(name,
      // org.apache.lucene.util.LuceneTestCase.newIOContext(randomState)))
      {
        IndexOutput out = in_->createOutput(
            name, org::apache::lucene::util::LuceneTestCase::newIOContext(
                      randomState));
        try {
          out->getFilePointer(); // just fake access to prevent compiler warning
        } catch (const IOException &ioe) {
          // C++ TODO: This exception's constructor requires only one argument:
          // ORIGINAL LINE: throw new RuntimeException("hit unexpected
          // IOException while trying to corrupt file " + name, ioe);
          throw runtime_error(
              L"hit unexpected IOException while trying to corrupt file " +
              name);
        }
      }
      break;

    default:
      throw make_shared<AssertionError>();
    }

    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockDirectoryWrapper: " << action << L" unsynced file: "
            << name << endl;
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::crash() 
{
  openFiles = unordered_map<>();
  openFilesForWrite = unordered_set<>();
  openFilesDeleted = unordered_set<>();
  // first force-close all files, so we can corrupt on windows etc.
  // clone the file map_obj, as these guys want to remove themselves on close.
  unordered_map<std::shared_ptr<Closeable>, runtime_error> m =
      make_shared<IdentityHashMap<std::shared_ptr<Closeable>, runtime_error>>(
          openFileHandles);
  for (auto f : m) {
    try {
      f->first->close();
    } catch (const runtime_error &ignored) {
    }
  }
  corruptFiles(unSyncedFiles);
  crashed = true;
  unSyncedFiles = unordered_set<>();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::clearCrash()
{
  crashed = false;
  openLocks->clear();
}

void MockDirectoryWrapper::setMaxSizeInBytes(int64_t maxSize)
{
  this->maxSize = maxSize;
}

int64_t MockDirectoryWrapper::getMaxSizeInBytes() { return this->maxSize; }

int64_t MockDirectoryWrapper::getMaxUsedSizeInBytes()
{
  return this->maxUsedSize;
}

void MockDirectoryWrapper::resetMaxUsedSizeInBytes() 
{
  this->maxUsedSize = getRecomputedActualSizeInBytes();
}

void MockDirectoryWrapper::setAssertNoDeleteOpenFile(bool value)
{
  this->assertNoDeleteOpenFile = value;
}

bool MockDirectoryWrapper::getAssertNoDeleteOpenFile()
{
  return assertNoDeleteOpenFile;
}

void MockDirectoryWrapper::setRandomIOExceptionRate(double rate)
{
  randomIOExceptionRate = rate;
}

double MockDirectoryWrapper::getRandomIOExceptionRate()
{
  return randomIOExceptionRate;
}

void MockDirectoryWrapper::setRandomIOExceptionRateOnOpen(double rate)
{
  randomIOExceptionRateOnOpen = rate;
}

double MockDirectoryWrapper::getRandomIOExceptionRateOnOpen()
{
  return randomIOExceptionRateOnOpen;
}

void MockDirectoryWrapper::maybeThrowIOException(const wstring &message) throw(
    IOException)
{
  if (randomState->nextDouble() < randomIOExceptionRate) {
    shared_ptr<IOException> ioe = make_shared<IOException>(
        L"a random IOException" +
        (message == L"" ? L"" : L" (" + message + L")"));
    if (LuceneTestCase::VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": MockDirectoryWrapper: now throw random exception"
            << (message == L"" ? L"" : L" (" << message << L")") << endl;
      ioe->printStackTrace(System::out);
    }
    throw ioe;
  }
}

void MockDirectoryWrapper::maybeThrowIOExceptionOnOpen(
    const wstring &name) 
{
  if (randomState->nextDouble() < randomIOExceptionRateOnOpen) {
    if (LuceneTestCase::VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": MockDirectoryWrapper: now throw random exception during "
               L"open file="
            << name << endl;
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
      (runtime_error())->printStackTrace(System::out);
    }
    if (allowRandomFileNotFoundException == false ||
        randomState->nextBoolean()) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"a random IOException (" + name + L")");
    } else {
      // C++ TODO: The following line could not be converted:
      throw randomState.nextBoolean()
          ? new java.io.FileNotFoundException(L"a random IOException (" + name +
                                              L")")
          : new java.nio.file.NoSuchFileException(L"a random IOException (" +
                                                  name + L")");
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t MockDirectoryWrapper::getFileHandleCount()
{
  return openFileHandles.size();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::deleteFile(const wstring &name) 
{
  maybeYield();

  maybeThrowDeterministicException();

  if (crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"cannot delete after crash");
  }

  if (openFiles.find(name) != openFiles.end()) {
    openFilesDeleted->add(name);
    if (assertNoDeleteOpenFile) {
      // C++ TODO: The following line could not be converted:
       fillOpenTrace(
          new java.io.IOException(L"MockDirectoryWrapper: file \"" + name +
                                  L"\" is still open: cannot delete"),
          name, true);
    }
  } else {
    openFilesDeleted->remove(name);
  }

  unSyncedFiles->remove(name);
  in_->deleteFile(name);
  createdFiles->remove(name);
}

// C++ WARNING: The following method was originally marked 'synchronized':
runtime_error MockDirectoryWrapper::fillOpenTrace(runtime_error t,
                                                  const wstring &name,
                                                  bool input)
{
  for (auto ent : openFileHandles) {
    if (input &&
        std::dynamic_pointer_cast<MockIndexInputWrapper>(ent.first) !=
            nullptr &&
        (std::static_pointer_cast<MockIndexInputWrapper>(ent.first))->name ==
            name) {
      t.initCause(ent.second);
      break;
    } else if (!input &&
               std::dynamic_pointer_cast<MockIndexOutputWrapper>(ent.first) !=
                   nullptr &&
               (std::static_pointer_cast<MockIndexOutputWrapper>(ent.first))
                       ->name == name) {
      t.initCause(ent.second);
      break;
    }
  }
  return t;
}

void MockDirectoryWrapper::maybeYield()
{
  if (randomState->nextBoolean()) {
    Thread::yield();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Set<wstring>> MockDirectoryWrapper::getOpenDeletedFiles()
{
  return unordered_set<>(openFilesDeleted);
}

void MockDirectoryWrapper::setFailOnCreateOutput(bool v)
{
  failOnCreateOutput = v;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexOutput> MockDirectoryWrapper::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  maybeThrowDeterministicException();
  maybeThrowIOExceptionOnOpen(name);
  maybeYield();
  if (failOnCreateOutput) {
    maybeThrowDeterministicException();
  }
  if (crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"cannot createOutput after crash");
  }
  init();
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (createdFiles->contains(name) && name != L"segments.gen") {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"file \"" + name +
                                L"\" was already written to");
    }
  }
  if (assertNoDeleteOpenFile && openFiles.find(name) != openFiles.end()) {
    throw make_shared<AssertionError>(L"MockDirectoryWrapper: file \"" + name +
                                      L"\" is still open: cannot overwrite");
  }

  unSyncedFiles->add(name);
  createdFiles->add(name);

  // System.out.println(Thread.currentThread().getName() + ": MDW: create " +
  // name);
  shared_ptr<IndexOutput> delegateOutput = in_->createOutput(
      name, LuceneTestCase::newIOContext(randomState, context));
  shared_ptr<IndexOutput> *const io = make_shared<MockIndexOutputWrapper>(
      shared_from_this(), delegateOutput, name);
  addFileHandle(io, name, Handle::Output);
  openFilesForWrite->add(name);

  // throttling REALLY slows down tests, so don't do it very often for
  // SOMETIMES.
  if (throttling == Throttling::ALWAYS ||
      (throttling == Throttling::SOMETIMES && randomState->nextInt(200) == 0)) {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockDirectoryWrapper: throttling indexOutput (" << name << L")"
            << endl;
    }
    return throttledOutput->newFromDelegate(io);
  } else {
    return io;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexOutput> MockDirectoryWrapper::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  maybeThrowDeterministicException();
  maybeThrowIOExceptionOnOpen(L"temp: prefix=" + prefix + L" suffix=" + suffix);
  maybeYield();
  if (failOnCreateOutput) {
    maybeThrowDeterministicException();
  }
  if (crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"cannot createTempOutput after crash");
  }
  init();

  shared_ptr<IndexOutput> delegateOutput = in_->createTempOutput(
      prefix, suffix, LuceneTestCase::newIOContext(randomState, context));
  wstring name = delegateOutput->getName();
  if (name.toLowerCase(Locale::ROOT)->endsWith(L".tmp") == false) {
    throw make_shared<IllegalStateException>(
        L"wrapped directory failed to use .tmp extension: got: " + name);
  }

  unSyncedFiles->add(name);
  createdFiles->add(name);
  shared_ptr<IndexOutput> *const io = make_shared<MockIndexOutputWrapper>(
      shared_from_this(), delegateOutput, name);
  addFileHandle(io, name, Handle::Output);
  openFilesForWrite->add(name);

  // throttling REALLY slows down tests, so don't do it very often for
  // SOMETIMES.
  if (throttling == Throttling::ALWAYS ||
      (throttling == Throttling::SOMETIMES && randomState->nextInt(200) == 0)) {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockDirectoryWrapper: throttling indexOutput (" << name << L")"
            << endl;
    }
    return throttledOutput->newFromDelegate(io);
  } else {
    return io;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::addFileHandle(shared_ptr<Closeable> c,
                                         const wstring &name, Handle handle)
{
  optional<int> v = openFiles[name];
  if (v) {
    v = static_cast<Integer>(v.value() + 1);
    openFiles.emplace(name, v);
  } else {
    openFiles.emplace(name, static_cast<Integer>(1));
  }

  openFileHandles.emplace(
      c, runtime_error(L"unclosed Index" + handle.name() + L": " + name));
}

void MockDirectoryWrapper::setFailOnOpenInput(bool v) { failOnOpenInput = v; }

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexInput> MockDirectoryWrapper::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  maybeThrowDeterministicException();
  maybeThrowIOExceptionOnOpen(name);
  maybeYield();
  if (failOnOpenInput) {
    maybeThrowDeterministicException();
  }
  if (!LuceneTestCase::slowFileExists(in_, name)) {
    // C++ TODO: The following line could not be converted:
    throw randomState.nextBoolean()
        ? new java.io.FileNotFoundException(name + L" in dir=" + in_)
        : new java.nio.file.NoSuchFileException(name + L" in dir=" + in_);
  }

  // cannot open a file for input if it's still open for
  // output, except for segments.gen and segments_N
  if (!allowReadingFilesStillOpenForWrite &&
      openFilesForWrite->contains(name) &&
      !StringHelper::startsWith(name, L"segments")) {
    // C++ TODO: The following line could not be converted:
     fillOpenTrace(
        new java.io.IOException(L"MockDirectoryWrapper: file \"" + name +
                                L"\" is still open for writing"),
        name, false);
  }

  shared_ptr<IndexInput> delegateInput =
      in_->openInput(name, LuceneTestCase::newIOContext(randomState, context));

  shared_ptr<IndexInput> *const ii;
  int randomInt = randomState->nextInt(500);
  if (useSlowOpenClosers && randomInt == 0) {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockDirectoryWrapper: using SlowClosingMockIndexInputWrapper "
               L"for file "
            << name << endl;
    }
    ii = make_shared<SlowClosingMockIndexInputWrapper>(shared_from_this(), name,
                                                       delegateInput);
  } else if (useSlowOpenClosers && randomInt == 1) {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockDirectoryWrapper: using SlowOpeningMockIndexInputWrapper "
               L"for file "
            << name << endl;
    }
    ii = make_shared<SlowOpeningMockIndexInputWrapper>(shared_from_this(), name,
                                                       delegateInput);
  } else {
    ii = make_shared<MockIndexInputWrapper>(shared_from_this(), name,
                                            delegateInput, nullptr);
  }
  addFileHandle(ii, name, Handle::Input);
  return ii;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t MockDirectoryWrapper::getRecomputedSizeInBytes() 
{
  if (!(std::dynamic_pointer_cast<RAMDirectory>(in_) != nullptr)) {
    return sizeInBytes();
  }
  int64_t size = 0;
  for (auto file : (std::static_pointer_cast<RAMDirectory>(in_))->fileMap) {
    size += file->second.ramBytesUsed();
  }
  return size;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
MockDirectoryWrapper::getRecomputedActualSizeInBytes() 
{
  if (!(std::dynamic_pointer_cast<RAMDirectory>(in_) != nullptr)) {
    return sizeInBytes();
  }
  int64_t size = 0;
  for (auto file : (std::static_pointer_cast<RAMDirectory>(in_))->fileMap) {
    size += file->second->length;
  }
  return size;
}

void MockDirectoryWrapper::setAssertNoUnrefencedFilesOnClose(bool v)
{
  assertNoUnreferencedFilesOnClose = v;
}

// C++ WARNING: The following method was originally marked 'synchronized':
MockDirectoryWrapper::~MockDirectoryWrapper()
{
  if (isOpen_) {
    isOpen_ = false;
  } else {
    delete in_; // but call it again on our wrapped dir
    return;
  }

  bool success = false;
  try {
    // files that we tried to delete, but couldn't because readers were open.
    // all that matters is that we tried! (they will eventually go away)
    //   still open when we tried to delete
    maybeYield();
    if (openFiles.empty()) {
      openFiles = unordered_map<>();
      openFilesDeleted = unordered_set<>();
    }
    if (openFiles.size() > 0) {
      // print the first one as it's very verbose otherwise
      runtime_error cause = nullptr;
      Iterator<runtime_error> stacktraces = openFileHandles.values().begin();
      if (stacktraces->hasNext()) {
        cause = stacktraces->next();
      }
      // RuntimeException instead of IOException because
      // super() does not throw IOException currently:
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("MockDirectoryWrapper: cannot
      // close: there are still " + openFiles.size() + " open files: " +
      // openFiles, cause);
      throw runtime_error(
          L"MockDirectoryWrapper: cannot close: there are still " +
          openFiles.size() + L" open files: " + openFiles);
    }
    if (openLocks->size() > 0) {
      runtime_error cause = nullptr;
      Iterator<runtime_error> stacktraces = openLocks->values().begin();
      if (stacktraces->hasNext()) {
        cause = stacktraces->next();
      }
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("MockDirectoryWrapper: cannot
      // close: there are still open locks: " + openLocks, cause);
      throw runtime_error(
          L"MockDirectoryWrapper: cannot close: there are still open locks: " +
          openLocks);
    }
    randomIOExceptionRate = 0.0;
    randomIOExceptionRateOnOpen = 0.0;

    if ((getCheckIndexOnClose() || assertNoUnreferencedFilesOnClose) &&
        DirectoryReader::indexExists(shared_from_this())) {
      if (getCheckIndexOnClose()) {

        if (LuceneTestCase::VERBOSE) {
          wcout << L"\nNOTE: MockDirectoryWrapper: now crush" << endl;
        }
        crash(); // corrupt any unsynced-files
        if (LuceneTestCase::VERBOSE) {
          wcout << L"\nNOTE: MockDirectoryWrapper: now run CheckIndex" << endl;
        }

        TestUtil::checkIndex(shared_from_this(),
                             getCrossCheckTermVectorsOnClose(), true, nullptr);
      }

      // TODO: factor this out / share w/ TestIW.assertNoUnreferencedFiles
      if (assertNoUnreferencedFilesOnClose) {
        wcout << L"MDW: now assert no unref'd files at close" << endl;

        // now look for unreferenced files: discount ones that we tried to
        // delete but could not
        shared_ptr<Set<wstring>> allFiles =
            unordered_set<wstring>(Arrays::asList(listAll()));
        std::deque<wstring> startFiles =
            allFiles->toArray(std::deque<wstring>(0));
        shared_ptr<IndexWriterConfig> iwc =
            make_shared<IndexWriterConfig>(nullptr);
        iwc->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE);

        // We must do this before opening writer otherwise writer will be angry
        // if there are pending deletions:
        TestUtil::disableVirusChecker(in_);

        (make_shared<IndexWriter>(in_, iwc))->rollback();
        std::deque<wstring> endFiles = in_->listAll();

        shared_ptr<Set<wstring>> startSet =
            set<wstring>(Arrays::asList(startFiles));
        shared_ptr<Set<wstring>> endSet =
            set<wstring>(Arrays::asList(endFiles));

        startFiles = startSet->toArray(std::deque<wstring>(0));
        endFiles = endSet->toArray(std::deque<wstring>(0));

        if (!Arrays::equals(startFiles, endFiles)) {
          deque<wstring> removed = deque<wstring>();
          for (auto fileName : startFiles) {
            if (!endSet->contains(fileName)) {
              removed.push_back(fileName);
            }
          }

          deque<wstring> added = deque<wstring>();
          for (auto fileName : endFiles) {
            if (!startSet->contains(fileName)) {
              added.push_back(fileName);
            }
          }

          wstring extras;
          if (removed.size() != 0) {
            extras = L"\n\nThese files were removed: " + removed;
          } else {
            extras = L"";
          }

          if (added.size() != 0) {
            extras += L"\n\nThese files were added (waaaaaaaaaat!): " + added;
          }

          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw runtime_error(L"unreferenced files: before delete:\n    " +
                              Arrays->toString(startFiles) +
                              L"\n  after delete:\n    " +
                              Arrays->toString(endFiles) + extras);
        }

        shared_ptr<DirectoryReader> ir1 =
            DirectoryReader::open(shared_from_this());
        int numDocs1 = ir1->numDocs();
        ir1->close();
        delete (make_shared<IndexWriter>(
            shared_from_this(), make_shared<IndexWriterConfig>(nullptr)));
        shared_ptr<DirectoryReader> ir2 =
            DirectoryReader::open(shared_from_this());
        int numDocs2 = ir2->numDocs();
        ir2->close();
        assert((numDocs1 == numDocs2,
                L"numDocs changed after opening/closing IW: before=" +
                    to_wstring(numDocs1) + L" after=" + to_wstring(numDocs2)));
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({in_});
    } else {
      IOUtils::closeWhileHandlingException({in_});
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::removeOpenFile(shared_ptr<Closeable> c,
                                          const wstring &name)
{
  optional<int> v = openFiles[name];
  // Could be null when crash() was called
  if (v) {
    if (v.value() == 1) {
      openFiles.erase(name);
    } else {
      v = static_cast<Integer>(v.value() - 1);
      openFiles.emplace(name, v);
    }
  }

  openFileHandles.erase(c);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::removeIndexOutput(shared_ptr<IndexOutput> out,
                                             const wstring &name)
{
  openFilesForWrite->remove(name);
  removeOpenFile(out, name);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::removeIndexInput(shared_ptr<IndexInput> in_,
                                            const wstring &name)
{
  removeOpenFile(in_, name);
}

void MockDirectoryWrapper::Failure::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
}

shared_ptr<Failure> MockDirectoryWrapper::Failure::reset()
{
  return shared_from_this();
}

void MockDirectoryWrapper::Failure::setDoFail() { doFail = true; }

void MockDirectoryWrapper::Failure::clearDoFail() { doFail = false; }

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::failOn(shared_ptr<Failure> fail)
{
  if (failures.empty()) {
    failures = deque<std::shared_ptr<Failure>>();
  }
  failures.push_back(fail);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MockDirectoryWrapper::maybeThrowDeterministicException() 
{
  if (failures.size() > 0) {
    for (int i = 0; i < failures.size(); i++) {
      try {
        failures[i]->eval(shared_from_this());
      } catch (const runtime_error &t) {
        if (LuceneTestCase::VERBOSE) {
          wcout << L"MockDirectoryWrapper: throw exc" << endl;
          t.printStackTrace(System::out);
        }
        throw IOUtils::rethrowAlways(t);
      }
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<wstring> MockDirectoryWrapper::listAll() 
{
  maybeYield();
  return in_->listAll();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
MockDirectoryWrapper::fileLength(const wstring &name) 
{
  maybeYield();
  return in_->fileLength(name);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Lock>
MockDirectoryWrapper::obtainLock(const wstring &name) 
{
  maybeYield();
  return BaseDirectoryWrapper::obtainLock(name);
  // TODO: consider mocking locks, but not all the time, can hide bugs
}

wstring MockDirectoryWrapper::toString()
{
  if (maxSize != 0) {
    return L"MockDirectoryWrapper(" + in_ + L", current=" +
           to_wstring(maxUsedSize) + L",max=" + to_wstring(maxSize) + L")";
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return BaseDirectoryWrapper::toString();
  }
}

shared_ptr<ChecksumIndexInput> MockDirectoryWrapper::openChecksumInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  return BaseDirectoryWrapper::openChecksumInput(name, context);
}

void MockDirectoryWrapper::copyFrom(
    shared_ptr<Directory> from, const wstring &src, const wstring &dest,
    shared_ptr<IOContext> context) 
{
  BaseDirectoryWrapper::copyFrom(from, src, dest, context);
}

void MockDirectoryWrapper::ensureOpen() 
{
  BaseDirectoryWrapper::ensureOpen();
}
} // namespace org::apache::lucene::store