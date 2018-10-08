using namespace std;

#include "SimpleFSLockFactory.h"

namespace org::apache::lucene::store
{

const shared_ptr<SimpleFSLockFactory> SimpleFSLockFactory::INSTANCE =
    make_shared<SimpleFSLockFactory>();

SimpleFSLockFactory::SimpleFSLockFactory() {}

shared_ptr<Lock>
SimpleFSLockFactory::obtainFSLock(shared_ptr<FSDirectory> dir,
                                  const wstring &lockName) 
{
  shared_ptr<Path> lockDir = dir->getDirectory();

  // Ensure that lockDir exists and is a directory.
  // note: this will fail if lockDir is a symlink
  Files::createDirectories(lockDir);

  shared_ptr<Path> lockFile = lockDir->resolve(lockName);

  // create the file: this will fail if it already exists
  try {
    Files::createFile(lockFile);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (FileAlreadyExistsException | AccessDeniedException e) {
    // convert optional specific exception to our optional specific exception
    throw make_shared<LockObtainFailedException>(
        L"Lock held elsewhere: " + lockFile, e);
  }

  // used as a best-effort check, to see if the underlying file has changed
  shared_ptr<FileTime> *const creationTime =
      Files::readAttributes(lockFile, BasicFileAttributes::typeid)
          .creationTime();

  return make_shared<SimpleFSLock>(lockFile, creationTime);
}

SimpleFSLockFactory::SimpleFSLock::SimpleFSLock(
    shared_ptr<Path> path, shared_ptr<FileTime> creationTime) 
    : path(path), creationTime(creationTime)
{
}

void SimpleFSLockFactory::SimpleFSLock::ensureValid() 
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"Lock instance already released: " + shared_from_this());
  }
  // try to validate the backing file name, that it still exists,
  // and has the same creation time as when we obtained the lock.
  // if it differs, someone deleted our lock file (and we are ineffective)
  shared_ptr<FileTime> ctime =
      Files::readAttributes(path, BasicFileAttributes::typeid).creationTime();
  if (!creationTime->equals(ctime)) {
    throw make_shared<AlreadyClosedException>(
        L"Underlying file changed by an external force at " + ctime +
        L", (lock=" + shared_from_this() + L")");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
SimpleFSLockFactory::SimpleFSLock::~SimpleFSLock()
{
  if (closed) {
    return;
  }
  try {
    // NOTE: unlike NativeFSLockFactory, we can potentially delete someone
    // else's lock if things have gone wrong. we do best-effort check
    // (ensureValid) to avoid doing this.
    try {
      ensureValid();
    } catch (const runtime_error &exc) {
      // notify the user they may need to intervene.
      throw make_shared<LockReleaseFailedException>(
          L"Lock file cannot be safely removed. Manual intervention is "
          L"recommended.",
          exc);
    }
    // we did a best effort check, now try to remove the file. if something goes
    // wrong, we need to make it clear to the user that the directory may still
    // remain locked.
    try {
      Files::delete (path);
    } catch (const runtime_error &exc) {
      throw make_shared<LockReleaseFailedException>(
          L"Unable to remove lock file. Manual intervention is recommended",
          exc);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    closed = true;
  }
}

wstring SimpleFSLockFactory::SimpleFSLock::toString()
{
  return L"SimpleFSLock(path=" + path + L",creationTime=" + creationTime + L")";
}
} // namespace org::apache::lucene::store