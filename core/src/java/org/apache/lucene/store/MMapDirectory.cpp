using namespace std;

#include "MMapDirectory.h"

namespace org::apache::lucene::store
{
using BufferCleaner =
    org::apache::lucene::store::ByteBufferGuard::BufferCleaner;
using Constants = org::apache::lucene::util::Constants;
using org::apache::lucene::util::SuppressForbidden;

MMapDirectory::MMapDirectory(
    shared_ptr<Path> path,
    shared_ptr<LockFactory> lockFactory) 
    : MMapDirectory(path, lockFactory, DEFAULT_MAX_CHUNK_SIZE)
{
}

MMapDirectory::MMapDirectory(shared_ptr<Path> path) 
    : MMapDirectory(path, FSLockFactory::getDefault())
{
}

MMapDirectory::MMapDirectory(shared_ptr<Path> path,
                             int maxChunkSize) 
    : MMapDirectory(path, FSLockFactory::getDefault(), maxChunkSize)
{
}

MMapDirectory::MMapDirectory(shared_ptr<Path> path,
                             shared_ptr<LockFactory> lockFactory,
                             int maxChunkSize) 
    : FSDirectory(path, lockFactory),
      chunkSizePower(31 - Integer::numberOfLeadingZeros(maxChunkSize))
{
  if (maxChunkSize <= 0) {
    throw invalid_argument(L"Maximum chunk size for mmap must be >0");
  }
  assert(this->chunkSizePower >= 0 && this->chunkSizePower <= 30);
}

void MMapDirectory::setUseUnmap(bool const useUnmapHack)
{
  if (useUnmapHack && !UNMAP_SUPPORTED) {
    throw invalid_argument(UNMAP_NOT_SUPPORTED_REASON);
  }
  this->useUnmapHack = useUnmapHack;
}

bool MMapDirectory::getUseUnmap() { return useUnmapHack; }

void MMapDirectory::setPreload(bool preload) { this->preload = preload; }

bool MMapDirectory::getPreload() { return preload; }

int MMapDirectory::getMaxChunkSize() { return 1 << chunkSizePower; }

shared_ptr<IndexInput>
MMapDirectory::openInput(const wstring &name,
                         shared_ptr<IOContext> context) 
{
  ensureOpen();
  ensureCanRead(name);
  shared_ptr<Path> path = directory->resolve(name);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.nio.channels.FileChannel c =
  // java.nio.channels.FileChannel.open(path,
  // java.nio.file.StandardOpenOption.READ))
  {
    java::nio::channels::FileChannel c = java::nio::channels::FileChannel::open(
        path, java::nio::file::StandardOpenOption::READ);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring resourceDescription =
        L"MMapIndexInput(path=\"" + path->toString() + L"\")";
    constexpr bool useUnmap = getUseUnmap();
    return ByteBufferIndexInput::newInstance(
        resourceDescription, map_obj(resourceDescription, c, 0, c.size()), c.size(),
        chunkSizePower,
        make_shared<ByteBufferGuard>(resourceDescription,
                                     useUnmap ? CLEANER : nullptr));
  }
}

std::deque<std::shared_ptr<ByteBuffer>>
MMapDirectory::map_obj(const wstring &resourceDescription,
                   shared_ptr<FileChannel> fc, int64_t offset,
                   int64_t length) 
{
  if ((static_cast<int64_t>(static_cast<uint64_t>(length) >>
                              chunkSizePower)) >= numeric_limits<int>::max()) {
    throw invalid_argument(L"RandomAccessFile too big for chunk size: " +
                           resourceDescription);
  }

  constexpr int64_t chunkSize = 1LL << chunkSizePower;

  // we always allocate one more buffer, the last one may be a 0 byte one
  constexpr int nrBuffers =
      static_cast<int>(static_cast<int64_t>(
          static_cast<uint64_t>(length) >> chunkSizePower)) +
      1;

  std::deque<std::shared_ptr<ByteBuffer>> buffers(nrBuffers);

  int64_t bufferStart = 0LL;
  for (int bufNr = 0; bufNr < nrBuffers; bufNr++) {
    int bufSize = static_cast<int>((length > (bufferStart + chunkSize))
                                       ? chunkSize
                                       : (length - bufferStart));
    shared_ptr<MappedByteBuffer> buffer;
    try {
      buffer = fc->map_obj(FileChannel::MapMode::READ_ONLY, offset + bufferStart,
                       bufSize);
    } catch (const IOException &ioe) {
      throw convertMapFailedIOException(ioe, resourceDescription, bufSize);
    }
    if (preload) {
      buffer->load();
    }
    buffers[bufNr] = buffer;
    bufferStart += bufSize;
  }

  return buffers;
}

shared_ptr<IOException>
MMapDirectory::convertMapFailedIOException(shared_ptr<IOException> ioe,
                                           const wstring &resourceDescription,
                                           int bufSize)
{
  const wstring originalMessage;
  constexpr runtime_error originalCause;
  if (std::dynamic_pointer_cast<OutOfMemoryError>(ioe->getCause()) != nullptr) {
    // nested OOM confuses users, because it's "incorrect", just print a plain
    // message:
    originalMessage = L"Map failed";
    originalCause = nullptr;
  } else {
    originalMessage = ioe->getMessage();
    originalCause = ioe->getCause();
  }
  const wstring moreInfo;
  if (!Constants::JRE_IS_64BIT) {
    moreInfo = L"MMapDirectory should only be used on 64bit platforms, because "
               L"the address space on 32bit operating systems is too small. ";
  } else if (Constants::WINDOWS) {
    moreInfo = L"Windows is unfortunately very limited on virtual address "
               L"space. If your index size is several hundred Gigabytes, "
               L"consider changing to Linux. ";
  } else if (Constants::LINUX) {
    moreInfo = L"Please review 'ulimit -v', 'ulimit -m' (both should return "
               L"'unlimited'), and 'sysctl vm.max_map_count'. ";
  } else {
    moreInfo = L"Please review 'ulimit -v', 'ulimit -m' (both should return "
               L"'unlimited'). ";
  }
  shared_ptr<IOException> *const newIoe = make_shared<IOException>(
      wstring::format(Locale::ENGLISH,
                      wstring(L"%s: %s [this may be caused by lack of enough "
                              L"unfragmented virtual address space ") +
                          L"or too restrictive virtual memory limits enforced "
                          L"by the operating system, " +
                          L"preventing us to map_obj a chunk of %d bytes. %sMore "
                          L"information: " +
                          L"http://blog.thetaphi.de/2012/07/"
                          L"use-lucenes-mmapdirectory-on-64bit.html]",
                      originalMessage, resourceDescription, bufSize, moreInfo),
      originalCause);
  newIoe->setStackTrace(ioe->getStackTrace());
  return newIoe;
}

const wstring MMapDirectory::UNMAP_NOT_SUPPORTED_REASON;
const org::apache::lucene::store::ByteBufferGuard::BufferCleaner
    MMapDirectory::CLEANER;

MMapDirectory::StaticConstructor::StaticConstructor()
{
  constexpr any hack = AccessController::doPrivileged(
      std::static_pointer_cast<PrivilegedAction<any>>(
          MMapDirectory::unmapHackImpl));
  if (dynamic_cast<BufferCleaner>(hack) != nullptr) {
    CLEANER = any_cast<BufferCleaner>(hack);
    UNMAP_SUPPORTED = true;
    UNMAP_NOT_SUPPORTED_REASON = L"";
  } else {
    CLEANER.reset();
    UNMAP_SUPPORTED = false;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    UNMAP_NOT_SUPPORTED_REASON = hack.toString();
  }
}

MMapDirectory::StaticConstructor MMapDirectory::staticConstructor;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "Needs access to private APIs in
// DirectBuffer, sun.misc.Cleaner, and sun.misc.Unsafe to enable hack") private
// static Object unmapHackImpl()
any MMapDirectory::unmapHackImpl()
{
  shared_ptr<Lookup> *const lookup = lookup();
  try {
    try {
      // *** sun.misc.Unsafe unmapping (Java 9+) ***
      constexpr type_info unsafeClass = type_info::forName(L"sun.misc.Unsafe");
      // first check if Unsafe has the right method, otherwise we can give up
      // without doing any security critical stuff:
      shared_ptr<MethodHandle> *const unmapper =
          lookup->findVirtual(unsafeClass, L"invokeCleaner",
                              methodType(void ::typeid, ByteBuffer::typeid));
      // fetch the unsafe instance and bind it to the virtual MH:
      shared_ptr<Field> *const f = unsafeClass.getDeclaredField(L"theUnsafe");
      f->setAccessible(true);
      constexpr any theUnsafe = f->get(nullptr);
      return newBufferCleaner(ByteBuffer::typeid, unmapper->bindTo(theUnsafe));
    } catch (const SecurityException &se) {
      // rethrow to report errors correctly (we need to catch it here, as we
      // also catch RuntimeException below!):
      throw se;
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (ReflectiveOperationException | runtime_error e) {
      // *** sun.misc.Cleaner unmapping (Java 8) ***
      constexpr type_info directBufferClass =
          type_info::forName(L"java.nio.DirectByteBuffer");

      shared_ptr<Method> *const m = directBufferClass.getMethod(L"cleaner");
      m->setAccessible(true);
      shared_ptr<MethodHandle> *const directBufferCleanerMethod =
          lookup->unreflect(m);
      constexpr type_info cleanerClass =
          directBufferCleanerMethod->type().returnType();

      /* "Compile" a MH that basically is equivalent to the following code:
       * void unmapper(ByteBuffer byteBuffer) {
       *   sun.misc.Cleaner cleaner = ((java.nio.DirectByteBuffer)
       * byteBuffer).cleaner(); if (Objects.nonNull(cleaner)) { cleaner.clean();
       *   } else {
       *     noop(cleaner); // the noop is needed because
       * MethodHandles#guardWithTest always needs ELSE
       *   }
       * }
       */
      shared_ptr<MethodHandle> *const cleanMethod = lookup->findVirtual(
          cleanerClass, L"clean", methodType(void ::typeid));
      shared_ptr<MethodHandle> *const nonNullTest =
          lookup
              ->findStatic(Objects::typeid, L"nonNull",
                           methodType(bool ::typeid, any::typeid))
              .asType(methodType(bool ::typeid, cleanerClass));
      shared_ptr<MethodHandle> *const noop = dropArguments(
          constant(Void::typeid, nullptr).asType(methodType(void ::typeid)), 0,
          cleanerClass);
      shared_ptr<MethodHandle> *const unmapper =
          filterReturnValue(directBufferCleanerMethod,
                            guardWithTest(nonNullTest, cleanMethod, noop))
              .asType(methodType(void ::typeid, ByteBuffer::typeid));
      return newBufferCleaner(directBufferClass, unmapper);
    }
  } catch (const SecurityException &se) {
    return L"Unmapping is not supported, because not all required permissions "
           L"are given to the Lucene JAR file: " +
           se +
           L" [Please grant at least the following permissions: "
           L"RuntimePermission(\"accessClassInPackage.sun.misc\") " +
           L" and ReflectPermission(\"suppressAccessChecks\")]";
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ReflectiveOperationException | runtime_error e) {
    return L"Unmapping is not supported on this platform, because internal "
           L"Java APIs are not compatible with this Lucene version: " +
           e;
  }
}

BufferCleaner
MMapDirectory::newBufferCleaner(type_info const unmappableBufferClass,
                                shared_ptr<MethodHandle> unmapper)
{
  assert((Objects::equals(methodType(void ::typeid, ByteBuffer::typeid),
                          unmapper->type())));
  return [&](const wstring &resourceDescription,
             shared_ptr<ByteBuffer> buffer) {
    if (!buffer->isDirect()) {
      throw invalid_argument(L"unmapping only works with direct buffers");
    }
    if (!unmappableBufferClass.isInstance(buffer)) {
      throw invalid_argument(L"buffer is not an instance of " +
                             unmappableBufferClass.getName());
    }
    constexpr runtime_error error = AccessController::doPrivileged(
        std::static_pointer_cast<PrivilegedAction<runtime_error>>[&]() {
          try {
            unmapper->invokeExact(buffer);
            return nullptr;
          } catch (const runtime_error &t) {
            return t;
          }
        });
    if (error != nullptr) {
      throw make_shared<IOException>(
          L"Unable to unmap the mapped buffer: " + resourceDescription, error);
    }
  };
}
} // namespace org::apache::lucene::store