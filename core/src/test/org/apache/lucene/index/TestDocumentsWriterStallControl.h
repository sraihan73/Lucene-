#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterStallControl.h"

#include  "core/src/java/org/apache/lucene/index/Synchronizer.h"

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
 * Tests for {@link DocumentsWriterStallControl}
 */
class TestDocumentsWriterStallControl : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocumentsWriterStallControl)

public:
  virtual void testSimpleStall() ;

  virtual void testRandom() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocumentsWriterStallControl> outerInstance;

    std::shared_ptr<org::apache::lucene::index::DocumentsWriterStallControl>
        ctrl;
    int stallProbability = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestDocumentsWriterStallControl> outerInstance,
        std::shared_ptr<org::apache::lucene::index::DocumentsWriterStallControl>
            ctrl,
        int stallProbability);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testAccquireReleaseRace() ;

private:
  void assertState(int numReleasers, int numStallers, int numWaiters,
                   std::deque<std::shared_ptr<Thread>> &threads,
                   std::shared_ptr<DocumentsWriterStallControl>
                       ctrl) ;

public:
  class Waiter : public Thread
  {
    GET_CLASS_NAME(Waiter)
  private:
    std::shared_ptr<Synchronizer> sync;
    std::shared_ptr<DocumentsWriterStallControl> ctrl;
    std::shared_ptr<AtomicBoolean> checkPoint;
    std::shared_ptr<AtomicBoolean> stop;
    std::deque<std::runtime_error> exceptions;

  public:
    Waiter(std::shared_ptr<AtomicBoolean> stop,
           std::shared_ptr<AtomicBoolean> checkPoint,
           std::shared_ptr<DocumentsWriterStallControl> ctrl,
           std::shared_ptr<Synchronizer> sync,
           std::deque<std::runtime_error> &exceptions);

    void run() override;

  protected:
    std::shared_ptr<Waiter> shared_from_this()
    {
      return std::static_pointer_cast<Waiter>(Thread::shared_from_this());
    }
  };

public:
  class Updater : public Thread
  {
    GET_CLASS_NAME(Updater)

  private:
    std::shared_ptr<Synchronizer> sync;
    std::shared_ptr<DocumentsWriterStallControl> ctrl;
    std::shared_ptr<AtomicBoolean> checkPoint;
    std::shared_ptr<AtomicBoolean> stop;
    bool release = false;
    std::deque<std::runtime_error> exceptions;

  public:
    Updater(std::shared_ptr<AtomicBoolean> stop,
            std::shared_ptr<AtomicBoolean> checkPoint,
            std::shared_ptr<DocumentsWriterStallControl> ctrl,
            std::shared_ptr<Synchronizer> sync, bool release,
            std::deque<std::runtime_error> &exceptions);

    void run() override;

  protected:
    std::shared_ptr<Updater> shared_from_this()
    {
      return std::static_pointer_cast<Updater>(Thread::shared_from_this());
    }
  };

public:
  static bool terminated(std::deque<std::shared_ptr<Thread>> &threads);

  static void start(std::deque<std::shared_ptr<Thread>> &tostart) throw(
      InterruptedException);

  static void join(std::deque<std::shared_ptr<Thread>> &toJoin) throw(
      InterruptedException);

  static std::deque<std::shared_ptr<Thread>>
  waitThreads(int num, std::shared_ptr<DocumentsWriterStallControl> ctrl);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::DocumentsWriterStallControl>
        ctrl;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::DocumentsWriterStallControl>
            ctrl);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /** Waits for all incoming threads to be in wait()
   *  methods. */
public:
  static void
  awaitState(std::shared_ptr<Thread::State> state,
             std::deque<Thread> &threads) ;

private:
  class Synchronizer final : public std::enable_shared_from_this<Synchronizer>
  {
    GET_CLASS_NAME(Synchronizer)
  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile java.util.concurrent.CountDownLatch waiter;
    std::shared_ptr<CountDownLatch> waiter;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile java.util.concurrent.CountDownLatch updateJoin;
    std::shared_ptr<CountDownLatch> updateJoin;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile java.util.concurrent.CountDownLatch
    // leftCheckpoint;
    std::shared_ptr<CountDownLatch> leftCheckpoint;

    Synchronizer(int numUpdater, int numThreads);

    void reset(int numUpdaters, int numThreads);

    bool await() ;
  };

protected:
  std::shared_ptr<TestDocumentsWriterStallControl> shared_from_this()
  {
    return std::static_pointer_cast<TestDocumentsWriterStallControl>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
