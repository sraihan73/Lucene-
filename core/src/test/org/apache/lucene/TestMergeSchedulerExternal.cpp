using namespace std;

#include "TestMergeSchedulerExternal.h"

namespace org::apache::lucene
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using ConcurrentMergeScheduler =
    org::apache::lucene::index::ConcurrentMergeScheduler;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using MergeScheduler = org::apache::lucene::index::MergeScheduler;
using MergeTrigger = org::apache::lucene::index::MergeTrigger;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using org::junit::AfterClass;
shared_ptr<org::apache::lucene::util::InfoStream>
    TestMergeSchedulerExternal::infoStream;

TestMergeSchedulerExternal::MyMergeScheduler::MyMergeScheduler(
    shared_ptr<TestMergeSchedulerExternal> outerInstance)
    : outerInstance(outerInstance)
{
}

TestMergeSchedulerExternal::MyMergeScheduler::MyMergeThread::MyMergeThread(
    shared_ptr<TestMergeSchedulerExternal::MyMergeScheduler> outerInstance,
    shared_ptr<IndexWriter> writer, shared_ptr<OneMerge> merge)
    : org::apache::lucene::index::ConcurrentMergeScheduler::MergeThread(
          outerInstance, writer, merge),
      outerInstance(outerInstance)
{
  outerInstance->outerInstance->mergeThreadCreated = true;
}

shared_ptr<ConcurrentMergeScheduler::MergeThread>
TestMergeSchedulerExternal::MyMergeScheduler::getMergeThread(
    shared_ptr<IndexWriter> writer,
    shared_ptr<OneMerge> merge) 
{
  shared_ptr<ConcurrentMergeScheduler::MergeThread> thread =
      make_shared<MyMergeThread>(shared_from_this(), writer, merge);
  thread->setDaemon(true);
  thread->setName(L"MyMergeThread");
  return thread;
}

void TestMergeSchedulerExternal::MyMergeScheduler::handleMergeException(
    shared_ptr<Directory> dir, runtime_error t)
{
  outerInstance->excCalled = true;
  if (infoStream->isEnabled(L"IW")) {
    infoStream->message(L"IW", L"TEST: now handleMergeException");
  }
}

void TestMergeSchedulerExternal::MyMergeScheduler::doMerge(
    shared_ptr<IndexWriter> writer,
    shared_ptr<OneMerge> merge) 
{
  outerInstance->mergeCalled = true;
  ConcurrentMergeScheduler::doMerge(writer, merge);
}

void TestMergeSchedulerExternal::FailOnlyOnMerge::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: StackTraceElement[] trace = new Exception().getStackTrace();
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      (runtime_error())->getStackTrace();
  for (int i = 0; i < trace.size(); i++) {
    if (L"doMerge" == trace[i]->getMethodName()) {
      shared_ptr<IOException> ioe =
          make_shared<IOException>(L"now failing during merge");
      shared_ptr<StringWriter> sw = make_shared<StringWriter>();
      shared_ptr<PrintWriter> pw = make_shared<PrintWriter>(sw);
      ioe->printStackTrace(pw);
      if (infoStream->isEnabled(L"IW")) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        infoStream->message(L"IW", L"TEST: now throw exc:\n" + sw->toString());
      }
      throw ioe;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestMergeSchedulerExternal::afterClass() { infoStream.reset(); }

void TestMergeSchedulerExternal::testSubclassConcurrentMergeScheduler() throw(
    IOException)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(make_shared<FailOnlyOnMerge>());

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::YES);
  doc->push_back(idField);

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergeScheduler(make_shared<MyMergeScheduler>(shared_from_this()))
          ->setMaxBufferedDocs(2)
          ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
          ->setMergePolicy(newLogMergePolicy());

  shared_ptr<ByteArrayOutputStream> baos = make_shared<ByteArrayOutputStream>();
  infoStream = make_shared<PrintStreamInfoStream>(
      make_shared<PrintStream>(baos, true, IOUtils::UTF_8));
  iwc->setInfoStream(infoStream);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<LogMergePolicy> logMP = std::static_pointer_cast<LogMergePolicy>(
      writer->getConfig()->getMergePolicy());
  logMP->setMergeFactor(10);
  for (int i = 0; i < 20; i++) {
    writer->addDocument(doc);
  }

  try {
    (std::static_pointer_cast<MyMergeScheduler>(
         writer->getConfig()->getMergeScheduler()))
        ->sync();
  } catch (const IllegalStateException &ise) {
    // OK
  }
  writer->rollback();

  try {
    assertTrue(mergeThreadCreated);
    assertTrue(mergeCalled);
    assertTrue(excCalled);
  } catch (const AssertionError &ae) {
    wcout << L"TEST FAILED; IW infoStream output:" << endl;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << baos->toString(IOUtils::UTF_8) << endl;
    throw ae;
  }
  delete dir;
}

void TestMergeSchedulerExternal::ReportingMergeScheduler::merge(
    shared_ptr<IndexWriter> writer, MergeTrigger trigger,
    bool newMergesFound) 
{
  shared_ptr<OneMerge> merge = nullptr;
  while ((merge = writer->getNextMerge()) != nullptr) {
    if (VERBOSE) {
      wcout << L"executing merge " << merge->segString() << endl;
    }
    writer->merge(merge);
  }
}

TestMergeSchedulerExternal::ReportingMergeScheduler::~ReportingMergeScheduler()
{
}

void TestMergeSchedulerExternal::testCustomMergeScheduler() 
{
  // we don't really need to execute anything, just to make sure the custom MS
  // compiles. But ensure that it can be used as well, e.g., no other hidden
  // dependencies or something. Therefore, don't use any random API !
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setMergeScheduler(make_shared<ReportingMergeScheduler>());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  writer->addDocument(make_shared<Document>());
  writer->commit(); // trigger flush
  writer->addDocument(make_shared<Document>());
  writer->commit(); // trigger flush
  writer->forceMerge(1);
  delete writer;
  delete dir;
}
} // namespace org::apache::lucene