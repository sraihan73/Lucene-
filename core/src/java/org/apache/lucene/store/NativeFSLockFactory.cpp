using namespace std;

#include "NativeFSLockFactory.h"

namespace org::apache::lucene::store
{
using IOUtils = org::apache::lucene::util::IOUtils;
const shared_ptr<NativeFSLockFactory> NativeFSLockFactory::INSTANCE =
    make_shared<NativeFSLockFactory>();
const shared_ptr<java::util::Set<wstring>> NativeFSLockFactory::LOCK_HELD =
    java::util::Collections::synchronizedSet(unordered_set<wstring>());

NativeFSLockFactory::NativeFSLockFactory() {}

shared_ptr<Lock>
NativeFSLockFactory::obtainFSLock(shared_ptr<FSDirectory> dir,
                                  const wstring &lockName) 
{
  shared_ptr<Path> lockDir = dir->getDirectory();

  // Ensure that lockDir exists and is a directory.
  // note: this will fail if lockDir is a symlink
  Files::createDirectories(lockDir);

  shared_ptr<Path> lockFile = lockDir->resolve(lockName);

  shared_ptr<IOException> creationException = nullptr;
  try {
    Files::createFile(lockFile);
  } catch (const IOException &ignore) {
    // we must create the file to have a truly canonical path.
    // if it's already created, we don't care. if it cant be created, it will
    // fail below.
    creationException = ignore;
  }

  // fails if the lock file does not exist
  shared_ptr<Path> *const realPath;
  try {
    realPath = lockFile->toRealPath();
  } catch (const IOException &e) {
    // if we couldn't resolve the lock file, it might be because we couldn't
    // create it. so append any exception from createFile as a suppressed
    // exception, in case its useful
    if (creationException != nullptr) {
      e->addSuppressed(creationException);
    }
    throw e;
  }

  // used as a best-effort check, to see if the underlying file has changed
  shared_ptr<FileTime> *const creationTime =
      Files::readAttributes(realPath, BasicFileAttributes::typeid)
          .creationTime();

  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (LOCK_HELD->add(realPath->toString())) {
    shared_ptr<FileChannel> channel = nullptr;
    shared_ptr<FileLock> lock = nullptr;
    try {
      channel = FileChannel::open(realPath, StandardOpenOption::CREATE,
                                  StandardOpenOption::WRITE);
      lock = channel->tryLock();
      if (lock != nullptr) {
        return make_shared<NativeFSLock>(lock, channel, realPath, creationTime);
      } else {
        throw make_shared<LockObtainFailedException>(
            L"Lock held by another program: " + realPath);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (lock == nullptr) { // not successful - clear up and move out
        IOUtils::closeWhileHandlingException({channel}); // TODO: addSuppressed
        clearLockHeld(realPath);                         // clear LOCK_HELD last
      }
    }
  } else {
    throw make_shared<LockObtainFailedException>(
        L"Lock held by this virtual machine: " + realPath);
  }
}

void NativeFSLockFactory::clearLockHeld(shared_ptr<Path> path) throw(
    IOException)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  bool remove = LOCK_HELD->remove(path->toString());
  if (remove == false) {
    throw make_shared<AlreadyClosedException>(
        L"Lock path was cleared but never marked as held: " + path);
  }
}

NativeFSLockFactory::NativeFSLock::NativeFSLock(
    shared_ptr<FileLock> lock, shared_ptr<FileChannel> channel,
    shared_ptr<Path> path, shared_ptr<FileTime> creationTime)
    : lock(lock), channel(channel), path(path), creationTime(creationTime)
{
}

void NativeFSLockFactory::NativeFSLock::ensureValid() 
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"Lock instance already released: " + shared_from_this());
  }
  // check we are still in the locks map_obj (some debugger or something crazy
  // didn't remove us)
  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (!LOCK_HELD->contains(path->toString())) {
    throw make_shared<AlreadyClosedException>(
        L"Lock path unexpectedly cleared from map_obj: " + shared_from_this());
  }
  // check our lock wasn't invalidated.
  if (!lock->isValid()) {
    throw make_shared<AlreadyClosedException>(
        L"FileLock invalidated by an external force: " + shared_from_this());
  }
  // try to validate the underlying file descriptor.
  // this will throw IOException if something is wrong.
  int64_t size = channel->size();
  if (size != 0) {
    throw make_shared<AlreadyClosedException>(L"Unexpected lock file size: " +
                                              to_wstring(size) + L", (lock=" +
                                              shared_from_this() + L")");
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
NativeFSLockFactory::NativeFSLock::~NativeFSLock()
{
  if (closed) {
    return;
  }
  // NOTE: we don't validate, as unlike SimpleFSLockFactory, we can't break
  // others locks first release the lock, then the channel
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.nio.channels.FileChannel channel =
  // this.channel; java.nio.channels.FileLock lock = this.lock)
  {
    java::nio::channels::FileChannel channel = this->channel;
    java::nio::channels::FileLock lock = this->lock;
    try {
      assert(lock != nullptr);
      assert(channel != nullptr);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      closed = true;
      clearLockHeld(path);
    }
  }
}

wstring NativeFSLockFactory::NativeFSLock::toString()
{
  return L"NativeFSLock(path=" + path + L",impl=" + lock + L",creationTime=" +
         creationTime + L")";
}
} // namespace org::apache::lucene::store