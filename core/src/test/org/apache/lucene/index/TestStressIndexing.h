#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/MergeScheduler.h"

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

using namespace org::apache::lucene::util;
using namespace org::apache::lucene::store;
using namespace org::apache::lucene::document;
using namespace org::apache::lucene::search;

class TestStressIndexing : public LuceneTestCase
{
  GET_CLASS_NAME(TestStressIndexing)
private:
  class TimedThread : public Thread
  {
    GET_CLASS_NAME(TimedThread)
  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool failed;
    bool failed = false;
    int count = 0;

  private:
    static int RUN_TIME_MSEC;
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
    std::shared_ptr<TestStressIndexing> outerInstance;

  public:
    std::shared_ptr<IndexWriter> writer;
    int nextID = 0;

    IndexerThread(std::shared_ptr<TestStressIndexing> outerInstance,
                  std::shared_ptr<IndexWriter> writer,
                  std::deque<std::shared_ptr<TimedThread>> &threads);

    void doWork()  override;

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
  private:
    std::shared_ptr<Directory> directory;

  public:
    SearcherThread(std::shared_ptr<Directory> directory,
                   std::deque<std::shared_ptr<TimedThread>> &threads);

    void doWork()  override;

  protected:
    std::shared_ptr<SearcherThread> shared_from_this()
    {
      return std::static_pointer_cast<SearcherThread>(
          TimedThread::shared_from_this());
    }
  };

  /*
    Run one indexer and 2 searchers against single index as
    stress test.
  */
public:
  virtual void runStressTest(
      std::shared_ptr<Directory> directory,
      std::shared_ptr<MergeScheduler> mergeScheduler) ;

  /*
    Run above stress test against RAMDirectory and then
    FSDirectory.
  */
  virtual void testStressIndexAndSearching() ;

protected:
  std::shared_ptr<TestStressIndexing> shared_from_this()
  {
    return std::static_pointer_cast<TestStressIndexing>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
