#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/store/AlreadyClosedException.h"

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

/** A Directory is a flat deque of files.  Files may be written once, when they
 * are created.  Once a file is created it may only be opened for read, or
 * deleted.  Random access is permitted both when reading and writing.
 *
 * <p> Java's i/o APIs not used directly, but rather all i/o is
 * through this API.  This permits things such as: <ul>
 * <li> implementation of RAM-based indices;
 * <li> implementation indices stored in a database, via JDBC;
 * <li> implementation of an index as a single file;
 * </ul>
 *
 * Directory locking is implemented by an instance of {@link
 * LockFactory}.
 *
 */
class Directory : public std::enable_shared_from_this<Directory>
{
  GET_CLASS_NAME(Directory)

  /**
   * Returns an array of strings, one for each entry in the directory, in sorted
   * (UTF16, java's std::wstring.compare) order.
   *
   * @throws IOException in case of IO error
   */
public:
  virtual std::deque<std::wstring> listAll() = 0;

  /** Removes an existing file in the directory. */
  virtual void deleteFile(const std::wstring &name) = 0;

  /**
   * Returns the length of a file in the directory. This method follows the
   * following contract:
   * <ul>
   * <li>Throws {@link FileNotFoundException} or {@link NoSuchFileException}
   * if the file does not exist.
   * <li>Returns a value &ge;0 if the file exists, which specifies its length.
   * </ul>
   *
   * @param name the name of the file for which to return the length.
   * @throws IOException if there was an IO error while retrieving the file's
   *         length.
   */
  virtual int64_t fileLength(const std::wstring &name) = 0;

  /** Creates a new, empty file in the directory with the given name.
      Returns a stream writing this file. */
  virtual std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context) = 0;

  /** Creates a new, empty file for writing in the directory, with a
   *  temporary file name including prefix and suffix, ending with the
   *  reserved extension <code>.tmp</code>.  Use
   *  {@link IndexOutput#getName} to see what name was used.  */
  virtual std::shared_ptr<IndexOutput>
  createTempOutput(const std::wstring &prefix, const std::wstring &suffix,
                   std::shared_ptr<IOContext> context) = 0;

  /**
   * Ensure that any writes to these files are moved to
   * stable storage.  Lucene uses this to properly commit
   * changes to the index, to prevent a machine/OS crash
   * from corrupting the index.
   * <br>
   * NOTE: Clients may call this method for same files over
   * and over again, so some impls might optimize for that.
   * For other impls the operation can be a noop, for various
   * reasons.
   */
  virtual void sync(std::shared_ptr<std::deque<std::wstring>> names) = 0;

  /**
   * Renames {@code source} to {@code dest} as an atomic operation,
   * where {@code dest} does not yet exist in the directory.
   * <p>
   * Notes: This method is used by IndexWriter to publish commits.
   * It is ok if this operation is not truly atomic, for example
   * both {@code source} and {@code dest} can be visible temporarily.
   * It is just important that the contents of {@code dest} appear
   * atomically, or an exception is thrown.
   */
  virtual void rename(const std::wstring &source, const std::wstring &dest) = 0;

  /**
   * Ensure that directory metadata, such as recent file renames, are made
   * durable.
   */
  virtual void syncMetaData() = 0;

  /** Returns a stream reading an existing file.
   * <p>Throws {@link FileNotFoundException} or {@link NoSuchFileException}
   * if the file does not exist.
   */
  virtual std::shared_ptr<IndexInput>
  openInput(const std::wstring &name, std::shared_ptr<IOContext> context) = 0;

  /** Returns a stream reading an existing file, computing checksum as it reads
   */
  virtual std::shared_ptr<ChecksumIndexInput>
  openChecksumInput(const std::wstring &name,
                    std::shared_ptr<IOContext> context) ;

  /**
   * Returns an obtained {@link Lock}.
   * @param name the name of the lock file
   * @throws LockObtainFailedException (optional specific exception) if the lock
   * could not be obtained because it is currently held elsewhere.
   * @throws IOException if any i/o error occurs attempting to gain the lock
   */
  virtual std::shared_ptr<Lock> obtainLock(const std::wstring &name) = 0;

  /** Closes the store. */
  void close() = 0;
  override

      std::wstring
      toString() override;

  /**
   * Copies the file <i>src</i> in <i>from</i> to this directory under the new
   * file name <i>dest</i>.
   * <p>
   * If you want to copy the entire source directory to the destination one, you
   * can do so like this:
   *
   * <pre class="prettyprint">
   * Directory to; // the directory to copy to
   * for (std::wstring file : dir.listAll()) {
   *   to.copyFrom(dir, file, newFile, IOContext.DEFAULT); // newFile can be
   * either file, or a new name
   * }
   * </pre>
   * <p>
   * <b>NOTE:</b> this method does not check whether <i>dest</i> exist and will
   * overwrite it if it does.
   */
  virtual void copyFrom(std::shared_ptr<Directory> from,
                        const std::wstring &src, const std::wstring &dest,
                        std::shared_ptr<IOContext> context) ;

  /**
   * @throws AlreadyClosedException if this Directory is closed
   */
protected:
  virtual void ensureOpen() ;

  /**
   * Returns the current pending deletions in this directory or an emtpy set
   * if there are no known pending deletions.
   * @lucene.internal
   */
public:
  virtual std::shared_ptr<Set<std::wstring>>
  getPendingDeletions() ;
};

} // #include  "core/src/java/org/apache/lucene/store/
