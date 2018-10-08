#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DirectoryReader;
}

namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class TermState;
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
namespace org::apache::lucene::search
{

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using Terms = org::apache::lucene::index::Terms;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTermQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermQuery)

public:
  virtual void testEquals() ;

  virtual void
  testCreateWeightDoesNotSeekIfScoresAreNotNeeded() ;

private:
  class NoSeekDirectoryReader : public FilterDirectoryReader
  {
    GET_CLASS_NAME(NoSeekDirectoryReader)

  public:
    NoSeekDirectoryReader(std::shared_ptr<DirectoryReader> in_) throw(
        IOException);

  private:
    class SubReaderWrapperAnonymousInnerClass : public SubReaderWrapper
    {
      GET_CLASS_NAME(SubReaderWrapperAnonymousInnerClass)
    public:
      SubReaderWrapperAnonymousInnerClass();

      std::shared_ptr<LeafReader>
      wrap(std::shared_ptr<LeafReader> reader) override;

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
    std::shared_ptr<NoSeekDirectoryReader> shared_from_this()
    {
      return std::static_pointer_cast<NoSeekDirectoryReader>(
          org.apache.lucene.index.FilterDirectoryReader::shared_from_this());
    }
  };

private:
  class NoSeekLeafReader : public FilterLeafReader
  {
    GET_CLASS_NAME(NoSeekLeafReader)

  public:
    NoSeekLeafReader(std::shared_ptr<LeafReader> in_);

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  private:
    class FilterTermsAnonymousInnerClass : public FilterTerms
    {
      GET_CLASS_NAME(FilterTermsAnonymousInnerClass)
    private:
      std::shared_ptr<NoSeekLeafReader> outerInstance;

    public:
      FilterTermsAnonymousInnerClass(
          std::shared_ptr<NoSeekLeafReader> outerInstance,
          std::shared_ptr<Terms> terms);

      std::shared_ptr<TermsEnum> iterator()  override;

    private:
      class FilterTermsEnumAnonymousInnerClass : public FilterTermsEnum
      {
        GET_CLASS_NAME(FilterTermsEnumAnonymousInnerClass)
      private:
        std::shared_ptr<FilterTermsAnonymousInnerClass> outerInstance;

      public:
        FilterTermsEnumAnonymousInnerClass(
            std::shared_ptr<FilterTermsAnonymousInnerClass> outerInstance,
            std::shared_ptr<UnknownType> iterator);

        SeekStatus
        seekCeil(std::shared_ptr<BytesRef> text)  override;
        void
        seekExact(std::shared_ptr<BytesRef> term,
                  std::shared_ptr<TermState> state)  override;
        bool
        seekExact(std::shared_ptr<BytesRef> text)  override;
        void seekExact(int64_t ord)  override;

      protected:
        std::shared_ptr<FilterTermsEnumAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterTermsEnumAnonymousInnerClass>(
              FilterTermsEnum::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<FilterTermsAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterTermsAnonymousInnerClass>(
            FilterTerms::shared_from_this());
      }
    };

  public:
    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<NoSeekLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<NoSeekLeafReader>(
          org.apache.lucene.index.FilterLeafReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestTermQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestTermQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
