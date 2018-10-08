#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/ByteBufferGuard.h"
#include  "core/src/java/org/apache/lucene/store/BufferCleaner.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::store
{

using BufferCleaner =
    org::apache::lucene::store::ByteBufferGuard::BufferCleaner;
using Constants = org::apache::lucene::util::Constants;

/** File-based {@link Directory} implementation that uses
 *  mmap for reading, and {@link
 *  FSDirectory.FSIndexOutput} for writing.
 *
 * <p><b>NOTE</b>: memory mapping uses up a portion of the
 * virtual memory address space in your process equal to the
 * size of the file being mapped.  Before using this class,
 * be sure your have plenty of virtual address space, e.g. by
 * using a 64 bit JRE, or a 32 bit JRE with indexes that are
 * guaranteed to fit within the address space.
 * On 32 bit platforms also consult {@link #MMapDirectory(Path, LockFactory,
int)}
 * if you have problems with mmap failing because of fragmented
 * address space. If you get an OutOfMemoryException, it is recommended
 * to reduce the chunk size, until it works.
 *
 * <p>Due to <a
href="http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4724038">
 * this bug</a> in Sun's JRE, MMapDirectory's {@link IndexInput#close}
 * is unable to close the underlying OS file handle.  Only when GC
 * finally collects the underlying objects, which could be quite
 * some time later, will the file handle be closed.
 *
 * <p>This will consume additional transient disk usage: on Windows,
 * attempts to delete or overwrite the files will result in an
 * exception; on other platforms, which typically have a &quot;delete on
 * last close&quot; semantics, while such operations will succeed, the bytes
 * are still consuming space on disk.  For many applications this
 * limitation is not a problem (e.g. if you have plenty of disk space,
 * and you don't rely on overwriting files on Windows) but it's still
 * an important limitation to be aware of.
 *
 * <p>This class supplies the workaround mentioned in the bug report
 * (see {@link #setUseUnmap}), which may fail on
GET_CLASS_NAME(supplies)
 * non-Oracle/OpenJDK JVMs. It forcefully unmaps the buffer on close by using
 * an undocumented internal cleanup functionality. If
 * {@link #UNMAP_SUPPORTED} is <code>true</code>, the workaround
 * will be automatically enabled (with no guarantees; if you discover
 * any problems, you can disable it).
 * <p>
 * <b>NOTE:</b> Accessing this class either directly or
 * indirectly from a thread while it's interrupted can close the
 * underlying channel immediately if at the same time the thread is
 * blocked on IO. The channel will remain closed and subsequent access
 * to {@link MMapDirectory} will throw a {@link ClosedChannelException}. If
 * your application uses either {@link Thread#interrupt()} or
 * {@link Future#cancel(bool)} you should use the legacy {@code RAFDirectory}
 * from the Lucene {@code misc} module in favor of {@link MMapDirectory}.
 * </p>
 * @see <a
href="http://blog.thetaphi.de/2012/07/use-lucenes-mmapdirectory-on-64bit.html">Blog
post about MMapDirectory</a>
 */
class MMapDirectory : public FSDirectory
{
  GET_CLASS_NAME(MMapDirectory)
private:
  bool useUnmapHack = UNMAP_SUPPORTED;
  bool preload = false;

  /**
   * Default max chunk size.
   * @see #MMapDirectory(Path, LockFactory, int)
   */
public:
  static const int DEFAULT_MAX_CHUNK_SIZE =
      Constants::JRE_IS_64BIT ? (1 << 30) : (1 << 28);
  const int chunkSizePower;

  /** Create a new MMapDirectory for the named location.
   *  The directory is created at the named location if it does not yet exist.
   *
   * @param path the path of the directory
   * @param lockFactory the lock factory to use
   * @throws IOException if there is a low-level I/O error
   */
  MMapDirectory(std::shared_ptr<Path> path,
                std::shared_ptr<LockFactory> lockFactory) ;

  /** Create a new MMapDirectory for the named location and {@link
   * FSLockFactory#getDefault()}. The directory is created at the named location
   * if it does not yet exist.
   *
   * @param path the path of the directory
   * @throws IOException if there is a low-level I/O error
   */
  MMapDirectory(std::shared_ptr<Path> path) ;

  /** Create a new MMapDirectory for the named location and {@link
   * FSLockFactory#getDefault()}. The directory is created at the named location
   * if it does not yet exist.
   *
   * @param path the path of the directory
   * @param maxChunkSize maximum chunk size (default is 1 GiBytes for
   * 64 bit JVMs and 256 MiBytes for 32 bit JVMs) used for memory mapping.
   * @throws IOException if there is a low-level I/O error
   */
  MMapDirectory(std::shared_ptr<Path> path,
                int maxChunkSize) ;

  /**
   * Create a new MMapDirectory for the named location, specifying the
   * maximum chunk size used for memory mapping.
   *  The directory is created at the named location if it does not yet exist.
   * <p>
   * Especially on 32 bit platform, the address space can be very fragmented,
   * so large index files cannot be mapped. Using a lower chunk size makes
   * the directory implementation a little bit slower (as the correct chunk
   * may be resolved on lots of seeks) but the chance is higher that mmap
   * does not fail. On 64 bit Java platforms, this parameter should always
   * be {@code 1 << 30}, as the address space is big enough.
   * <p>
   * <b>Please note:</b> The chunk size is always rounded down to a power of 2.
   *
   * @param path the path of the directory
   * @param lockFactory the lock factory to use, or null for the default
   * ({@link NativeFSLockFactory});
   * @param maxChunkSize maximum chunk size (default is 1 GiBytes for
   * 64 bit JVMs and 256 MiBytes for 32 bit JVMs) used for memory mapping.
   * @throws IOException if there is a low-level I/O error
   */
  MMapDirectory(std::shared_ptr<Path> path,
                std::shared_ptr<LockFactory> lockFactory,
                int maxChunkSize) ;

  /**
   * This method enables the workaround for unmapping the buffers
   * from address space after closing {@link IndexInput}, that is
   * mentioned in the bug report. This hack may fail on non-Oracle/OpenJDK JVMs.
   * It forcefully unmaps the buffer on close by using
   * an undocumented internal cleanup functionality.
   * <p><b>NOTE:</b> Enabling this is completely unsupported
   * by Java and may lead to JVM crashes if <code>IndexInput</code>
   * is closed while another thread is still accessing it (SIGSEGV).
   * <p>To enable the hack, the following requirements need to be
   * fulfilled: The used JVM must be Oracle Java / OpenJDK 8
   * <em>(preliminary support for Java 9 EA build 150+ was added with
   * Lucene 6.4)</em>. In addition, the following permissions need to be granted
   * to {@code lucene-core.jar} in your
   * <a
   * href="http://docs.oracle.com/javase/8/docs/technotes/guides/security/PolicyFiles.html">policy
   * file</a>: <ul> <li>{@code permission java.lang.reflect.ReflectPermission
   * "suppressAccessChecks";}</li> <li>{@code permission
   * java.lang.RuntimePermission "accessClassInPackage.sun.misc";}</li>
   * </ul>
   * @throws IllegalArgumentException if {@link #UNMAP_SUPPORTED}
   * is <code>false</code> and the workaround cannot be enabled.
   * The exception message also contains an explanation why the hack
   * cannot be enabled (e.g., missing permissions).
   */
  virtual void setUseUnmap(bool const useUnmapHack);

  /**
   * Returns <code>true</code>, if the unmap workaround is enabled.
   * @see #setUseUnmap
   */
  virtual bool getUseUnmap();

  /**
   * Set to {@code true} to ask mapped pages to be loaded
   * into physical memory on init. The behavior is best-effort
   * and operating system dependent.
   * @see MappedByteBuffer#load
   */
  virtual void setPreload(bool preload);

  /**
   * Returns {@code true} if mapped pages should be loaded.
   * @see #setPreload
   */
  virtual bool getPreload();

  /**
   * Returns the current mmap chunk size.
   * @see #MMapDirectory(Path, LockFactory, int)
   */
  int getMaxChunkSize();

  /** Creates an IndexInput for the file with the given name. */
  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  /** Maps a file into a set of buffers */
  std::deque<std::shared_ptr<ByteBuffer>>
  map_obj(const std::wstring &resourceDescription, std::shared_ptr<FileChannel> fc,
      int64_t offset, int64_t length) ;

private:
  std::shared_ptr<IOException>
  convertMapFailedIOException(std::shared_ptr<IOException> ioe,
                              const std::wstring &resourceDescription,
                              int bufSize);

  /**
   * <code>true</code>, if this platform supports unmapping mmapped files.
   */
public:
  static const bool UNMAP_SUPPORTED = false;

  /**
   * if {@link #UNMAP_SUPPORTED} is {@code false}, this contains the reason why
   * unmapping is not supported.
   */
  static const std::wstring UNMAP_NOT_SUPPORTED_REASON;

  /** Reference to a BufferCleaner that does unmapping; {@code null} if not
   * supported. */
private:
  static const BufferCleaner CLEANER;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static MMapDirectory::StaticConstructor staticConstructor;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "Needs access to private APIs in
  // DirectBuffer, sun.misc.Cleaner, and sun.misc.Unsafe to enable hack") private
  // static Object unmapHackImpl()
  static std::any unmapHackImpl();

  static BufferCleaner
  newBufferCleaner(std::type_info const unmappableBufferClass,
                   std::shared_ptr<MethodHandle> unmapper);

protected:
  std::shared_ptr<MMapDirectory> shared_from_this()
  {
    return std::static_pointer_cast<MMapDirectory>(
        FSDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
