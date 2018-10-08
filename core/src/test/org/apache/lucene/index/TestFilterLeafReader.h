#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Terms.h"

#include  "core/src/java/org/apache/lucene/index/TestTerms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TestTermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFilterLeafReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestFilterLeafReader)

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

      /** Scan for terms containing the letter 'e'.*/
      std::shared_ptr<BytesRef> next()  override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse,
               int flags)  override;

    protected:
      std::shared_ptr<TestTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<TestTermsEnum>(
            FilterTermsEnum::shared_from_this());
      }
    };

    /** Filter that only returns odd numbered documents. */
  private:
    class TestPositions : public FilterPostingsEnum
    {
      GET_CLASS_NAME(TestPositions)
    public:
      TestPositions(std::shared_ptr<PostingsEnum> in_);

      /** Scan for odd numbered documents. */
      int nextDoc()  override;

    protected:
      std::shared_ptr<TestPositions> shared_from_this()
      {
        return std::static_pointer_cast<TestPositions>(
            FilterPostingsEnum::shared_from_this());
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
   * Tests the IndexReader.getFieldNames implementation
   * @throws Exception on error
   */
public:
  virtual void testFilterIndexReader() ;

private:
  static void
  checkOverrideMethods(std::type_info clazz) throw(NoSuchMethodException,
                                                   SecurityException);

public:
  virtual void testOverrideMethods() ;

  virtual void testUnwrap() ;

private:
  class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
  {
    GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
  private:
    std::shared_ptr<TestFilterLeafReader> outerInstance;

  public:
    FilterLeafReaderAnonymousInnerClass(
        std::shared_ptr<TestFilterLeafReader> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LeafReader> r);

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
  std::shared_ptr<TestFilterLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<TestFilterLeafReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
