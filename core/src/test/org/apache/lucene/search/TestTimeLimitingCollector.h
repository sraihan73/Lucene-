#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/util/Counter.h"
#include  "core/src/java/org/apache/lucene/search/TimeLimitingCollector.h"
#include  "core/src/java/org/apache/lucene/search/TimerThread.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/search/MyHitCollector.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using TimerThread =
    org::apache::lucene::search::TimeLimitingCollector::TimerThread;
using Directory = org::apache::lucene::store::Directory;
using Counter = org::apache::lucene::util::Counter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests the {@link TimeLimitingCollector}.  This test checks (1) search
 * correctness (regardless of timeout), (2) expected timeout behavior,
 * and (3) a sanity test with multiple searching threads.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressSysoutChecks(bugUrl =
// "http://test.is.timing.sensitive.so.it.prints.instead.of.failing") public
// class TestTimeLimitingCollector extends org.apache.lucene.util.LuceneTestCase
class TestTimeLimitingCollector : public LuceneTestCase
{
private:
  static constexpr int SLOW_DOWN = 3;
  static const int64_t TIME_ALLOWED =
      17 * SLOW_DOWN; // so searches can find about 17 docs.

  // max time allowed is relaxed for multithreading tests.
  // the multithread case fails when setting this to 1 (no slack) and launching
  // many threads (>2000). but this is not a real failure, just noise.
  static constexpr double MULTI_THREAD_SLACK = 7;

  static constexpr int N_DOCS = 3000;
  static constexpr int N_THREADS = 50;

  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;

  const std::wstring FIELD_NAME = L"body";
  std::shared_ptr<Query> query;
  std::shared_ptr<Counter> counter;
  std::shared_ptr<TimerThread> counterThread;

  /**
   * initializes searcher with a document set
   */
public:
  void setUp()  override;

  void tearDown()  override;

private:
  void add(const std::wstring &value,
           std::shared_ptr<RandomIndexWriter> iw) ;

  void search(std::shared_ptr<Collector> collector) ;

  /**
   * test search correctness with no timeout
   */
public:
  virtual void testSearch();

private:
  void doTestSearch();

  std::shared_ptr<Collector>
  createTimedCollector(std::shared_ptr<MyHitCollector> hc,
                       int64_t timeAllowed, bool greedy);

  /**
   * Test that timeout is obtained, and soon enough!
   */
public:
  virtual void testTimeoutGreedy();

  /**
   * Test that timeout is obtained, and soon enough!
   */
  virtual void testTimeoutNotGreedy();

private:
  void doTestTimeout(bool multiThreaded, bool greedy);

  int64_t maxTime(bool multiThreaded);

  std::wstring maxTimeStr(bool multiThreaded);

  /**
   * Test timeout behavior when resolution is modified.
   */
public:
  virtual void testModifyResolution();

  virtual void testNoHits() ;

  /**
   * Test correctness with multiple searching threads.
   */
  virtual void testSearchMultiThreaded() ;

  /**
   * Test correctness with multiple searching threads.
   */
  virtual void testTimeoutMultiThreaded() ;

private:
  void doTestMultiThreads(bool const withTimeout) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestTimeLimitingCollector> outerInstance;

    bool withTimeout = false;
    std::shared_ptr<BitSet> success;
    int num = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestTimeLimitingCollector> outerInstance,
        bool withTimeout, std::shared_ptr<BitSet> success, int num);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  // counting collector that can slow down at collect().
private:
  class MyHitCollector : public SimpleCollector
  {
    GET_CLASS_NAME(MyHitCollector)
  private:
    const std::shared_ptr<BitSet> bits = std::make_shared<BitSet>();
    int slowdown = 0;
    int lastDocCollected = -1;
    int docBase = 0;

    /**
     * amount of time to wait on each collect to simulate a long iteration
     */
  public:
    virtual void setSlowDown(int milliseconds);

    virtual int hitCount();

    virtual int getLastDocCollected();

    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int const doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<MyHitCollector> shared_from_this()
    {
      return std::static_pointer_cast<MyHitCollector>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestTimeLimitingCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestTimeLimitingCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
