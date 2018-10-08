#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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

/**
 * Base class for Directory implementations that store index
 * files in the file system.
 * <a name="subclasses"></a>
 * There are currently three core
 * subclasses:
 *
 * <ul>
 *
 *  <li>{@link SimpleFSDirectory} is a straightforward
 *       implementation using Files.newByteChannel.
 *       However, it has poor concurrent performance
 *       (multiple threads will bottleneck) as it
 *       synchronizes when multiple threads read from the
 *       same file.
 *
 *  <li>{@link NIOFSDirectory} uses java.nio's
 *       FileChannel's positional io when reading to avoid
 *       synchronization when reading from the same file.
 *       Unfortunately, due to a Windows-only <a
 *       href="http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6265734">Sun
 *       JRE bug</a> this is a poor choice for Windows, but
 *       on all other platforms this is the preferred
 *       choice. Applications using {@link Thread#interrupt()} or
 *       {@link Future#cancel(bool)} should use
 *       {@code RAFDirectory} instead. See {@link NIOFSDirectory} java doc
 *       for details.
 *
 *  <li>{@link MMapDirectory} uses memory-mapped IO when
 *       reading. This is a good choice if you have plenty
 *       of virtual memory relative to your index size, eg
 *       if you are running on a 64 bit JRE, or you are
 *       running on a 32 bit JRE but your index sizes are
 *       small enough to fit into the virtual memory space.
 *       Java has currently the limitation of not being able to
 *       unmap files from user code. The files are unmapped, when GC
 *       releases the byte buffers. Due to
 *       <a href="http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4724038">
 *       this bug</a> in Sun's JRE, MMapDirectory's {@link IndexInput#close}
 *       is unable to close the underlying OS file handle. Only when
 *       GC finally collects the underlying objects, which could be
 *       quite some time later, will the file handle be closed.
 *       This will consume additional transient disk usage: on Windows,
 *       attempts to delete or overwrite the files will result in an
 *       exception; on other platforms, which typically have a &quot;delete on
 *       last close&quot; semantics, while such operations will succeed, the
 * bytes are still consuming space on disk.  For many applications this
 *       limitation is not a problem (e.g. if you have plenty of disk space,
 *       and you don't rely on overwriting files on Windows) but it's still
 *       an important limitation to be aware of. This class supplies a
 *       (possibly dangerous) workaround mentioned in the bug report,
 *       which may fail on non-Sun JVMs.
 * </ul>
 *
 * <p>Unfortunately, because of system peculiarities, there is
 * no single overall best implementation.  Therefore, we've
 * added the {@link #open} method, to allow Lucene to choose
 * the best FSDirectory implementation given your
 * environment, and the known limitations of each
 * implementation.  For users who have no reason to prefer a
 * specific implementation, it's best to simply use {@link
 * #open}.  For all others, you should instantiate the
 * desired implementation directly.
 *
 * <p><b>NOTE:</b> Accessing one of the above subclasses either directly or
 * indirectly from a thread while it's interrupted can close the
 * underlying channel immediately if at the same time the thread is
 * blocked on IO. The channel will remain closed and subsequent access
 * to the index will throw a {@link ClosedChannelException}.
 * Applications using {@link Thread#interrupt()} or
 * {@link Future#cancel(bool)} should use the slower legacy
 * {@code RAFDirectory} from the {@code misc} Lucene module instead.
 *
 * <p>The locking implementation is by default {@link
 * NativeFSLockFactory}, but can be changed by
 * passing in a custom {@link LockFactory} instance.
 *
 * @see Directory
 */
class FSDirectory : public BaseDirectory
{
  GET_CLASS_NAME(FSDirectory)

protected:
  const std::shared_ptr<Path> directory; // The underlying filesystem directory

  /** Maps files that we are trying to delete (or we tried already but failed)
   *  before attempting to delete that key. */
private:
  const std::shared_ptr<Set<std::wstring>> pendingDeletes =
      Collections::newSetFromMap(
          std::make_shared<ConcurrentHashMap<std::wstring, bool>>());

  const std::shared_ptr<AtomicInteger> opsSinceLastDelete =
      std::make_shared<AtomicInteger>();

  /** Used to generate temp file names in {@link #createTempOutput}. */
  const std::shared_ptr<AtomicLong> nextTempFileCounter =
      std::make_shared<AtomicLong>();

  /** Create a new FSDirectory for the named location (ctor for subclasses).
   * The directory is created at the named location if it does not yet exist.
   *
   * <p>{@code FSDirectory} resolves the given Path to a canonical /
   * real path to ensure it can correctly lock the index directory and no other
   * process can interfere with changing possible symlinks to the index
   * directory inbetween. If you want to use symlinks and change them
   * dynamically, close all
   * {@code IndexWriters} and create a new {@code FSDirecory} instance.
   * @param path the path of the directory
   * @param lockFactory the lock factory to use, or null for the default
   * ({@link NativeFSLockFactory});
   * @throws IOException if there is a low-level I/O error
   */
protected:
  FSDirectory(std::shared_ptr<Path> path,
              std::shared_ptr<LockFactory> lockFactory) ;

  /** Creates an FSDirectory instance, trying to pick the
   *  best implementation given the current environment.
   *  The directory returned uses the {@link NativeFSLockFactory}.
   *  The directory is created at the named location if it does not yet exist.
   *
   * <p>{@code FSDirectory} resolves the given Path when calling this method to
   * a canonical / real path to ensure it can correctly lock the index directory
   * and no other process can interfere with changing possible symlinks to the
   * index directory inbetween. If you want to use symlinks and change them
   * dynamically, close all
   * {@code IndexWriters} and create a new {@code FSDirecory} instance.
   *
   *  <p>Currently this returns {@link MMapDirectory} for Linux, MacOSX,
   * Solaris, and Windows 64-bit JREs, {@link NIOFSDirectory} for other
   *  non-Windows JREs, and {@link SimpleFSDirectory} for other
   *  JREs on Windows. It is highly recommended that you consult the
   *  implementation's documentation for your platform before
   *  using this method.
   *
   * <p><b>NOTE</b>: this method may suddenly change which
   * implementation is returned from release to release, in
   * the event that higher performance defaults become
   * possible; if the precise implementation is important to
   * your application, please instantiate it directly,
   * instead. For optimal performance you should consider using
   * {@link MMapDirectory} on 64 bit JVMs.
   *
   * <p>See <a href="#subclasses">above</a> */
public:
  static std::shared_ptr<FSDirectory>
  open(std::shared_ptr<Path> path) ;

  /** Just like {@link #open(Path)}, but allows you to
   *  also specify a custom {@link LockFactory}. */
  static std::shared_ptr<FSDirectory>
  open(std::shared_ptr<Path> path,
       std::shared_ptr<LockFactory> lockFactory) ;

  /** Lists all files (including subdirectories) in the directory.
   *
   *  @throws IOException if there was an I/O error during listing */
  static std::deque<std::wstring>
  listAll(std::shared_ptr<Path> dir) ;

private:
  static std::deque<std::wstring>
  listAll(std::shared_ptr<Path> dir,
          std::shared_ptr<Set<std::wstring>> skipNames) ;

public:
  std::deque<std::wstring> listAll()  override;

  int64_t fileLength(const std::wstring &name)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

protected:
  virtual void ensureCanRead(const std::wstring &name) ;

public:
  void sync(std::shared_ptr<std::deque<std::wstring>> names) throw(
      IOException) override;

  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  void syncMetaData()  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~FSDirectory();

  /** @return the underlying filesystem directory */
  virtual std::shared_ptr<Path> getDirectory();

  virtual std::wstring toString();

protected:
  virtual void fsync(const std::wstring &name) ;

public:
  void deleteFile(const std::wstring &name)  override;

  /** Try to delete any pending files that we had previously tried to delete but
   * failed because we are on Windows and the files were still held open. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void deletePendingFiles() ;

private:
  void maybeDeletePendingFiles() ;

  void privateDeleteFile(const std::wstring &name,
                         bool isPendingDelete) ;

public:
  class FSIndexOutput final : public OutputStreamIndexOutput
  {
    GET_CLASS_NAME(FSIndexOutput)
  private:
    std::shared_ptr<FSDirectory> outerInstance;

    /**
     * The maximum chunk size is 8192 bytes, because file channel mallocs
     * a native buffer outside of stack if the write buffer size is larger.
     */
  public:
    static constexpr int CHUNK_SIZE = 8192;

    FSIndexOutput(std::shared_ptr<FSDirectory> outerInstance,
                  const std::wstring &name) ;

    FSIndexOutput(std::shared_ptr<FSDirectory> outerInstance,
                  const std::wstring &name,
                  std::deque<OpenOption> &options) ;

  private:
    class FilterOutputStreamAnonymousInnerClass : public FilterOutputStream
    {
      GET_CLASS_NAME(FilterOutputStreamAnonymousInnerClass)
    public:
      FilterOutputStreamAnonymousInnerClass(
          std::shared_ptr<UnknownType> newOutputStream);

      // This implementation ensures, that we never write more than CHUNK_SIZE
      // bytes:
      void write(std::deque<char> &b, int offset,
                 int length)  override;

    protected:
      std::shared_ptr<FilterOutputStreamAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterOutputStreamAnonymousInnerClass>(
            java.io.FilterOutputStream::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FSIndexOutput> shared_from_this()
    {
      return std::static_pointer_cast<FSIndexOutput>(
          OutputStreamIndexOutput::shared_from_this());
    }
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Set<std::wstring>>
  getPendingDeletions()  override;

protected:
  std::shared_ptr<FSDirectory> shared_from_this()
  {
    return std::static_pointer_cast<FSDirectory>(
        BaseDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
