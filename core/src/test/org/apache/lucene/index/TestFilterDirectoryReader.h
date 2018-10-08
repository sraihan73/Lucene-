#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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

using SubReaderWrapper =
    org::apache::lucene::index::FilterDirectoryReader::SubReaderWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFilterDirectoryReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestFilterDirectoryReader)

private:
  class DummySubReaderWrapper : public SubReaderWrapper
  {
    GET_CLASS_NAME(DummySubReaderWrapper)

  public:
    std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) override;

  protected:
    std::shared_ptr<DummySubReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<DummySubReaderWrapper>(
          org.apache.lucene.index.FilterDirectoryReader
              .SubReaderWrapper::shared_from_this());
    }
  };

private:
  class DummyFilterDirectoryReader : public FilterDirectoryReader
  {
    GET_CLASS_NAME(DummyFilterDirectoryReader)

  public:
    DummyFilterDirectoryReader(std::shared_ptr<DirectoryReader> in_) throw(
        IOException);

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<DummyFilterDirectoryReader> shared_from_this()
    {
      return std::static_pointer_cast<DummyFilterDirectoryReader>(
          FilterDirectoryReader::shared_from_this());
    }
  };

public:
  virtual void testDoubleClose() ;

protected:
  std::shared_ptr<TestFilterDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<TestFilterDirectoryReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
