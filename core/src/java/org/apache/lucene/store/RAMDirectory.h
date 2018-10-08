#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RAMFile;
}

namespace org::apache::lucene::store
{
class LockFactory;
}
namespace org::apache::lucene::store
{
class FSDirectory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::store
{

using Accountable = org::apache::lucene::util::Accountable;

/**
 * A memory-resident {@link Directory} implementation.  Locking
 * implementation is by default the {@link SingleInstanceLockFactory}.
 *
 * <p><b>Warning:</b> This class is not intended to work with huge
 * indexes. Everything beyond several hundred megabytes will waste
 * resources (GC cycles), because it uses an internal buffer size
 * of 1024 bytes, producing millions of {@code byte[1024]} arrays.
 * This class is optimized for small memory-resident indexes.
 * It also has bad concurrency on multithreaded environments.
 *
 * <p>It is recommended to materialize large indexes on disk and use
 * {@link MMapDirectory}, which is a high-performance directory
 * implementation working directly on the file system cache of the
 * operating system, so copying data to Java heap space is not useful.
 */
class RAMDirectory : public BaseDirectory, public Accountable
{
  GET_CLASS_NAME(RAMDirectory)
protected:
  const std::unordered_map<std::wstring, std::shared_ptr<RAMFile>> fileMap =
      std::make_shared<
          ConcurrentHashMap<std::wstring, std::shared_ptr<RAMFile>>>();
  const std::shared_ptr<AtomicLong> sizeInBytes =
      std::make_shared<AtomicLong>();

  /** Used to generate temp file names in {@link #createTempOutput}. */
private:
  const std::shared_ptr<AtomicLong> nextTempFileCounter =
      std::make_shared<AtomicLong>();

  /** Constructs an empty {@link Directory}. */
public:
  RAMDirectory();

  /** Constructs an empty {@link Directory} with the given {@link LockFactory}.
   */
  RAMDirectory(std::shared_ptr<LockFactory> lockFactory);

  /**
   * Creates a new <code>RAMDirectory</code> instance from a different
   * <code>Directory</code> implementation.  This can be used to load
   * a disk-based index into memory.
   *
   * <p><b>Warning:</b> This class is not intended to work with huge
   * indexes. Everything beyond several hundred megabytes will waste
   * resources (GC cycles), because it uses an internal buffer size
   * of 1024 bytes, producing millions of {@code byte[1024]} arrays.
   * This class is optimized for small memory-resident indexes.
   * It also has bad concurrency on multithreaded environments.
   *
   * <p>For disk-based indexes it is recommended to use
   * {@link MMapDirectory}, which is a high-performance directory
   * implementation working directly on the file system cache of the
   * operating system, so copying data to Java heap space is not useful.
   *
   * <p>Note that the resulting <code>RAMDirectory</code> instance is fully
   * independent from the original <code>Directory</code> (it is a
   * complete copy).  Any subsequent changes to the
   * original <code>Directory</code> will not be visible in the
   * <code>RAMDirectory</code> instance.
   *
   * @param dir a <code>Directory</code> value
   * @exception IOException if an error occurs
   */
  RAMDirectory(std::shared_ptr<FSDirectory> dir,
               std::shared_ptr<IOContext> context) ;

private:
  RAMDirectory(std::shared_ptr<FSDirectory> dir, bool closeDir,
               std::shared_ptr<IOContext> context) ;

public:
  std::deque<std::wstring> listAll() override final;

  bool fileNameExists(const std::wstring &name);

  /** Returns the length in bytes of a file in the directory.
   * @throws IOException if the file does not exist
   */
  int64_t
  fileLength(const std::wstring &name)  override final;

  /**
   * Return total size in bytes of all files in this directory. This is
   * currently quantized to RAMOutputStream.BUFFER_SIZE.
   */
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void deleteFile(const std::wstring &name)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

  /**
   * Returns a new {@link RAMFile} for storing data. This method can be
   * overridden to return different {@link RAMFile} impls, that e.g. override
   * {@link RAMFile#newBuffer(int)}.
   */
protected:
  virtual std::shared_ptr<RAMFile> newRAMFile();

public:
  void sync(std::shared_ptr<std::deque<std::wstring>> names) throw(
      IOException) override;

  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  void syncMetaData()  override;

  /** Returns a stream reading an existing file. */
  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  /** Closes the store to future operations, releasing associated memory. */
  virtual ~RAMDirectory();

protected:
  std::shared_ptr<RAMDirectory> shared_from_this()
  {
    return std::static_pointer_cast<RAMDirectory>(
        BaseDirectory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
