#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
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

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs("SimpleText") public class
// TestIndexWriterReader extends org.apache.lucene.util.LuceneTestCase
class TestIndexWriterReader : public LuceneTestCase
{

private:
  const int numThreads = TEST_NIGHTLY ? 5 : 3;

public:
  static int count(std::shared_ptr<Term> t,
                   std::shared_ptr<IndexReader> r) ;

  virtual void testAddCloseOpen() ;

  virtual void testUpdateDocument() ;

  virtual void testIsCurrent() ;

  /**
   * Test using IW.addIndexes
   */
  virtual void testAddIndexes() ;

  virtual void testAddIndexes2() ;

  /**
   * Deletes using IW.deleteDocuments
   */
  virtual void testDeleteFromIndexWriter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testAddIndexesAndDoDeletesThreads() throws
  // Throwable
  virtual void testAddIndexesAndDoDeletesThreads() ;

private:
  class AddDirectoriesThreads
      : public std::enable_shared_from_this<AddDirectoriesThreads>
  {
    GET_CLASS_NAME(AddDirectoriesThreads)
  private:
    std::shared_ptr<TestIndexWriterReader> outerInstance;

  public:
    std::shared_ptr<Directory> addDir;
    static constexpr int NUM_INIT_DOCS = 100;
    int numDirs = 0;
    std::deque<std::shared_ptr<Thread>> const threads =
        std::deque<std::shared_ptr<Thread>>(outerInstance->numThreads);
    std::shared_ptr<IndexWriter> mainWriter;
    const std::deque<std::runtime_error> failures =
        std::deque<std::runtime_error>();
    std::deque<std::shared_ptr<DirectoryReader>> readers;
    bool didClose = false;
    std::shared_ptr<AtomicInteger> count = std::make_shared<AtomicInteger>(0);
    std::shared_ptr<AtomicInteger> numaddIndexes =
        std::make_shared<AtomicInteger>(0);

    AddDirectoriesThreads(
        std::shared_ptr<TestIndexWriterReader> outerInstance, int numDirs,
        std::shared_ptr<IndexWriter> mainWriter) ;

    virtual void joinThreads();

    virtual void close(bool doWait) ;

    virtual void closeDir() ;

    virtual void handle(std::runtime_error t);

    virtual void launchThreads(int const numIter);

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    private:
      std::shared_ptr<AddDirectoriesThreads> outerInstance;

      int numIter = 0;

    public:
      ThreadAnonymousInnerClass(
          std::shared_ptr<AddDirectoriesThreads> outerInstance, int numIter);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  public:
    virtual void doBody(int j, std::deque<std::shared_ptr<Directory>>
                                   &dirs) ;
  };

public:
  virtual void
  testIndexWriterReopenSegmentFullMerge() ;

  virtual void testIndexWriterReopenSegment() ;

  /**
   * Tests creating a segment, then check to insure the segment can be seen via
   * IW.getReader
   */
  virtual void
  doTestIndexWriterReopenSegment(bool doFullMerge) ;

  /*
   * Delete a document by term and return the doc id
   *
   * public static int deleteDocument(Term term, IndexWriter writer) throws
   * IOException { IndexReader reader = writer.getReader(); TermDocs td =
   * reader.termDocs(term); int doc = -1; //if (td.next()) { // doc = td.doc();
   * //} //writer.deleteDocuments(term); td.close(); return doc; }
   */

  static void createIndex(std::shared_ptr<Random> random,
                          std::shared_ptr<Directory> dir1,
                          const std::wstring &indexName,
                          bool multiSegment) ;

  static void
  createIndexNoClose(bool multiSegment, const std::wstring &indexName,
                     std::shared_ptr<IndexWriter> w) ;

  virtual void testMergeWarmer() ;

  virtual void testAfterCommit() ;

  // Make sure reader remains usable even if IndexWriter closes
  virtual void testAfterClose() ;

  // Stress test reopen during addIndexes
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testDuringAddIndexes() throws Exception
  virtual void testDuringAddIndexes() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterReader> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    std::deque<std::shared_ptr<Directory>> dirs;
    int numIterations = 0;
    std::deque<std::runtime_error> excs;
    std::shared_ptr<AtomicBoolean> threadDone;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterReader> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        std::deque<std::shared_ptr<Directory>> &dirs, int numIterations,
        std::deque<std::runtime_error> &excs,
        std::shared_ptr<AtomicBoolean> threadDone);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  std::shared_ptr<Directory>
  getAssertNoDeletesDirectory(std::shared_ptr<Directory> directory);

  // Stress test reopen during add/delete
public:
  virtual void testDuringAddDelete() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterReader> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    std::shared_ptr<org::apache::lucene::index::DirectoryReader> r;
    int iters = 0;
    std::deque<std::runtime_error> excs;
    std::shared_ptr<AtomicInteger> remainingThreads;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterReader> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> r,
        int iters, std::deque<std::runtime_error> &excs,
        std::shared_ptr<AtomicInteger> remainingThreads);

    const std::shared_ptr<Random> r;

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testForceMergeDeletes() ;

  virtual void testDeletesNumDocs() ;

  virtual void testEmptyIndex() ;

  virtual void testSegmentWarmer() ;

  virtual void testSimpleMergedSegmentWarmer() ;

private:
  class InfoStreamAnonymousInnerClass : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterReader> outerInstance;

    std::shared_ptr<AtomicBoolean> didWarm;

  public:
    InfoStreamAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterReader> outerInstance,
        std::shared_ptr<AtomicBoolean> didWarm);

    virtual ~InfoStreamAnonymousInnerClass();
    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

  protected:
    std::shared_ptr<InfoStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamAnonymousInnerClass>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

public:
  virtual void testReopenAfterNoRealChange() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNRTOpenExceptions() throws Exception
  virtual void testNRTOpenExceptions() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterReader> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;
    std::shared_ptr<AtomicBoolean> shouldFail;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterReader> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir,
        std::shared_ptr<AtomicBoolean> shouldFail);

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

  /** Make sure if all we do is open NRT reader against
   *  writer, we don't see merge starvation. */
public:
  virtual void testTooManySegments() ;

  // LUCENE-5912: make sure when you reopen an NRT reader using a commit point,
  // the SegmentReaders are in fact shared:
  virtual void testReopenNRTReaderOnCommit() ;

protected:
  std::shared_ptr<TestIndexWriterReader> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
