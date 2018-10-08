#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/OneMerge.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"

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

class TestTragicIndexWriterDeadlock : public LuceneTestCase
{
  GET_CLASS_NAME(TestTragicIndexWriterDeadlock)

public:
  virtual void testDeadlockExcNRTReaderCommit() ;

private:
  class SuppressingConcurrentMergeSchedulerAnonymousInnerClass
      : public SuppressingConcurrentMergeScheduler
  {
    GET_CLASS_NAME(SuppressingConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance;

  public:
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance);

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

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicBoolean> done;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<AtomicBoolean> done);

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
    std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance;

    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicBoolean> done;
    std::shared_ptr<org::apache::lucene::index::DirectoryReader> r0;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<AtomicBoolean> done,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> r0);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

  // LUCENE-7570
public:
  virtual void testDeadlockStalledMerges() ;

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance;

    std::shared_ptr<Directory> dir;
    std::shared_ptr<CountDownLatch> done;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        std::shared_ptr<Directory> dir, std::shared_ptr<CountDownLatch> done);

  protected:
    void doMerge(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<MergePolicy::OneMerge>
                     merge)  override;

    // C++ WARNING: The following method was originally marked 'synchronized':
    void doStall() override;

    void handleMergeException(std::shared_ptr<Directory> dir,
                              std::runtime_error exc) override;

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
  class IndexWriterAnonymousInnerClass : public IndexWriter
  {
    GET_CLASS_NAME(IndexWriterAnonymousInnerClass)
  private:
    std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance;

  public:
    IndexWriterAnonymousInnerClass(
        std::shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        std::shared_ptr<Directory> dir,
        std::shared_ptr<org::apache::lucene::index::IndexWriterConfig> iwc);

    void mergeSuccess(std::shared_ptr<MergePolicy::OneMerge> merge) override;

  protected:
    std::shared_ptr<IndexWriterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexWriterAnonymousInnerClass>(
          IndexWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestTragicIndexWriterDeadlock> shared_from_this()
  {
    return std::static_pointer_cast<TestTragicIndexWriterDeadlock>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
