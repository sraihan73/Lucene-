#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::store
{
class IOContext;
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

/** A delegating Directory that records which files were
 *  written to and deleted. */
class TrackingDirectoryWrapper final : public FilterDirectory
{
  GET_CLASS_NAME(TrackingDirectoryWrapper)

private:
  const std::shared_ptr<Set<std::wstring>> createdFileNames =
      Collections::synchronizedSet(std::unordered_set<std::wstring>());

public:
  TrackingDirectoryWrapper(std::shared_ptr<Directory> in_);

  void deleteFile(const std::wstring &name)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

  void copyFrom(std::shared_ptr<Directory> from, const std::wstring &src,
                const std::wstring &dest,
                std::shared_ptr<IOContext> context)  override;

  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  /** NOTE: returns a copy of the created files. */
  std::shared_ptr<Set<std::wstring>> getCreatedFiles();

  void clearCreatedFiles();

protected:
  std::shared_ptr<TrackingDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TrackingDirectoryWrapper>(
        FilterDirectory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
