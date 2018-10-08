using namespace std;

#include "TestIndexWriterOnVMError.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using MockVariableLengthPayloadFilter =
    org::apache::lucene::analysis::MockVariableLengthPayloadFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Failure = org::apache::lucene::store::MockDirectoryWrapper::Failure;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using Rethrow = org::apache::lucene::util::Rethrow;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexWriterOnVMError::doTest(shared_ptr<Failure> failOn) throw(
    runtime_error)
{
  // log all exceptions we hit, in case we fail (for debugging)
  shared_ptr<ByteArrayOutputStream> exceptionLog =
      make_shared<ByteArrayOutputStream>();
  shared_ptr<PrintStream> exceptionStream =
      make_shared<PrintStream>(exceptionLog, true, L"UTF-8");
  // PrintStream exceptionStream = System.out;

  constexpr int64_t analyzerSeed = random()->nextLong();
  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(),
                                               analyzerSeed);

  shared_ptr<MockDirectoryWrapper> dir = nullptr;

  constexpr int numIterations = TEST_NIGHTLY ? atLeast(100) : atLeast(5);

  for (int iter = 0; iter < numIterations; iter++) {
    try {
      // close from last run
      if (dir != nullptr) {
        delete dir;
      }
      // disable slow things: we don't rely upon sleeps here.
      dir = newMockDirectory();
      dir->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
      dir->setUseSlowOpenClosers(false);

      shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
      // just for now, try to keep this test reproducible
      conf->setMergeScheduler(make_shared<SerialMergeScheduler>());

      // test never makes it this far...
      int numDocs = atLeast(2000);

      shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, conf);
      iw->commit(); // ensure there is always a commit

      dir->failOn(failOn);

      for (int i = 0; i < numDocs; i++) {
        shared_ptr<Document> doc = make_shared<Document>();
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(
            newStringField(L"id", Integer::toString(i), Field::Store::NO));
        doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(make_shared<BinaryDocValuesField>(
            L"dv2", make_shared<BytesRef>(Integer::toString(i))));
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(make_shared<SortedDocValuesField>(
            L"dv3", make_shared<BytesRef>(Integer::toString(i))));
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(make_shared<SortedSetDocValuesField>(
            L"dv4", make_shared<BytesRef>(Integer::toString(i))));
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(make_shared<SortedSetDocValuesField>(
            L"dv4", make_shared<BytesRef>(Integer::toString(i - 1))));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv5", i));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv5", i - 1));
        doc->push_back(newTextField(
            L"text1", TestUtil::randomAnalysisString(random(), 20, true),
            Field::Store::NO));
        // ensure we store something
        doc->push_back(make_shared<StoredField>(L"stored1", L"foo"));
        doc->push_back(make_shared<StoredField>(L"stored1", L"bar"));
        // ensure we get some payloads
        doc->push_back(newTextField(
            L"text_payloads", TestUtil::randomAnalysisString(random(), 6, true),
            Field::Store::NO));
        // ensure we get some vectors
        shared_ptr<FieldType> ft =
            make_shared<FieldType>(TextField::TYPE_NOT_STORED);
        ft->setStoreTermVectors(true);
        doc->push_back(
            newField(L"text_vectors",
                     TestUtil::randomAnalysisString(random(), 6, true), ft));
        doc->push_back(make_shared<IntPoint>(L"point", random()->nextInt()));
        doc->push_back(make_shared<IntPoint>(L"point2d", random()->nextInt(),
                                             random()->nextInt()));

        if (random()->nextInt(10) > 0) {
          // single doc
          try {
            iw->addDocument(doc);
            // we made it, sometimes delete our doc, or update a dv
            int thingToDo = random()->nextInt(4);
            if (thingToDo == 0) {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              iw->deleteDocuments(
                  {make_shared<Term>(L"id", Integer::toString(i))});
            } else if (thingToDo == 1) {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              iw->updateNumericDocValue(
                  make_shared<Term>(L"id", Integer::toString(i)), L"dv",
                  i + 1LL);
            } else if (thingToDo == 2) {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              iw->updateBinaryDocValue(
                  make_shared<Term>(L"id", Integer::toString(i)), L"dv2",
                  make_shared<BytesRef>(Integer::toString(i + 1)));
            }
          }
          // C++ TODO: There is no equivalent in C++ to Java 'multi-catch'
          // syntax:
          catch (VirtualMachineError | AlreadyClosedException disaster) {
            getTragedy(disaster, iw, exceptionStream);
            goto STARTOVERContinue;
          }
        } else {
          // block docs
          shared_ptr<Document> doc2 = make_shared<Document>();
          // C++ TODO: There is no native C++ equivalent to 'toString':
          doc2->push_back(
              newStringField(L"id", Integer::toString(-i), Field::Store::NO));
          doc2->push_back(newTextField(
              L"text1", TestUtil::randomAnalysisString(random(), 20, true),
              Field::Store::NO));
          doc2->push_back(make_shared<StoredField>(L"stored1", L"foo"));
          doc2->push_back(make_shared<StoredField>(L"stored1", L"bar"));
          doc2->push_back(
              newField(L"text_vectors",
                       TestUtil::randomAnalysisString(random(), 6, true), ft));

          try {
            iw->addDocuments(Arrays::asList(doc, doc2));
            // we made it, sometimes delete our docs
            if (random()->nextBoolean()) {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              iw->deleteDocuments(
                  {make_shared<Term>(L"id", Integer::toString(i)),
                   make_shared<Term>(L"id", Integer::toString(-i))});
            }
          }
          // C++ TODO: There is no equivalent in C++ to Java 'multi-catch'
          // syntax:
          catch (VirtualMachineError | AlreadyClosedException disaster) {
            getTragedy(disaster, iw, exceptionStream);
            goto STARTOVERContinue;
          }
        }

        if (random()->nextInt(10) == 0) {
          // trigger flush:
          try {
            if (random()->nextBoolean()) {
              shared_ptr<DirectoryReader> ir = nullptr;
              try {
                ir = DirectoryReader::open(iw, random()->nextBoolean(), false);
                TestUtil::checkReader(ir);
              }
              // C++ TODO: There is no native C++ equivalent to the exception
              // 'finally' clause:
              finally {
                IOUtils::closeWhileHandlingException({ir});
              }
            } else {
              iw->commit();
            }
            if (DirectoryReader::indexExists(dir)) {
              TestUtil::checkIndex(dir);
            }
          }
          // C++ TODO: There is no equivalent in C++ to Java 'multi-catch'
          // syntax:
          catch (VirtualMachineError | AlreadyClosedException disaster) {
            getTragedy(disaster, iw, exceptionStream);
            goto STARTOVERContinue;
          }
        }
      }

      try {
        delete iw;
      }
      // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
      catch (VirtualMachineError | AlreadyClosedException disaster) {
        getTragedy(disaster, iw, exceptionStream);
        goto STARTOVERContinue;
      }
    } catch (const runtime_error &t) {
      wcout << L"Unexpected exception: dumping fake-exception-log:..." << endl;
      exceptionStream->flush();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << exceptionLog->toString(L"UTF-8") << endl;
      System::out::flush();
      Rethrow::rethrow(t);
    }
  STARTOVERContinue:;
  }
STARTOVERBreak:
  delete dir;
  if (VERBOSE) {
    wcout << L"TEST PASSED: dumping fake-exception-log:..." << endl;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << exceptionLog->toString(L"UTF-8") << endl;
  }
}

TestIndexWriterOnVMError::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterOnVMError> outerInstance,
        int64_t analyzerSeed)
{
  this->outerInstance = outerInstance;
  this->analyzerSeed = analyzerSeed;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterOnVMError::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setEnableChecks(false); // we are gonna make it angry
  shared_ptr<TokenStream> stream = tokenizer;
  // emit some payloads
  if (fieldName.find(L"payloads") != wstring::npos) {
    stream = make_shared<MockVariableLengthPayloadFilter>(
        make_shared<Random>(analyzerSeed), stream);
  }
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

shared_ptr<VirtualMachineError>
TestIndexWriterOnVMError::getTragedy(runtime_error disaster,
                                     shared_ptr<IndexWriter> writer,
                                     shared_ptr<PrintStream> log)
{
  runtime_error e = disaster;
  if (std::dynamic_pointer_cast<AlreadyClosedException>(e) != nullptr) {
    e = e.getCause();
  }

  if (std::dynamic_pointer_cast<VirtualMachineError>(e) != nullptr &&
      e.what() != nullptr && e.what()->startsWith(L"Fake")) {
    log->println(L"\nTEST: got expected fake exc:" + e.what());
    e.printStackTrace(log);
    // TODO: remove rollback here, and add this assert to ensure "full OOM
    // protection" anywhere IW does writes assertTrue("hit OOM but writer is
    // still open, WTF: ", writer.isClosed());
    try {
      writer->rollback();
    } catch (const runtime_error &t) {
    }
    return std::static_pointer_cast<VirtualMachineError>(e);
  } else {
    Rethrow::rethrow(disaster);
    return nullptr; // dead
  }
}

void TestIndexWriterOnVMError::testOOM() 
{
  shared_ptr<Random> *const r = make_shared<Random>(random()->nextLong());
  doTest(make_shared<FailureAnonymousInnerClass>(shared_from_this(), r));
}

TestIndexWriterOnVMError::FailureAnonymousInnerClass::
    FailureAnonymousInnerClass(
        shared_ptr<TestIndexWriterOnVMError> outerInstance,
        shared_ptr<Random> r)
{
  this->outerInstance = outerInstance;
  this->r = r;
}

void TestIndexWriterOnVMError::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (r->nextInt(3000) == 0) {
    std::deque<std::shared_ptr<StackTraceElement>> stack =
        Thread::currentThread().getStackTrace();
    bool ok = false;
    for (int i = 0; i < stack.size(); i++) {
      if (stack[i]->getClassName().equals(IndexWriter::typeid->getName())) {
        ok = true;
      }
    }
    if (ok) {
      throw make_shared<OutOfMemoryError>(L"Fake OutOfMemoryError");
    }
  }
}

void TestIndexWriterOnVMError::testUnknownError() 
{
  shared_ptr<Random> *const r = make_shared<Random>(random()->nextLong());
  doTest(make_shared<FailureAnonymousInnerClass2>(shared_from_this(), r));
}

TestIndexWriterOnVMError::FailureAnonymousInnerClass2::
    FailureAnonymousInnerClass2(
        shared_ptr<TestIndexWriterOnVMError> outerInstance,
        shared_ptr<Random> r)
{
  this->outerInstance = outerInstance;
  this->r = r;
}

void TestIndexWriterOnVMError::FailureAnonymousInnerClass2::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (r->nextInt(3000) == 0) {
    std::deque<std::shared_ptr<StackTraceElement>> stack =
        Thread::currentThread().getStackTrace();
    bool ok = false;
    for (int i = 0; i < stack.size(); i++) {
      if (stack[i]->getClassName().equals(IndexWriter::typeid->getName())) {
        ok = true;
      }
    }
    if (ok) {
      throw make_shared<UnknownError>(L"Fake UnknownError");
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testCheckpoint() throws Exception
void TestIndexWriterOnVMError::testCheckpoint() 
{
  shared_ptr<Random> *const r = make_shared<Random>(random()->nextLong());
  doTest(make_shared<FailureAnonymousInnerClass3>(shared_from_this(), r));
}

TestIndexWriterOnVMError::FailureAnonymousInnerClass3::
    FailureAnonymousInnerClass3(
        shared_ptr<TestIndexWriterOnVMError> outerInstance,
        shared_ptr<Random> r)
{
  this->outerInstance = outerInstance;
  this->r = r;
}

void TestIndexWriterOnVMError::FailureAnonymousInnerClass3::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  std::deque<std::shared_ptr<StackTraceElement>> stack =
      Thread::currentThread().getStackTrace();
  bool ok = false;
  for (int i = 0; i < stack.size(); i++) {
    if (stack[i]->getClassName().equals(IndexFileDeleter::typeid->getName()) &&
        stack[i]->getMethodName().equals(L"checkpoint")) {
      ok = true;
    }
  }
  if (ok && r->nextInt(4) == 0) {
    throw make_shared<OutOfMemoryError>(L"Fake OutOfMemoryError");
  }
}
} // namespace org::apache::lucene::index