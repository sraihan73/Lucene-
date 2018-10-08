#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReader;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class CacheKey;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 */
class TestIndexReaderClose : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexReaderClose)

public:
  virtual void testCloseUnderException() ;

private:
  class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
  {
    GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexReaderClose> outerInstance;

    bool throwOnClose = false;

  public:
    FilterLeafReaderAnonymousInnerClass(
        std::shared_ptr<TestIndexReaderClose> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LeafReader> leaf,
        bool throwOnClose);

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    void doClose()  override;

  protected:
    std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
          FilterLeafReader::shared_from_this());
    }
  };

public:
  virtual void
  testCoreListenerOnWrapperWithDifferentCacheKey() ;

private:
  class CountListener final
      : public std::enable_shared_from_this<CountListener>,
        public IndexReader::ClosedListener
  {
    GET_CLASS_NAME(CountListener)

  private:
    const std::shared_ptr<AtomicInteger> count;
    const std::any coreCacheKey;

  public:
    CountListener(std::shared_ptr<AtomicInteger> count, std::any coreCacheKey);

    void onClose(std::shared_ptr<IndexReader::CacheKey> coreCacheKey) override;
  };

private:
  class FaultyListener final
      : public std::enable_shared_from_this<FaultyListener>,
        public IndexReader::ClosedListener
  {
    GET_CLASS_NAME(FaultyListener)

  public:
    void onClose(std::shared_ptr<IndexReader::CacheKey> cacheKey) override;
  };

public:
  virtual void testRegisterListenerOnClosedReader() ;

protected:
  std::shared_ptr<TestIndexReaderClose> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexReaderClose>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
