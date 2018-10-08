#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReader;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
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
namespace org::apache::lucene::index
{

/**
 * A {@link DirectoryReader} that wraps all its subreaders with
 * {@link AssertingLeafReader}
 */
class AssertingDirectoryReader : public FilterDirectoryReader
{
  GET_CLASS_NAME(AssertingDirectoryReader)

public:
  class AssertingSubReaderWrapper : public SubReaderWrapper
  {
    GET_CLASS_NAME(AssertingSubReaderWrapper)
  public:
    std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) override;

  protected:
    std::shared_ptr<AssertingSubReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<AssertingSubReaderWrapper>(
          SubReaderWrapper::shared_from_this());
    }
  };

public:
  AssertingDirectoryReader(std::shared_ptr<DirectoryReader> in_) throw(
      IOException);

protected:
  std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
      std::shared_ptr<DirectoryReader> in_)  override;

public:
  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<AssertingDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<AssertingDirectoryReader>(
        FilterDirectoryReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
