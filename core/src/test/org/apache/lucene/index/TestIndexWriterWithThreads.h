#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class AlreadyClosedException;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}
namespace org::apache::lucene::store
{
class Failure;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class BaseDirectoryWrapper;
}
namespace org::apache::lucene::util
{
class LineFileDocs;
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

using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * MultiThreaded IndexWriter tests
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public class TestIndexWriterWithThreads extends
// org.apache.lucene.util.LuceneTestCase
class TestIndexWriterWithThreads : public LuceneTestCase
{

  // Used by test cases below
private:
  class IndexerThread : public Thread
  {
    GET_CLASS_NAME(IndexerThread)

  private:
    const std::shared_ptr<CyclicBarrier> syncStart;

  public:
    bool diskFull = false;
    std::runtime_error error;
    std::shared_ptr<AlreadyClosedException> ace;
    std::shared_ptr<IndexWriter> writer;
    bool noErrors = false;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile int addCount;
    int addCount = 0;

    IndexerThread(std::shared_ptr<IndexWriter> writer, bool noErrors,
                  std::shared_ptr<CyclicBarrier> syncStart);

    void run() override;

  protected:
    std::shared_ptr<IndexerThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexerThread>(
          Thread::shared_from_this());
    }
  };

  // LUCENE-1130: make sure immediate disk full on creating
  // an IndexWriter (hit during DW.ThreadState.init()), with
  // multiple threads, is OK:
public:
  virtual void testImmediateDiskFullWithThreads() ;

  // LUCENE-1130: make sure we can close() even while
  // threads are trying to add documents.  Strictly
  // speaking, this isn't valid us of Lucene's APIs, but we
  // still want to be robust to this case:
  virtual void testCloseWithThreads() ;

  // Runs test, with multiple threads, using the specific
  // failure to trigger an IOException
  virtual void
  _testMultipleThreadsFailure(std::shared_ptr<MockDirectoryWrapper::Failure>
                                  failure) ;

  // Runs test, with one thread, using the specific failure
  // to trigger an IOException
  virtual void
  _testSingleThreadFailure(std::shared_ptr<MockDirectoryWrapper::Failure>
                               failure) ;

private:
  class SuppressingConcurrentMergeSchedulerAnonymousInnerClass
      : public SuppressingConcurrentMergeScheduler
  {
    GET_CLASS_NAME(SuppressingConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterWithThreads> outerInstance;

  public:
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterWithThreads> outerInstance);

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

  // Throws IOException during FieldsWriter.flushDocument and during
  // DocumentsWriter.abort
private:
  class FailOnlyOnAbortOrFlush : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyOnAbortOrFlush)
  private:
    bool onlyOnce = false;

  public:
    FailOnlyOnAbortOrFlush(bool onlyOnce);
    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyOnAbortOrFlush> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyOnAbortOrFlush>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // LUCENE-1130: make sure initial IOException, and then 2nd
  // IOException during rollback(), is OK:
public:
  virtual void testIOExceptionDuringAbort() ;

  // LUCENE-1130: make sure initial IOException, and then 2nd
  // IOException during rollback(), is OK:
  virtual void testIOExceptionDuringAbortOnlyOnce() ;

  // LUCENE-1130: make sure initial IOException, and then 2nd
  // IOException during rollback(), with multiple threads, is OK:
  virtual void
  testIOExceptionDuringAbortWithThreads() ;

  // LUCENE-1130: make sure initial IOException, and then 2nd
  // IOException during rollback(), with multiple threads, is OK:
  virtual void
  testIOExceptionDuringAbortWithThreadsOnlyOnce() ;

  // Throws IOException during DocumentsWriter.writeSegment
private:
  class FailOnlyInWriteSegment : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyInWriteSegment)
  private:
    bool onlyOnce = false;

  public:
    FailOnlyInWriteSegment(bool onlyOnce);
    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyInWriteSegment> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyInWriteSegment>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // LUCENE-1130: test IOException in writeSegment
public:
  virtual void testIOExceptionDuringWriteSegment() ;

  // LUCENE-1130: test IOException in writeSegment
  virtual void testIOExceptionDuringWriteSegmentOnlyOnce() ;

  // LUCENE-1130: test IOException in writeSegment, with threads
  virtual void
  testIOExceptionDuringWriteSegmentWithThreads() ;

  // LUCENE-1130: test IOException in writeSegment, with threads
  virtual void testIOExceptionDuringWriteSegmentWithThreadsOnlyOnce() throw(
      std::runtime_error);

  //  LUCENE-3365: Test adding two documents with the same field from two
  //  different IndexWriters that we attempt to open at the same time.  As long
  //  as the first IndexWriter completes and closes before the second
  //  IndexWriter time's out trying to get the Lock, we should see both
  //  documents
  virtual void
  testOpenTwoIndexWritersOnDifferentThreads() throw(IOException,
                                                    InterruptedException);

public:
  class DelayedIndexAndCloseRunnable : public Thread
  {
    GET_CLASS_NAME(DelayedIndexAndCloseRunnable)
  private:
    const std::shared_ptr<Directory> dir;

  public:
    bool failed = false;
    std::runtime_error failure = nullptr;

  private:
    std::shared_ptr<CyclicBarrier> syncStart;

  public:
    DelayedIndexAndCloseRunnable(std::shared_ptr<Directory> dir,
                                 std::shared_ptr<CyclicBarrier> syncStart);

    void run() override;

  protected:
    std::shared_ptr<DelayedIndexAndCloseRunnable> shared_from_this()
    {
      return std::static_pointer_cast<DelayedIndexAndCloseRunnable>(
          Thread::shared_from_this());
    }
  };

  // LUCENE-4147
public:
  virtual void testRollbackAndCommitWithThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterWithThreads> outerInstance;

    std::shared_ptr<BaseDirectoryWrapper> d;
    std::shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef;
    std::shared_ptr<LineFileDocs> docs;
    int iters = 0;
    std::shared_ptr<AtomicBoolean> failed;
    std::shared_ptr<Lock> rollbackLock;
    std::shared_ptr<Lock> commitLock;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterWithThreads> outerInstance,
        std::shared_ptr<BaseDirectoryWrapper> d,
        std::shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>>
            writerRef,
        std::shared_ptr<LineFileDocs> docs, int iters,
        std::shared_ptr<AtomicBoolean> failed,
        std::shared_ptr<Lock> rollbackLock, std::shared_ptr<Lock> commitLock);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testUpdateSingleDocWithThreads() ;

  virtual void testSoftUpdateSingleDocWithThreads() ;

  virtual void
  stressUpdateSingleDocWithThreads(bool useSoftDeletes,
                                   bool forceMerge) ;

protected:
  std::shared_ptr<TestIndexWriterWithThreads> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterWithThreads>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
