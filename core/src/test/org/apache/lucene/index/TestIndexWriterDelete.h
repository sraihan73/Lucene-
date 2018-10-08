#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class RandomIndexWriter;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}
namespace org::apache::lucene::store
{
class Failure;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
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
// TestIndexWriterDelete extends org.apache.lucene.util.LuceneTestCase
class TestIndexWriterDelete : public LuceneTestCase
{

  // test the simple case
public:
  virtual void testSimpleCase() ;

  // test when delete terms only apply to disk segments
  virtual void testNonRAMDelete() ;

  // test when delete terms only apply to ram segments
  virtual void testRAMDeletes() ;

  // test when delete terms apply to both disk and ram segments
  virtual void testBothDeletes() ;

  // test that batched delete terms are flushed together
  virtual void testBatchDeletes() ;

  // test deleteAll()
  virtual void testDeleteAllSimple() ;

  virtual void testDeleteAllNoDeadLock() throw(IOException,
                                               InterruptedException);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterDelete> outerInstance;

    std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> modifier;
    std::shared_ptr<CountDownLatch> latch;
    std::shared_ptr<CountDownLatch> doneLatch;
    int offset = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterDelete> outerInstance,
        std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> modifier,
        std::shared_ptr<CountDownLatch> latch,
        std::shared_ptr<CountDownLatch> doneLatch, int offset);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  // test rollback of deleteAll()
public:
  virtual void testDeleteAllRollback() ;

  // test deleteAll() w/ near real-time reader
  virtual void testDeleteAllNRT() ;

private:
  void updateDoc(std::shared_ptr<IndexWriter> modifier, int id,
                 int value) ;

  void addDoc(std::shared_ptr<IndexWriter> modifier, int id,
              int value) ;

  int64_t getHitCount(std::shared_ptr<Directory> dir,
                        std::shared_ptr<Term> term) ;

public:
  virtual void testDeletesOnDiskFull() ;

  virtual void testUpdatesOnDiskFull() ;

  /**
   * Make sure if modifier tries to commit but hits disk full that modifier
   * remains consistent and usable. Similar to TestIndexReader.testDiskFull().
   */
private:
  void doTestOperationsOnDiskFull(bool updates) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testErrorAfterApplyDeletes() throws
  // java.io.IOException
  virtual void testErrorAfterApplyDeletes() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterDelete> outerInstance;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterDelete> outerInstance);

    bool sawMaybe = false;
    bool failed = false;
    std::shared_ptr<Thread> thread;
    std::shared_ptr<MockDirectoryWrapper::Failure> reset() override;
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

  // This test tests that the files created by the docs writer before
  // a segment is written are cleaned up if there's an i/o error

public:
  virtual void testErrorInDocsWriterAdd() ;

private:
  class FailureAnonymousInnerClass2 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterDelete> outerInstance;

  public:
    FailureAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterDelete> outerInstance);

    bool failed = false;
    std::shared_ptr<MockDirectoryWrapper::Failure> reset() override;
    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass2>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  virtual void testDeleteNullQuery() ;

  virtual void testDeleteAllSlowly() ;

  virtual void testIndexingThenDeleting() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterDelete> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterDelete> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-3340: make sure deletes that we don't apply
  // during flush (ie are just pushed into the stream) are
  // in fact later flushed due to their RAM usage:
public:
  virtual void testFlushPushedDeletesByRAM() ;

  // Make sure buffered (pushed) deletes don't use up so
  // much RAM that it forces long tail of tiny segments:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testApplyDeletesOnFlush() throws
  // Exception
  virtual void testApplyDeletesOnFlush() ;

private:
  class IndexWriterAnonymousInnerClass : public IndexWriter
  {
    GET_CLASS_NAME(IndexWriterAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterDelete> outerInstance;

    std::shared_ptr<AtomicInteger> docsInSegment;
    std::shared_ptr<AtomicBoolean> closing;
    std::shared_ptr<AtomicBoolean> sawAfterFlush;

  public:
    IndexWriterAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterDelete> outerInstance,
        std::shared_ptr<Directory> dir,
        std::shared_ptr<org::apache::lucene::index::IndexWriterConfig>
            setUseCompoundFile,
        std::shared_ptr<AtomicInteger> docsInSegment,
        std::shared_ptr<AtomicBoolean> closing,
        std::shared_ptr<AtomicBoolean> sawAfterFlush);

    void doAfterFlush() override;

  protected:
    std::shared_ptr<IndexWriterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexWriterAnonymousInnerClass>(
          IndexWriter::shared_from_this());
    }
  };

  // LUCENE-4455
public:
  virtual void testDeletesCheckIndexOutput() ;

  virtual void testTryDeleteDocument() ;

  virtual void testNRTIsCurrentAfterDelete() ;

  virtual void testOnlyDeletesTriggersMergeOnClose() ;

  virtual void
  testOnlyDeletesTriggersMergeOnGetReader() ;

  virtual void testOnlyDeletesTriggersMergeOnFlush() ;

  virtual void testOnlyDeletesDeleteAllDocs() ;

  // Make sure merges still kick off after IW.deleteAll!
  virtual void testMergingAfterDeleteAll() ;

protected:
  std::shared_ptr<TestIndexWriterDelete> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterDelete>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
