#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::store
{
class Directory;
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

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterThreadsToSegments : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterThreadsToSegments)

  // LUCENE-5644: for first segment, two threads each indexed one doc (likely
  // concurrently), but for second segment, each thread indexed the doc NOT at
  // the same time, and should have shared the same thread state / segment
public:
  virtual void testSegmentCountOnFlushBasic() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<CountDownLatch> startDone;
    std::shared_ptr<CountDownLatch> middleGun;
    std::shared_ptr<CountDownLatch> finalGun;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<CountDownLatch> startDone,
        std::shared_ptr<CountDownLatch> middleGun,
        std::shared_ptr<CountDownLatch> finalGun, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /** Maximum number of simultaneous threads to use for each iteration. */
private:
  static constexpr int MAX_THREADS_AT_ONCE = 10;

public:
  class CheckSegmentCount
      : public std::enable_shared_from_this<CheckSegmentCount>,
        public Runnable
  {
    GET_CLASS_NAME(CheckSegmentCount)
  private:
    const std::shared_ptr<IndexWriter> w;
    const std::shared_ptr<AtomicInteger> maxThreadCountPerIter;
    const std::shared_ptr<AtomicInteger> indexingCount;
    std::shared_ptr<DirectoryReader> r;

  public:
    CheckSegmentCount(
        std::shared_ptr<IndexWriter> w,
        std::shared_ptr<AtomicInteger> maxThreadCountPerIter,
        std::shared_ptr<AtomicInteger> indexingCount) ;

    void run() override;

  private:
    void setNextIterThreadCount();

  public:
    virtual ~CheckSegmentCount();
  };

  // LUCENE-5644: index docs w/ multiple threads but in between flushes we limit
  // how many threads can index concurrently in the next iteration, and then
  // verify that no more segments were flushed than number of threads:
public:
  virtual void testSegmentCountOnFlushRandom() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<AtomicInteger> indexingCount;
    std::shared_ptr<AtomicInteger> maxThreadCount;
    int ITERS = 0;
    std::shared_ptr<CyclicBarrier> barrier;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<AtomicInteger> indexingCount,
        std::shared_ptr<AtomicInteger> maxThreadCount, int ITERS,
        std::shared_ptr<CyclicBarrier> barrier);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testManyThreadsClose() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance;

    std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testDocsStuckInRAMForever() ;

private:
  class ThreadAnonymousInnerClass3 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass3(
        std::shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass3>(
          Thread::shared_from_this());
    }
  };

  // TODO: remove this hack and fix this test to be better?
  // the whole thing relies on default codec too...
public:
  virtual std::deque<char>
  readSegmentInfoID(std::shared_ptr<Directory> dir,
                    const std::wstring &file) ;

protected:
  std::shared_ptr<TestIndexWriterThreadsToSegments> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterThreadsToSegments>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
