#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

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

namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

class TrackingTmpOutputDirectoryWrapper final : public FilterDirectory
{
  GET_CLASS_NAME(TrackingTmpOutputDirectoryWrapper)
private:
  const std::unordered_map<std::wstring, std::wstring> fileNames =
      std::unordered_map<std::wstring, std::wstring>();

public:
  TrackingTmpOutputDirectoryWrapper(std::shared_ptr<Directory> in_);

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  std::unordered_map<std::wstring, std::wstring> getTemporaryFiles();

protected:
  std::shared_ptr<TrackingTmpOutputDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TrackingTmpOutputDirectoryWrapper>(
        org.apache.lucene.store.FilterDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
