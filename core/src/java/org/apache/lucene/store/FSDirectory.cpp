using namespace std;

#include "FSDirectory.h"

namespace org::apache::lucene::store
{
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;

FSDirectory::FSDirectory(shared_ptr<Path> path,
                         shared_ptr<LockFactory> lockFactory) 
    : BaseDirectory(lockFactory), directory(path->toRealPath())
{
  // If only read access is permitted, createDirectories fails even if the
  // directory already exists.
  if (!Files::isDirectory(path)) {
    Files::createDirectories(path); // create directory, if it doesn't exist
  }
}

shared_ptr<FSDirectory>
FSDirectory::open(shared_ptr<Path> path) 
{
  return open(path, FSLockFactory::getDefault());
}

shared_ptr<FSDirectory>
FSDirectory::open(shared_ptr<Path> path,
                  shared_ptr<LockFactory> lockFactory) 
{
  if (Constants::JRE_IS_64BIT && MMapDirectory::UNMAP_SUPPORTED) {
    return make_shared<MMapDirectory>(path, lockFactory);
  } else if (Constants::WINDOWS) {
    return make_shared<SimpleFSDirectory>(path, lockFactory);
  } else {
    return make_shared<NIOFSDirectory>(path, lockFactory);
  }
}

std::deque<wstring>
FSDirectory::listAll(shared_ptr<Path> dir) 
{
  return listAll(dir, nullptr);
}

std::deque<wstring>
FSDirectory::listAll(shared_ptr<Path> dir,
                     shared_ptr<Set<wstring>> skipNames) 
{
  deque<wstring> entries = deque<wstring>();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dir);
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring name = path->getFileName()->toString();
      if (skipNames == nullptr || skipNames->contains(name) == false) {
        entries.push_back(name);
      }
    }
  }

  std::deque<wstring> array_ =
      entries.toArray(std::deque<wstring>(entries.size()));
  // Directory.listAll javadocs state that we sort the results here, so we don't
  // let filesystem specifics leak out of this abstraction:
  Arrays::sort(array_);
  return array_;
}

std::deque<wstring> FSDirectory::listAll() 
{
  ensureOpen();
  return listAll(directory, pendingDeletes);
}

int64_t FSDirectory::fileLength(const wstring &name) 
{
  ensureOpen();
  if (pendingDeletes->contains(name)) {
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.NoSuchFileException(L"file \"" + name +
                                            L"\" is pending delete");
  }
  return Files->size(directory->resolve(name));
}

shared_ptr<IndexOutput>
FSDirectory::createOutput(const wstring &name,
                          shared_ptr<IOContext> context) 
{
  ensureOpen();
  maybeDeletePendingFiles();
  // If this file was pending delete, we are now bringing it back to life:
  if (pendingDeletes->remove(name)) {
    privateDeleteFile(name,
                      true); // try again to delete it - this is best effort
    pendingDeletes->remove(name); // watch out - if the delete fails it put
  }
  return make_shared<FSIndexOutput>(shared_from_this(), name);
}

shared_ptr<IndexOutput>
FSDirectory::createTempOutput(const wstring &prefix, const wstring &suffix,
                              shared_ptr<IOContext> context) 
{
  ensureOpen();
  maybeDeletePendingFiles();
  while (true) {
    try {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring name = IndexFileNames::segmentFileName(
          prefix,
          suffix + L"_" +
              Long::toString(nextTempFileCounter->getAndIncrement(),
                             Character::MAX_RADIX),
          L"tmp");
      if (pendingDeletes->contains(name)) {
        continue;
      }
      return make_shared<FSIndexOutput>(shared_from_this(), name,
                                        StandardOpenOption::WRITE,
                                        StandardOpenOption::CREATE_NEW);
    } catch (const FileAlreadyExistsException &faee) {
      // Retry with next incremented name
    }
  }
}

void FSDirectory::ensureCanRead(const wstring &name) 
{
  if (pendingDeletes->contains(name)) {
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.NoSuchFileException(
        L"file \"" + name +
        L"\" is pending delete and cannot be opened for read");
  }
}

void FSDirectory::sync(shared_ptr<deque<wstring>> names) 
{
  ensureOpen();

  for (auto name : names) {
    fsync(name);
  }
  maybeDeletePendingFiles();
}

void FSDirectory::rename(const wstring &source,
                         const wstring &dest) 
{
  ensureOpen();
  if (pendingDeletes->contains(source)) {
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.NoSuchFileException(
        L"file \"" + source + L"\" is pending delete and cannot be moved");
  }
  maybeDeletePendingFiles();
  if (pendingDeletes->remove(dest)) {
    privateDeleteFile(dest,
                      true); // try again to delete it - this is best effort
    pendingDeletes->remove(
        dest); // watch out if the delete fails it's back in here.
  }
  Files::move(directory->resolve(source), directory->resolve(dest),
              StandardCopyOption::ATOMIC_MOVE);
}

void FSDirectory::syncMetaData() 
{
  // TODO: to improve listCommits(), IndexFileDeleter could call this after
  // deleting segments_Ns
  ensureOpen();
  IOUtils::fsync(directory, true);
  maybeDeletePendingFiles();
}

// C++ WARNING: The following method was originally marked 'synchronized':
FSDirectory::~FSDirectory()
{
  isOpen = false;
  deletePendingFiles();
}

shared_ptr<Path> FSDirectory::getDirectory()
{
  ensureOpen();
  return directory;
}

wstring FSDirectory::toString()
{
  return this->getClass().getSimpleName() + L"@" + directory +
         L" lockFactory=" + lockFactory;
}

void FSDirectory::fsync(const wstring &name) 
{
  IOUtils::fsync(directory->resolve(name), false);
}

void FSDirectory::deleteFile(const wstring &name) 
{
  if (pendingDeletes->contains(name)) {
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.NoSuchFileException(L"file \"" + name +
                                            L"\" is already pending delete");
  }
  privateDeleteFile(name, false);
  maybeDeletePendingFiles();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FSDirectory::deletePendingFiles() 
{
  if (pendingDeletes->isEmpty() == false) {

    // TODO: we could fix IndexInputs from FSDirectory subclasses to call this
    // when they are closed?

    // Clone the set since we mutate it in privateDeleteFile:
    for (auto name : unordered_set<>(pendingDeletes)) {
      privateDeleteFile(name, true);
    }
  }
}

void FSDirectory::maybeDeletePendingFiles() 
{
  if (pendingDeletes->isEmpty() == false) {
    // This is a silly heuristic to try to avoid O(N^2), where N = number of
    // files pending deletion, behaviour on Windows:
    int count = opsSinceLastDelete->incrementAndGet();
    if (count >= pendingDeletes->size()) {
      opsSinceLastDelete->addAndGet(-count);
      deletePendingFiles();
    }
  }
}

void FSDirectory::privateDeleteFile(const wstring &name,
                                    bool isPendingDelete) 
{
  try {
    Files::delete (directory->resolve(name));
    pendingDeletes->remove(name);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | FileNotFoundException e) {
    // We were asked to delete a non-existent file:
    pendingDeletes->remove(name);
    if (isPendingDelete && Constants::WINDOWS) {
      // TODO: can we remove this OS-specific hacky logic?  If windows
      // deleteFile is buggy, we should instead contain this workaround in a
      // WindowsFSDirectory ... LUCENE-6684: we suppress this check for Windows,
      // since a file could be in a confusing "pending delete" state, failing
      // the first delete attempt with access denied and then apparently falsely
      // failing here when we try ot delete it again, with NSFE/FNFE
    } else {
      throw e;
    }
  } catch (const IOException &ioe) {
    // On windows, a file delete can fail because there's still an open
    // file handle against it.  We record this in pendingDeletes and
    // try again later.

    // TODO: this is hacky/lenient (we don't know which IOException this is),
    // and it should only happen on filesystems that can do this, so really we
    // should move this logic to WindowsDirectory or something

    // TODO: can/should we do if (Constants.WINDOWS) here, else throw the exc?
    // but what about a Linux box with a CIFS mount?
    pendingDeletes->add(name);
  }
}

FSDirectory::FSIndexOutput::FSIndexOutput(
    shared_ptr<FSDirectory> outerInstance,
    const wstring &name) 
    : FSIndexOutput(outerInstance, name, StandardOpenOption::WRITE,
                    StandardOpenOption::CREATE_NEW)
{
}

FSDirectory::FSIndexOutput::FSIndexOutput(
    shared_ptr<FSDirectory> outerInstance, const wstring &name,
    deque<OpenOption> &options) 
    : outerInstance(outerInstance)
{
  OutputStreamIndexOutput(
      L"FSIndexOutput(path=\"" + outerInstance->directory->resolve(name) +
          L"\")",
      name,
      make_shared<FilterOutputStreamAnonymousInnerClass>(Files::newOutputStream(
          outerInstance->directory->resolve(name), options)),
      CHUNK_SIZE);
}

FSDirectory::FSIndexOutput::FilterOutputStreamAnonymousInnerClass::
    FilterOutputStreamAnonymousInnerClass(
        shared_ptr<UnknownType> newOutputStream)
    : java::io::FilterOutputStream(newOutputStream)
{
}

void FSDirectory::FSIndexOutput::FilterOutputStreamAnonymousInnerClass::write(
    std::deque<char> &b, int offset, int length) 
{
  while (length > 0) {
    constexpr int chunk = min(length, CHUNK_SIZE);
    out::write(b, offset, chunk);
    length -= chunk;
    offset += chunk;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Set<wstring>> FSDirectory::getPendingDeletions() 
{
  deletePendingFiles();
  if (pendingDeletes->isEmpty()) {
    return Collections::emptySet();
  } else {
    return Collections::unmodifiableSet(unordered_set<>(pendingDeletes));
  }
}
} // namespace org::apache::lucene::store