#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

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
namespace org::apache::lucene::search::join
{

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestQueryBitSetProducer : public LuceneTestCase
{
  GET_CLASS_NAME(TestQueryBitSetProducer)

public:
  virtual void testSimple() ;

  virtual void testReaderNotSuitedForCaching() ;

  // a reader whose sole purpose is to not be cacheable
private:
  class DummyDirectoryReader : public FilterDirectoryReader
  {
    GET_CLASS_NAME(DummyDirectoryReader)

  public:
    DummyDirectoryReader(std::shared_ptr<DirectoryReader> in_) throw(
        IOException);

  private:
    class SubReaderWrapperAnonymousInnerClass : public SubReaderWrapper
    {
      GET_CLASS_NAME(SubReaderWrapperAnonymousInnerClass)
    public:
      SubReaderWrapperAnonymousInnerClass();

      std::shared_ptr<LeafReader>
      wrap(std::shared_ptr<LeafReader> reader) override;

    private:
      class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
      {
        GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
      private:
        std::shared_ptr<SubReaderWrapperAnonymousInnerClass> outerInstance;

      public:
        FilterLeafReaderAnonymousInnerClass(
            std::shared_ptr<SubReaderWrapperAnonymousInnerClass> outerInstance,
            std::shared_ptr<LeafReader> reader);

        std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

        std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

      protected:
        std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
              org.apache.lucene.index.FilterLeafReader::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<SubReaderWrapperAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SubReaderWrapperAnonymousInnerClass>(
            SubReaderWrapper::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<DummyDirectoryReader> shared_from_this()
    {
      return std::static_pointer_cast<DummyDirectoryReader>(
          org.apache.lucene.index.FilterDirectoryReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestQueryBitSetProducer> shared_from_this()
  {
    return std::static_pointer_cast<TestQueryBitSetProducer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
