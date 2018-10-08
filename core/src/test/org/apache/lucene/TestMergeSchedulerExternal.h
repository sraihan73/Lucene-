#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/OneMerge.h"
#include  "core/src/java/org/apache/lucene/index/ConcurrentMergeScheduler.h"
#include  "core/src/java/org/apache/lucene/index/MergeThread.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

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

namespace org::apache::lucene
{

using ConcurrentMergeScheduler =
    org::apache::lucene::index::ConcurrentMergeScheduler;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using MergeScheduler = org::apache::lucene::index::MergeScheduler;
using MergeTrigger = org::apache::lucene::index::MergeTrigger;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Holds tests cases to verify external APIs are accessible
 * while not being in org.apache.lucene.index package.
 */
class TestMergeSchedulerExternal : public LuceneTestCase
{
  GET_CLASS_NAME(TestMergeSchedulerExternal)

public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile bool mergeCalled;
  bool mergeCalled = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile bool mergeThreadCreated;
  bool mergeThreadCreated = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile bool excCalled;
  bool excCalled = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile static org.apache.lucene.util.InfoStream
  // infoStream;
  static std::shared_ptr<InfoStream> infoStream;

private:
  class MyMergeScheduler : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(MyMergeScheduler)
  private:
    std::shared_ptr<TestMergeSchedulerExternal> outerInstance;

  public:
    MyMergeScheduler(std::shared_ptr<TestMergeSchedulerExternal> outerInstance);

  private:
    class MyMergeThread : public ConcurrentMergeScheduler::MergeThread
    {
      GET_CLASS_NAME(MyMergeThread)
    private:
      std::shared_ptr<TestMergeSchedulerExternal::MyMergeScheduler>
          outerInstance;

    public:
      MyMergeThread(
          std::shared_ptr<TestMergeSchedulerExternal::MyMergeScheduler>
              outerInstance,
          std::shared_ptr<IndexWriter> writer, std::shared_ptr<OneMerge> merge);

    protected:
      std::shared_ptr<MyMergeThread> shared_from_this()
      {
        return std::static_pointer_cast<MyMergeThread>(
            org.apache.lucene.index.ConcurrentMergeScheduler
                .MergeThread::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ConcurrentMergeScheduler::MergeThread>
    getMergeThread(std::shared_ptr<IndexWriter> writer,
                   std::shared_ptr<OneMerge> merge)  override;

    void handleMergeException(std::shared_ptr<Directory> dir,
                              std::runtime_error t) override;

    void doMerge(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<OneMerge> merge)  override;

  protected:
    std::shared_ptr<MyMergeScheduler> shared_from_this()
    {
      return std::static_pointer_cast<MyMergeScheduler>(
          org.apache.lucene.index.ConcurrentMergeScheduler::shared_from_this());
    }
  };

private:
  class FailOnlyOnMerge : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyOnMerge)
  public:
    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyOnMerge> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyOnMerge>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass()
  static void afterClass();

  virtual void testSubclassConcurrentMergeScheduler() ;

private:
  class ReportingMergeScheduler : public MergeScheduler
  {
    GET_CLASS_NAME(ReportingMergeScheduler)

  public:
    void merge(std::shared_ptr<IndexWriter> writer, MergeTrigger trigger,
               bool newMergesFound)  override;

    virtual ~ReportingMergeScheduler();

  protected:
    std::shared_ptr<ReportingMergeScheduler> shared_from_this()
    {
      return std::static_pointer_cast<ReportingMergeScheduler>(
          org.apache.lucene.index.MergeScheduler::shared_from_this());
    }
  };

public:
  virtual void testCustomMergeScheduler() ;

protected:
  std::shared_ptr<TestMergeSchedulerExternal> shared_from_this()
  {
    return std::static_pointer_cast<TestMergeSchedulerExternal>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/
