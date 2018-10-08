#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class Lock;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::store
{
class IOContext;
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

/**
 * Expert: A Directory instance that switches files between
 * two other Directory instances.

 * <p>Files with the specified extensions are placed in the
 * primary directory; others are placed in the secondary
 * directory.  The provided Set must not change once passed
 * to this class, and must allow multiple threads to call
 * contains at once.</p>
 *
 * <p>Locks with a name having the specified extensions are
 * delegated to the primary directory; others are delegated
 * to the secondary directory. Ideally, both Directory
 * instances should use the same lock factory.</p>
 *
 * @lucene.experimental
 */

class FileSwitchDirectory : public Directory
{
  GET_CLASS_NAME(FileSwitchDirectory)
private:
  const std::shared_ptr<Directory> secondaryDir;
  const std::shared_ptr<Directory> primaryDir;
  const std::shared_ptr<Set<std::wstring>> primaryExtensions;
  bool doClose = false;

public:
  FileSwitchDirectory(std::shared_ptr<Set<std::wstring>> primaryExtensions,
                      std::shared_ptr<Directory> primaryDir,
                      std::shared_ptr<Directory> secondaryDir, bool doClose);

  /** Return the primary directory */
  virtual std::shared_ptr<Directory> getPrimaryDir();

  /** Return the secondary directory */
  virtual std::shared_ptr<Directory> getSecondaryDir();

  std::shared_ptr<Lock>
  obtainLock(const std::wstring &name)  override;

  virtual ~FileSwitchDirectory();

  std::deque<std::wstring> listAll()  override;

  /** Utility method to return a file's extension. */
  static std::wstring getExtension(const std::wstring &name);

private:
  std::shared_ptr<Directory> getDirectory(const std::wstring &name);

public:
  void deleteFile(const std::wstring &name)  override;

  int64_t fileLength(const std::wstring &name)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

  void sync(std::shared_ptr<std::deque<std::wstring>> names) throw(
      IOException) override;

  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  void syncMetaData()  override;

  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<Set<std::wstring>>
  getPendingDeletions()  override;

protected:
  std::shared_ptr<FileSwitchDirectory> shared_from_this()
  {
    return std::static_pointer_cast<FileSwitchDirectory>(
        Directory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
