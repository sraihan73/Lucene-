#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SearcherLifetimeManager;
}

namespace org::apache::lucene::search
{
class Pruner;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class SearcherManager;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class CacheHelper;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::analysis
{
class MockAnalyzer;
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
using ThreadedIndexingAndSearchingTestCase =
    org::apache::lucene::index::ThreadedIndexingAndSearchingTestCase;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" }) public
// class TestSearcherManager extends
// org.apache.lucene.index.ThreadedIndexingAndSearchingTestCase
class TestSearcherManager : public ThreadedIndexingAndSearchingTestCase
{

public:
  bool warmCalled = false;

private:
  std::shared_ptr<SearcherLifetimeManager::Pruner> pruner;

public:
  virtual void testSearcherManager() ;

protected:
  std::shared_ptr<IndexSearcher>
  getFinalSearcher()  override;

private:
  std::shared_ptr<SearcherManager> mgr;
  std::shared_ptr<SearcherLifetimeManager> lifetimeMGR;
  const std::deque<int64_t> pastSearchers = std::deque<int64_t>();
  bool isNRT = false;

protected:
  void doAfterWriter(std::shared_ptr<ExecutorService> es) throw(
      std::runtime_error) override;

private:
  class SearcherFactoryAnonymousInnerClass : public SearcherFactory
  {
    GET_CLASS_NAME(SearcherFactoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<ExecutorService> es;

  public:
    SearcherFactoryAnonymousInnerClass(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<ExecutorService> es);

    std::shared_ptr<IndexSearcher> LuceneTestCase::newSearcher(
        std::shared_ptr<IndexReader> r,
        std::shared_ptr<IndexReader> previous)  override;

  protected:
    std::shared_ptr<SearcherFactoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SearcherFactoryAnonymousInnerClass>(
          SearcherFactory::shared_from_this());
    }
  };

protected:
  void doSearching(std::shared_ptr<ExecutorService> es,
                   int64_t const stopTime)  override;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    int64_t stopTime = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestSearcherManager> outerInstance, int64_t stopTime);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IndexSearcher>
  getCurrentSearcher()  override;

  void releaseSearcher(std::shared_ptr<IndexSearcher> s) throw(
      std::runtime_error) override;

  void doClose()  override;

public:
  virtual void testIntermediateClose() ;

private:
  class SearcherFactoryAnonymousInnerClass2 : public SearcherFactory
  {
    GET_CLASS_NAME(SearcherFactoryAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<CountDownLatch> awaitEnterWarm;
    std::shared_ptr<CountDownLatch> awaitClose;
    std::shared_ptr<AtomicBoolean> triedReopen;
    std::shared_ptr<ExecutorService> es;

  public:
    SearcherFactoryAnonymousInnerClass2(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<CountDownLatch> awaitEnterWarm,
        std::shared_ptr<CountDownLatch> awaitClose,
        std::shared_ptr<AtomicBoolean> triedReopen,
        std::shared_ptr<ExecutorService> es);

    std::shared_ptr<IndexSearcher>
    LuceneTestCase::newSearcher(std::shared_ptr<IndexReader> r,
                                std::shared_ptr<IndexReader> previous) override;

  protected:
    std::shared_ptr<SearcherFactoryAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SearcherFactoryAnonymousInnerClass2>(
          SearcherFactory::shared_from_this());
    }
  };

public:
  virtual void testCloseTwice() ;

  virtual void testReferenceDecrementIllegally() ;

  virtual void testEnsureOpen() ;

  virtual void testListenerCalled() ;

private:
  class RefreshListenerAnonymousInnerClass
      : public std::enable_shared_from_this<RefreshListenerAnonymousInnerClass>,
        public ReferenceManager::RefreshListener
  {
    GET_CLASS_NAME(RefreshListenerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<AtomicBoolean> afterRefreshCalled;

  public:
    RefreshListenerAnonymousInnerClass(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<AtomicBoolean> afterRefreshCalled);

    void beforeRefresh() override;
    void afterRefresh(bool didRefresh) override;
  };

public:
  virtual void testEvilSearcherFactory() ;

private:
  class SearcherFactoryAnonymousInnerClass3 : public SearcherFactory
  {
    GET_CLASS_NAME(SearcherFactoryAnonymousInnerClass3)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<IndexReader> other;

  public:
    SearcherFactoryAnonymousInnerClass3(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<IndexReader> other);

    std::shared_ptr<IndexSearcher>
    LuceneTestCase::newSearcher(std::shared_ptr<IndexReader> ignored,
                                std::shared_ptr<IndexReader> previous) override;

  protected:
    std::shared_ptr<SearcherFactoryAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<SearcherFactoryAnonymousInnerClass3>(
          SearcherFactory::shared_from_this());
    }
  };

public:
  virtual void testMaybeRefreshBlockingLock() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<org::apache::lucene::search::SearcherManager> sm;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<org::apache::lucene::search::SearcherManager> sm);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

private:
  class MyFilterLeafReader : public FilterLeafReader
  {
    GET_CLASS_NAME(MyFilterLeafReader)
  public:
    MyFilterLeafReader(std::shared_ptr<LeafReader> in_);

    std::shared_ptr<IndexReader::CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<MyFilterLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<MyFilterLeafReader>(
          org.apache.lucene.index.FilterLeafReader::shared_from_this());
    }
  };

private:
  class MyFilterDirectoryReader : public FilterDirectoryReader
  {
    GET_CLASS_NAME(MyFilterDirectoryReader)
  public:
    MyFilterDirectoryReader(std::shared_ptr<DirectoryReader> in_) throw(
        IOException);

  private:
    class SubReaderWrapperAnonymousInnerClass
        : public FilterDirectoryReader::SubReaderWrapper
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
            org.apache.lucene.index.FilterDirectoryReader
                .SubReaderWrapper::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<MyFilterDirectoryReader> shared_from_this()
    {
      return std::static_pointer_cast<MyFilterDirectoryReader>(
          org.apache.lucene.index.FilterDirectoryReader::shared_from_this());
    }
  };

  // LUCENE-6087
public:
  virtual void testCustomDirectoryReader() ;

  virtual void testPreviousReaderIsPassed() ;

  virtual void
  testConcurrentIndexCloseSearchAndRefresh() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<Directory> dir;
    std::shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef;
    std::shared_ptr<MockAnalyzer> analyzer;
    std::shared_ptr<AtomicBoolean> stop;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<Directory> dir,
        std::shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>>
            writerRef,
        std::shared_ptr<MockAnalyzer> analyzer,
        std::shared_ptr<AtomicBoolean> stop);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef;
    std::shared_ptr<AtomicBoolean> stop;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>>
            mgrRef,
        std::shared_ptr<AtomicBoolean> stop);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass3 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass3)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef;
    std::shared_ptr<AtomicBoolean> stop;

  public:
    ThreadAnonymousInnerClass3(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>>
            mgrRef,
        std::shared_ptr<AtomicBoolean> stop);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass3>(
          Thread::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass4 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass4)
  private:
    std::shared_ptr<TestSearcherManager> outerInstance;

    std::shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef;
    std::shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef;
    std::shared_ptr<AtomicBoolean> stop;

  public:
    ThreadAnonymousInnerClass4(
        std::shared_ptr<TestSearcherManager> outerInstance,
        std::shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>>
            writerRef,
        std::shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>>
            mgrRef,
        std::shared_ptr<AtomicBoolean> stop);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass4>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSearcherManager> shared_from_this()
  {
    return std::static_pointer_cast<TestSearcherManager>(
        org.apache.lucene.index
            .ThreadedIndexingAndSearchingTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
