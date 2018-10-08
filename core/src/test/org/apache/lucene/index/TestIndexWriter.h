#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
#include  "core/src/java/org/apache/lucene/index/OneMerge.h"
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriter : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriter)

private:
  static const std::shared_ptr<FieldType> storedTextType;

public:
  virtual void testDocCount() ;

  static void addDoc(std::shared_ptr<IndexWriter> writer) ;

  static void addDocWithIndex(std::shared_ptr<IndexWriter> writer,
                              int index) ;

  // TODO: we have the logic in MDW to do this check, and it's better, because
  // it knows about files it tried to delete but couldn't: we should replace
  // this!!!!
  static void
  assertNoUnreferencedFiles(std::shared_ptr<Directory> dir,
                            const std::wstring &message) ;

  static std::wstring arrayToString(std::deque<std::wstring> &l);

  // Make sure we can open an index for create even when a
  // reader holds it open (this fails pre lock-less
  // commits on windows):
  virtual void testCreateWithReader() ;

  virtual void testChangesAfterClose() ;

  virtual void testIndexNoDocuments() ;

  virtual void testSmallRAMBuffer() ;

  /** Returns how many unique segment names are in the directory. */
private:
  static int getSegmentCount(std::shared_ptr<Directory> dir) ;

  // Make sure it's OK to change RAM buffer size and
  // maxBufferedDocs in a write session
public:
  virtual void testChangingRAMBuffer() ;

  virtual void testEnablingNorms() ;

  virtual void testHighFreqTerm() ;

  virtual void testFlushWithNoMerging() ;

  // Make sure we can flush segment w/ norms, then add
  // empty doc (no norms) and flush
  virtual void testEmptyDocAfterFlushingRealDoc() ;

  /**
   * Test that no NullPointerException will be raised,
   * when adding one document with a single, empty field
   * and term vectors enabled.
   */
  virtual void testBadSegment() ;

  // LUCENE-1036
  virtual void testMaxThreadPriority() ;

  virtual void testVariableSchema() ;

  // LUCENE-1084: test unlimited field length
  virtual void testUnlimitedMaxFieldLength() ;

  // LUCENE-1179
  virtual void testEmptyFieldName() ;

  virtual void testEmptyFieldNameTerms() ;

  virtual void testEmptyFieldNameWithEmptyTerm() ;

private:
  class MockIndexWriter final : public IndexWriter
  {
    GET_CLASS_NAME(MockIndexWriter)

  public:
    MockIndexWriter(std::shared_ptr<Directory> dir,
                    std::shared_ptr<IndexWriterConfig> conf) ;

    bool afterWasCalled = false;
    bool beforeWasCalled = false;

    void doAfterFlush() override;

  protected:
    void doBeforeFlush() override;

  protected:
    std::shared_ptr<MockIndexWriter> shared_from_this()
    {
      return std::static_pointer_cast<MockIndexWriter>(
          IndexWriter::shared_from_this());
    }
  };

  // LUCENE-1222
public:
  virtual void testDoBeforeAfterFlush() ;

  // LUCENE-1255
  virtual void testNegativePositions() ;

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<TestIndexWriter> outerInstance);

    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;

    const std::shared_ptr<Iterator<std::wstring>> terms;
    bool first = false;

    bool incrementToken() override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

  // LUCENE-2529
public:
  virtual void testPositionIncrementGapEmptyField() ;

  virtual void testDeadlock() ;

private:
  class IndexerThreadInterrupt : public Thread
  {
    GET_CLASS_NAME(IndexerThreadInterrupt)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool failed;
    bool failed = false;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool finish;
    bool finish = false;

    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool allowInterrupt = false;
    bool allowInterrupt = false;
    const std::shared_ptr<Random> random;
    const std::shared_ptr<Directory> adder;
    const std::shared_ptr<ByteArrayOutputStream> bytesLog =
        std::make_shared<ByteArrayOutputStream>();
    const std::shared_ptr<PrintStream> log =
        std::make_shared<PrintStream>(bytesLog, true, IOUtils::UTF_8);
    const int id;

    IndexerThreadInterrupt(std::shared_ptr<TestIndexWriter> outerInstance,
                           int id) ;

  private:
    class SuppressingConcurrentMergeSchedulerAnonymousInnerClass
        : public SuppressingConcurrentMergeScheduler
    {
      GET_CLASS_NAME(SuppressingConcurrentMergeSchedulerAnonymousInnerClass)
    private:
      std::shared_ptr<IndexerThreadInterrupt> outerInstance;

    public:
      SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
          std::shared_ptr<IndexerThreadInterrupt> outerInstance);

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

  public:
    void run() override;

  private:
    class SuppressingConcurrentMergeSchedulerAnonymousInnerClass2
        : public SuppressingConcurrentMergeScheduler
    {
      GET_CLASS_NAME(SuppressingConcurrentMergeSchedulerAnonymousInnerClass2)
    private:
      std::shared_ptr<IndexerThreadInterrupt> outerInstance;

    public:
      SuppressingConcurrentMergeSchedulerAnonymousInnerClass2(
          std::shared_ptr<IndexerThreadInterrupt> outerInstance);

    protected:
      bool isOK(std::runtime_error th) override;

    protected:
      std::shared_ptr<SuppressingConcurrentMergeSchedulerAnonymousInnerClass2>
      shared_from_this()
      {
        return std::static_pointer_cast<
            SuppressingConcurrentMergeSchedulerAnonymousInnerClass2>(
            SuppressingConcurrentMergeScheduler::shared_from_this());
      }
    };

  private:
    void listIndexFiles(std::shared_ptr<PrintStream> log,
                        std::shared_ptr<Directory> dir);

  protected:
    std::shared_ptr<IndexerThreadInterrupt> shared_from_this()
    {
      return std::static_pointer_cast<IndexerThreadInterrupt>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testThreadInterruptDeadlock() ;

  virtual void testIndexStoreCombos() ;

  virtual void testNoDocsIndex() ;

  virtual void testDeleteUnusedFiles() ;

  virtual void testDeleteUnusedFiles2() ;

  virtual void testEmptyFSDirWithNoLock() ;

  virtual void testEmptyDirRollback() ;

  virtual void testNoUnwantedTVFiles() ;

public:
  class StringSplitAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(StringSplitAnalyzer)
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<StringSplitAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<StringSplitAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class StringSplitTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(StringSplitTokenizer)
  private:
    std::deque<std::wstring> tokens;
    int upto = 0;
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

  public:
    StringSplitTokenizer();

    bool incrementToken() override final;

    void reset()  override;

  protected:
    std::shared_ptr<StringSplitTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<StringSplitTokenizer>(
          org.apache.lucene.analysis.Tokenizer::shared_from_this());
    }
  };

  /**
   * Make sure we skip wicked long terms.
   */
public:
  virtual void testWickedLongTerm() ;

  virtual void testDeleteAllNRTLeftoverFiles() ;

  virtual void testNRTReaderVersion() ;

  virtual void testWhetherDeleteAllDeletesWriteLock() ;

  virtual void testChangeIndexOptions() ;

  virtual void testOnlyUpdateDocuments() ;

  // LUCENE-3872
  virtual void testPrepareCommitThenClose() ;

  // LUCENE-3872
  virtual void testPrepareCommitThenRollback() ;

  // LUCENE-3872
  virtual void testPrepareCommitThenRollback2() ;

  virtual void
  testDontInvokeAnalyzerForUnAnalyzedFields() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<TestIndexWriter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  public:
    int getPositionIncrementGap(const std::wstring &fieldName) override;

    int getOffsetGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-1468 -- make sure opening an IndexWriter with
  // create=true does not remove non-index files

public:
  virtual void testOtherFiles() ;

  // LUCENE-3849
  virtual void testStopwordsPosIncHole() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-3849
public:
  virtual void testStopwordsPosIncHole2() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

    std::shared_ptr<Automaton> secondSet;

  public:
    AnalyzerAnonymousInnerClass3(std::shared_ptr<TestIndexWriter> outerInstance,
                                 std::shared_ptr<Automaton> secondSet);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-4575
public:
  virtual void testCommitWithUserDataOnly() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    HashMapAnonymousInnerClass(std::shared_ptr<TestIndexWriter> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

private:
  class HashMapAnonymousInnerClass2
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    HashMapAnonymousInnerClass2(std::shared_ptr<TestIndexWriter> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass2>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

private:
  class HashMapAnonymousInnerClass3
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    HashMapAnonymousInnerClass3(std::shared_ptr<TestIndexWriter> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass3>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

private:
  std::unordered_map<std::wstring, std::wstring>
  getLiveCommitData(std::shared_ptr<IndexWriter> writer);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetCommitData() throws Exception
  virtual void testGetCommitData() ;

private:
  class HashMapAnonymousInnerClass4
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass4)
  private:
    std::shared_ptr<TestIndexWriter> outerInstance;

  public:
    HashMapAnonymousInnerClass4(std::shared_ptr<TestIndexWriter> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass4>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testNullAnalyzer() ;

  virtual void testNullDocument() ;

  virtual void testNullDocuments() ;

  virtual void testIterableFieldThrowsException() ;

  virtual void testIterableThrowsException() ;

  virtual void testIterableThrowsException2() ;
       );
       TestUtil::assertEquals(L"boom", expected::getMessage());

       w->close();
       IOUtils::close({dir});

     protected:
       std::shared_ptr<TestIndexWriter> shared_from_this()
       {
         return std::static_pointer_cast<TestIndexWriter>(
             org.apache.lucene.util.LuceneTestCase::shared_from_this());
       }
};

template <typename T>
class RandomFailingIterable
    : public std::enable_shared_from_this<RandomFailingIterable>,
      public std::deque<T>
{
  GET_CLASS_NAME(RandomFailingIterable)
private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final Iterable<? extends T> deque;
  const std::deque < ? extends T > deque;
  const int failOn;

public:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public RandomFailingIterable(Iterable<? extends
  // T> deque, java.util.Random random)
  RandomFailingIterable(std::deque<T1> deque, std::shared_ptr<Random> random)
      : deque(deque), failOn(random->nextInt(5))
  {
  }

  std::shared_ptr<Iterator<T>> iterator() override
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.Iterator<? extends T> docIter =
    // deque.iterator();
    constexpr std::deque < ? extends T > ::const_iterator docIter =
                                  deque.begin();
    return std::make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                         docIter);
  }

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<T>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<RandomFailingIterable<std::shared_ptr<T>>> outerInstance;

    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private std::deque<? extends T>::const_iterator docIter;
    std::deque < ? extends T > ::const_iterator docIter;

  public:
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: public
    // IteratorAnonymousInnerClass(RandomFailingIterable<T> outerInstance,
    // std::deque<? extends T>::const_iterator docIter)
            IteratorAnonymousInnerClass(std::shared_ptr<RandomFailingIterable<std::shared_ptr<T>>> outerInstance, std::deque<? extends T>::const_iterator docIter);

            int count = 0;

            bool hasNext();

            std::shared_ptr<T> next();

            void remove();
  };
};

// LUCENE-2727/LUCENE-2812/LUCENE-4738:
public:
virtual void testCorruptFirstCommit() ;

virtual void testHasUncommittedChanges() ;

virtual void testMergeAllDeleted() ;

class FilterMergePolicyAnonymousInnerClass : public FilterMergePolicy
{
  GET_CLASS_NAME(FilterMergePolicyAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<AtomicBoolean> keepFullyDeletedSegments;

public:
  FilterMergePolicyAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<org::apache::lucene::index::MergePolicy> getMergePolicy,
      std::shared_ptr<AtomicBoolean> keepFullyDeletedSegments);

  bool
  keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                              readerIOSupplier)  override;

protected:
  std::shared_ptr<FilterMergePolicyAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<FilterMergePolicyAnonymousInnerClass>(
        FilterMergePolicy::shared_from_this());
  }
};

class TestPointAnonymousInnerClass
    : public std::enable_shared_from_this<TestPointAnonymousInnerClass>,
      public RandomIndexWriter::TestPoint
{
  GET_CLASS_NAME(TestPointAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<AtomicBoolean> keepFullyDeletedSegments;

public:
  TestPointAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<AtomicBoolean> keepFullyDeletedSegments);

  void apply(const std::wstring &message) override;
};

// LUCENE-5239
public:
virtual void testDeleteSameTermAcrossFields() ;

virtual void testHasUncommittedChangesAfterException() ;

virtual void testDoubleClose() ;

virtual void testRollbackThenClose() ;

virtual void testCloseThenRollback() ;

virtual void testCloseWhileMergeIsRunning() ;

class InfoStreamAnonymousInnerClass : public InfoStream
{
  GET_CLASS_NAME(InfoStreamAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<CountDownLatch> closeStarted;

public:
  InfoStreamAnonymousInnerClass(std::shared_ptr<MissingClass> outerInstance,
                                std::shared_ptr<CountDownLatch> closeStarted);

  bool isEnabled(const std::wstring &component) override;

  void message(const std::wstring &component,
               const std::wstring &message) override;

  virtual ~InfoStreamAnonymousInnerClass();

protected:
  std::shared_ptr<InfoStreamAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<InfoStreamAnonymousInnerClass>(
        org.apache.lucene.util.InfoStream::shared_from_this());
  }
};

class ConcurrentMergeSchedulerAnonymousInnerClass
    : public ConcurrentMergeScheduler
{
  GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<CountDownLatch> mergeStarted;
  std::shared_ptr<CountDownLatch> closeStarted;

public:
  ConcurrentMergeSchedulerAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<CountDownLatch> mergeStarted,
      std::shared_ptr<CountDownLatch> closeStarted);

  void doMerge(
      std::shared_ptr<IndexWriter> writer,
      std::shared_ptr<MergePolicy::OneMerge> merge)  override;

  virtual ~ConcurrentMergeSchedulerAnonymousInnerClass();

protected:
  std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass>
  shared_from_this()
  {
    return std::static_pointer_cast<
        ConcurrentMergeSchedulerAnonymousInnerClass>(
        ConcurrentMergeScheduler::shared_from_this());
  }
};

/** Make sure that close waits for any still-running commits. */
public:
virtual void testCloseDuringCommit() ;

class TestPointAnonymousInnerClass2
    : public std::enable_shared_from_this<TestPointAnonymousInnerClass2>,
      public RandomIndexWriter::TestPoint
{
  GET_CLASS_NAME(TestPointAnonymousInnerClass2)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<CountDownLatch> startCommit;

public:
  TestPointAnonymousInnerClass2(std::shared_ptr<MissingClass> outerInstance,
                                std::shared_ptr<CountDownLatch> startCommit);

  void apply(const std::wstring &message) override;
};

class ThreadAnonymousInnerClass : public Thread
{
  GET_CLASS_NAME(ThreadAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<CountDownLatch> finishCommit;
  std::shared_ptr<org::apache::lucene::index::IndexWriter> iw;

public:
  ThreadAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<CountDownLatch> finishCommit,
      std::shared_ptr<org::apache::lucene::index::IndexWriter> iw);

  void run() override;

protected:
  std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<ThreadAnonymousInnerClass>(
        Thread::shared_from_this());
  }
};

// LUCENE-5895:

/** Make sure we see ids per segment and per commit. */
public:
virtual void testIds() ;

virtual void testEmptyNorm() ;

virtual void testManySeparateThreads() ;

class ThreadAnonymousInnerClass2 : public Thread
{
  GET_CLASS_NAME(ThreadAnonymousInnerClass2)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<org::apache::lucene::index::IndexWriter> w;

public:
  ThreadAnonymousInnerClass2(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<org::apache::lucene::index::IndexWriter> w);

  void run() override;

protected:
  std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
        Thread::shared_from_this());
  }
};

// LUCENE-6505
public:
virtual void testNRTSegmentsFile() ;

// LUCENE-6505
virtual void testNRTAfterCommit() ;

// LUCENE-6505
virtual void testNRTAfterSetUserDataWithoutCommit() ;

// LUCENE-6505
virtual void testNRTAfterSetUserDataWithCommit() ;

// LUCENE-6523
virtual void testCommitImmediatelyAfterNRTReopen() ;

virtual void testPendingDeleteDVGeneration() ;

virtual void testWithPendingDeletions() ;

virtual void testPendingDeletesAlreadyWrittenFiles() ;

virtual void testLeftoverTempFiles() ;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("requires running tests with biggish heap") public
// void testMassiveField() throws Exception
virtual void testMassiveField() ;

virtual void testRecordsIndexCreatedVersion() ;

virtual void testFlushLargestWriter() ;

private:
int indexDocsForMultipleThreadStates(std::shared_ptr<IndexWriter> w) throw(
    InterruptedException);

public:
virtual void testNeverCheckOutOnFullFlush() throw(IOException,
                                                  InterruptedException);

virtual void testHoldLockOnLargestWriter() throw(IOException,
                                                 InterruptedException);

virtual void testCheckPendingFlushPostUpdate() throw(IOException,
                                                     InterruptedException);

class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
{
  GET_CLASS_NAME(FailureAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<MockDirectoryWrapper> dir;
  std::shared_ptr<Set<std::wstring>> flushingThreads;

public:
  FailureAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<MockDirectoryWrapper> dir,
      std::shared_ptr<Set<std::wstring>> flushingThreads);

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

private:
static void waitForDocsInBuffers(std::shared_ptr<IndexWriter> w,
                                 int buffersWithDocs);

public:
virtual void testSoftUpdateDocuments() ;

virtual void testSoftUpdatesConcurrently() throw(IOException,
                                                 InterruptedException);

virtual void
testSoftUpdatesConcurrentlyMixedDeletes() throw(IOException,
                                                InterruptedException);

virtual void
softUpdatesConcurrently(bool mixDeletes) throw(IOException,
                                               InterruptedException);

class FilterCodecReaderAnonymousInnerClass : public FilterCodecReader
{
  GET_CLASS_NAME(FilterCodecReaderAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

public:
  FilterCodecReaderAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<org::apache::lucene::index::CodecReader> wrapped);

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  std::shared_ptr<Bits> getLiveDocs() override;

  int numDocs() override;

protected:
  std::shared_ptr<FilterCodecReaderAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<FilterCodecReaderAnonymousInnerClass>(
        FilterCodecReader::shared_from_this());
  }
};

public:
virtual void testDeleteHappensBeforeWhileFlush() throw(IOException,
                                                       InterruptedException);

class FilterDirectoryAnonymousInnerClass : public FilterDirectory
{
  GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
private:
  std::shared_ptr<MissingClass> outerInstance;

  std::shared_ptr<CountDownLatch> latch;
  std::shared_ptr<CountDownLatch> inFlush;

public:
  FilterDirectoryAnonymousInnerClass(
      std::shared_ptr<MissingClass> outerInstance,
      std::shared_ptr<UnknownType> newDirectory,
      std::shared_ptr<CountDownLatch> latch,
      std::shared_ptr<CountDownLatch> inFlush);

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

protected:
  std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
        org.apache.lucene.store.FilterDirectory::shared_from_this());
  }
};

private:
static void assertFiles(std::shared_ptr<IndexWriter> writer) ;

public:
virtual void testFullyDeletedSegmentsReleaseFiles() ;

virtual void testSegmentInfoIsSnapshot() ;

virtual void testPreventChangingSoftDeletesField() ;

virtual void testPreventAddingIndexesWithDifferentSoftDeletesField() throw(
    std::runtime_error);

virtual void
testNotAllowUsingExistingFieldAsSoftDeletes() ;
} // namespace org::apache::lucene::index
}
