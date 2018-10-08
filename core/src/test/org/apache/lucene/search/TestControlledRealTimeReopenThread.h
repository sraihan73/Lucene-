#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SearcherManager;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class LatchedIndexWriter;
}
namespace org::apache::lucene::search
{
template <typename T>
class ControlledRealTimeReopenThread;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
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

using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using Term = org::apache::lucene::index::Term;
using ThreadedIndexingAndSearchingTestCase =
    org::apache::lucene::index::ThreadedIndexingAndSearchingTestCase;
using Directory = org::apache::lucene::store::Directory;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" }) public
// class TestControlledRealTimeReopenThread extends
// org.apache.lucene.index.ThreadedIndexingAndSearchingTestCase
class TestControlledRealTimeReopenThread
    : public ThreadedIndexingAndSearchingTestCase
{

  // Not guaranteed to reflect deletes:
private:
  std::shared_ptr<SearcherManager> nrtNoDeletes;

  // Is guaranteed to reflect deletes:
  std::shared_ptr<SearcherManager> nrtDeletes;

  std::shared_ptr<IndexWriter> genWriter;

  std::shared_ptr<
      ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>
      nrtDeletesThread;
  std::shared_ptr<
      ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>
      nrtNoDeletesThread;

  const std::shared_ptr<ThreadLocal<int64_t>> lastGens =
      std::make_shared<ThreadLocal<int64_t>>();
  bool warmCalled = false;

public:
  virtual void testControlledRealTimeReopenThread() ;

protected:
  std::shared_ptr<IndexSearcher>
  getFinalSearcher()  override;

  std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Directory> in_) override;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override protected void
  // updateDocuments(org.apache.lucene.index.Term id, java.util.List<? extends
  // Iterable<? extends org.apache.lucene.index.IndexableField>> docs) throws
  // Exception
  void updateDocuments(std::shared_ptr<Term> id,
                       std::deque<T1> docs) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override protected void
  // addDocuments(org.apache.lucene.index.Term id, java.util.List<? extends
  // Iterable<? extends org.apache.lucene.index.IndexableField>> docs) throws
  // Exception
  void addDocuments(std::shared_ptr<Term> id,
                    std::deque<T1> docs) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override protected void
  // addDocument(org.apache.lucene.index.Term id, Iterable<? extends
  // org.apache.lucene.index.IndexableField> doc) throws Exception
  void addDocument(std::shared_ptr<Term> id,
                   std::deque<T1> doc) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override protected void
  // updateDocument(org.apache.lucene.index.Term id, Iterable<? extends
  // org.apache.lucene.index.IndexableField> doc) throws Exception
  void updateDocument(std::shared_ptr<Term> id,
                      std::deque<T1> doc) ;

  void
  deleteDocuments(std::shared_ptr<Term> id)  override;

  void doAfterWriter(std::shared_ptr<ExecutorService> es) throw(
      std::runtime_error) override;

private:
  class SearcherFactoryAnonymousInnerClass : public SearcherFactory
  {
    GET_CLASS_NAME(SearcherFactoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance;

    std::shared_ptr<ExecutorService> es;

  public:
    SearcherFactoryAnonymousInnerClass(
        std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
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
  void doAfterIndexingThreadDone() override;

private:
  int64_t maxGen = -1;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void addMaxGen(int64_t gen);

protected:
  void doSearching(std::shared_ptr<ExecutorService> es,
                   int64_t stopTime)  override;

  std::shared_ptr<IndexSearcher>
  getCurrentSearcher()  override;

  void releaseSearcher(std::shared_ptr<IndexSearcher> s) throw(
      std::runtime_error) override;

  void doClose()  override;

  /*
   * LUCENE-3528 - NRTManager hangs in certain situations
   */
public:
  virtual void
  testThreadStarvationNoDeleteNRTReader() throw(IOException,
                                                InterruptedException);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance;

    std::shared_ptr<CountDownLatch> latch;
    std::shared_ptr<CountDownLatch> signal;
    std::shared_ptr<org::apache::lucene::search::
                        TestControlledRealTimeReopenThread::LatchedIndexWriter>
        writer;
    std::shared_ptr<org::apache::lucene::search::SearcherManager> manager;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        std::shared_ptr<CountDownLatch> latch,
        std::shared_ptr<CountDownLatch> signal,
        std::shared_ptr<
            org::apache::lucene::search::TestControlledRealTimeReopenThread::
                LatchedIndexWriter>
            writer,
        std::shared_ptr<org::apache::lucene::search::SearcherManager> manager);

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
    std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance;

    int64_t lastGen = 0;
    std::shared_ptr<org::apache::lucene::search::ControlledRealTimeReopenThread<
        std::shared_ptr<IndexSearcher>>>
        thread;
    std::shared_ptr<AtomicBoolean> finished;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        int64_t lastGen,
        std::shared_ptr<
            org::apache::lucene::search::ControlledRealTimeReopenThread<
                std::shared_ptr<IndexSearcher>>>
            thread,
        std::shared_ptr<AtomicBoolean> finished);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  class LatchedIndexWriter : public IndexWriter
  {
    GET_CLASS_NAME(LatchedIndexWriter)

  private:
    std::shared_ptr<CountDownLatch> latch;

  public:
    bool waitAfterUpdate = false;

  private:
    std::shared_ptr<CountDownLatch> signal;

  public:
    LatchedIndexWriter(
        std::shared_ptr<Directory> d, std::shared_ptr<IndexWriterConfig> conf,
        std::shared_ptr<CountDownLatch> latch,
        std::shared_ptr<CountDownLatch> signal) ;

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public long
    // updateDocument(org.apache.lucene.index.Term term, Iterable<? extends
    // org.apache.lucene.index.IndexableField> doc) throws java.io.IOException
    int64_t updateDocument(std::shared_ptr<Term> term,
                             std::deque<T1> doc) ;

  protected:
    std::shared_ptr<LatchedIndexWriter> shared_from_this()
    {
      return std::static_pointer_cast<LatchedIndexWriter>(
          org.apache.lucene.index.IndexWriter::shared_from_this());
    }
  };

public:
  virtual void testEvilSearcherFactory() ;

private:
  class SearcherFactoryAnonymousInnerClass : public SearcherFactory
  {
    GET_CLASS_NAME(SearcherFactoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance;

    std::shared_ptr<IndexReader> other;

  public:
    SearcherFactoryAnonymousInnerClass(
        std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        std::shared_ptr<IndexReader> other);

    std::shared_ptr<IndexSearcher>
    LuceneTestCase::newSearcher(std::shared_ptr<IndexReader> ignored,
                                std::shared_ptr<IndexReader> previous) override;

  protected:
    std::shared_ptr<SearcherFactoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SearcherFactoryAnonymousInnerClass>(
          SearcherFactory::shared_from_this());
    }
  };

public:
  virtual void testListenerCalled() ;

private:
  class RefreshListenerAnonymousInnerClass
      : public std::enable_shared_from_this<RefreshListenerAnonymousInnerClass>,
        public ReferenceManager::RefreshListener
  {
    GET_CLASS_NAME(RefreshListenerAnonymousInnerClass)
  private:
    std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance;

    std::shared_ptr<AtomicBoolean> afterRefreshCalled;

  public:
    RefreshListenerAnonymousInnerClass(
        std::shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        std::shared_ptr<AtomicBoolean> afterRefreshCalled);

    void beforeRefresh() override;
    void afterRefresh(bool didRefresh) override;
  };

  // Relies on wall clock time, so it can easily false-fail when the machine is
  // otherwise busy:
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AwaitsFix(bugUrl =
  // "https://issues.apache.org/jira/browse/LUCENE-5737") public void
  // testCRTReopen() throws Exception
  virtual void testCRTReopen() ;

  virtual void testDeleteAll() ;

protected:
  std::shared_ptr<TestControlledRealTimeReopenThread> shared_from_this()
  {
    return std::static_pointer_cast<TestControlledRealTimeReopenThread>(
        org.apache.lucene.index
            .ThreadedIndexingAndSearchingTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
