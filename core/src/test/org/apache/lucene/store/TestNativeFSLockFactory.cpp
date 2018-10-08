using namespace std;

#include "TestNativeFSLockFactory.h"

namespace org::apache::lucene::store
{
using FilterFileSystemProvider =
    org::apache::lucene::mockfile::FilterFileSystemProvider;
using FilterPath = org::apache::lucene::mockfile::FilterPath;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Directory>
TestNativeFSLockFactory::getDirectory(shared_ptr<Path> path) 
{
  return newFSDirectory(path, NativeFSLockFactory::INSTANCE);
}

void TestNativeFSLockFactory::testLockFileExists() 
{
  shared_ptr<Path> tempDir = createTempDir();
  shared_ptr<Path> lockFile = tempDir->resolve(L"test.lock");
  Files::createFile(lockFile);

  shared_ptr<Directory> dir = getDirectory(tempDir);
  shared_ptr<Lock> l = dir->obtainLock(L"test.lock");
  delete l;
  delete dir;
}

void TestNativeFSLockFactory::testInvalidateLock() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  shared_ptr<NativeFSLockFactory::NativeFSLock> lock =
      std::static_pointer_cast<NativeFSLockFactory::NativeFSLock>(
          dir->obtainLock(L"test.lock"));
  lock->ensureValid();
  lock->lock->release();
  expectThrows(AlreadyClosedException::typeid, [&]() { lock->ensureValid(); });

  IOUtils::closeWhileHandlingException({lock});
  delete dir;
}

void TestNativeFSLockFactory::testInvalidateChannel() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  shared_ptr<NativeFSLockFactory::NativeFSLock> lock =
      std::static_pointer_cast<NativeFSLockFactory::NativeFSLock>(
          dir->obtainLock(L"test.lock"));
  lock->ensureValid();
  lock->channel->close();
  expectThrows(AlreadyClosedException::typeid, [&]() { lock->ensureValid(); });

  IOUtils::closeWhileHandlingException({lock});
  delete dir;
}

void TestNativeFSLockFactory::testDeleteLockFile() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Directory dir =
  // getDirectory(createTempDir()))
  {
    Directory dir = getDirectory(createTempDir());
    assumeFalse(L"we must be able to delete an open file",
                TestUtil::hasWindowsFS(dir));

    shared_ptr<Lock> lock = dir->obtainLock(L"test.lock");
    lock->ensureValid();

    dir->deleteFile(L"test.lock");

    expectThrows(IOException::typeid, [&]() { lock->ensureValid(); });

    IOUtils::closeWhileHandlingException({lock});
  }
}

TestNativeFSLockFactory::MockBadPermissionsFileSystem::
    MockBadPermissionsFileSystem(shared_ptr<FileSystem> delegateInstance)
    : org::apache::lucene::mockfile::FilterFileSystemProvider(
          L"mockbadpermissions://", delegateInstance)
{
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<SeekableByteChannel> TestNativeFSLockFactory::MockBadPermissionsFileSystem::newByteChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (path->getFileName()->toString()->equals(L"test.lock")) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<AccessDeniedException>(path->toString(), nullptr,
                                             L"fake access denied");
  }
  return FilterFileSystemProvider::newByteChannel(path, options, {attrs});
}

void TestNativeFSLockFactory::testBadPermissions() 
{
  // create a mock filesystem that will throw exc on creating test.lock
  shared_ptr<Path> tmpDir = createTempDir();
  tmpDir = FilterPath::unwrap(tmpDir)->toRealPath();
  shared_ptr<FileSystem> mock =
      (make_shared<MockBadPermissionsFileSystem>(tmpDir->getFileSystem()))
          ->getFileSystem(nullptr);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mock->getPath(tmpDir->toString());

  // we should get an IOException (typically NoSuchFileException but no
  // guarantee) with our fake AccessDenied added as suppressed.
  shared_ptr<Directory> dir = getDirectory(mockPath->resolve(L"indexDir"));
  shared_ptr<IOException> expected = expectThrows(
      IOException::typeid, [&]() { dir->obtainLock(L"test.lock"); });
  shared_ptr<AccessDeniedException> suppressed =
      std::static_pointer_cast<AccessDeniedException>(
          expected->getSuppressed()[0]);
  assertTrue(suppressed->getMessage()->contains(L"fake access denied"));

  delete dir;
}
} // namespace org::apache::lucene::store