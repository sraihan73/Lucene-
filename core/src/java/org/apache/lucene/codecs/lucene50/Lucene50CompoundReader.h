#pragma once
#include "../../store/Directory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::store
{
class Lock;
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
namespace org::apache::lucene::codecs::lucene50
{

using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Lock = org::apache::lucene::store::Lock;

/**
 * Class for accessing a compound stream.
 * This class implements a directory, but is limited to only read operations.
 * Directory methods that would normally modify data throw an exception.
 * @lucene.experimental
 */
class Lucene50CompoundReader final : public Directory
{
  GET_CLASS_NAME(Lucene50CompoundReader)

  /** Offset/Length for a slice inside of a compound file */
public:
  class FileEntry final : public std::enable_shared_from_this<FileEntry>
  {
    GET_CLASS_NAME(FileEntry)
  public:
    int64_t offset = 0;
    int64_t length = 0;
  };

private:
  const std::shared_ptr<Directory> directory;
  const std::wstring segmentName;
  const std::unordered_map<std::wstring, std::shared_ptr<FileEntry>> entries;
  const std::shared_ptr<IndexInput> handle;
  int version = 0;

  /**
   * Create a new CompoundFileDirectory.
   */
  // TODO: we should just pre-strip "entries" and append segment name up-front
  // like simpletext? this need not be a "general purpose" directory anymore (it
  // only writes index files)
public:
  Lucene50CompoundReader(std::shared_ptr<Directory> directory,
                         std::shared_ptr<SegmentInfo> si,
                         std::shared_ptr<IOContext> context) ;

  /** Helper method that reads CFS entries from an input stream */
private:
  std::unordered_map<std::wstring, std::shared_ptr<FileEntry>>
  readEntries(std::deque<char> &segmentID, std::shared_ptr<Directory> dir,
              const std::wstring &entriesFileName) ;

public:
  virtual ~Lucene50CompoundReader();

  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  /** Returns an array of strings, one for each file in the directory. */
  std::deque<std::wstring> listAll() override;

  /** Not implemented
   * @throws UnsupportedOperationException always: not supported by CFS */
  void deleteFile(const std::wstring &name) override;

  /** Not implemented
   * @throws UnsupportedOperationException always: not supported by CFS */
  void rename(const std::wstring &from, const std::wstring &to) override;

  void syncMetaData() override;

  /** Returns the length of a file in the directory.
   * @throws IOException if the file does not exist */
  int64_t fileLength(const std::wstring &name)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

  void sync(std::shared_ptr<std::deque<std::wstring>> names) override;

  std::shared_ptr<Lock> obtainLock(const std::wstring &name) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<Lucene50CompoundReader> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50CompoundReader>(
        org.apache.lucene.store.Directory::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene50
