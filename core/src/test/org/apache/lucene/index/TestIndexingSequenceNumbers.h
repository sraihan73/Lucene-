#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

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

class TestIndexingSequenceNumbers : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexingSequenceNumbers)

public:
  virtual void testBasic() ;

  virtual void testAfterRefresh() ;

  virtual void testAfterCommit() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testStressUpdateSameID() throws Exception
  virtual void testStressUpdateSameID() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexingSequenceNumbers> outerInstance;

    std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::deque<int64_t> seqNos;
    std::shared_ptr<org::apache::lucene::index::Term> id;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexingSequenceNumbers> outerInstance,
        std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun,
        std::deque<int64_t> &seqNos,
        std::shared_ptr<org::apache::lucene::index::Term> id, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  class Operation : public std::enable_shared_from_this<Operation>
  {
    GET_CLASS_NAME(Operation)
    // 0 = update, 1 = delete, 2 = commit, 3 = add
  public:
    char what = 0;
    int id = 0;
    int threadID = 0;
    int64_t seqNo = 0;
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testStressConcurrentCommit() throws
  // Exception
  virtual void testStressConcurrentCommit() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexingSequenceNumbers> outerInstance;

    int opCount = 0;
    int idCount = 0;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::any commitLock;
    std::deque<std::shared_ptr<Operation>> commits;
    int i = 0;
    std::deque<std::shared_ptr<Operation>> ops;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexingSequenceNumbers> outerInstance, int opCount,
        int idCount, std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun, std::any commitLock,
        std::deque<std::shared_ptr<Operation>> &commits, int i,
        std::deque<std::shared_ptr<Operation>> &ops, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testStressConcurrentDocValuesUpdatesCommit() throws Exception
  virtual void
  testStressConcurrentDocValuesUpdatesCommit() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexingSequenceNumbers> outerInstance;

    int opCount = 0;
    int idCount = 0;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::any commitLock;
    std::deque<std::shared_ptr<Operation>> commits;
    int i = 0;
    std::deque<std::shared_ptr<Operation>> ops;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestIndexingSequenceNumbers> outerInstance, int opCount,
        int idCount, std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun, std::any commitLock,
        std::deque<std::shared_ptr<Operation>> &commits, int i,
        std::deque<std::shared_ptr<Operation>> &ops, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testStressConcurrentAddAndDeleteAndCommit() throws Exception
  virtual void
  testStressConcurrentAddAndDeleteAndCommit() ;

private:
  class ThreadAnonymousInnerClass3 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexingSequenceNumbers> outerInstance;

    int opCount = 0;
    int idCount = 0;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::any commitLock;
    std::deque<std::shared_ptr<Operation>> commits;
    int i = 0;
    std::deque<std::shared_ptr<Operation>> ops;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass3(
        std::shared_ptr<TestIndexingSequenceNumbers> outerInstance, int opCount,
        int idCount, std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun, std::any commitLock,
        std::deque<std::shared_ptr<Operation>> &commits, int i,
        std::deque<std::shared_ptr<Operation>> &ops, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass3>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testDeleteAll() ;

protected:
  std::shared_ptr<TestIndexingSequenceNumbers> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexingSequenceNumbers>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
