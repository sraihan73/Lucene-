#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/OneMerge.h"

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

using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestConcurrentMergeScheduler : public LuceneTestCase
{
  GET_CLASS_NAME(TestConcurrentMergeScheduler)

private:
  class FailOnlyOnFlush : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyOnFlush)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

  public:
    FailOnlyOnFlush(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance);

    bool doFail = false;
    bool hitExc = false;

    void setDoFail() override;
    void clearDoFail() override;

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyOnFlush> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyOnFlush>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // Make sure running BG merges still work fine even when
  // we are hitting exceptions during flushing.
public:
  virtual void testFlushExceptions() ;

private:
  class SuppressingConcurrentMergeSchedulerAnonymousInnerClass
      : public SuppressingConcurrentMergeScheduler
  {
    GET_CLASS_NAME(SuppressingConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

  public:
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance);

  protected:
    bool isOK(std::runtime_error th) override;

  protected:
    std::shared_ptr<SuppressingConcurrentMergeSchedulerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SuppressingConcurrentMergeSchedulerAnonymousInnerClass>(
          SuppressingConcurrentMergeScheduler::shared_from_this());
    }
  };

  // Test that deletes committed after a merge started and
  // before it finishes, are correctly merged back:
public:
  virtual void testDeleteMerging() ;

  virtual void testNoExtraFiles() ;

  virtual void testNoWaitClose() ;

  // LUCENE-4544
  virtual void testMaxMergeCount() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

    int maxMergeCount = 0;
    std::shared_ptr<CountDownLatch> enoughMergesWaiting;
    std::shared_ptr<AtomicInteger> runningMergeCount;
    std::shared_ptr<AtomicBoolean> failed;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        int maxMergeCount, std::shared_ptr<CountDownLatch> enoughMergesWaiting,
        std::shared_ptr<AtomicInteger> runningMergeCount,
        std::shared_ptr<AtomicBoolean> failed);

  protected:
    void doMerge(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<MergePolicy::OneMerge>
                     merge)  override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

private:
  class TrackingCMS : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(TrackingCMS)
  public:
    int64_t totMergedBytes = 0;
    std::shared_ptr<CountDownLatch> atLeastOneMerge;

    TrackingCMS(std::shared_ptr<CountDownLatch> atLeastOneMerge);

    void doMerge(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<MergePolicy::OneMerge>
                     merge)  override;

  protected:
    std::shared_ptr<TrackingCMS> shared_from_this()
    {
      return std::static_pointer_cast<TrackingCMS>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

public:
  virtual void testTotalBytesSize() ;

  virtual void testInvalidMaxMergeCountAndThreads() ;

  virtual void testLiveMaxMergeCount() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

    std::shared_ptr<AtomicInteger> maxRunningMergeCount;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        std::shared_ptr<AtomicInteger> maxRunningMergeCount);

    const std::shared_ptr<AtomicInteger> runningMergeCount;

    void doMerge(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<MergePolicy::OneMerge>
                     merge)  override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

  // LUCENE-6063
public:
  virtual void testMaybeStallCalled() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass2
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

    std::shared_ptr<AtomicBoolean> wasCalled;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass2(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        std::shared_ptr<AtomicBoolean> wasCalled);

  protected:
    bool maybeStall(std::shared_ptr<IndexWriter> writer) override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass2>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

  // LUCENE-6094
public:
  virtual void testHangDuringRollback() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass3
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

    std::shared_ptr<CountDownLatch> mergeStart;
    std::shared_ptr<CountDownLatch> mergeFinish;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass3(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        std::shared_ptr<CountDownLatch> mergeStart,
        std::shared_ptr<CountDownLatch> mergeFinish);

  protected:
    void doMerge(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<MergePolicy::OneMerge>
                     merge)  override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass3>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

    std::shared_ptr<CountDownLatch> mergeFinish;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        std::shared_ptr<CountDownLatch> mergeFinish,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testDynamicDefaults() ;

  virtual void testResetToAutoDefault() ;

  virtual void testSpinningDefaults() ;

  virtual void testAutoIOThrottleGetter() ;

  virtual void testNonSpinningDefaults() ;

  // LUCENE-6197
  virtual void testNoStallMergeThreads() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass4
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestConcurrentMergeScheduler> outerInstance;

    std::shared_ptr<AtomicBoolean> failed;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass4(
        std::shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        std::shared_ptr<AtomicBoolean> failed);

  protected:
    void doStall() override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass4>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestConcurrentMergeScheduler> shared_from_this()
  {
    return std::static_pointer_cast<TestConcurrentMergeScheduler>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
