#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class LeafReader;
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
namespace org::apache::lucene::search::uhighlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using namespace org::apache::lucene::index;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests highlighting for matters *expressly* relating to term vectors.
 * <p>
 * This test DOES NOT represent all testing for highlighting when term vectors
 * are used.  Other tests pick the offset source at random (to include term
 * vectors) and in-effect test term vectors generally.
 */
class TestUnifiedHighlighterTermVec : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighterTermVec)

private:
  std::shared_ptr<Analyzer> indexAnalyzer;
  std::shared_ptr<Directory> dir;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
  virtual void doBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
  virtual void doAfter() ;

  virtual void testFetchTermVecsOncePerDoc() ;

private:
  class AssertOnceTermVecDirectoryReader : public FilterDirectoryReader
  {
    GET_CLASS_NAME(AssertOnceTermVecDirectoryReader)
  public:
    static const std::shared_ptr<SubReaderWrapper> SUB_READER_WRAPPER;

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
            std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

        std::shared_ptr<BitSet> seenDocIDs;

        std::shared_ptr<Fields>
        getTermVectors(int docID)  override;

        std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

        std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

      protected:
        std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
              FilterLeafReader::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<SubReaderWrapperAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SubReaderWrapperAnonymousInnerClass>(
            SubReaderWrapper::shared_from_this());
      }
    };

  public:
    AssertOnceTermVecDirectoryReader(
        std::shared_ptr<DirectoryReader> in_) ;

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<AssertOnceTermVecDirectoryReader> shared_from_this()
    {
      return std::static_pointer_cast<AssertOnceTermVecDirectoryReader>(
          FilterDirectoryReader::shared_from_this());
    }
  };

private:
  static bool calledBy(std::type_info clazz);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
  // testUserFailedToIndexOffsets() throws java.io.IOException
  virtual void testUserFailedToIndexOffsets() ;

protected:
  std::shared_ptr<TestUnifiedHighlighterTermVec> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighterTermVec>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
