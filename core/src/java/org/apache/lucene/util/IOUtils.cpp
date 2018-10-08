using namespace std;

#include "IOUtils.h"

namespace org::apache::lucene::util
{
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FileSwitchDirectory = org::apache::lucene::store::FileSwitchDirectory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
const wstring IOUtils::UTF_8 =
    java::nio::charset::StandardCharsets::UTF_8::name();

IOUtils::IOUtils() {} // no instance

void IOUtils::close(deque<Closeable> &objects) 
{
  close({Arrays::asList(objects)});
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static void close(Iterable<? extends
// java.io.Closeable> objects) throws java.io.IOException
void IOUtils::close(deque<T1> objects) 
{
  runtime_error th = nullptr;
  for (auto object : objects) {
    try {
      if (object != nullptr) {
        object->close();
      }
    } catch (const runtime_error &t) {
      th = useOrSuppress(th, t);
    }
  }

  if (th != nullptr) {
    throw rethrowAlways(th);
  }
}

void IOUtils::closeWhileHandlingException(deque<Closeable> &objects)
{
  closeWhileHandlingException({Arrays::asList(objects)});
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static void
// closeWhileHandlingException(Iterable<? extends java.io.Closeable> objects)
void IOUtils::closeWhileHandlingException(deque<T1> objects)
{
  shared_ptr<VirtualMachineError> firstError = nullptr;
  runtime_error firstThrowable = nullptr;
  for (auto object : objects) {
    try {
      if (object != nullptr) {
        object->close();
      }
    } catch (const VirtualMachineError &e) {
      firstError = useOrSuppress(firstError, e);
    } catch (const runtime_error &t) {
      firstThrowable = useOrSuppress(firstThrowable, t);
    }
  }
  if (firstError != nullptr) {
    // we ensure that we bubble up any errors. We can't recover from these but
    // need to make sure they are bubbled up. if a non-VMError is thrown we also
    // add the suppressed exceptions to it.
    if (firstThrowable != nullptr) {
      firstError->addSuppressed(firstThrowable);
    }
    throw firstError;
  }
}

shared_ptr<Reader> IOUtils::getDecodingReader(shared_ptr<InputStream> stream,
                                              shared_ptr<Charset> charSet)
{
  shared_ptr<CharsetDecoder> *const charSetDecoder =
      charSet->newDecoder()
          .onMalformedInput(CodingErrorAction::REPORT)
          .onUnmappableCharacter(CodingErrorAction::REPORT);
  return make_shared<BufferedReader>(
      make_shared<InputStreamReader>(stream, charSetDecoder));
}

shared_ptr<Reader>
IOUtils::getDecodingReader(type_info clazz, const wstring &resource,
                           shared_ptr<Charset> charSet) 
{
  shared_ptr<InputStream> stream = nullptr;
  bool success = false;
  try {
    stream = clazz.getResourceAsStream(resource);
    shared_ptr<Reader> *const reader = getDecodingReader(stream, charSet);
    success = true;
    return reader;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::close({stream});
    }
  }
}

void IOUtils::deleteFilesIgnoringExceptions(
    shared_ptr<Directory> dir, shared_ptr<deque<wstring>> files)
{
  for (auto name : files) {
    try {
      dir->deleteFile(name);
    } catch (const runtime_error &ignored) {
      // ignore
    }
  }
}

void IOUtils::deleteFilesIgnoringExceptions(shared_ptr<Directory> dir,
                                            deque<wstring> &files)
{
  deleteFilesIgnoringExceptions({dir, Arrays::asList(files)});
}

void IOUtils::deleteFiles(
    shared_ptr<Directory> dir,
    shared_ptr<deque<wstring>> names) 
{
  runtime_error th = nullptr;
  for (auto name : names) {
    if (name != L"") {
      try {
        dir->deleteFile(name);
      } catch (const runtime_error &t) {
        th = useOrSuppress(th, t);
      }
    }
  }

  if (th != nullptr) {
    throw rethrowAlways(th);
  }
}

void IOUtils::deleteFilesIgnoringExceptions(deque<Path> &files)
{
  deleteFilesIgnoringExceptions({Arrays::asList(files)});
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static void
// deleteFilesIgnoringExceptions(java.util.deque<? extends
// java.nio.file.Path> files)
void IOUtils::deleteFilesIgnoringExceptions(shared_ptr<deque<T1>> files)
{
  for (auto name : files) {
    if (name != nullptr) {
      try {
        Files::delete (name);
      } catch (const runtime_error &ignored) {
        // ignore
      }
    }
  }
}

void IOUtils::deleteFilesIfExist(deque<Path> &files) 
{
  deleteFilesIfExist({Arrays::asList(files)});
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static void
// deleteFilesIfExist(java.util.deque<? extends java.nio.file.Path> files)
// throws java.io.IOException
void IOUtils::deleteFilesIfExist(shared_ptr<deque<T1>> files) throw(
    IOException)
{
  runtime_error th = nullptr;
  for (auto file : files) {
    try {
      if (file != nullptr) {
        Files::deleteIfExists(file);
      }
    } catch (const runtime_error &t) {
      th = useOrSuppress(th, t);
    }
  }

  if (th != nullptr) {
    throw rethrowAlways(th);
  }
}

void IOUtils::rm(deque<Path> &locations) 
{
  shared_ptr<LinkedHashMap<std::shared_ptr<Path>, runtime_error>> unremoved =
      rm(make_shared<LinkedHashMap<std::shared_ptr<Path>, runtime_error>>(),
         locations);
  if (!unremoved->isEmpty()) {
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>(
        L"Could not remove the following files (in the order of attempts):\n");
    for (auto kv : unremoved) {
      b->append(L"   ")
          ->append(kv.first::toAbsolutePath())
          ->append(L": ")
          ->append(kv.second)
          ->append(L"\n");
    }
    throw make_shared<IOException>(b->toString());
  }
}

shared_ptr<LinkedHashMap<std::shared_ptr<Path>, runtime_error>> IOUtils::rm(
    shared_ptr<LinkedHashMap<std::shared_ptr<Path>, runtime_error>> unremoved,
    deque<Path> &locations)
{
  if (locations != nullptr) {
    for (shared_ptr<Path> location : locations) {
      // TODO: remove this leniency!
      if (location != nullptr && Files::exists(location)) {
        try {
          Files::walkFileTree(
              location, make_shared<FileVisitorAnonymousInnerClass>(unremoved));
        } catch (const IOException &impossible) {
          throw make_shared<AssertionError>(L"visitor threw exception",
                                            impossible);
        }
      }
    }
  }
  return unremoved;
}

IOUtils::FileVisitorAnonymousInnerClass::FileVisitorAnonymousInnerClass(
    shared_ptr<LinkedHashMap<std::shared_ptr<Path>, runtime_error>> unremoved)
{
  this->unremoved = unremoved;
}

shared_ptr<FileVisitResult>
IOUtils::FileVisitorAnonymousInnerClass::preVisitDirectory(
    shared_ptr<Path> dir,
    shared_ptr<BasicFileAttributes> attrs) 
{
  return FileVisitResult::CONTINUE;
}

shared_ptr<FileVisitResult>
IOUtils::FileVisitorAnonymousInnerClass::postVisitDirectory(
    shared_ptr<Path> dir, shared_ptr<IOException> impossible) 
{
  assert(impossible == nullptr);

  try {
    Files::delete (dir);
  } catch (const IOException &e) {
    unremoved->put(dir, e);
  }
  return FileVisitResult::CONTINUE;
}

shared_ptr<FileVisitResult> IOUtils::FileVisitorAnonymousInnerClass::visitFile(
    shared_ptr<Path> file,
    shared_ptr<BasicFileAttributes> attrs) 
{
  try {
    Files::delete (file);
  } catch (const IOException &exc) {
    unremoved->put(file, exc);
  }
  return FileVisitResult::CONTINUE;
}

shared_ptr<FileVisitResult>
IOUtils::FileVisitorAnonymousInnerClass::visitFileFailed(
    shared_ptr<Path> file, shared_ptr<IOException> exc) 
{
  outerInstance->if (exc != nullptr) { unremoved->put(file, exc); }
  return FileVisitResult::CONTINUE;
}

shared_ptr<Error> IOUtils::rethrowAlways(runtime_error th) throw(IOException,
                                                                 runtime_error)
{
  if (th == nullptr) {
    throw make_shared<AssertionError>(L"rethrow argument must not be null.");
  }

  if (std::dynamic_pointer_cast<IOException>(th) != nullptr) {
    throw std::static_pointer_cast<IOException>(th);
  }

  if (dynamic_cast<runtime_error>(th) != nullptr) {
    throw static_cast<runtime_error>(th);
  }

  if (std::dynamic_pointer_cast<Error>(th) != nullptr) {
    throw std::static_pointer_cast<Error>(th);
  }

  throw runtime_error(th);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static void reThrow(Throwable th) throws
// java.io.IOException
void IOUtils::reThrow(runtime_error th) 
{
  if (th != nullptr) {
    throw rethrowAlways(th);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static void reThrowUnchecked(Throwable th)
void IOUtils::reThrowUnchecked(runtime_error th)
{
  if (th != nullptr) {
    if (std::dynamic_pointer_cast<Error>(th) != nullptr) {
      throw std::static_pointer_cast<Error>(th);
    }
    if (dynamic_cast<runtime_error>(th) != nullptr) {
      throw static_cast<runtime_error>(th);
    }
    throw runtime_error(th);
  }
}

void IOUtils::fsync(shared_ptr<Path> fileToSync, bool isDir) 
{
  // If the file is a directory we have to open read-only, for regular files we
  // must open r/w for the fsync to have an effect. See
  // http://blog.httrack.com/blog/2013/11/15/everything-you-always-wanted-to-know-about-fsync/
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final java.nio.channels.FileChannel file =
  // java.nio.channels.FileChannel.open(fileToSync, isDir ?
  // java.nio.file.StandardOpenOption.READ :
  // java.nio.file.StandardOpenOption.WRITE))
  {
    shared_ptr<java::nio::channels::FileChannel> *const file =
        java.nio::channels::FileChannel::open(
            fileToSync, isDir ? java.nio::file::StandardOpenOption::READ
                              : java.nio::file::StandardOpenOption::WRITE);
    try {
      file->force(true);
    } catch (const IOException &ioe) {
      if (isDir) {
        assert(Constants::LINUX || Constants::MAC_OS_X) == false
            : wstring(L"On Linux and MacOSX fsyncing a directory should not "
                      L"throw IOException, ") +
              L"we just don't want to rely on that in production "
              L"(undocumented). Got: " +
              ioe;
        // Ignore exception if it is a directory
        return;
      }
      // Throw original exception
      throw ioe;
    }
  }
}

bool IOUtils::spins(shared_ptr<Directory> dir) 
{
  dir = FilterDirectory::unwrap(dir);
  if (std::dynamic_pointer_cast<FileSwitchDirectory>(dir) != nullptr) {
    shared_ptr<FileSwitchDirectory> fsd =
        std::static_pointer_cast<FileSwitchDirectory>(dir);
    // Spinning is contagious:
    return spins(fsd->getPrimaryDir()) || spins(fsd->getSecondaryDir());
  } else if (std::dynamic_pointer_cast<RAMDirectory>(dir) != nullptr) {
    return false;
  } else if (std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr) {
    return spins((std::static_pointer_cast<FSDirectory>(dir))->getDirectory());
  } else {
    return true;
  }
}

bool IOUtils::spins(shared_ptr<Path> path) 
{
  // resolve symlinks (this will throw exception if the path does not exist)
  path = path->toRealPath();

  // Super cowboy approach, but seems to work!
  if (!Constants::LINUX) {
    return true; // no detection
  }

  try {
    return spinsLinux(path);
  } catch (const runtime_error &exc) {
    // our crazy heuristics can easily trigger SecurityException, AIOOBE, etc
    // ...
    return true;
  }
}

bool IOUtils::spinsLinux(shared_ptr<Path> path) 
{
  shared_ptr<FileStore> store = getFileStore(path);

  // if fs type is tmpfs, it doesn't spin.
  // this won't have a corresponding block device
  if (L"tmpfs" == store->type()) {
    return false;
  }

  // get block device name
  wstring devName = store->name();

  // not a device (e.g. NFS server)
  if (!StringHelper::startsWith(devName, L"/")) {
    return true;
  }

  // resolve any symlinks to real block device (e.g. LVM)
  // /dev/sda0 -> sda0
  // /devices/XXX -> sda0
  // C++ TODO: There is no native C++ equivalent to 'toString':
  devName =
      path->getRoot().resolve(devName).toRealPath().getFileName()->toString();

  // now try to find the longest matching device folder in /sys/block
  // (that starts with our dev name):
  shared_ptr<Path> sysinfo = path->getRoot().resolve(L"sys").resolve(L"block");
  shared_ptr<Path> devsysinfo = nullptr;
  int matchlen = 0;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(sysinfo))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(sysinfo);
    for (auto device : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring name = device->getFileName()->toString();
      if (name.length() > matchlen && StringHelper::startsWith(devName, name)) {
        devsysinfo = device;
        matchlen = name.length();
      }
    }
  }

  if (devsysinfo == nullptr) {
    return true; // give up
  }

  // read first byte from rotational, it's a 1 if it spins.
  shared_ptr<Path> rotational =
      devsysinfo->resolve(L"queue").resolve(L"rotational");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream stream =
  // java.nio.file.Files.newInputStream(rotational))
  {
    java::io::InputStream stream =
        java::nio::file::Files::newInputStream(rotational);
    return stream.read() == L'1';
  }
}

shared_ptr<FileStore>
IOUtils::getFileStore(shared_ptr<Path> path) 
{
  shared_ptr<FileStore> store = Files::getFileStore(path);
  wstring mount = getMountPoint(store);

  // find the "matching" FileStore from system deque, it's the one we want, but
  // only return that if it's unambiguous (only one matching):
  shared_ptr<FileStore> sameMountPoint = nullptr;
  for (shared_ptr<FileStore> fs : path->getFileSystem().getFileStores()) {
    if (mount == getMountPoint(fs)) {
      if (sameMountPoint == nullptr) {
        sameMountPoint = fs;
      } else {
        // more than one filesystem has the same mount point; something is
        // wrong! fall back to crappy one we got from Files.getFileStore
        return store;
      }
    }
  }

  if (sameMountPoint != nullptr) {
    // ok, we found only one, use it:
    return sameMountPoint;
  } else {
    // fall back to crappy one we got from Files.getFileStore
    return store;
  }
}

wstring IOUtils::getMountPoint(shared_ptr<FileStore> store)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring desc = store->toString();
  int index = (int)desc.rfind(L" (");
  if (index != -1) {
    return desc.substr(0, index);
  } else {
    return desc;
  }
}

template <typename T>
T IOUtils::useOrSuppress(T first, T second)
{
  static_assert(is_base_of<runtime_error, T>::value,
                L"T must inherit from std::runtime_error");

  if (first == nullptr) {
    return second;
  } else {
    first.addSuppressed(second);
  }
  return first;
}

template <typename T>
void IOUtils::applyToAll(shared_ptr<deque<T>> collection,
                         IOConsumer<T> consumer) 
{
  IOUtils::close({collection->stream().filter(Objects::nonNull).map_obj([&](any t) {
    std::static_pointer_cast<java::io::Closeable>[&]() { consumer(t); };
  })::iterator});
}
} // namespace org::apache::lucene::util