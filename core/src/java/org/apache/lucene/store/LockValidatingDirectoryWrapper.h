#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Lock.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
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
 * This class makes a best-effort check that a provided {@link Lock}
 * is valid before any destructive filesystem operation.
 */
class LockValidatingDirectoryWrapper final : public FilterDirectory
{
  GET_CLASS_NAME(LockValidatingDirectoryWrapper)
private:
  const std::shared_ptr<Lock> writeLock;

public:
  LockValidatingDirectoryWrapper(std::shared_ptr<Directory> in_,
                                 std::shared_ptr<Lock> writeLock);

  void deleteFile(const std::wstring &name)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  void copyFrom(std::shared_ptr<Directory> from, const std::wstring &src,
                const std::wstring &dest,
                std::shared_ptr<IOContext> context)  override;

  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  void syncMetaData()  override;

  void sync(std::shared_ptr<std::deque<std::wstring>> names) throw(
      IOException) override;

protected:
  std::shared_ptr<LockValidatingDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<LockValidatingDirectoryWrapper>(
        FilterDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
