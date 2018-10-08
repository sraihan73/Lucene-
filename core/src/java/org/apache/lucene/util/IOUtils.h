#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::util
{
template <typename T>
class IOConsumer;
}

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
namespace org::apache::lucene::util
{

using Directory = org::apache::lucene::store::Directory;

/** This class emulates the new Java 7 "Try-With-Resources" statement.
 * Remove once Lucene is on Java 7.
 * @lucene.internal */
class IOUtils final : public std::enable_shared_from_this<IOUtils>
{
  GET_CLASS_NAME(IOUtils)

  /**
   * UTF-8 charset string.
   * <p>Where possible, use {@link StandardCharsets#UTF_8} instead,
   * as using the std::wstring constant may slow things down.
   * @see StandardCharsets#UTF_8
   */
public:
  static const std::wstring UTF_8;

private:
  IOUtils();

  /**
   * Closes all given <tt>Closeable</tt>s.  Some of the
   * <tt>Closeable</tt>s may be null; they are
   * ignored.  After everything is closed, the method either
   * throws the first exception it hit while closing, or
   * completes normally if there were no exceptions.
   *
   * @param objects
   *          objects to call <tt>close()</tt> on
   */
public:
  static void close(std::deque<Closeable> &objects) ;

  /**
   * Closes all given <tt>Closeable</tt>s.
   * @see #close(Closeable...)
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static void close(Iterable<? extends
  // java.io.Closeable> objects) throws java.io.IOException
  static void close(std::deque<T1> objects) ;

  /**
   * Closes all given <tt>Closeable</tt>s, suppressing all thrown exceptions.
   * Some of the <tt>Closeable</tt>s may be null, they are ignored.
   *
   * @param objects
   *          objects to call <tt>close()</tt> on
   */
  static void closeWhileHandlingException(std::deque<Closeable> &objects);

  /**
   * Closes all given <tt>Closeable</tt>s, suppressing all thrown non {@link
   * VirtualMachineError} exceptions. Even if a {@link VirtualMachineError} is
   * thrown all given closeable are closed.
   * @see #closeWhileHandlingException(Closeable...)
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static void
  // closeWhileHandlingException(Iterable<? extends java.io.Closeable> objects)
  static void closeWhileHandlingException(std::deque<T1> objects);

  /**
   * Wrapping the given {@link InputStream} in a reader using a {@link
   * CharsetDecoder}. Unlike Java's defaults this reader will throw an exception
   * if your it detects the read charset doesn't match the expected {@link
   * Charset}. <p> Decoding readers are useful to load configuration files,
   * stopword lists or synonym files to detect character set problems. However,
   * it's not recommended to use as a common purpose reader.
   *
   * @param stream the stream to wrap in a reader
   * @param charSet the expected charset
   * @return a wrapping reader
   */
  static std::shared_ptr<Reader>
  getDecodingReader(std::shared_ptr<InputStream> stream,
                    std::shared_ptr<Charset> charSet);

  /**
   * Opens a Reader for the given resource using a {@link CharsetDecoder}.
   * Unlike Java's defaults this reader will throw an exception if your it
   * detects the read charset doesn't match the expected {@link Charset}. <p>
   * Decoding readers are useful to load configuration files, stopword lists or
   * synonym files to detect character set problems. However, it's not
   * recommended to use as a common purpose reader.
   * @param clazz the class used to locate the resource
   * @param resource the resource name to load
   * @param charSet the expected charset
   * @return a reader to read the given file
   *
   */
  static std::shared_ptr<Reader>
  getDecodingReader(std::type_info clazz, const std::wstring &resource,
                    std::shared_ptr<Charset> charSet) ;

  /**
   * Deletes all given files, suppressing all thrown IOExceptions.
   * <p>
   * Note that the files should not be null.
   */
  static void deleteFilesIgnoringExceptions(
      std::shared_ptr<Directory> dir,
      std::shared_ptr<std::deque<std::wstring>> files);

  static void deleteFilesIgnoringExceptions(std::shared_ptr<Directory> dir,
                                            std::deque<std::wstring> &files);

  /**
   * Deletes all given file names.  Some of the
   * file names may be null; they are
   * ignored.  After everything is deleted, the method either
   * throws the first exception it hit while deleting, or
   * completes normally if there were no exceptions.
   *
   * @param dir Directory to delete files from
   * @param names file names to delete
   */
  static void deleteFiles(
      std::shared_ptr<Directory> dir,
      std::shared_ptr<std::deque<std::wstring>> names) ;

  /**
   * Deletes all given files, suppressing all thrown IOExceptions.
   * <p>
   * Some of the files may be null, if so they are ignored.
   */
  static void deleteFilesIgnoringExceptions(std::deque<Path> &files);

  /**
   * Deletes all given files, suppressing all thrown IOExceptions.
   * <p>
   * Some of the files may be null, if so they are ignored.
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static void
  // deleteFilesIgnoringExceptions(java.util.std::deque<? extends
  // java.nio.file.Path> files)
  static void
  deleteFilesIgnoringExceptions(std::shared_ptr<std::deque<T1>> files);

  /**
   * Deletes all given <tt>Path</tt>s, if they exist.  Some of the
   * <tt>File</tt>s may be null; they are
   * ignored.  After everything is deleted, the method either
   * throws the first exception it hit while deleting, or
   * completes normally if there were no exceptions.
   *
   * @param files files to delete
   */
  static void deleteFilesIfExist(std::deque<Path> &files) ;

  /**
   * Deletes all given <tt>Path</tt>s, if they exist.  Some of the
   * <tt>File</tt>s may be null; they are
   * ignored.  After everything is deleted, the method either
   * throws the first exception it hit while deleting, or
   * completes normally if there were no exceptions.
   *
   * @param files files to delete
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static void
  // deleteFilesIfExist(java.util.std::deque<? extends java.nio.file.Path> files)
  // throws java.io.IOException
  static void
  deleteFilesIfExist(std::shared_ptr<std::deque<T1>> files) ;

  /**
   * Deletes one or more files or directories (and everything underneath it).
   *
   * @throws IOException if any of the given files (or their subhierarchy files
   * in case of directories) cannot be removed.
   */
  static void rm(std::deque<Path> &locations) ;

private:
  static std::shared_ptr<
      LinkedHashMap<std::shared_ptr<Path>, std::runtime_error>>
  rm(std::shared_ptr<LinkedHashMap<std::shared_ptr<Path>, std::runtime_error>>
         unremoved,
     std::deque<Path> &locations);

private:
  class FileVisitorAnonymousInnerClass
      : public FileVisitor<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(FileVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<LinkedHashMap<std::shared_ptr<Path>, std::runtime_error>>
        unremoved;

  public:
    FileVisitorAnonymousInnerClass(
        std::shared_ptr<
            LinkedHashMap<std::shared_ptr<Path>, std::runtime_error>>
            unremoved);

    std::shared_ptr<FileVisitResult> preVisitDirectory(
        std::shared_ptr<Path> dir,
        std::shared_ptr<BasicFileAttributes> attrs)  override;

    std::shared_ptr<FileVisitResult> postVisitDirectory(
        std::shared_ptr<Path> dir,
        std::shared_ptr<IOException> impossible)  override;

    std::shared_ptr<FileVisitResult> visitFile(
        std::shared_ptr<Path> file,
        std::shared_ptr<BasicFileAttributes> attrs)  override;

    std::shared_ptr<FileVisitResult> visitFileFailed(
        std::shared_ptr<Path> file,
        std::shared_ptr<IOException> exc)  override;

  protected:
    std::shared_ptr<FileVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FileVisitorAnonymousInnerClass>(
          java.nio.file.FileVisitor<java.nio.file.Path>::shared_from_this());
    }
  };

  /**
   * This utility method takes a previously caught (non-null)
   * {@code Throwable} and rethrows either the original argument
   * if it was a subclass of the {@code IOException} or an
   * {@code RuntimeException} with the cause set to the argument.
GET_CLASS_NAME(of)
   *
   * <p>This method <strong>never returns any value</strong>, even though it
declares
   * a return value of type {@link Error}. The return value declaration
   * is very useful to let the compiler know that the code path following
   * the invocation of this method is unreachable. So in most cases the
   * invocation of this method will be guarded by an {@code if} and
   * used together with a {@code throw} statement, as in:
   * </p>
   * <pre>{@code
   *   if (t != null) throw IOUtils.rethrowAlways(t)
   * }
   * </pre>
   *
   * @param th The throwable to rethrow, <strong>must not be null</strong>.
   * @return This method always results in an exception, it never returns any
value.
   *         See method documentation for detailsa and usage example.
   * @throws IOException if the argument was an instance of IOException
   * @throws RuntimeException with the {@link RuntimeException#getCause()} set
   *         to the argument, if it was not an instance of IOException.
   */
public:
  static std::shared_ptr<Error>
  rethrowAlways(std::runtime_error th) ;

  /**
   * Rethrows the argument as {@code IOException} or {@code RuntimeException}
   * if it's not null.
   *
   * @deprecated This method is deprecated in favor of {@link #rethrowAlways}.
   * Code should be updated to {@link #rethrowAlways} and guarded with an
   * additional null-argument check (because {@link #rethrowAlways} is not
   * accepting null arguments).
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static void reThrow(Throwable th) throws
  // java.io.IOException
  static void reThrow(std::runtime_error th) ;

  /**
   * @deprecated This method is deprecated in favor of {@link #rethrowAlways}.
   * Code should be updated to {@link #rethrowAlways} and guarded with an
   * additional null-argument check (because {@link #rethrowAlways} is not
   * accepting null arguments).
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static void reThrowUnchecked(Throwable
  // th)
  static void reThrowUnchecked(std::runtime_error th);

  /**
   * Ensure that any writes to the given file is written to the storage device
   * that contains it.
   * @param fileToSync the file to fsync
   * @param isDir if true, the given file is a directory (we open for read and
   * ignore IOExceptions, because not all file systems and operating systems
   * allow to fsync on a directory)
   */
  static void fsync(std::shared_ptr<Path> fileToSync,
                    bool isDir) ;

  /** If the dir is an {@link FSDirectory} or wraps one via possibly
   *  nested {@link FilterDirectory} or {@link FileSwitchDirectory},
   *  this returns {@link #spins(Path)} for the wrapped directory,
   *  else, true.
   *
   *  @throws IOException if {@code path} does not exist.
   *
   *  @lucene.internal */
  static bool spins(std::shared_ptr<Directory> dir) ;

  /** Rough Linux-only heuristics to determine whether the provided
   *  {@code Path} is backed by spinning storage.  For example, this
   *  returns false if the disk is a solid-state disk.
   *
   *  @param path a location to check which must exist. the mount point will be
   * determined from this location.
   *  @return false if the storage is non-rotational (e.g. an SSD), or true if
   * it is spinning or could not be determined
   *  @throws IOException if {@code path} does not exist.
   *
   *  @lucene.internal */
  static bool spins(std::shared_ptr<Path> path) ;

  // following methods are package-private for testing ONLY

  // note: requires a real or fake linux filesystem!
  static bool spinsLinux(std::shared_ptr<Path> path) ;

  // Files.getFileStore(Path) useless here!
  // don't complain, just try it yourself
  static std::shared_ptr<FileStore>
  getFileStore(std::shared_ptr<Path> path) ;

  // these are hacks that are not guaranteed, may change across JVM versions,
  // etc.
  static std::wstring getMountPoint(std::shared_ptr<FileStore> store);

  /**
   * Returns the second throwable if the first is null otherwise adds the second
   * as suppressed to the first and returns it.
   */
  template <typename T>
  static T useOrSuppress(T first, T second);

  /**
   * Applies the consumer to all non-null elements in the collection even if an
   * exception is thrown. The first exception thrown by the consumer is
   * re-thrown and subsequent exceptions are suppressed.
   */
  template <typename T>
  static void applyToAll(std::shared_ptr<std::deque<T>> collection,
                         IOConsumer<T> consumer) ;

  /**
   * An IO operation with a single input.
   * @see java.util.function.Consumer
   */
  template <typename T>
  using IOConsumer = std::function<void(T input)>;
};

} // namespace org::apache::lucene::util
