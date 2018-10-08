#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis
{
class MockTokenizer;
}
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}
namespace org::apache::lucene::index
{
class MergePolicy;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::store
{
class Failure;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::store
{
class Directory;
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

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs("SimpleText") public class
// TestIndexWriterExceptions extends org.apache.lucene.util.LuceneTestCase
class TestIndexWriterExceptions : public LuceneTestCase
{

private:
  class DocCopyIterator : public std::enable_shared_from_this<DocCopyIterator>,
                          public std::deque<std::shared_ptr<Document>>
  {
    GET_CLASS_NAME(DocCopyIterator)
  private:
    const std::shared_ptr<Document> doc;
    const int count;

    /* private field types */
    /* private field types */

    static const std::shared_ptr<FieldType> custom1;
    static const std::shared_ptr<FieldType> custom2;
    static const std::shared_ptr<FieldType> custom3;
    static const std::shared_ptr<FieldType> custom4;
    static const std::shared_ptr<FieldType> custom5;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static DocCopyIterator::StaticConstructor staticConstructor;

  public:
    DocCopyIterator(std::shared_ptr<Document> doc, int count);

    std::shared_ptr<Iterator<std::shared_ptr<Document>>> iterator() override;

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<Document>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<DocCopyIterator> outerInstance;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<DocCopyIterator> outerInstance);

      int upto = 0;

      bool hasNext();

      std::shared_ptr<Document> next();

      void remove();
    };
  };

private:
  class IndexerThread : public Thread
  {
    GET_CLASS_NAME(IndexerThread)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    std::shared_ptr<IndexWriter> writer;

    const std::shared_ptr<Random> r =
        std::make_shared<Random>(LuceneTestCase::random()->nextLong());
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile Throwable failure;
    std::runtime_error failure;

    IndexerThread(std::shared_ptr<TestIndexWriterExceptions> outerInstance,
                  int i, std::shared_ptr<IndexWriter> writer);

    void run() override;

  protected:
    std::shared_ptr<IndexerThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexerThread>(
          Thread::shared_from_this());
    }
  };

public:
  std::shared_ptr<ThreadLocal<std::shared_ptr<Thread>>> doFail =
      std::make_shared<ThreadLocal<std::shared_ptr<Thread>>>();

private:
  class TestPoint1 : public std::enable_shared_from_this<TestPoint1>,
                     public RandomIndexWriter::TestPoint
  {
    GET_CLASS_NAME(TestPoint1)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    TestPoint1(std::shared_ptr<TestIndexWriterExceptions> outerInstance);

    std::shared_ptr<Random> r =
        std::make_shared<Random>(LuceneTestCase::random()->nextLong());
    void apply(const std::wstring &name) override;
  };

public:
  virtual void testRandomExceptions() ;

  virtual void testRandomExceptionsThreads() ;

  // LUCENE-1198
private:
  class TestPoint2 final : public std::enable_shared_from_this<TestPoint2>,
                           public RandomIndexWriter::TestPoint
  {
    GET_CLASS_NAME(TestPoint2)
  public:
    bool doFail = false;

    void apply(const std::wstring &name) override;
  };

private:
  static std::wstring CRASH_FAIL_MESSAGE;

private:
  class CrashingFilter : public TokenFilter
  {
    GET_CLASS_NAME(CrashingFilter)
  public:
    std::wstring fieldName;
    int count = 0;

    CrashingFilter(const std::wstring &fieldName,
                   std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<CrashingFilter> shared_from_this()
    {
      return std::static_pointer_cast<CrashingFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testExceptionDocumentsWriterInit() ;

  // LUCENE-1208
  virtual void testExceptionJustBeforeFlush() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    std::shared_ptr<AtomicBoolean> doCrash;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY,
        std::shared_ptr<AtomicBoolean> doCrash);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class TestPoint3 final : public std::enable_shared_from_this<TestPoint3>,
                           public RandomIndexWriter::TestPoint
  {
    GET_CLASS_NAME(TestPoint3)
  public:
    bool doFail = false;
    bool failed = false;
    void apply(const std::wstring &name) override;
  };

  // LUCENE-1210
public:
  virtual void testExceptionOnMergeInit() ;

  // LUCENE-1072
  virtual void testExceptionFromTokenStream() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenFilterAnonymousInnerClass : public TokenFilter
    {
      GET_CLASS_NAME(TokenFilterAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance;

    public:
      TokenFilterAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
          std::shared_ptr<MockTokenizer> tokenizer);

    private:
      int count = 0;

    public:
      bool incrementToken()  override;

      void reset()  override;

    protected:
      std::shared_ptr<TokenFilterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TokenFilterAnonymousInnerClass>(
            org.apache.lucene.analysis.TokenFilter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class FailOnlyOnFlush : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyOnFlush)
  public:
    bool doFail = false;
    int count = 0;

    void setDoFail() override;
    void clearDoFail() override;

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyOnFlush> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyOnFlush>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // make sure an aborting exception closes the writer:
public:
  virtual void testDocumentsWriterAbort() ;

  virtual void testDocumentsWriterExceptions() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void
  testDocumentsWriterExceptionFailOneDoc() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class FilterMergePolicyAnonymousInnerClass : public FilterMergePolicy
  {
    GET_CLASS_NAME(FilterMergePolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    FilterMergePolicyAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<org::apache::lucene::index::MergePolicy> INSTANCE);

    bool keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>> readerIOSupplier) override;

  protected:
    std::shared_ptr<FilterMergePolicyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterMergePolicyAnonymousInnerClass>(
          FilterMergePolicy::shared_from_this());
    }
  };

public:
  virtual void testDocumentsWriterExceptionThreads() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class FilterMergePolicyAnonymousInnerClass2 : public FilterMergePolicy
  {
    GET_CLASS_NAME(FilterMergePolicyAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    FilterMergePolicyAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<org::apache::lucene::index::MergePolicy> INSTANCE);

    // don't use a merge policy here they depend on the DWPThreadPool and its
    // max thread states etc. we also need to keep fully deleted segments since
    // otherwise we clean up fully deleted ones and if we flush the one that has
    // only the failed document the docFreq checks will be off below.
    bool keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>> readerIOSupplier) override;

  protected:
    std::shared_ptr<FilterMergePolicyAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FilterMergePolicyAnonymousInnerClass2>(
          FilterMergePolicy::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    int NUM_ITER = 0;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    int finalI = 0;
    int t = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance, int NUM_ITER,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        int finalI, int t);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  // Throws IOException during MockDirectoryWrapper.sync
private:
  class FailOnlyInSync : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyInSync)
  public:
    bool didFail = false;
    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyInSync> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyInSync>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // TODO: these are also in TestIndexWriter... add a simple doc-writing method
  // like this to LuceneTestCase?
private:
  void addDoc(std::shared_ptr<IndexWriter> writer) ;

  // LUCENE-1044: test exception during sync
public:
  virtual void testExceptionDuringSync() ;

private:
  class FailOnlyInCommit : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnlyInCommit)

  public:
    bool failOnCommit = false, failOnDeleteFile = false,
         failOnSyncMetadata = false;

  private:
    const bool dontFailDuringGlobalFieldMap;
    const bool dontFailDuringSyncMetadata;
    static const std::wstring PREPARE_STAGE;
    static const std::wstring FINISH_STAGE;
    const std::wstring stage;

  public:
    FailOnlyInCommit(bool dontFailDuringGlobalFieldMap,
                     bool dontFailDuringSyncMetadata,
                     const std::wstring &stage);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnlyInCommit> shared_from_this()
    {
      return std::static_pointer_cast<FailOnlyInCommit>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  virtual void testExceptionsDuringCommit() ;

  virtual void testForceMergeExceptions() ;

  // LUCENE-1429
  virtual void
  testOutOfMemoryErrorCausesCloseToFail() ;

private:
  class InfoStreamAnonymousInnerClass : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    std::shared_ptr<AtomicBoolean> thrown;

  public:
    InfoStreamAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<AtomicBoolean> thrown);

    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

    virtual ~InfoStreamAnonymousInnerClass();

  protected:
    std::shared_ptr<InfoStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamAnonymousInnerClass>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

  /** If IW hits OOME during indexing, it should refuse to commit any further
   * changes. */
public:
  virtual void testOutOfMemoryErrorRollback() ;

private:
  class InfoStreamAnonymousInnerClass2 : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    std::shared_ptr<AtomicBoolean> thrown;

  public:
    InfoStreamAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<AtomicBoolean> thrown);

    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

    virtual ~InfoStreamAnonymousInnerClass2();

  protected:
    std::shared_ptr<InfoStreamAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamAnonymousInnerClass2>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

  // LUCENE-1347
private:
  class TestPoint4 final : public std::enable_shared_from_this<TestPoint4>,
                           public RandomIndexWriter::TestPoint
  {
    GET_CLASS_NAME(TestPoint4)

  public:
    bool doFail = false;

    void apply(const std::wstring &name) override;
  };

  // LUCENE-1347
public:
  virtual void testRollbackExceptionHang() ;

  // LUCENE-1044: Simulate checksum error in segments_N
  virtual void testSegmentsChecksumError() ;

  // Simulate a corrupt index by removing last byte of
  // latest segments file and make sure we get an
  // IOException trying to open the index:
  virtual void testSimulatedCorruptIndex1() ;

  // Simulate a corrupt index by removing one of the
  // files and make sure we get an IOException trying to
  // open the index:
  virtual void testSimulatedCorruptIndex2() ;

  virtual void testTermVectorExceptions() ;

private:
  class FailOnTermVectors : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailOnTermVectors)

  private:
    static const std::wstring INIT_STAGE;
    static const std::wstring AFTER_INIT_STAGE;
    static const std::wstring EXC_MSG;
    const std::wstring stage;

  public:
    FailOnTermVectors(const std::wstring &stage);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailOnTermVectors> shared_from_this()
    {
      return std::static_pointer_cast<FailOnTermVectors>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  virtual void testAddDocsNonAbortingException() ;

  virtual void testUpdateDocsNonAbortingException() ;

  /** test a null string value doesn't abort the entire segment */
  virtual void testNullStoredField() ;

  /** test a null string value doesn't abort the entire segment */
  virtual void testNullStoredFieldReuse() ;

  /** test a null byte[] value doesn't abort the entire segment */
  virtual void testNullStoredBytesField() ;

  /** test a null byte[] value doesn't abort the entire segment */
  virtual void testNullStoredBytesFieldReuse() ;

  /** test a null bytesref value doesn't abort the entire segment */
  virtual void testNullStoredBytesRefField() ;

  /** test a null bytesref value doesn't abort the entire segment */
  virtual void testNullStoredBytesRefFieldReuse() ;

  virtual void testCrazyPositionIncrementGap() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  public:
    int getPositionIncrementGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // TODO: we could also check isValid, to catch "broken" bytesref values, might
  // be too much?

public:
  class UOEDirectory : public RAMDirectory
  {
    GET_CLASS_NAME(UOEDirectory)
  public:
    bool doFail = false;

    std::shared_ptr<IndexInput>
    openInput(const std::wstring &name,
              std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<UOEDirectory> shared_from_this()
    {
      return std::static_pointer_cast<UOEDirectory>(
          org.apache.lucene.store.RAMDirectory::shared_from_this());
    }
  };

public:
  virtual void testExceptionOnCtor() ;

  // See LUCENE-4870 TooManyOpenFiles errors are thrown as
  // FNFExceptions which can trigger data loss.
  virtual void testTooManyFileException() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance);

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

  // kind of slow, but omits positions, so just CPU
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testTooManyTokens() throws Exception
  virtual void testTooManyTokens() ;

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance);

    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<PositionIncrementAttribute> posIncAtt;
    int64_t num = 0;

    bool incrementToken()  override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testExceptionDuringRollback() ;

private:
  class InfoStreamAnonymousInnerClass : public InfoStream
  {
    GET_CLASS_NAME(InfoStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    std::wstring messageToFailOn;

  public:
    InfoStreamAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        const std::wstring &messageToFailOn);

    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

    virtual ~InfoStreamAnonymousInnerClass();

  protected:
    std::shared_ptr<InfoStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InfoStreamAnonymousInnerClass>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

private:
  class IndexWriterAnonymousInnerClass : public IndexWriter
  {
    GET_CLASS_NAME(IndexWriterAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

  public:
    IndexWriterAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<Directory> dir,
        std::shared_ptr<org::apache::lucene::index::IndexWriterConfig> iwc);

  protected:
    bool isEnableTestPoints() override;

  protected:
    std::shared_ptr<IndexWriterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexWriterAnonymousInnerClass>(
          IndexWriter::shared_from_this());
    }
  };

public:
  virtual void testRandomExceptionDuringRollback() ;

private:
  class FailureAnonymousInnerClass2 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;

  public:
    FailureAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir);

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
  virtual void testMergeExceptionIsTragic() ;

private:
  class FailureAnonymousInnerClass3 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexWriterExceptions> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;
    std::shared_ptr<AtomicBoolean> didFail;

  public:
    FailureAnonymousInnerClass3(
        std::shared_ptr<TestIndexWriterExceptions> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir,
        std::shared_ptr<AtomicBoolean> didFail);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass3>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexWriterExceptions> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterExceptions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
