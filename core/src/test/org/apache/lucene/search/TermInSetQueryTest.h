#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/CacheHelper.h"

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

class TermInSetQueryTest : public LuceneTestCase
{
  GET_CLASS_NAME(TermInSetQueryTest)

public:
  virtual void testDuel() ;

private:
  void assertSameMatches(std::shared_ptr<IndexSearcher> searcher,
                         std::shared_ptr<Query> q1, std::shared_ptr<Query> q2,
                         bool scores) ;

public:
  virtual void testHashCodeAndEquals();

  virtual void testSimpleEquals();

  virtual void testToString();

  virtual void testDedup();

  virtual void testOrderDoesNotMatter();

  virtual void testRamBytesUsed();

private:
  class TermsCountingDirectoryReaderWrapper : public FilterDirectoryReader
  {
    GET_CLASS_NAME(TermsCountingDirectoryReaderWrapper)

  private:
    const std::shared_ptr<AtomicInteger> counter;

  public:
    TermsCountingDirectoryReaderWrapper(
        std::shared_ptr<DirectoryReader> in_,
        std::shared_ptr<AtomicInteger> counter) ;

  private:
    class TermsCountingSubReaderWrapper : public SubReaderWrapper
    {
      GET_CLASS_NAME(TermsCountingSubReaderWrapper)
    private:
      const std::shared_ptr<AtomicInteger> counter;

    public:
      TermsCountingSubReaderWrapper(std::shared_ptr<AtomicInteger> counter);

      std::shared_ptr<LeafReader>
      wrap(std::shared_ptr<LeafReader> reader) override;

    protected:
      std::shared_ptr<TermsCountingSubReaderWrapper> shared_from_this()
      {
        return std::static_pointer_cast<TermsCountingSubReaderWrapper>(
            SubReaderWrapper::shared_from_this());
      }
    };

  private:
    class TermsCountingLeafReaderWrapper : public FilterLeafReader
    {
      GET_CLASS_NAME(TermsCountingLeafReaderWrapper)

    private:
      const std::shared_ptr<AtomicInteger> counter;

    public:
      TermsCountingLeafReaderWrapper(std::shared_ptr<LeafReader> in_,
                                     std::shared_ptr<AtomicInteger> counter);

      std::shared_ptr<Terms>
      terms(const std::wstring &field)  override;

    private:
      class FilterTermsAnonymousInnerClass : public FilterTerms
      {
        GET_CLASS_NAME(FilterTermsAnonymousInnerClass)
      private:
        std::shared_ptr<TermsCountingLeafReaderWrapper> outerInstance;

      public:
        FilterTermsAnonymousInnerClass(
            std::shared_ptr<TermsCountingLeafReaderWrapper> outerInstance,
            std::shared_ptr<Terms> terms);

        std::shared_ptr<TermsEnum> iterator()  override;

      protected:
        std::shared_ptr<FilterTermsAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterTermsAnonymousInnerClass>(
              FilterTerms::shared_from_this());
        }
      };

    public:
      std::shared_ptr<IndexReader::CacheHelper> getCoreCacheHelper() override;

      std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

    protected:
      std::shared_ptr<TermsCountingLeafReaderWrapper> shared_from_this()
      {
        return std::static_pointer_cast<TermsCountingLeafReaderWrapper>(
            org.apache.lucene.index.FilterLeafReader::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<TermsCountingDirectoryReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<TermsCountingDirectoryReaderWrapper>(
          org.apache.lucene.index.FilterDirectoryReader::shared_from_this());
    }
  };

public:
  virtual void testPullOneTermsEnum() ;

  virtual void testBinaryToString();

  virtual void testIsConsideredCostlyByQueryCache() ;

protected:
  std::shared_ptr<TermInSetQueryTest> shared_from_this()
  {
    return std::static_pointer_cast<TermInSetQueryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
