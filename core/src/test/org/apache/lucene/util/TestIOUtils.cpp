using namespace std;

#include "TestIOUtils.h"

namespace org::apache::lucene::util
{
using FilterFileSystem = org::apache::lucene::mockfile::FilterFileSystem;
using FilterFileSystemProvider =
    org::apache::lucene::mockfile::FilterFileSystemProvider;
using FilterPath = org::apache::lucene::mockfile::FilterPath;

void TestIOUtils::testDeleteFileIgnoringExceptions() 
{
  shared_ptr<Path> dir = createTempDir();
  shared_ptr<Path> file1 = dir->resolve(L"file1");
  Files::createFile(file1);
  IOUtils::deleteFilesIgnoringExceptions({file1});
  assertFalse(Files::exists(file1));
  // actually deletes
}

void TestIOUtils::testDontDeleteFileIgnoringExceptions() 
{
  shared_ptr<Path> dir = createTempDir();
  shared_ptr<Path> file1 = dir->resolve(L"file1");
  IOUtils::deleteFilesIgnoringExceptions({file1});
  // no exception
}

void TestIOUtils::testDeleteTwoFilesIgnoringExceptions() 
{
  shared_ptr<Path> dir = createTempDir();
  shared_ptr<Path> file1 = dir->resolve(L"file1");
  shared_ptr<Path> file2 = dir->resolve(L"file2");
  // only create file2
  Files::createFile(file2);
  IOUtils::deleteFilesIgnoringExceptions({file1, file2});
  assertFalse(Files::exists(file2));
  // no exception
  // actually deletes file2
}

void TestIOUtils::testDeleteFileIfExists() 
{
  shared_ptr<Path> dir = createTempDir();
  shared_ptr<Path> file1 = dir->resolve(L"file1");
  Files::createFile(file1);
  IOUtils::deleteFilesIfExist({file1});
  assertFalse(Files::exists(file1));
  // actually deletes
}

void TestIOUtils::testDontDeleteDoesntExist() 
{
  shared_ptr<Path> dir = createTempDir();
  shared_ptr<Path> file1 = dir->resolve(L"file1");
  IOUtils::deleteFilesIfExist({file1});
  // no exception
}

void TestIOUtils::testDeleteTwoFilesIfExist() 
{
  shared_ptr<Path> dir = createTempDir();
  shared_ptr<Path> file1 = dir->resolve(L"file1");
  shared_ptr<Path> file2 = dir->resolve(L"file2");
  // only create file2
  Files::createFile(file2);
  IOUtils::deleteFilesIfExist({file1, file2});
  assertFalse(Files::exists(file2));
  // no exception
  // actually deletes file2
}

void TestIOUtils::testSpinsBasics() 
{
  shared_ptr<Path> dir = createTempDir();
  // no exception, directory exists
  IOUtils::spins(dir);
  shared_ptr<Path> file = dir->resolve(L"exists");
  Files::createFile(file);
  // no exception, file exists
  IOUtils::spins(file);

  // exception: file doesn't exist
  shared_ptr<Path> fake = dir->resolve(L"nonexistent");
  expectThrows(IOException::typeid, [&]() { IOUtils::spins(fake); });
}

TestIOUtils::MockFileStore::MockFileStore(const wstring &description,
                                          const wstring &type,
                                          const wstring &name)
    : description(description), type(type), name(name)
{
}

wstring TestIOUtils::MockFileStore::type() { return type_; }

wstring TestIOUtils::MockFileStore::name() { return name_; }

wstring TestIOUtils::MockFileStore::toString() { return description; }

bool TestIOUtils::MockFileStore::isReadOnly() { return false; }

int64_t TestIOUtils::MockFileStore::getTotalSpace() 
{
  return 1000;
}

int64_t TestIOUtils::MockFileStore::getUsableSpace() 
{
  return 800;
}

int64_t TestIOUtils::MockFileStore::getUnallocatedSpace() 
{
  return 1000;
}

bool TestIOUtils::MockFileStore::supportsFileAttributeView(type_info type)
{
  return false;
}

bool TestIOUtils::MockFileStore::supportsFileAttributeView(const wstring &name)
{
  return false;
}

template <typename V>
V TestIOUtils::MockFileStore::getFileStoreAttributeView(type_info<V> &type)
{
  static_assert(
      is_base_of<java.nio.file.attribute.FileStoreAttributeView, V>::value,
      L"V must inherit from java.nio.file.attribute.FileStoreAttributeView");

  return nullptr;
}

any TestIOUtils::MockFileStore::getAttribute(const wstring &attribute) throw(
    IOException)
{
  return nullptr;
}

void TestIOUtils::testGetMountPoint() 
{
  assertEquals(L"/", IOUtils::getMountPoint(make_shared<MockFileStore>(
                         L"/ (/dev/sda1)", L"ext4", L"/dev/sda1")));
  assertEquals(
      L"/test/ space(((trash)))/",
      IOUtils::getMountPoint(make_shared<MockFileStore>(
          L"/test/ space(((trash)))/ (/dev/sda1)", L"ext3", L"/dev/sda1")));
  assertEquals(L"/", IOUtils::getMountPoint(make_shared<MockFileStore>(
                         L"/ (notreal)", L"ext2", L"notreal")));
}

TestIOUtils::MockLinuxFileSystemProvider::MockLinuxFileSystemProvider(
    shared_ptr<FileSystem> delegateInstance,
    unordered_map<wstring, std::shared_ptr<FileStore>> &filesToStore,
    shared_ptr<Path> root)
    : org::apache::lucene::mockfile::FilterFileSystemProvider(L"mocklinux://",
                                                              delegateInstance),
      filesToStore(filesToStore),
      root(make_shared<MockLinuxPath>(shared_from_this(), root,
                                      this->fileSystem))
{
  shared_ptr<deque<std::shared_ptr<FileStore>>> *const allStores =
      unordered_set<std::shared_ptr<FileStore>>(filesToStore.values());
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  this->fileSystem = make_shared<FilterFileSystemAnonymousInnerClass>(
      shared_from_this(), delegateInstance, allStores);
}

TestIOUtils::MockLinuxFileSystemProvider::FilterFileSystemAnonymousInnerClass::
    FilterFileSystemAnonymousInnerClass(
        shared_ptr<MockLinuxFileSystemProvider> outerInstance,
        shared_ptr<FileSystem> delegateInstance,
        shared_ptr<deque<std::shared_ptr<FileStore>>> allStores)
    : org::apache::lucene::mockfile::FilterFileSystem(outerInstance,
                                                      delegateInstance)
{
  this->outerInstance = outerInstance;
  this->delegateInstance = delegateInstance;
  this->allStores = allStores;
}

deque<std::shared_ptr<FileStore>> TestIOUtils::MockLinuxFileSystemProvider::
    FilterFileSystemAnonymousInnerClass::getFileStores()
{
  return allStores;
}

shared_ptr<Path> TestIOUtils::MockLinuxFileSystemProvider::
    FilterFileSystemAnonymousInnerClass::getPath(const wstring &first,
                                                 deque<wstring> &more)
{
  return make_shared<MockLinuxPath>(outerInstance,
                                    delegateInstance->getPath(first, more),
                                    shared_from_this());
}

shared_ptr<FileStore> TestIOUtils::MockLinuxFileSystemProvider::getFileStore(
    shared_ptr<Path> path) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> ret = filesToStore[path->toString()];
  if (ret == nullptr) {
    throw invalid_argument(L"this mock doesnt know wtf to do with: " + path);
  }
  // act like the linux fs provider here, return a crazy rootfs one
  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (ret->toString()->startsWith(root + L" (")) {
    return make_shared<MockFileStore>(root + L" (rootfs)", L"rootfs",
                                      L"rootfs");
  }

  return ret;
}

shared_ptr<Path>
TestIOUtils::MockLinuxFileSystemProvider::maybeChroot(shared_ptr<Path> path)
{
  if (path->toAbsolutePath()->startsWith(L"/sys") ||
      path->toAbsolutePath()->startsWith(L"/dev")) {
    // map_obj to our chrooted location;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return path->getRoot().resolve(root).resolve(path->toString()->substr(1));
  } else {
    return path;
  }
}

shared_ptr<Path>
TestIOUtils::MockLinuxFileSystemProvider::toDelegate(shared_ptr<Path> path)
{
  return FilterFileSystemProvider::toDelegate(maybeChroot(path));
}

TestIOUtils::MockLinuxFileSystemProvider::MockLinuxPath::MockLinuxPath(
    shared_ptr<TestIOUtils::MockLinuxFileSystemProvider> outerInstance,
    shared_ptr<Path> delegate_, shared_ptr<FileSystem> fileSystem)
    : org::apache::lucene::mockfile::FilterPath(delegate_, fileSystem),
      outerInstance(outerInstance)
{
}

shared_ptr<Path>
TestIOUtils::MockLinuxFileSystemProvider::MockLinuxPath::toRealPath(
    deque<LinkOption> &options) 
{
  shared_ptr<Path> p = outerInstance->maybeChroot(shared_from_this());
  if (p == shared_from_this()) {
    return FilterPath::toRealPath({options});
  } else {
    return p->toRealPath(options);
  }
}

shared_ptr<Path> TestIOUtils::MockLinuxFileSystemProvider::MockLinuxPath::wrap(
    shared_ptr<Path> other)
{
  return make_shared<MockLinuxPath>(outerInstance, other, fileSystem);
}

void TestIOUtils::testGetFileStore() 
{
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // now we can create some fake mount points:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/sda1)", L"ntfs", L"/dev/sda1");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> usr = make_shared<MockFileStore>(
      dir->resolve(L"usr")->toString() + L" (/dev/sda2)", L"xfs", L"/dev/sda2");

  // associate some preset files to these
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      unordered_map<wstring, std::shared_ptr<FileStore>>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  mappings.emplace(dir->toString(), root);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  mappings.emplace(dir->resolve(L"foo.txt")->toString(), root);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  mappings.emplace(dir->resolve(L"usr")->toString(), usr);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  mappings.emplace(dir->resolve(L"usr/bar.txt")->toString(), usr);

  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());

  // sanity check our mock:
  assertSame(usr, Files::getFileStore(mockPath->resolve(L"usr")));
  assertSame(usr, Files::getFileStore(mockPath->resolve(L"usr/bar.txt")));
  // for root filesystem we get a crappy one
  assertNotSame(root, Files::getFileStore(mockPath));
  assertNotSame(usr, Files::getFileStore(mockPath));
  assertNotSame(root, Files::getFileStore(mockPath->resolve(L"foo.txt")));
  assertNotSame(usr, Files::getFileStore(mockPath->resolve(L"foo.txt")));

  // now test our method:
  assertSame(usr, IOUtils::getFileStore(mockPath->resolve(L"usr")));
  assertSame(usr, IOUtils::getFileStore(mockPath->resolve(L"usr/bar.txt")));
  assertSame(root, IOUtils::getFileStore(mockPath));
  assertSame(root, IOUtils::getFileStore(mockPath->resolve(L"foo.txt")));
}

void TestIOUtils::testTmpfsDoesntSpin() 
{
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake tmpfs
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/sda1)", L"tmpfs", L"/dev/sda1");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertFalse(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testNfsSpins() 
{
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake nfs
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (somenfsserver:/some/mount)", L"nfs",
      L"somenfsserver:/some/mount");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertTrue(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testSSD() 
{
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake ssd
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/zzz1)", L"btrfs", L"/dev/zzz1");
  // make a fake /dev/zzz1 for it
  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  Files::createFile(devdir->resolve(L"zzz1"));
  // make a fake /sys/block/zzz/queue/rotational file for it
  shared_ptr<Path> sysdir =
      dir->resolve(L"sys").resolve(L"block").resolve(L"zzz").resolve(L"queue");
  Files::createDirectories(sysdir);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.OutputStream o =
  // java.nio.file.Files.newOutputStream(sysdir.resolve("rotational")))
  {
    java::io::OutputStream o =
        java::nio::file::Files::newOutputStream(sysdir->resolve(L"rotational"));
    o.write((wstring(L"0\n")).getBytes(StandardCharsets::US_ASCII));
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertFalse(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testNVME() 
{
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake ssd
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/nvme0n1p1)", L"btrfs", L"/dev/nvme0n1p1");
  // make a fake /dev/nvme0n1p1 for it
  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  Files::createFile(devdir->resolve(L"nvme0n1p1"));
  // make a fake /sys/block/nvme0n1/queue/rotational file for it
  shared_ptr<Path> sysdir = dir->resolve(L"sys")
                                .resolve(L"block")
                                .resolve(L"nvme0n1")
                                .resolve(L"queue");
  Files::createDirectories(sysdir);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.OutputStream o =
  // java.nio.file.Files.newOutputStream(sysdir.resolve("rotational")))
  {
    java::io::OutputStream o =
        java::nio::file::Files::newOutputStream(sysdir->resolve(L"rotational"));
    o.write((wstring(L"0\n")).getBytes(StandardCharsets::US_ASCII));
  }
  // As test for the longest path match, add some other devices (that have no
  // queue/rotational), too:
  Files::createFile(dir->resolve(L"sys").resolve(L"block").resolve(L"nvme0"));
  Files::createFile(dir->resolve(L"sys").resolve(L"block").resolve(L"dummy"));
  Files::createFile(dir->resolve(L"sys").resolve(L"block").resolve(L"nvm"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertFalse(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testRotatingPlatters() 
{
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake ssd
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/zzz1)", L"reiser4", L"/dev/zzz1");
  // make a fake /dev/zzz1 for it
  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  Files::createFile(devdir->resolve(L"zzz1"));
  // make a fake /sys/block/zzz/queue/rotational file for it
  shared_ptr<Path> sysdir =
      dir->resolve(L"sys").resolve(L"block").resolve(L"zzz").resolve(L"queue");
  Files::createDirectories(sysdir);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.OutputStream o =
  // java.nio.file.Files.newOutputStream(sysdir.resolve("rotational")))
  {
    java::io::OutputStream o =
        java::nio::file::Files::newOutputStream(sysdir->resolve(L"rotational"));
    o.write((wstring(L"1\n")).getBytes(StandardCharsets::US_ASCII));
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertTrue(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testManyPartitions() 
{
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake ssd
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/zzz12)", L"zfs", L"/dev/zzz12");
  // make a fake /dev/zzz11 for it
  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  Files::createFile(devdir->resolve(L"zzz12"));
  // make a fake /sys/block/zzz/queue/rotational file for it
  shared_ptr<Path> sysdir =
      dir->resolve(L"sys").resolve(L"block").resolve(L"zzz").resolve(L"queue");
  Files::createDirectories(sysdir);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.OutputStream o =
  // java.nio.file.Files.newOutputStream(sysdir.resolve("rotational")))
  {
    java::io::OutputStream o =
        java::nio::file::Files::newOutputStream(sysdir->resolve(L"rotational"));
    o.write((wstring(L"0\n")).getBytes(StandardCharsets::US_ASCII));
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertFalse(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testSymlinkSSD() 
{
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  // fake SSD with a symlink mount (Ubuntu-like):
  shared_ptr<Random> rnd = random();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring partitionUUID =
      (make_shared<UUID>(rnd->nextLong(), rnd->nextLong()))->toString();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileStore> root = make_shared<MockFileStore>(
      dir->toString() + L" (/dev/disk/by-uuid/" + partitionUUID + L")",
      L"btrfs", L"/dev/disk/by-uuid/" + partitionUUID);
  // make a fake /dev/sda1 for it
  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  shared_ptr<Path> deviceFile = devdir->resolve(L"sda1");
  Files::createFile(deviceFile);
  // create a symlink to the above device file
  shared_ptr<Path> symlinkdir = devdir->resolve(L"disk").resolve(L"by-uuid");
  Files::createDirectories(symlinkdir);
  try {
    Files::createSymbolicLink(symlinkdir->resolve(partitionUUID), deviceFile);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (UnsupportedOperationException | IOException e) {
    assumeNoException(L"test requires filesystem that supports symbolic links",
                      e);
  }
  // make a fake /sys/block/sda/queue/rotational file for it
  shared_ptr<Path> sysdir =
      dir->resolve(L"sys").resolve(L"block").resolve(L"sda").resolve(L"queue");
  Files::createDirectories(sysdir);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.OutputStream o =
  // java.nio.file.Files.newOutputStream(sysdir.resolve("rotational")))
  {
    java::io::OutputStream o =
        java::nio::file::Files::newOutputStream(sysdir->resolve(L"rotational"));
    o.write((wstring(L"0\n")).getBytes(StandardCharsets::US_ASCII));
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  unordered_map<wstring, std::shared_ptr<FileStore>> mappings =
      Collections::singletonMap(dir->toString(), root);
  shared_ptr<FileSystem> mockLinux = (make_shared<MockLinuxFileSystemProvider>(
                                          dir->getFileSystem(), mappings, dir))
                                         ->getFileSystem(nullptr);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Path> mockPath = mockLinux->getPath(dir->toString());
  assertFalse(IOUtils::spinsLinux(mockPath));
}

void TestIOUtils::testFsyncDirectory() 
{
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  IOUtils::fsync(devdir, true);
  // no exception
}

void TestIOUtils::testFsyncFile() 
{
  shared_ptr<Path> dir = createTempDir();
  dir = FilterPath::unwrap(dir)->toRealPath();

  shared_ptr<Path> devdir = dir->resolve(L"dev");
  Files::createDirectories(devdir);
  shared_ptr<Path> somefile = devdir->resolve(L"somefile");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.OutputStream o =
  // java.nio.file.Files.newOutputStream(somefile))
  {
    java::io::OutputStream o =
        java::nio::file::Files::newOutputStream(somefile);
    o.write((wstring(L"0\n")).getBytes(StandardCharsets::US_ASCII));
  }
  IOUtils::fsync(somefile, false);
  // no exception
}

void TestIOUtils::testApplyToAll()
{
  deque<int> closed = deque<int>();
  runtime_error runtimeException = expectThrows(runtime_error::typeid, [&]() {
    IOUtils::applyToAll(java::util::Arrays::asList(1, 2), [&](any i) {
      closed.add(i);
      throw runtime_error(L"" + i);
    });
  });
  assertEquals(L"1", runtimeException.what());
  assertEquals(1, runtimeException.getSuppressed()->length);
  assertEquals(L"2", runtimeException.getSuppressed()[0].getMessage());
  assertEquals(2, closed.size());
  assertEquals(1, closed[0]);
  assertEquals(2, closed[1]);
}
} // namespace org::apache::lucene::util