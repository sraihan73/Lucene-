#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/Lock.h"

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

/** Directory implementation that delegates calls to another directory.
 *  This class can be used to add limitations on top of an existing
 *  {@link Directory} implementation such as
GET_CLASS_NAME(can)
 *  {@link NRTCachingDirectory} or to add additional
 *  sanity checks for tests. However, if you plan to write your own
 *  {@link Directory} implementation, you should consider extending directly
 *  {@link Directory} or {@link BaseDirectory} rather than try to reuse
 *  functionality of existing {@link Directory}s by extending this class.
 *  @lucene.internal */
class FilterDirectory : public Directory
{
  GET_CLASS_NAME(FilterDirectory)

  /** Get the wrapped instance by <code>dir</code> as long as this reader is
   *  an instance of {@link FilterDirectory}.  */
public:
  static std::shared_ptr<Directory> unwrap(std::shared_ptr<Directory> dir);

protected:
  const std::shared_ptr<Directory> in_;

  /** Sole constructor, typically called from sub-classes. */
  FilterDirectory(std::shared_ptr<Directory> in_);

  /** Return the wrapped {@link Directory}. */
public:
  std::shared_ptr<Directory> getDelegate();

  std::deque<std::wstring> listAll()  override;

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

  std::shared_ptr<Lock>
  obtainLock(const std::wstring &name)  override;

  virtual ~FilterDirectory();

  virtual std::wstring toString();

  std::shared_ptr<Set<std::wstring>>
  getPendingDeletions()  override;

protected:
  std::shared_ptr<FilterDirectory> shared_from_this()
  {
    return std::static_pointer_cast<FilterDirectory>(
        Directory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
