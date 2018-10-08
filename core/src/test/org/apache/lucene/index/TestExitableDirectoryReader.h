#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Terms;
}

namespace org::apache::lucene::index
{
class TestTerms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class TestTermsEnum;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test that uses a default/lucene Implementation of {@link QueryTimeout}
 * to exit out long running queries that take too long to iterate over Terms.
 */
class TestExitableDirectoryReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestExitableDirectoryReader)
private:
  class TestReader : public FilterLeafReader
  {
    GET_CLASS_NAME(TestReader)

  private:
    class TestTerms : public FilterTerms
    {
      GET_CLASS_NAME(TestTerms)
    public:
      TestTerms(std::shared_ptr<Terms> in_);

      std::shared_ptr<TermsEnum> iterator()  override;

    protected:
      std::shared_ptr<TestTerms> shared_from_this()
      {
        return std::static_pointer_cast<TestTerms>(
            FilterTerms::shared_from_this());
      }
    };

  private:
    class TestTermsEnum : public FilterTermsEnum
    {
      GET_CLASS_NAME(TestTermsEnum)
    public:
      TestTermsEnum(std::shared_ptr<TermsEnum> in_);

      /**
       * Sleep between iterations to timeout things.
       */
      std::shared_ptr<BytesRef> next()  override;

    protected:
      std::shared_ptr<TestTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<TestTermsEnum>(
            FilterTermsEnum::shared_from_this());
      }
    };

  public:
    TestReader(std::shared_ptr<LeafReader> reader) ;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<TestReader> shared_from_this()
    {
      return std::static_pointer_cast<TestReader>(
          FilterLeafReader::shared_from_this());
    }
  };

  /**
   * Tests timing out of TermsEnum iterations
   * @throws Exception on error
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore("this test relies on wall clock time and sometimes
  // false fails") public void testExitableFilterIndexReader() throws Exception
  virtual void testExitableFilterIndexReader() ;

protected:
  std::shared_ptr<TestExitableDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<TestExitableDirectoryReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
