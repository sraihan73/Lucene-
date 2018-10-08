#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

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
 * {@link MismatchedLeafReader}
 */
class MismatchedDirectoryReader : public FilterDirectoryReader
{
  GET_CLASS_NAME(MismatchedDirectoryReader)

public:
  class MismatchedSubReaderWrapper : public SubReaderWrapper
  {
    GET_CLASS_NAME(MismatchedSubReaderWrapper)
  public:
    const std::shared_ptr<Random> random;

    MismatchedSubReaderWrapper(std::shared_ptr<Random> random);

    std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) override;

  protected:
    std::shared_ptr<MismatchedSubReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<MismatchedSubReaderWrapper>(
          SubReaderWrapper::shared_from_this());
    }
  };

public:
  MismatchedDirectoryReader(std::shared_ptr<DirectoryReader> in_,
                            std::shared_ptr<Random> random) ;

protected:
  std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
      std::shared_ptr<DirectoryReader> in_)  override;

public:
  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<MismatchedDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<MismatchedDirectoryReader>(
        FilterDirectoryReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
