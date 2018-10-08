#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"

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

using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

class TestAddIndexes : public LuceneTestCase
{
  GET_CLASS_NAME(TestAddIndexes)

public:
  virtual void testSimpleCase() ;

  virtual void testWithPendingDeletes() ;

  virtual void testWithPendingDeletes2() ;

  virtual void testWithPendingDeletes3() ;

  // case 0: add self or exceed maxMergeDocs, expect exception
  virtual void testAddSelf() ;

  // in all the remaining tests, make the doc count of the oldest segment
  // in dir large so that it is never merged in addIndexes()
  // case 1: no tail segments
  virtual void testNoTailSegments() ;

  // case 2: tail segments, invariants hold, no copy
  virtual void testNoCopySegments() ;

  // case 3: tail segments, invariants hold, copy, invariants hold
  virtual void testNoMergeAfterCopy() ;

  // case 4: tail segments, invariants hold, copy, invariants not hold
  virtual void testMergeAfterCopy() ;

  // case 5: tail segments, invariants not hold
  virtual void testMoreMerges() ;

private:
  std::shared_ptr<IndexWriter>
  newWriter(std::shared_ptr<Directory> dir,
            std::shared_ptr<IndexWriterConfig> conf) ;

  void addDocs(std::shared_ptr<IndexWriter> writer,
               int numDocs) ;

  void addDocs2(std::shared_ptr<IndexWriter> writer,
                int numDocs) ;

  void verifyNumDocs(std::shared_ptr<Directory> dir,
                     int numDocs) ;

  void verifyTermDocs(std::shared_ptr<Directory> dir,
                      std::shared_ptr<Term> term,
                      int numDocs) ;

  void setUpDirs(std::shared_ptr<Directory> dir,
                 std::shared_ptr<Directory> aux) ;

  void setUpDirs(std::shared_ptr<Directory> dir, std::shared_ptr<Directory> aux,
                 bool withID) ;

  // LUCENE-1270
public:
  virtual void testHangOnClose() ;

  // TODO: these are also in TestIndexWriter... add a simple doc-writing method
  // like this to LuceneTestCase?
private:
  void addDoc(std::shared_ptr<IndexWriter> writer) ;

private:
  class RunAddIndexesThreads
      : public std::enable_shared_from_this<RunAddIndexesThreads>
  {
    GET_CLASS_NAME(RunAddIndexesThreads)
  private:
    std::shared_ptr<TestAddIndexes> outerInstance;

  public:
    std::shared_ptr<Directory> dir, dir2;
    static constexpr int NUM_INIT_DOCS = 17;
    std::shared_ptr<IndexWriter> writer2;
    const std::deque<std::runtime_error> failures =
        std::deque<std::runtime_error>();
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool didClose;
    bool didClose = false;
    std::deque<std::shared_ptr<DirectoryReader>> const readers;
    const int NUM_COPY;
    static constexpr int NUM_THREADS = 5;
    std::deque<std::shared_ptr<Thread>> const threads =
        std::deque<std::shared_ptr<Thread>>(NUM_THREADS);

    RunAddIndexesThreads(std::shared_ptr<TestAddIndexes> outerInstance,
                         int numCopy) ;

    virtual void launchThreads(int const numIter);

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    private:
      std::shared_ptr<RunAddIndexesThreads> outerInstance;

      int numIter = 0;

    public:
      ThreadAnonymousInnerClass(
          std::shared_ptr<RunAddIndexesThreads> outerInstance, int numIter);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  public:
    virtual void joinThreads() ;

    virtual void close(bool doWait) ;

    virtual void closeDir() ;

    virtual void doBody(int j,
                        std::deque<std::shared_ptr<Directory>> &dirs) = 0;
    virtual void handle(std::runtime_error t) = 0;
  };

private:
  class CommitAndAddIndexes : public RunAddIndexesThreads
  {
    GET_CLASS_NAME(CommitAndAddIndexes)
  private:
    std::shared_ptr<TestAddIndexes> outerInstance;

  public:
    CommitAndAddIndexes(std::shared_ptr<TestAddIndexes> outerInstance,
                        int numCopy) ;

    void handle(std::runtime_error t) override;

    void doBody(int j, std::deque<std::shared_ptr<Directory>> &dirs) throw(
        std::runtime_error) override;

  protected:
    std::shared_ptr<CommitAndAddIndexes> shared_from_this()
    {
      return std::static_pointer_cast<CommitAndAddIndexes>(
          RunAddIndexesThreads::shared_from_this());
    }
  };

  // LUCENE-1335: test simultaneous addIndexes & commits
  // from multiple threads
public:
  virtual void testAddIndexesWithThreads() ;

private:
  class CommitAndAddIndexes2 : public CommitAndAddIndexes
  {
    GET_CLASS_NAME(CommitAndAddIndexes2)
  private:
    std::shared_ptr<TestAddIndexes> outerInstance;

  public:
    CommitAndAddIndexes2(std::shared_ptr<TestAddIndexes> outerInstance,
                         int numCopy) ;

    void handle(std::runtime_error t) override;

  protected:
    std::shared_ptr<CommitAndAddIndexes2> shared_from_this()
    {
      return std::static_pointer_cast<CommitAndAddIndexes2>(
          CommitAndAddIndexes::shared_from_this());
    }
  };

  // LUCENE-1335: test simultaneous addIndexes & close
public:
  virtual void testAddIndexesWithClose() ;

private:
  class CommitAndAddIndexes3 : public RunAddIndexesThreads
  {
    GET_CLASS_NAME(CommitAndAddIndexes3)
  private:
    std::shared_ptr<TestAddIndexes> outerInstance;

  public:
    CommitAndAddIndexes3(std::shared_ptr<TestAddIndexes> outerInstance,
                         int numCopy) ;

    void doBody(int j, std::deque<std::shared_ptr<Directory>> &dirs) throw(
        std::runtime_error) override;

    void handle(std::runtime_error t) override;

  protected:
    std::shared_ptr<CommitAndAddIndexes3> shared_from_this()
    {
      return std::static_pointer_cast<CommitAndAddIndexes3>(
          RunAddIndexesThreads::shared_from_this());
    }
  };

  // LUCENE-1335: test simultaneous addIndexes & close
public:
  virtual void testAddIndexesWithCloseNoWait() ;

  // LUCENE-1335: test simultaneous addIndexes & close
  virtual void testAddIndexesWithRollback() ;

  // LUCENE-2996: tests that addIndexes(IndexReader) applies existing deletes
  // correctly.
  virtual void testExistingDeletes() ;

  // just like addDocs but with ID, starting from docStart
private:
  void addDocsWithID(std::shared_ptr<IndexWriter> writer, int numDocs,
                     int docStart) ;

public:
  virtual void testSimpleCaseCustomCodec() ;

private:
  class CustomPerFieldCodec final : public AssertingCodec
  {
    GET_CLASS_NAME(CustomPerFieldCodec)
  private:
    const std::shared_ptr<PostingsFormat> directFormat =
        PostingsFormat::forName(L"Direct");
    const std::shared_ptr<PostingsFormat> defaultFormat =
        TestUtil::getDefaultPostingsFormat();
    const std::shared_ptr<PostingsFormat> memoryFormat =
        PostingsFormat::forName(L"Memory");

  public:
    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<CustomPerFieldCodec> shared_from_this()
    {
      return std::static_pointer_cast<CustomPerFieldCodec>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

  // LUCENE-2790: tests that the non CFS files were deleted by addIndexes
public:
  virtual void testNonCFSLeftovers() ;

private:
  class UnRegisteredCodec final : public FilterCodec
  {
    GET_CLASS_NAME(UnRegisteredCodec)
  public:
    UnRegisteredCodec();

  protected:
    std::shared_ptr<UnRegisteredCodec> shared_from_this()
    {
      return std::static_pointer_cast<UnRegisteredCodec>(
          org.apache.lucene.codecs.FilterCodec::shared_from_this());
    }
  };

  /*
   * simple test that ensures we getting expected exceptions
   */
public:
  virtual void testAddIndexMissingCodec() ;

  // LUCENE-3575
  virtual void testFieldNamesChanged() ;

  virtual void testAddEmpty() ;

  // Currently it's impossible to end up with a segment with all documents
  // deleted, as such segments are dropped. Still, to validate that addIndexes
  // works with such segments, or readers that end up in such state, we fake an
  // all deleted segment.
  virtual void testFakeAllDeleted() ;

  /** Make sure an open IndexWriter on an incoming Directory
   *  causes a LockObtainFailedException */
  virtual void testLocksBlock() ;

  virtual void testIllegalIndexSortChange1() ;

  virtual void testIllegalIndexSortChange2() ;

  virtual void
  testAddIndexesDVUpdateSameSegmentName() ;

  virtual void testAddIndexesDVUpdateNewSegmentName() ;

  virtual void testAddIndicesWithSoftDeletes() ;

protected:
  std::shared_ptr<TestAddIndexes> shared_from_this()
  {
    return std::static_pointer_cast<TestAddIndexes>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
