#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTransactions : public LuceneTestCase
{
  GET_CLASS_NAME(TestTransactions)

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private static volatile bool doFail;
  static bool doFail;

private:
  class RandomFailure : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(RandomFailure)
  private:
    std::shared_ptr<TestTransactions> outerInstance;

  public:
    RandomFailure(std::shared_ptr<TestTransactions> outerInstance);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<RandomFailure> shared_from_this()
    {
      return std::static_pointer_cast<RandomFailure>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

private:
  class TimedThread : public Thread
  {
    GET_CLASS_NAME(TimedThread)
  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool failed;
    bool failed = false;

  private:
    static float RUN_TIME_MSEC;
    std::deque<std::shared_ptr<TimedThread>> allThreads;

  public:
    virtual void doWork() = 0;

    TimedThread(std::deque<std::shared_ptr<TimedThread>> &threads);

    void run() override;

  private:
    bool anyErrors();

  protected:
    std::shared_ptr<TimedThread> shared_from_this()
    {
      return std::static_pointer_cast<TimedThread>(Thread::shared_from_this());
    }
  };

private:
  class IndexerThread : public TimedThread
  {
    GET_CLASS_NAME(IndexerThread)
  private:
    std::shared_ptr<TestTransactions> outerInstance;

  public:
    std::shared_ptr<Directory> dir1;
    std::shared_ptr<Directory> dir2;
    std::mutex lock;
    int nextID = 0;

    IndexerThread(std::shared_ptr<TestTransactions> outerInstance,
                  std::any lock, std::shared_ptr<Directory> dir1,
                  std::shared_ptr<Directory> dir2,
                  std::deque<std::shared_ptr<TimedThread>> &threads);

    void doWork()  override;

    virtual void update(std::shared_ptr<IndexWriter> writer) ;

  protected:
    std::shared_ptr<IndexerThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexerThread>(
          TimedThread::shared_from_this());
    }
  };

private:
  class SearcherThread : public TimedThread
  {
    GET_CLASS_NAME(SearcherThread)
  public:
    std::shared_ptr<Directory> dir1;
    std::shared_ptr<Directory> dir2;
    std::mutex lock;

    SearcherThread(std::any lock, std::shared_ptr<Directory> dir1,
                   std::shared_ptr<Directory> dir2,
                   std::deque<std::shared_ptr<TimedThread>> &threads);

    void doWork()  override;

  protected:
    std::shared_ptr<SearcherThread> shared_from_this()
    {
      return std::static_pointer_cast<SearcherThread>(
          TimedThread::shared_from_this());
    }
  };

public:
  virtual void
  initIndex(std::shared_ptr<Directory> dir) ;

  virtual void testTransactions() ;

protected:
  std::shared_ptr<TestTransactions> shared_from_this()
  {
    return std::static_pointer_cast<TestTransactions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
