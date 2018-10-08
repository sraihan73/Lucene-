using namespace std;

#include "TestIndexWriterExceptions.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using FakeIOException =
    org::apache::lucene::store::MockDirectoryWrapper::FakeIOException;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;
const shared_ptr<org::apache::lucene::document::FieldType>
    TestIndexWriterExceptions::DocCopyIterator::custom1 =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_NOT_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    TestIndexWriterExceptions::DocCopyIterator::custom2 =
        make_shared<org::apache::lucene::document::FieldType>();
const shared_ptr<org::apache::lucene::document::FieldType>
    TestIndexWriterExceptions::DocCopyIterator::custom3 =
        make_shared<org::apache::lucene::document::FieldType>();
const shared_ptr<org::apache::lucene::document::FieldType>
    TestIndexWriterExceptions::DocCopyIterator::custom4 =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::StringField::TYPE_NOT_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    TestIndexWriterExceptions::DocCopyIterator::custom5 =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);

TestIndexWriterExceptions::DocCopyIterator::StaticConstructor::
    StaticConstructor()
{

  custom1->setStoreTermVectors(true);
  custom1->setStoreTermVectorPositions(true);
  custom1->setStoreTermVectorOffsets(true);

  custom2->setStored(true);
  custom2->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);

  custom3->setStored(true);

  custom4->setStoreTermVectors(true);
  custom4->setStoreTermVectorPositions(true);
  custom4->setStoreTermVectorOffsets(true);

  custom5->setStoreTermVectors(true);
  custom5->setStoreTermVectorPositions(true);
  custom5->setStoreTermVectorOffsets(true);
}

DocCopyIterator::StaticConstructor
    TestIndexWriterExceptions::DocCopyIterator::staticConstructor;

TestIndexWriterExceptions::DocCopyIterator::DocCopyIterator(
    shared_ptr<Document> doc, int count)
    : doc(doc), count(count)
{
}

shared_ptr<Iterator<std::shared_ptr<Document>>>
TestIndexWriterExceptions::DocCopyIterator::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

TestIndexWriterExceptions::DocCopyIterator::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(shared_ptr<DocCopyIterator> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriterExceptions::DocCopyIterator::IteratorAnonymousInnerClass::
    hasNext()
{
  return upto < outerInstance->count;
}

shared_ptr<Document>
TestIndexWriterExceptions::DocCopyIterator::IteratorAnonymousInnerClass::next()
{
  upto++;
  return outerInstance->doc;
}

void TestIndexWriterExceptions::DocCopyIterator::IteratorAnonymousInnerClass::
    remove()
{
  throw make_shared<UnsupportedOperationException>();
}

TestIndexWriterExceptions::IndexerThread::IndexerThread(
    shared_ptr<TestIndexWriterExceptions> outerInstance, int i,
    shared_ptr<IndexWriter> writer)
    : outerInstance(outerInstance)
{
  setName(L"Indexer " + to_wstring(i));
  this->writer = writer;
}

void TestIndexWriterExceptions::IndexerThread::run()
{

  shared_ptr<Document> *const doc = make_shared<Document>();

  doc->push_back(LuceneTestCase::newTextField(
      r, L"content1", L"aaa bbb ccc ddd", Field::Store::YES));
  doc->push_back(LuceneTestCase::newField(r, L"content6", L"aaa bbb ccc ddd",
                                          DocCopyIterator::custom1));
  doc->push_back(LuceneTestCase::newField(r, L"content2", L"aaa bbb ccc ddd",
                                          DocCopyIterator::custom2));
  doc->push_back(LuceneTestCase::newField(r, L"content3", L"aaa bbb ccc ddd",
                                          DocCopyIterator::custom3));

  doc->push_back(LuceneTestCase::newTextField(
      r, L"content4", L"aaa bbb ccc ddd", Field::Store::NO));
  doc->push_back(LuceneTestCase::newStringField(
      r, L"content5", L"aaa bbb ccc ddd", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"numericdv", 5));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"binarydv", make_shared<BytesRef>(L"hello")));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"sorteddv", make_shared<BytesRef>(L"world")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sortedsetdv", make_shared<BytesRef>(L"hellllo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sortedsetdv", make_shared<BytesRef>(L"again")));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sortednumericdv", 10));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sortednumericdv", 5));

  doc->push_back(LuceneTestCase::newField(r, L"content7", L"aaa bbb ccc ddd",
                                          DocCopyIterator::custom4));

  shared_ptr<Field> *const idField =
      LuceneTestCase::newField(r, L"id", L"", DocCopyIterator::custom2);
  doc->push_back(idField);

  constexpr int64_t stopTime = System::currentTimeMillis() + 500;

  do {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": TEST: IndexerThread: cycle" << endl;
    }
    outerInstance->doFail->set(shared_from_this());
    const wstring id = L"" + r->nextInt(50);
    idField->setStringValue(id);
    shared_ptr<Term> idTerm = make_shared<Term>(L"id", id);
    try {
      if (r->nextBoolean()) {
        writer->updateDocuments(idTerm, make_shared<DocCopyIterator>(
                                            doc, TestUtil::nextInt(r, 1, 20)));
      } else {
        writer->updateDocument(idTerm, doc);
      }
    } catch (const runtime_error &re) {
      if (VERBOSE) {
        wcout << Thread::currentThread().getName() << L": EXC: " << endl;
        re.printStackTrace(System::out);
      }
      try {
        TestUtil::checkIndex(writer->getDirectory());
      } catch (const IOException &ioe) {
        wcout << Thread::currentThread().getName() << L": unexpected exception1"
              << endl;
        ioe->printStackTrace(System::out);
        failure = ioe;
        break;
      }
    } catch (const runtime_error &t) {
      wcout << Thread::currentThread().getName() << L": unexpected exception2"
            << endl;
      t.printStackTrace(System::out);
      failure = t;
      break;
    }

    outerInstance->doFail->set(nullptr);

    // After a possible exception (above) I should be able
    // to add a new document without hitting an
    // exception:
    try {
      writer->updateDocument(idTerm, doc);
    } catch (const runtime_error &t) {
      wcout << Thread::currentThread().getName() << L": unexpected exception3"
            << endl;
      t.printStackTrace(System::out);
      failure = t;
      break;
    }
  } while (System::currentTimeMillis() < stopTime);
}

TestIndexWriterExceptions::TestPoint1::TestPoint1(
    shared_ptr<TestIndexWriterExceptions> outerInstance)
    : outerInstance(outerInstance)
{
}

void TestIndexWriterExceptions::TestPoint1::apply(const wstring &name)
{
  if (outerInstance->doFail->get() != nullptr && name != L"startDoFlush" &&
      r->nextInt(40) == 17) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L": NOW FAIL: " << name
            << endl;
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
      (runtime_error())->printStackTrace(System::out);
    }
    throw runtime_error(Thread::currentThread().getName() +
                        L": intentionally failing at " + name);
  }
}

void TestIndexWriterExceptions::testRandomExceptions() 
{
  if (VERBOSE) {
    wcout << L"\nTEST: start testRandomExceptions" << endl;
  }
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setEnableChecks(false); // disable workflow checking as we
                                    // forcefully close() in exceptional cases.

  shared_ptr<IndexWriter> writer = RandomIndexWriter::mockIndexWriter(
      random(), dir,
      newIndexWriterConfig(analyzer)
          ->setRAMBufferSizeMB(0.1)
          ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()),
      make_shared<TestPoint1>(shared_from_this()));
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer->getConfig()->getMergeScheduler()))
      ->setSuppressExceptions();
  // writer.setMaxBufferedDocs(10);
  if (VERBOSE) {
    wcout << L"TEST: initial commit" << endl;
  }
  writer->commit();

  shared_ptr<IndexerThread> thread =
      make_shared<IndexerThread>(shared_from_this(), 0, writer);
  thread->run();
  if (thread->failure != nullptr) {
    thread->failure.printStackTrace(System::out);
    fail(L"thread " + thread->getName() + L": hit unexpected failure");
  }

  if (VERBOSE) {
    wcout << L"TEST: commit after thread start" << endl;
  }
  writer->commit();

  try {
    delete writer;
  } catch (const runtime_error &t) {
    wcout << L"exception during close:" << endl;
    t.printStackTrace(System::out);
    writer->rollback();
  }

  // Confirm that when doc hits exception partway through tokenization, it's
  // deleted:
  shared_ptr<IndexReader> r2 = DirectoryReader::open(dir);
  constexpr int count = r2->docFreq(make_shared<Term>(L"content4", L"aaa"));
  constexpr int count2 = r2->docFreq(make_shared<Term>(L"content4", L"ddd"));
  TestUtil::assertEquals(count, count2);
  delete r2;

  delete dir;
}

void TestIndexWriterExceptions::testRandomExceptionsThreads() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setEnableChecks(false); // disable workflow checking as we
                                    // forcefully close() in exceptional cases.
  shared_ptr<IndexWriter> writer = RandomIndexWriter::mockIndexWriter(
      random(), dir,
      newIndexWriterConfig(analyzer)
          ->setRAMBufferSizeMB(0.2)
          ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()),
      make_shared<TestPoint1>(shared_from_this()));
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer->getConfig()->getMergeScheduler()))
      ->setSuppressExceptions();
  // writer.setMaxBufferedDocs(10);
  writer->commit();

  constexpr int NUM_THREADS = 4;

  std::deque<std::shared_ptr<IndexerThread>> threads(NUM_THREADS);
  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i] = make_shared<IndexerThread>(shared_from_this(), i, writer);
    threads[i]->start();
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i]->join();
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    if (threads[i]->failure != nullptr) {
      fail(L"thread " + threads[i]->getName() + L": hit unexpected failure");
    }
  }

  writer->commit();

  try {
    delete writer;
  } catch (const runtime_error &t) {
    wcout << L"exception during close:" << endl;
    t.printStackTrace(System::out);
    writer->rollback();
  }

  // Confirm that when doc hits exception partway through tokenization, it's
  // deleted:
  shared_ptr<IndexReader> r2 = DirectoryReader::open(dir);
  constexpr int count = r2->docFreq(make_shared<Term>(L"content4", L"aaa"));
  constexpr int count2 = r2->docFreq(make_shared<Term>(L"content4", L"ddd"));
  TestUtil::assertEquals(count, count2);
  delete r2;

  delete dir;
}

void TestIndexWriterExceptions::TestPoint2::apply(const wstring &name)
{
  if (doFail && name == L"DocumentsWriterPerThread addDocument start") {
    throw runtime_error(L"intentionally failing");
  }
}

wstring TestIndexWriterExceptions::CRASH_FAIL_MESSAGE =
    L"I'm experiencing problems";

TestIndexWriterExceptions::CrashingFilter::CrashingFilter(
    const wstring &fieldName, shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  this->fieldName = fieldName;
}

bool TestIndexWriterExceptions::CrashingFilter::incrementToken() throw(
    IOException)
{
  if (this->fieldName == L"crash" && count++ >= 4) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(CRASH_FAIL_MESSAGE);
  }
  return input->incrementToken();
}

void TestIndexWriterExceptions::CrashingFilter::reset() 
{
  TokenFilter::reset();
  count = 0;
}

void TestIndexWriterExceptions::testExceptionDocumentsWriterInit() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<TestPoint2> testPoint = make_shared<TestPoint2>();
  shared_ptr<IndexWriter> w = RandomIndexWriter::mockIndexWriter(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())),
      testPoint);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a field", Field::Store::YES));
  w->addDocument(doc);

  testPoint->doFail = true;
  expectThrows(runtime_error::typeid, [&]() { w->addDocument(doc); });

  delete w;
  delete dir;
}

void TestIndexWriterExceptions::testExceptionJustBeforeFlush() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<AtomicBoolean> *const doCrash = make_shared<AtomicBoolean>();

  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY, doCrash);

  shared_ptr<IndexWriter> w = RandomIndexWriter::mockIndexWriter(
      random(), dir, newIndexWriterConfig(analyzer)->setMaxBufferedDocs(2),
      make_shared<TestPoint1>(shared_from_this()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a field", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<Document> crashDoc = make_shared<Document>();
  crashDoc->push_back(
      newTextField(L"crash", L"do it on token 4", Field::Store::YES));
  doCrash->set(true);
  expectThrows(IOException::typeid, [&]() { w->addDocument(crashDoc); });

  w->addDocument(doc);
  delete w;
  delete dir;
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY,
        shared_ptr<AtomicBoolean> doCrash)
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
  this->doCrash = doCrash;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setEnableChecks(false); // disable workflow checking as we
                                     // forcefully close() in exceptional cases.
  shared_ptr<TokenStream> stream = tokenizer;
  if (doCrash->get()) {
    stream = make_shared<CrashingFilter>(fieldName, stream);
  }
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestIndexWriterExceptions::TestPoint3::apply(const wstring &name)
{
  if (doFail && name == L"startMergeInit") {
    failed = true;
    throw runtime_error(L"intentionally failing");
  }
}

void TestIndexWriterExceptions::testExceptionOnMergeInit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(2)
          ->setMergePolicy(newLogMergePolicy());
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  cms->setSuppressExceptions();
  conf->setMergeScheduler(cms);
  (std::static_pointer_cast<LogMergePolicy>(conf->getMergePolicy()))
      ->setMergeFactor(2);
  shared_ptr<TestPoint3> testPoint = make_shared<TestPoint3>();
  shared_ptr<IndexWriter> w =
      RandomIndexWriter::mockIndexWriter(random(), dir, conf, testPoint);
  testPoint->doFail = true;
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a field", Field::Store::YES));
  for (int i = 0; i < 10; i++) {
    try {
      w->addDocument(doc);
    } catch (const runtime_error &re) {
      break;
    }
  }

  try {
    (std::static_pointer_cast<ConcurrentMergeScheduler>(
         w->getConfig()->getMergeScheduler()))
        ->sync();
  } catch (const IllegalStateException &ise) {
    // OK: merge exc causes tragedy
  }
  assertTrue(testPoint->failed);
  delete w;
  delete dir;
}

void TestIndexWriterExceptions::testExceptionFromTokenStream() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this()));
  conf->setMaxBufferedDocs(max(3, conf->getMaxBufferedDocs()));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> brokenDoc = make_shared<Document>();
  wstring contents = L"aa bb cc dd ee ff gg hh ii jj kk";
  brokenDoc->push_back(newTextField(L"content", contents, Field::Store::NO));
  expectThrows(runtime_error::typeid,
               [&]() { writer->addDocument(brokenDoc); });

  // Make sure we can add another normal document
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aa bb cc dd", Field::Store::NO));
  writer->addDocument(doc);

  // Make sure we can add another normal document
  doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aa bb cc dd", Field::Store::NO));
  writer->addDocument(doc);

  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<Term> *const t = make_shared<Term>(L"content", L"aa");
  TestUtil::assertEquals(3, reader->docFreq(t));

  // Make sure the doc that hit the exception was marked
  // as deleted:
  shared_ptr<PostingsEnum> tdocs =
      TestUtil::docs(random(), reader, t->field(),
                     make_shared<BytesRef>(t->text()), nullptr, 0);

  shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(reader);
  int count = 0;
  while (tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    if (liveDocs == nullptr || liveDocs->get(tdocs->docID())) {
      count++;
    }
  }
  TestUtil::assertEquals(2, count);

  TestUtil::assertEquals(reader->docFreq(make_shared<Term>(L"content", L"gg")),
                         0);
  delete reader;
  delete dir;
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  tokenizer->setEnableChecks(false); // disable workflow checking as we
                                     // forcefully close() in exceptional cases.
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<TokenFilterAnonymousInnerClass>(shared_from_this(),
                                                             tokenizer));
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    TokenFilterAnonymousInnerClass::TokenFilterAnonymousInnerClass(
        shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
        shared_ptr<MockTokenizer> tokenizer)
    : org::apache::lucene::analysis::TokenFilter(tokenizer)
{
  this->outerInstance = outerInstance;
  count = 0;
}

bool TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    TokenFilterAnonymousInnerClass::incrementToken() 
{
  if (count++ == 5) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException();
  }
  return input::incrementToken();
}

void TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    TokenFilterAnonymousInnerClass::reset() 
{
  outerInstance->outerInstance.super.reset();
  this->count = 0;
}

void TestIndexWriterExceptions::FailOnlyOnFlush::setDoFail()
{
  this->doFail = true;
}

void TestIndexWriterExceptions::FailOnlyOnFlush::clearDoFail()
{
  this->doFail = false;
}

void TestIndexWriterExceptions::FailOnlyOnFlush::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (doFail) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    bool sawFlush = false;
    bool sawFinishDocument = false;
    for (int i = 0; i < trace.size(); i++) {
      if (L"flush" == trace[i]->getMethodName()) {
        sawFlush = true;
      }
      if (L"finishDocument" == trace[i]->getMethodName()) {
        sawFinishDocument = true;
      }
    }

    if (sawFlush && sawFinishDocument == false && count++ >= 30) {
      doFail = false;
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"now failing during flush");
    }
  }
}

void TestIndexWriterExceptions::testDocumentsWriterAbort() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<FailOnlyOnFlush> failure = make_shared<FailOnlyOnFlush>();
  failure->setDoFail();
  dir->failOn(failure);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
  shared_ptr<Document> doc = make_shared<Document>();
  wstring contents = L"aa bb cc dd ee ff gg hh ii jj kk";
  doc->push_back(newTextField(L"content", contents, Field::Store::NO));
  bool hitError = false;
  writer->addDocument(doc);

  expectThrows(IOException::typeid, [&]() { writer->addDocument(doc); });

  // only one flush should fail:
  assertFalse(hitError);
  hitError = true;
  assertTrue(writer->deleter->isClosed());
  assertTrue(writer->isClosed());
  assertFalse(DirectoryReader::indexExists(dir));

  delete dir;
}

void TestIndexWriterExceptions::testDocumentsWriterExceptions() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY);

  for (int i = 0; i < 2; i++) {
    if (VERBOSE) {
      wcout << L"TEST: cycle i=" << i << endl;
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir,
        newIndexWriterConfig(analyzer)->setMergePolicy(newLogMergePolicy()));

    // don't allow a sudden merge to clean up the deleted
    // doc below:
    shared_ptr<LogMergePolicy> lmp = std::static_pointer_cast<LogMergePolicy>(
        writer->getConfig()->getMergePolicy());
    lmp->setMergeFactor(max(lmp->getMergeFactor(), 5));

    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(L"contents", L"here are some contents",
                            DocCopyIterator::custom5));
    writer->addDocument(doc);
    writer->addDocument(doc);
    doc->push_back(newField(L"crash", L"this should crash after 4 terms",
                            DocCopyIterator::custom5));
    doc->push_back(newField(L"other", L"this will not get indexed",
                            DocCopyIterator::custom5));
    try {
      writer->addDocument(doc);
      fail(L"did not hit expected exception");
    } catch (const IOException &ioe) {
      if (VERBOSE) {
        wcout << L"TEST: hit expected exception" << endl;
        ioe->printStackTrace(System::out);
      }
    }

    if (0 == i) {
      doc = make_shared<Document>();
      doc->push_back(newField(L"contents", L"here are some contents",
                              DocCopyIterator::custom5));
      writer->addDocument(doc);
      writer->addDocument(doc);
    }
    delete writer;

    if (VERBOSE) {
      wcout << L"TEST: open reader" << endl;
    }
    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    if (i == 0) {
      int expected = 5;
      TestUtil::assertEquals(
          expected, reader->docFreq(make_shared<Term>(L"contents", L"here")));
      TestUtil::assertEquals(expected, reader->maxDoc());
      int numDel = 0;
      shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(reader);
      assertNotNull(liveDocs);
      for (int j = 0; j < reader->maxDoc(); j++) {
        if (!liveDocs->get(j)) {
          numDel++;
        } else {
          reader->document(j);
          reader->getTermVectors(j);
        }
      }
      TestUtil::assertEquals(1, numDel);
    }
    delete reader;

    writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(analyzer)->setMaxBufferedDocs(10));
    doc = make_shared<Document>();
    doc->push_back(newField(L"contents", L"here are some contents",
                            DocCopyIterator::custom5));
    for (int j = 0; j < 17; j++) {
      writer->addDocument(doc);
    }
    writer->forceMerge(1);
    delete writer;

    reader = DirectoryReader::open(dir);
    int expected = 19 + (1 - i) * 2;
    TestUtil::assertEquals(
        expected, reader->docFreq(make_shared<Term>(L"contents", L"here")));
    TestUtil::assertEquals(expected, reader->maxDoc());
    int numDel = 0;
    assertNull(MultiFields::getLiveDocs(reader));
    for (int j = 0; j < reader->maxDoc(); j++) {
      reader->document(j);
      reader->getTermVectors(j);
    }
    delete reader;
    TestUtil::assertEquals(0, numDel);

    delete dir;
  }
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY)
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY),
      outerInstance(outerInstance)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setEnableChecks(false); // disable workflow checking as we
                                     // forcefully close() in exceptional cases.
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CrashingFilter>(fieldName, tokenizer));
}

void TestIndexWriterExceptions::testDocumentsWriterExceptionFailOneDoc() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY);
  for (int i = 0; i < 10; i++) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
    // newDirectory(); final IndexWriter writer = new IndexWriter(dir,
    // newIndexWriterConfig(analyzer).setMaxBufferedDocs(-1).setRAMBufferSizeMB(random().nextBoolean()
    // ? 0.00001 : std::numeric_limits<int>::max()).setMergePolicy(new
    // FilterMergePolicy(NoMergePolicy.INSTANCE)
    {
      org::apache::lucene::store::Directory dir = newDirectory();
      shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(analyzer)
                   ->setMaxBufferedDocs(-1)
                   ->setRAMBufferSizeMB(random()->nextBoolean()
                                            ? 0.00001
                                            : numeric_limits<int>::max())
                   ->setMergePolicy(
                       make_shared<FilterMergePolicyAnonymousInnerClass>(
                           shared_from_this(), NoMergePolicy::INSTANCE)));
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(newField(L"contents", L"here are some contents",
                              DocCopyIterator::custom5));
      writer->addDocument(doc);
      doc->push_back(newField(L"crash", L"this should crash after 4 terms",
                              DocCopyIterator::custom5));
      doc->push_back(newField(L"other", L"this will not get indexed",
                              DocCopyIterator::custom5));
      expectThrows(IOException::typeid, [&]() { writer->addDocument(doc); });
      writer->commit();
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (IndexReader reader =
      // DirectoryReader.open(dir))
      {
        IndexReader reader = DirectoryReader::open(dir);
        TestUtil::assertEquals(
            2, reader->docFreq(make_shared<Term>(L"contents", L"here")));
        TestUtil::assertEquals(2, reader->maxDoc());
        TestUtil::assertEquals(1, reader->numDocs());
      }
    }
  }
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY)
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setEnableChecks(false); // disable workflow checking as we
                                     // forcefully close() in exceptional cases.
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CrashingFilter>(fieldName, tokenizer));
}

TestIndexWriterExceptions::FilterMergePolicyAnonymousInnerClass::
    FilterMergePolicyAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<org::apache::lucene::index::MergePolicy> INSTANCE)
    : FilterMergePolicy(INSTANCE)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriterExceptions::FilterMergePolicyAnonymousInnerClass::
    keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>> readerIOSupplier)
{
  return true;
}

void TestIndexWriterExceptions::testDocumentsWriterExceptionThreads() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass3>(
      shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY);

  constexpr int NUM_THREAD = 3;
  constexpr int NUM_ITER = 100;

  for (int i = 0; i < 2; i++) {
    shared_ptr<Directory> dir = newDirectory();
    {
      shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(analyzer)
                   ->setMaxBufferedDocs(numeric_limits<int>::max())
                   ->setRAMBufferSizeMB(-1)
                   ->setMergePolicy(
                       make_shared<FilterMergePolicyAnonymousInnerClass2>(
                           shared_from_this(), NoMergePolicy::INSTANCE)));

      constexpr int finalI = i;

      std::deque<std::shared_ptr<Thread>> threads(NUM_THREAD);
      for (int t = 0; t < NUM_THREAD; t++) {
        threads[t] = make_shared<ThreadAnonymousInnerClass>(
            shared_from_this(), NUM_ITER, writer, finalI, t);
        threads[t]->start();
      }

      for (int t = 0; t < NUM_THREAD; t++) {
        threads[t]->join();
      }

      delete writer;
    }

    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    int expected = (3 + (1 - i) * 2) * NUM_THREAD * NUM_ITER;
    assertEquals(L"i=" + to_wstring(i), expected,
                 reader->docFreq(make_shared<Term>(L"contents", L"here")));
    TestUtil::assertEquals(expected, reader->maxDoc());
    int numDel = 0;
    shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(reader);
    assertNotNull(liveDocs);
    for (int j = 0; j < reader->maxDoc(); j++) {
      if (!liveDocs->get(j)) {
        numDel++;
      } else {
        reader->document(j);
        reader->getTermVectors(j);
      }
    }
    delete reader;

    TestUtil::assertEquals(NUM_THREAD * NUM_ITER, numDel);

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(analyzer)->setMaxBufferedDocs(10));
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(L"contents", L"here are some contents",
                            DocCopyIterator::custom5));
    for (int j = 0; j < 17; j++) {
      writer->addDocument(doc);
    }
    writer->forceMerge(1);
    delete writer;

    reader = DirectoryReader::open(dir);
    expected += 17 - NUM_THREAD * NUM_ITER;
    TestUtil::assertEquals(
        expected, reader->docFreq(make_shared<Term>(L"contents", L"here")));
    TestUtil::assertEquals(expected, reader->maxDoc());
    assertNull(MultiFields::getLiveDocs(reader));
    for (int j = 0; j < reader->maxDoc(); j++) {
      reader->document(j);
      reader->getTermVectors(j);
    }
    delete reader;

    delete dir;
  }
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY)
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setEnableChecks(false); // disable workflow checking as we
                                     // forcefully close() in exceptional cases.
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CrashingFilter>(fieldName, tokenizer));
}

TestIndexWriterExceptions::FilterMergePolicyAnonymousInnerClass2::
    FilterMergePolicyAnonymousInnerClass2(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<org::apache::lucene::index::MergePolicy> INSTANCE)
    : FilterMergePolicy(INSTANCE)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriterExceptions::FilterMergePolicyAnonymousInnerClass2::
    keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>> readerIOSupplier)
{
  return true;
}

TestIndexWriterExceptions::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexWriterExceptions> outerInstance, int NUM_ITER,
    shared_ptr<org::apache::lucene::index::IndexWriter> writer, int finalI,
    int t)
{
  this->outerInstance = outerInstance;
  this->NUM_ITER = NUM_ITER;
  this->writer = writer;
  this->finalI = finalI;
  this->t = t;
}

void TestIndexWriterExceptions::ThreadAnonymousInnerClass::run()
{
  try {
    for (int iter = 0; iter < NUM_ITER; iter++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(LuceneTestCase::newField(
          L"contents", L"here are some contents", DocCopyIterator::custom5));
      writer->addDocument(doc);
      writer->addDocument(doc);
      doc->push_back(
          LuceneTestCase::newField(L"crash", L"this should crash after 4 terms",
                                   DocCopyIterator::custom5));
      doc->push_back(LuceneTestCase::newField(
          L"other", L"this will not get indexed", DocCopyIterator::custom5));
      LuceneTestCase::expectThrows(IOException::typeid,
                                   [&]() { writer->addDocument(doc); });

      if (0 == finalI) {
        shared_ptr<Document> extraDoc = make_shared<Document>();
        extraDoc->push_back(LuceneTestCase::newField(
            L"contents", L"here are some contents", DocCopyIterator::custom5));
        writer->addDocument(extraDoc);
        writer->addDocument(extraDoc);
      }
    }
  } catch (const runtime_error &t) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      wcout << Thread::currentThread().getName()
            << L": ERROR: hit unexpected exception" << endl;
      t.printStackTrace(System::out);
    }
    fail();
  }
}

void TestIndexWriterExceptions::FailOnlyInSync::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (doFail) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    for (int i = 0; i < trace.size(); i++) {
      if (doFail &&
          MockDirectoryWrapper::typeid->getName().equals(
              trace[i]->getClassName()) &&
          L"sync" == trace[i]->getMethodName()) {
        didFail = true;
        if (VERBOSE) {
          wcout << L"TEST: now throw exc:" << endl;
          // C++ TODO: This exception's constructor requires an argument:
          // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
          (runtime_error())->printStackTrace(System::out);
        }
        // C++ TODO: The following line could not be converted:
        throw java.io.IOException(L"now failing on purpose during sync");
      }
    }
  }
}

void TestIndexWriterExceptions::addDoc(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}

void TestIndexWriterExceptions::testExceptionDuringSync() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<FailOnlyInSync> failure = make_shared<FailOnlyInSync>();
  dir->failOn(failure);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
               ->setMergePolicy(newLogMergePolicy(5)));
  failure->setDoFail();

  for (int i = 0; i < 23; i++) {
    addDoc(writer);
    if ((i - 1) % 2 == 0) {
      try {
        writer->commit();
      } catch (const IOException &ioe) {
        // expected
      }
    }
  }
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer->getConfig()->getMergeScheduler()))
      ->sync();
  assertTrue(failure->didFail);
  failure->clearDoFail();
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(23, reader->numDocs());
  delete reader;
  delete dir;
}

const wstring TestIndexWriterExceptions::FailOnlyInCommit::PREPARE_STAGE =
    L"prepareCommit";
const wstring TestIndexWriterExceptions::FailOnlyInCommit::FINISH_STAGE =
    L"finishCommit";

TestIndexWriterExceptions::FailOnlyInCommit::FailOnlyInCommit(
    bool dontFailDuringGlobalFieldMap, bool dontFailDuringSyncMetadata,
    const wstring &stage)
    : dontFailDuringGlobalFieldMap(dontFailDuringGlobalFieldMap),
      dontFailDuringSyncMetadata(dontFailDuringSyncMetadata), stage(stage)
{
}

void TestIndexWriterExceptions::FailOnlyInCommit::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: StackTraceElement[] trace = new Exception().getStackTrace();
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      (runtime_error())->getStackTrace();
  bool isCommit = false;
  bool isDelete = false;
  bool isSyncMetadata = false;
  bool isInGlobalFieldMap = false;
  for (int i = 0; i < trace.size(); i++) {
    if (isCommit && isDelete && isInGlobalFieldMap && isSyncMetadata) {
      break;
    }
    if (SegmentInfos::typeid->getName().equals(trace[i]->getClassName()) &&
        stage == trace[i]->getMethodName()) {
      isCommit = true;
    }
    if (MockDirectoryWrapper::typeid->getName().equals(
            trace[i]->getClassName()) &&
        L"deleteFile" == trace[i]->getMethodName()) {
      isDelete = true;
    }
    if (SegmentInfos::typeid->getName().equals(trace[i]->getClassName()) &&
        L"writeGlobalFieldMap" == trace[i]->getMethodName()) {
      isInGlobalFieldMap = true;
    }
    if (MockDirectoryWrapper::typeid->getName().equals(
            trace[i]->getClassName()) &&
        L"syncMetaData" == trace[i]->getMethodName()) {
      isSyncMetadata = true;
    }
  }
  if (isInGlobalFieldMap && dontFailDuringGlobalFieldMap) {
    isCommit = false;
  }
  if (isSyncMetadata && dontFailDuringSyncMetadata) {
    isCommit = false;
  }
  if (isCommit) {
    if (!isDelete) {
      failOnCommit = true;
      failOnSyncMetadata = isSyncMetadata;
      throw runtime_error(L"now fail first");
    } else {
      failOnDeleteFile = true;
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"now fail during delete");
    }
  }
}

void TestIndexWriterExceptions::testExceptionsDuringCommit() throw(
    runtime_error)
{
  std::deque<std::shared_ptr<FailOnlyInCommit>> failures = {
      make_shared<FailOnlyInCommit>(false, true,
                                    FailOnlyInCommit::PREPARE_STAGE),
      make_shared<FailOnlyInCommit>(true, false,
                                    FailOnlyInCommit::PREPARE_STAGE),
      make_shared<FailOnlyInCommit>(true, true,
                                    FailOnlyInCommit::PREPARE_STAGE),
      make_shared<FailOnlyInCommit>(false, true,
                                    FailOnlyInCommit::FINISH_STAGE)};

  for (auto failure : failures) {
    shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
    dir->setFailOnCreateOutput(false);
    int fileCount = dir->listAll().size();
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"field", L"a field", Field::Store::YES));
    w->addDocument(doc);
    dir->failOn(failure);
    expectThrows(runtime_error::typeid, [&]() { delete w; });
    assertTrue(L"failOnCommit=" +
                   StringHelper::toString(failure->failOnCommit) +
                   L" failOnDeleteFile=" +
                   StringHelper::toString(failure->failOnDeleteFile) +
                   L" failOnSyncMetadata=" +
                   StringHelper::toString(failure->failOnSyncMetadata) + L"",
               failure->failOnCommit &&
                   (failure->failOnDeleteFile || failure->failOnSyncMetadata));
    w->rollback();
    std::deque<wstring> files = dir->listAll();
    assertTrue(files.size() == fileCount ||
               (files.size() == fileCount + 1 &&
                Arrays::asList(files)->contains(IndexWriter::WRITE_LOCK_NAME)));
    delete dir;
  }
}

void TestIndexWriterExceptions::testForceMergeExceptions() 
{
  shared_ptr<Directory> startDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(2)
          ->setMergePolicy(newLogMergePolicy());
  (std::static_pointer_cast<LogMergePolicy>(conf->getMergePolicy()))
      ->setMergeFactor(100);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(startDir, conf);
  for (int i = 0; i < 27; i++) {
    addDoc(w);
  }
  delete w;

  int iter = TEST_NIGHTLY ? 200 : 10;
  for (int i = 0; i < iter; i++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter " << i << endl;
    }
    shared_ptr<MockDirectoryWrapper> dir = make_shared<MockDirectoryWrapper>(
        random(), TestUtil::ramCopyOf(startDir));
    conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>());
    (std::static_pointer_cast<ConcurrentMergeScheduler>(
         conf->getMergeScheduler()))
        ->setSuppressExceptions();
    w = make_shared<IndexWriter>(dir, conf);
    dir->setRandomIOExceptionRate(0.5);
    try {
      w->forceMerge(1);
    } catch (const IllegalStateException &ise) {
      // expected
    } catch (const IOException &ioe) {
      if (ioe->getCause() == nullptr) {
        fail(L"forceMerge threw IOException without root cause");
      }
    }
    dir->setRandomIOExceptionRate(0);
    // System.out.println("TEST: now close IW");
    try {
      delete w;
    } catch (const IllegalStateException &ise) {
      // ok
    }
    delete dir;
  }
  delete startDir;
}

void TestIndexWriterExceptions::testOutOfMemoryErrorCausesCloseToFail() throw(
    runtime_error)
{

  shared_ptr<AtomicBoolean> *const thrown = make_shared<AtomicBoolean>(false);
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setInfoStream(make_shared<InfoStreamAnonymousInnerClass>(
                   shared_from_this(), thrown)));

  expectThrows(OutOfMemoryError::typeid, [&]() { writer->close(); });

  // throws IllegalStateEx w/o bug fix
  delete writer;
  delete dir;
}

TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::
    InfoStreamAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<AtomicBoolean> thrown)
{
  this->outerInstance = outerInstance;
  this->thrown = thrown;
}

void TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::message(
    const wstring &component, const wstring &message)
{
  if (StringHelper::startsWith(message, L"now flush at close") &&
      thrown->compareAndSet(false, true)) {
    throw make_shared<OutOfMemoryError>(L"fake OOME at " + message);
  }
}

bool TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::isEnabled(
    const wstring &component)
{
  return true;
}

TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::
    ~InfoStreamAnonymousInnerClass()
{
}

void TestIndexWriterExceptions::testOutOfMemoryErrorRollback() throw(
    runtime_error)
{

  shared_ptr<AtomicBoolean> *const thrown = make_shared<AtomicBoolean>(false);
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setInfoStream(make_shared<InfoStreamAnonymousInnerClass2>(
                   shared_from_this(), thrown)));
  writer->addDocument(make_shared<Document>());

  expectThrows(OutOfMemoryError::typeid, [&]() { writer->commit(); });

  try {
    delete writer;
  } catch (const invalid_argument &ok) {
    // ok
  }

  expectThrows(AlreadyClosedException::typeid,
               [&]() { writer->addDocument(make_shared<Document>()); });

  // IW should have done rollback() during close, since it hit OOME, and so no
  // index should exist:
  assertFalse(DirectoryReader::indexExists(dir));

  delete dir;
}

TestIndexWriterExceptions::InfoStreamAnonymousInnerClass2::
    InfoStreamAnonymousInnerClass2(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<AtomicBoolean> thrown)
{
  this->outerInstance = outerInstance;
  this->thrown = thrown;
}

void TestIndexWriterExceptions::InfoStreamAnonymousInnerClass2::message(
    const wstring &component, const wstring &message)
{
  if (message.find(L"startFullFlush") != wstring::npos &&
      thrown->compareAndSet(false, true)) {
    throw make_shared<OutOfMemoryError>(L"fake OOME at " + message);
  }
}

bool TestIndexWriterExceptions::InfoStreamAnonymousInnerClass2::isEnabled(
    const wstring &component)
{
  return true;
}

TestIndexWriterExceptions::InfoStreamAnonymousInnerClass2::
    ~InfoStreamAnonymousInnerClass2()
{
}

void TestIndexWriterExceptions::TestPoint4::apply(const wstring &name)
{
  if (doFail && name == L"rollback before checkpoint") {
    throw runtime_error(L"intentionally failing");
  }
}

void TestIndexWriterExceptions::testRollbackExceptionHang() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<TestPoint4> testPoint = make_shared<TestPoint4>();
  shared_ptr<IndexWriter> w = RandomIndexWriter::mockIndexWriter(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())),
      testPoint);

  addDoc(w);
  testPoint->doFail = true;
  expectThrows(runtime_error::typeid, [&]() { w->rollback(); });

  testPoint->doFail = false;
  w->rollback();
  delete dir;
}

void TestIndexWriterExceptions::testSegmentsChecksumError() 
{
  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false); // we corrupt the index

  shared_ptr<IndexWriter> writer = nullptr;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // add 100 documents
  for (int i = 0; i < 100; i++) {
    addDoc(writer);
  }

  // close
  delete writer;

  int64_t gen = SegmentInfos::getLastCommitGeneration(dir);
  assertTrue(L"segment generation should be > 0 but got " + to_wstring(gen),
             gen > 0);

  const wstring segmentsFileName =
      SegmentInfos::getLastCommitSegmentsFileName(dir);
  shared_ptr<IndexInput> in_ =
      dir->openInput(segmentsFileName, newIOContext(random()));
  shared_ptr<IndexOutput> out =
      dir->createOutput(IndexFileNames::fileNameFromGeneration(
                            IndexFileNames::SEGMENTS, L"", 1 + gen),
                        newIOContext(random()));
  out->copyBytes(in_, in_->length() - 1);
  char b = in_->readByte();
  out->writeByte(static_cast<char>(1 + b));
  delete out;
  delete in_;

  expectThrows(CorruptIndexException::typeid,
               [&]() { DirectoryReader::open(dir); });

  delete dir;
}

void TestIndexWriterExceptions::testSimulatedCorruptIndex1() 
{
  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false); // we are corrupting it!

  shared_ptr<IndexWriter> writer = nullptr;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // add 100 documents
  for (int i = 0; i < 100; i++) {
    addDoc(writer);
  }

  // close
  delete writer;

  int64_t gen = SegmentInfos::getLastCommitGeneration(dir);
  assertTrue(L"segment generation should be > 0 but got " + to_wstring(gen),
             gen > 0);

  wstring fileNameIn = SegmentInfos::getLastCommitSegmentsFileName(dir);
  wstring fileNameOut = IndexFileNames::fileNameFromGeneration(
      IndexFileNames::SEGMENTS, L"", 1 + gen);
  shared_ptr<IndexInput> in_ =
      dir->openInput(fileNameIn, newIOContext(random()));
  shared_ptr<IndexOutput> out =
      dir->createOutput(fileNameOut, newIOContext(random()));
  int64_t length = in_->length();
  for (int i = 0; i < length - 1; i++) {
    out->writeByte(in_->readByte());
  }
  delete in_;
  delete out;
  dir->deleteFile(fileNameIn);

  expectThrows(runtime_error::typeid, [&]() { DirectoryReader::open(dir); });

  delete dir;
}

void TestIndexWriterExceptions::testSimulatedCorruptIndex2() 
{
  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false); // we are corrupting it!
  shared_ptr<IndexWriter> writer = nullptr;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy(true))
               ->setUseCompoundFile(true));
  shared_ptr<MergePolicy> lmp = writer->getConfig()->getMergePolicy();
  // Force creation of CFS:
  lmp->setNoCFSRatio(1.0);
  lmp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());

  // add 100 documents
  for (int i = 0; i < 100; i++) {
    addDoc(writer);
  }

  // close
  delete writer;

  int64_t gen = SegmentInfos::getLastCommitGeneration(dir);
  assertTrue(L"segment generation should be > 0 but got " + to_wstring(gen),
             gen > 0);

  bool corrupted = false;
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  for (auto si : sis) {
    assertTrue(si->info->getUseCompoundFile());
    deque<wstring> victims = deque<wstring>(si->info->files());
    Collections::shuffle(victims, random());
    dir->deleteFile(victims[0]);
    corrupted = true;
    break;
  }

  assertTrue(L"failed to find cfs file to remove: ", corrupted);

  expectThrows(runtime_error::typeid, [&]() { DirectoryReader::open(dir); });

  delete dir;
}

void TestIndexWriterExceptions::testTermVectorExceptions() 
{
  std::deque<std::shared_ptr<FailOnTermVectors>> failures = {
      make_shared<FailOnTermVectors>(FailOnTermVectors::AFTER_INIT_STAGE),
      make_shared<FailOnTermVectors>(FailOnTermVectors::INIT_STAGE)};
  int num = atLeast(1);
  for (int j = 0; j < num; j++) {
    for (auto failure : failures) {
      shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
      dir->failOn(failure);
      int numDocs = 10 + random()->nextInt(30);
      for (int i = 0; i < numDocs; i++) {
        shared_ptr<Document> doc = make_shared<Document>();
        // random TV
        shared_ptr<Field> field =
            newTextField(random(), L"field", L"a field", Field::Store::YES);
        doc->push_back(field);
        try {
          w->addDocument(doc);
          assertFalse(field->fieldType()->storeTermVectors());
        } catch (const runtime_error &e) {
          assertTrue(e.what()->startsWith(FailOnTermVectors::EXC_MSG));
          // This is an aborting exception, so writer is closed:
          assertTrue(w->deleter->isClosed());
          assertTrue(w->isClosed());
          delete dir;
          goto itersContinue;
        }
        if (random()->nextInt(20) == 0) {
          w->commit();
          TestUtil::checkIndex(dir);
        }
      }
      shared_ptr<Document> document = make_shared<Document>();
      document->push_back(
          make_shared<TextField>(L"field", L"a field", Field::Store::YES));
      w->addDocument(document);

      for (int i = 0; i < numDocs; i++) {
        shared_ptr<Document> doc = make_shared<Document>();
        shared_ptr<Field> field =
            newTextField(random(), L"field", L"a field", Field::Store::YES);
        doc->push_back(field);
        // random TV
        try {
          w->addDocument(doc);
          assertFalse(field->fieldType()->storeTermVectors());
        } catch (const runtime_error &e) {
          assertTrue(e.what()->startsWith(FailOnTermVectors::EXC_MSG));
        }
        if (random()->nextInt(20) == 0) {
          w->commit();
          TestUtil::checkIndex(dir);
        }
      }
      document = make_shared<Document>();
      document->push_back(
          make_shared<TextField>(L"field", L"a field", Field::Store::YES));
      w->addDocument(document);
      delete w;
      shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
      assertTrue(reader->numDocs() > 0);
      shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
      for (auto context : reader->leaves()) {
        assertFalse(context->reader()->getFieldInfos()->hasVectors());
      }
      delete reader;
      delete dir;
    }
  itersContinue:;
  }
itersBreak:;
}

const wstring TestIndexWriterExceptions::FailOnTermVectors::INIT_STAGE =
    L"initTermVectorsWriter";
const wstring TestIndexWriterExceptions::FailOnTermVectors::AFTER_INIT_STAGE =
    L"finishDocument";
const wstring TestIndexWriterExceptions::FailOnTermVectors::EXC_MSG = L"FOTV";

TestIndexWriterExceptions::FailOnTermVectors::FailOnTermVectors(
    const wstring &stage)
    : stage(stage)
{
}

void TestIndexWriterExceptions::FailOnTermVectors::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{

  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: StackTraceElement[] trace = new Exception().getStackTrace();
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      (runtime_error())->getStackTrace();
  bool fail = false;
  for (int i = 0; i < trace.size(); i++) {
    if (TermVectorsConsumer::typeid->getName().equals(
            trace[i]->getClassName()) &&
        stage == trace[i]->getMethodName()) {
      fail = true;
      break;
    }
  }

  if (fail) {
    throw runtime_error(EXC_MSG);
  }
}

void TestIndexWriterExceptions::testAddDocsNonAbortingException() throw(
    runtime_error)
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs1 = random()->nextInt(25);
  for (int docCount = 0; docCount < numDocs1; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"good content", Field::Store::NO));
    w->addDocument(doc);
  }

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();
  for (int docCount = 0; docCount < 7; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    docs.push_back(doc);
    doc->push_back(
        newStringField(L"id", to_wstring(docCount) + L"", Field::Store::NO));
    doc->push_back(newTextField(L"content",
                                L"silly content " + to_wstring(docCount),
                                Field::Store::NO));
    if (docCount == 4) {
      shared_ptr<Field> f = newTextField(L"crash", L"", Field::Store::NO);
      doc->push_back(f);
      shared_ptr<MockTokenizer> tokenizer =
          make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
      tokenizer->setReader(
          make_shared<StringReader>(L"crash me on the 4th token"));
      tokenizer->setEnableChecks(
          false); // disable workflow checking as we forcefully close() in
                  // exceptional cases.
      f->setTokenStream(make_shared<CrashingFilter>(L"crash", tokenizer));
    }
  }

  shared_ptr<IOException> expected =
      expectThrows(IOException::typeid, [&]() { w->addDocuments(docs); });
  TestUtil::assertEquals(CRASH_FAIL_MESSAGE, expected->getMessage());

  constexpr int numDocs2 = random()->nextInt(25);
  for (int docCount = 0; docCount < numDocs2; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"good content", Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> *const s = newSearcher(r);
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"content", L"silly", L"good");
  TestUtil::assertEquals(0, s->search(pq, 1)->totalHits);

  pq = make_shared<PhraseQuery>(L"content", L"good", L"content");
  TestUtil::assertEquals(numDocs1 + numDocs2, s->search(pq, 1)->totalHits);
  delete r;
  delete dir;
}

void TestIndexWriterExceptions::testUpdateDocsNonAbortingException() throw(
    runtime_error)
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs1 = random()->nextInt(25);
  for (int docCount = 0; docCount < numDocs1; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"good content", Field::Store::NO));
    w->addDocument(doc);
  }

  // Use addDocs (no exception) to get docs in the index:
  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();
  constexpr int numDocs2 = random()->nextInt(25);
  for (int docCount = 0; docCount < numDocs2; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    docs.push_back(doc);
    doc->push_back(newStringField(L"subid", L"subs", Field::Store::NO));
    doc->push_back(
        newStringField(L"id", to_wstring(docCount) + L"", Field::Store::NO));
    doc->push_back(newTextField(L"content",
                                L"silly content " + to_wstring(docCount),
                                Field::Store::NO));
  }
  w->addDocuments(docs);

  constexpr int numDocs3 = random()->nextInt(25);
  for (int docCount = 0; docCount < numDocs3; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"good content", Field::Store::NO));
    w->addDocument(doc);
  }

  docs.clear();
  constexpr int limit = TestUtil::nextInt(random(), 2, 25);
  constexpr int crashAt = random()->nextInt(limit);
  for (int docCount = 0; docCount < limit; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    docs.push_back(doc);
    doc->push_back(
        newStringField(L"id", to_wstring(docCount) + L"", Field::Store::NO));
    doc->push_back(newTextField(L"content",
                                L"silly content " + to_wstring(docCount),
                                Field::Store::NO));
    if (docCount == crashAt) {
      shared_ptr<Field> f = newTextField(L"crash", L"", Field::Store::NO);
      doc->push_back(f);
      shared_ptr<MockTokenizer> tokenizer =
          make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
      tokenizer->setReader(
          make_shared<StringReader>(L"crash me on the 4th token"));
      tokenizer->setEnableChecks(
          false); // disable workflow checking as we forcefully close() in
                  // exceptional cases.
      f->setTokenStream(make_shared<CrashingFilter>(L"crash", tokenizer));
    }
  }

  shared_ptr<IOException> expected = expectThrows(IOException::typeid, [&]() {
    w->updateDocuments(make_shared<Term>(L"subid", L"subs"), docs);
  });
  TestUtil::assertEquals(CRASH_FAIL_MESSAGE, expected->getMessage());

  constexpr int numDocs4 = random()->nextInt(25);
  for (int docCount = 0; docCount < numDocs4; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"good content", Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> *const s = newSearcher(r);
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"content", L"silly", L"content");
  TestUtil::assertEquals(numDocs2, s->search(pq, 1)->totalHits);

  pq = make_shared<PhraseQuery>(L"content", L"good", L"content");
  TestUtil::assertEquals(numDocs1 + numDocs3 + numDocs4,
                         s->search(pq, 1)->totalHits);
  delete r;
  delete dir;
}

void TestIndexWriterExceptions::testNullStoredField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);
  expectThrows(invalid_argument::typeid, [&]() {
    wstring value = nullptr;
    doc->add(make_shared<StoredField>(L"foo", value));
    iw->addDocument(doc);
  });

  assertNull(iw->getTragicException());
  delete iw;
  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

void TestIndexWriterExceptions::testNullStoredFieldReuse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> theField =
      make_shared<StoredField>(L"foo", L"hello", StoredField::TYPE);
  doc->push_back(theField);
  iw->addDocument(doc);
  expectThrows(invalid_argument::typeid, [&]() {
    theField->setStringValue(L"");
    iw->addDocument(doc);
  });

  assertNull(iw->getTragicException());
  delete iw;
  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

void TestIndexWriterExceptions::testNullStoredBytesField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);

  expectThrows(NullPointerException::typeid, [&]() {
    std::deque<char> v = nullptr;
    shared_ptr<Field> theField = make_shared<StoredField>(L"foo", v);
    doc->add(theField);
    iw->addDocument(doc);
  });

  assertNull(iw->getTragicException());
  delete iw;
  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

void TestIndexWriterExceptions::testNullStoredBytesFieldReuse() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> theField = make_shared<StoredField>(
      L"foo", (make_shared<BytesRef>(L"hello"))->bytes);
  doc->push_back(theField);
  iw->addDocument(doc);
  expectThrows(NullPointerException::typeid, [&]() {
    std::deque<char> v = nullptr;
    theField->setBytesValue(v);
    iw->addDocument(doc);
  });

  assertNull(iw->getTragicException());
  delete iw;
  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

void TestIndexWriterExceptions::testNullStoredBytesRefField() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);

  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<BytesRef> v = nullptr;
    shared_ptr<Field> theField = make_shared<StoredField>(L"foo", v);
    doc->add(theField);
    iw->addDocument(doc);
    fail(L"didn't get expected exception");
  });

  assertNull(iw->getTragicException());
  delete iw;
  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

void TestIndexWriterExceptions::testNullStoredBytesRefFieldReuse() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> theField =
      make_shared<StoredField>(L"foo", make_shared<BytesRef>(L"hello"));
  doc->push_back(theField);
  iw->addDocument(doc);
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<BytesRef> v = nullptr;
    theField->setBytesValue(v);
    iw->addDocument(doc);
    fail(L"didn't get expected exception");
  });

  assertNull(iw->getTragicException());
  delete iw;
  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

void TestIndexWriterExceptions::testCrazyPositionIncrementGap() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);
  expectThrows(invalid_argument::typeid, [&]() {
    doc->add(newTextField(L"foo", L"bar", Field::Store::NO));
    doc->add(newTextField(L"foo", L"bar", Field::Store::NO));
    iw->addDocument(doc);
  });

  assertNull(iw->getTragicException());
  delete iw;

  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false));
}

int TestIndexWriterExceptions::AnalyzerAnonymousInnerClass::
    getPositionIncrementGap(const wstring &fieldName)
{
  return -2;
}

shared_ptr<IndexInput> TestIndexWriterExceptions::UOEDirectory::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  if (doFail && StringHelper::startsWith(name, L"segments_")) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    for (int i = 0; i < trace.size(); i++) {
      if (L"readCommit" == trace[i]->getMethodName() ||
          L"readLatestCommit" == trace[i]->getMethodName()) {
        throw make_shared<UnsupportedOperationException>(L"expected UOE");
      }
    }
  }
  return RAMDirectory::openInput(name, context);
}

void TestIndexWriterExceptions::testExceptionOnCtor() 
{
  shared_ptr<UOEDirectory> uoe = make_shared<UOEDirectory>();
  shared_ptr<Directory> d = make_shared<MockDirectoryWrapper>(random(), uoe);
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(d, newIndexWriterConfig(nullptr));
  iw->addDocument(make_shared<Document>());
  delete iw;
  uoe->doFail = true;
  expectThrows(UnsupportedOperationException::typeid, [&]() {
    make_shared<IndexWriter>(d, newIndexWriterConfig(nullptr));
  });

  uoe->doFail = false;
  delete d;
}

void TestIndexWriterExceptions::testTooManyFileException() 
{

  // Create failure that throws Too many open files exception randomly
  shared_ptr<MockDirectoryWrapper::Failure> failure =
      make_shared<FailureAnonymousInnerClass>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  // The exception is only thrown on open input
  dir->setFailOnOpenInput(true);
  dir->failOn(failure);

  // Create an index with one document
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
  iw->addDocument(doc); // add a document
  iw->commit();
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, ir->numDocs());
  ir->close();
  delete iw;

  // Open and close the index a few times
  for (int i = 0; i < 10; i++) {
    failure->setDoFail();
    iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
    try {
      iw = make_shared<IndexWriter>(dir, iwc);
    } catch (const AssertionError &ex) {
      // This is fine: we tripped IW's assert that all files it's about to fsync
      // do exist:
      assertTrue(
          ex->getMessage().matches(L"file .* does not exist; files=\\[.*\\]"));
    } catch (const CorruptIndexException &ex) {
      // Exceptions are fine - we are running out of file handlers here
      continue;
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (FileNotFoundException | NoSuchFileException ex) {
      continue;
    }
    failure->clearDoFail();
    delete iw;
    ir = DirectoryReader::open(dir);
    assertEquals(L"lost document after iteration: " + to_wstring(i), 1,
                 ir->numDocs());
    ir->close();
  }

  // Check if document is still there
  failure->clearDoFail();
  ir = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, ir->numDocs());
  ir->close();

  delete dir;
}

TestIndexWriterExceptions::FailureAnonymousInnerClass::
    FailureAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<MockDirectoryWrapper::Failure>
TestIndexWriterExceptions::FailureAnonymousInnerClass::reset()
{
  outerInstance->doFail = false;
  return shared_from_this();
}

void TestIndexWriterExceptions::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (outerInstance->doFail) {
    if (LuceneTestCase::random()->nextBoolean()) {
      // C++ TODO: The following line could not be converted:
      throw java.io.FileNotFoundException(
          L"some/file/name.ext (Too many open files)");
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testTooManyTokens() throws Exception
void TestIndexWriterExceptions::testTooManyTokens() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  doc->push_back(make_shared<Field>(
      L"foo", make_shared<TokenStreamAnonymousInnerClass>(shared_from_this()),
      ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { iw->addDocument(doc); });
  assertTrue(expected.what()->contains(L"too many tokens"));

  delete iw;
  delete dir;
}

TestIndexWriterExceptions::TokenStreamAnonymousInnerClass::
    TokenStreamAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance)
{
  this->outerInstance = outerInstance;
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncAtt = addAttribute(PositionIncrementAttribute::typeid);
  num = 0;
}

bool TestIndexWriterExceptions::TokenStreamAnonymousInnerClass::
    incrementToken() 
{
  if (num == numeric_limits<int>::max() + 1) {
    return false;
  }
  clearAttributes();
  if (num == 0) {
    posIncAtt::setPositionIncrement(1);
  } else {
    posIncAtt::setPositionIncrement(0);
  }
  termAtt->append(L"a");
  num++;
  if (VERBOSE && num % 1000000 == 0) {
    wcout << L"indexed: " << num << endl;
  }
  return true;
}

void TestIndexWriterExceptions::testExceptionDuringRollback() throw(
    runtime_error)
{
  // currently: fail in two different places
  const wstring messageToFailOn = random()->nextBoolean()
                                      ? L"rollback: done finish merges"
                                      : L"rollback before checkpoint";

  // infostream that throws exception during rollback
  shared_ptr<InfoStream> evilInfoStream =
      make_shared<InfoStreamAnonymousInnerClass>(shared_from_this(),
                                                 messageToFailOn);

  shared_ptr<Directory> dir =
      newMockDirectory(); // we want to ensure we don't leak any locks or file
                          // handles
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  iwc->setInfoStream(evilInfoStream);
  // TODO: cutover to RandomIndexWriter.mockIndexWriter?
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriterAnonymousInnerClass>(shared_from_this(), dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  for (int i = 0; i < 10; i++) {
    iw->addDocument(doc);
  }
  iw->commit();

  iw->addDocument(doc);

  // pool readers
  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);

  // sometimes sneak in a pending commit: we don't want to leak a file handle to
  // that segments_N
  if (random()->nextBoolean()) {
    iw->prepareCommit();
  }

  runtime_error expected =
      expectThrows(runtime_error::typeid, [&]() { iw->rollback(); });
  TestUtil::assertEquals(L"BOOM!", expected.what());

  r->close();

  // even though we hit exception: we are closed, no locks or files held, index
  // in good state
  assertTrue(iw->isClosed());
  delete dir->obtainLock(IndexWriter::WRITE_LOCK_NAME);

  r = DirectoryReader::open(dir);
  TestUtil::assertEquals(10, r->maxDoc());
  r->close();

  // no leaks
  delete dir;
}

TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::
    InfoStreamAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        const wstring &messageToFailOn)
{
  this->outerInstance = outerInstance;
  this->messageToFailOn = messageToFailOn;
}

void TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::message(
    const wstring &component, const wstring &message)
{
  if (messageToFailOn == message) {
    throw runtime_error(L"BOOM!");
  }
}

bool TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::isEnabled(
    const wstring &component)
{
  return true;
}

TestIndexWriterExceptions::InfoStreamAnonymousInnerClass::
    ~InfoStreamAnonymousInnerClass()
{
}

TestIndexWriterExceptions::IndexWriterAnonymousInnerClass::
    IndexWriterAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<Directory> dir,
        shared_ptr<org::apache::lucene::index::IndexWriterConfig> iwc)
    : IndexWriter(dir, iwc)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriterExceptions::IndexWriterAnonymousInnerClass::
    isEnableTestPoints()
{
  return true;
}

void TestIndexWriterExceptions::testRandomExceptionDuringRollback() throw(
    runtime_error)
{
  // fail in random places on i/o
  constexpr int numIters = RANDOM_MULTIPLIER * 75;
  for (int iter = 0; iter < numIters; iter++) {
    shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
    dir->failOn(
        make_shared<FailureAnonymousInnerClass2>(shared_from_this(), dir));

    shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
    shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
    shared_ptr<Document> doc = make_shared<Document>();
    for (int i = 0; i < 10; i++) {
      iw->addDocument(doc);
    }
    iw->commit();

    iw->addDocument(doc);

    // pool readers
    shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);

    // sometimes sneak in a pending commit: we don't want to leak a file handle
    // to that segments_N
    if (random()->nextBoolean()) {
      iw->prepareCommit();
    }

    try {
      iw->rollback();
    } catch (const MockDirectoryWrapper::FakeIOException &expected) {
      // ok, we randomly hit exc here
    }

    r->close();

    // even though we hit exception: we are closed, no locks or files held,
    // index in good state
    assertTrue(iw->isClosed());
    delete dir->obtainLock(IndexWriter::WRITE_LOCK_NAME);

    r = DirectoryReader::open(dir);
    TestUtil::assertEquals(10, r->maxDoc());
    r->close();

    // no leaks
    delete dir;
  }
}

TestIndexWriterExceptions::FailureAnonymousInnerClass2::
    FailureAnonymousInnerClass2(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<MockDirectoryWrapper> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

void TestIndexWriterExceptions::FailureAnonymousInnerClass2::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (LuceneTestCase::random()->nextInt(10) != 0) {
    return;
  }
  bool maybeFail = false;
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      Thread::currentThread().getStackTrace();

  for (int i = 0; i < trace.size(); i++) {
    if (L"rollbackInternal" == trace[i]->getMethodName()) {
      maybeFail = true;
      break;
    }
  }

  if (maybeFail) {
    if (VERBOSE) {
      wcout << L"TEST: now fail; thread=" << Thread::currentThread().getName()
            << L" exc:" << endl;
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
      (runtime_error())->printStackTrace(System::out);
    }
    throw make_shared<MockDirectoryWrapper::FakeIOException>();
  }
}

void TestIndexWriterExceptions::testMergeExceptionIsTragic() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<AtomicBoolean> *const didFail = make_shared<AtomicBoolean>();
  dir->failOn(make_shared<FailureAnonymousInnerClass3>(shared_from_this(), dir,
                                                       didFail));

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  shared_ptr<MergePolicy> mp = iwc->getMergePolicy();
  if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
    shared_ptr<TieredMergePolicy> tmp =
        std::static_pointer_cast<TieredMergePolicy>(mp);
    if (tmp->getMaxMergedSegmentMB() < 0.2) {
      tmp->setMaxMergedSegmentMB(0.2);
    }
  }
  shared_ptr<MergeScheduler> ms = iwc->getMergeScheduler();
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
        ->setSuppressExceptions();
  }
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  while (true) {
    try {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(newStringField(L"field", L"string", Field::Store::NO));
      w->addDocument(doc);
      if (random()->nextInt(10) == 7) {
        // Flush new segment:
        DirectoryReader::open(w)->close();
      }
    } catch (const AlreadyClosedException &ace) {
      // OK: e.g. CMS hit the exc in BG thread and closed the writer
      break;
    } catch (const MockDirectoryWrapper::FakeIOException &fioe) {
      // OK: e.g. SMS hit the exception
      break;
    }
  }

  assertNotNull(w->getTragicException());
  assertFalse(w->isOpen());
  assertTrue(didFail->get());

  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    // Sneaky: CMS's merge thread will be concurrently rolling back IW due
    // to the tragedy, with this main thread, so we have to wait here
    // to ensure the rollback has finished, else MDW still sees open files:
    (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))->sync();
  }

  delete dir;
}

TestIndexWriterExceptions::FailureAnonymousInnerClass3::
    FailureAnonymousInnerClass3(
        shared_ptr<TestIndexWriterExceptions> outerInstance,
        shared_ptr<MockDirectoryWrapper> dir, shared_ptr<AtomicBoolean> didFail)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->didFail = didFail;
}

void TestIndexWriterExceptions::FailureAnonymousInnerClass3::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (LuceneTestCase::random()->nextInt(10) != 0) {
    return;
  }
  if (didFail->get()) {
    // Already failed
    return;
  }
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      Thread::currentThread().getStackTrace();

  for (int i = 0; i < trace.size(); i++) {
    if (L"merge" == trace[i]->getMethodName()) {
      if (VERBOSE) {
        wcout << L"TEST: now fail; thread=" << Thread::currentThread().getName()
              << L" exc:" << endl;
        // C++ TODO: This exception's constructor requires an argument:
        // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
        (runtime_error())->printStackTrace(System::out);
      }
      didFail->set(true);
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}
} // namespace org::apache::lucene::index