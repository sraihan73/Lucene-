#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/index/TestReopen.h"
#include  "core/src/java/org/apache/lucene/index/ReaderCouple.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
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
namespace org::apache::lucene::index
{

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDirectoryReaderReopen : public LuceneTestCase
{
  GET_CLASS_NAME(TestDirectoryReaderReopen)

public:
  virtual void testReopen() ;

private:
  class TestReopenAnonymousInnerClass : public TestReopen
  {
    GET_CLASS_NAME(TestReopenAnonymousInnerClass)
  private:
    std::shared_ptr<TestDirectoryReaderReopen> outerInstance;

    std::shared_ptr<Directory> dir1;

  public:
    TestReopenAnonymousInnerClass(
        std::shared_ptr<TestDirectoryReaderReopen> outerInstance,
        std::shared_ptr<Directory> dir1);

  protected:
    void modifyIndex(int i)  override;

    std::shared_ptr<DirectoryReader> openReader()  override;

  protected:
    std::shared_ptr<TestReopenAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TestReopenAnonymousInnerClass>(
          TestReopen::shared_from_this());
    }
  };

private:
  class TestReopenAnonymousInnerClass2 : public TestReopen
  {
    GET_CLASS_NAME(TestReopenAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDirectoryReaderReopen> outerInstance;

    std::shared_ptr<Directory> dir2;

  public:
    TestReopenAnonymousInnerClass2(
        std::shared_ptr<TestDirectoryReaderReopen> outerInstance,
        std::shared_ptr<Directory> dir2);

  protected:
    void modifyIndex(int i)  override;

    std::shared_ptr<DirectoryReader> openReader()  override;

  protected:
    std::shared_ptr<TestReopenAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<TestReopenAnonymousInnerClass2>(
          TestReopen::shared_from_this());
    }
  };

  // LUCENE-1228: IndexWriter.commit() does not update the index version
  // populate an index in iterations.
  // at the end of every iteration, commit the index and reopen/recreate the
  // reader. in each iteration verify the work of previous iteration. try this
  // once with reopen once recreate, on both RAMDir and FSDir.
public:
  virtual void testCommitReopen() ;
  virtual void testCommitRecreate() ;

private:
  void doTestReopenWithCommit(std::shared_ptr<Random> random,
                              std::shared_ptr<Directory> dir,
                              bool withReopen) ;

  void performDefaultTests(std::shared_ptr<TestReopen> test) throw(
      std::runtime_error);

public:
  virtual void testThreadSafety() ;

private:
  class TestReopenAnonymousInnerClass3 : public TestReopen
  {
    GET_CLASS_NAME(TestReopenAnonymousInnerClass3)
  private:
    std::shared_ptr<TestDirectoryReaderReopen> outerInstance;

    std::shared_ptr<Directory> dir;
    int n = 0;

  public:
    TestReopenAnonymousInnerClass3(
        std::shared_ptr<TestDirectoryReaderReopen> outerInstance,
        std::shared_ptr<Directory> dir, int n);

  protected:
    void modifyIndex(int i)  override;

    std::shared_ptr<DirectoryReader> openReader()  override;

  protected:
    std::shared_ptr<TestReopenAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<TestReopenAnonymousInnerClass3>(
          TestReopen::shared_from_this());
    }
  };

private:
  class ReaderThreadTaskAnonymousInnerClass : public ReaderThreadTask
  {
    GET_CLASS_NAME(ReaderThreadTaskAnonymousInnerClass)
  private:
    std::shared_ptr<TestDirectoryReaderReopen> outerInstance;

    std::shared_ptr<
        org::apache::lucene::index::TestDirectoryReaderReopen::TestReopen>
        test;
    std::deque<std::shared_ptr<ReaderCouple>> readers;
    std::shared_ptr<Set<std::shared_ptr<DirectoryReader>>> readersToClose;
    std::shared_ptr<org::apache::lucene::index::DirectoryReader> r;
    int index = 0;

  public:
    ReaderThreadTaskAnonymousInnerClass(
        std::shared_ptr<TestDirectoryReaderReopen> outerInstance,
        std::shared_ptr<
            org::apache::lucene::index::TestDirectoryReaderReopen::TestReopen>
            test,
        std::deque<std::shared_ptr<ReaderCouple>> &readers,
        std::shared_ptr<Set<std::shared_ptr<DirectoryReader>>> readersToClose,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> r,
        int index);

    void run()  override;

  protected:
    std::shared_ptr<ReaderThreadTaskAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ReaderThreadTaskAnonymousInnerClass>(
          ReaderThreadTask::shared_from_this());
    }
  };

private:
  class ReaderThreadTaskAnonymousInnerClass2 : public ReaderThreadTask
  {
    GET_CLASS_NAME(ReaderThreadTaskAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDirectoryReaderReopen> outerInstance;

    std::deque<std::shared_ptr<ReaderCouple>> readers;

  public:
    ReaderThreadTaskAnonymousInnerClass2(
        std::shared_ptr<TestDirectoryReaderReopen> outerInstance,
        std::deque<std::shared_ptr<ReaderCouple>> &readers);

    void run()  override;

  protected:
    std::shared_ptr<ReaderThreadTaskAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ReaderThreadTaskAnonymousInnerClass2>(
          ReaderThreadTask::shared_from_this());
    }
  };

private:
  class ReaderCouple : public std::enable_shared_from_this<ReaderCouple>
  {
    GET_CLASS_NAME(ReaderCouple)
  public:
    ReaderCouple(std::shared_ptr<DirectoryReader> r1,
                 std::shared_ptr<DirectoryReader> r2);

    std::shared_ptr<DirectoryReader> newReader;
    std::shared_ptr<DirectoryReader> refreshedReader;
  };

public:
  class ReaderThreadTask : public std::enable_shared_from_this<ReaderThreadTask>
  {
    GET_CLASS_NAME(ReaderThreadTask)
  protected:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: protected volatile bool stopped;
    bool stopped = false;

  public:
    virtual void stop();

    virtual void run() = 0;
  };

private:
  class ReaderThread : public Thread
  {
    GET_CLASS_NAME(ReaderThread)
  public:
    std::shared_ptr<ReaderThreadTask> task;
    std::runtime_error error;

    ReaderThread(std::shared_ptr<ReaderThreadTask> task);

    virtual void stopThread();

    void run() override;

  protected:
    std::shared_ptr<ReaderThread> shared_from_this()
    {
      return std::static_pointer_cast<ReaderThread>(Thread::shared_from_this());
    }
  };

private:
  std::mutex createReaderMutex;

  std::shared_ptr<ReaderCouple>
  refreshReader(std::shared_ptr<DirectoryReader> reader,
                bool hasChanges) ;

public:
  virtual std::shared_ptr<ReaderCouple>
  refreshReader(std::shared_ptr<DirectoryReader> reader,
                std::shared_ptr<TestReopen> test, int modify,
                bool hasChanges) ;

  static void createIndex(std::shared_ptr<Random> random,
                          std::shared_ptr<Directory> dir,
                          bool multiSegment) ;

  static std::shared_ptr<Document> createDocument(int n, int numFields);

  static void modifyIndex(int i,
                          std::shared_ptr<Directory> dir) ;

  static void assertReaderClosed(std::shared_ptr<IndexReader> reader,
                                 bool checkSubReaders);

public:
  class TestReopen : public std::enable_shared_from_this<TestReopen>
  {
    GET_CLASS_NAME(TestReopen)
  protected:
    virtual std::shared_ptr<DirectoryReader> openReader() = 0;
    virtual void modifyIndex(int i) = 0;
  };

public:
  class KeepAllCommits : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(KeepAllCommits)
  public:
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits)
    void onInit(std::deque<T1> commits);
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits)
    void onCommit(std::deque<T1> commits);

  protected:
    std::shared_ptr<KeepAllCommits> shared_from_this()
    {
      return std::static_pointer_cast<KeepAllCommits>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

public:
  virtual void testReopenOnCommit() ;

  virtual void testOpenIfChangedNRTToCommit() ;

  virtual void testOverDecRefDuringReopen() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<TestDirectoryReaderReopen> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<TestDirectoryReaderReopen> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir);

    bool failed = false;

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  virtual void testNPEAfterInvalidReindex1() ;

  virtual void testNPEAfterInvalidReindex2() ;

  /** test reopening backwards from a non-NRT reader (with document deletes) */
  virtual void testNRTMdeletes() ;

  /** test reopening backwards from an NRT reader (with document deletes) */
  virtual void testNRTMdeletes2() ;

  /** test reopening backwards from a non-NRT reader with DV updates */
  virtual void testNRTMupdates() ;

  /** test reopening backwards from an NRT reader with DV updates */
  virtual void testNRTMupdates2() ;

  // LUCENE-5931: we make a "best effort" to catch this abuse and throw a
  // clear(er) exception than what would otherwise look like hard to explain
  // index corruption during searching
  virtual void
  testDeleteIndexFilesWhileReaderStillOpen() ;

  virtual void testReuseUnchangedLeafReaderOnDVUpdate() ;

protected:
  std::shared_ptr<TestDirectoryReaderReopen> shared_from_this()
  {
    return std::static_pointer_cast<TestDirectoryReaderReopen>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
