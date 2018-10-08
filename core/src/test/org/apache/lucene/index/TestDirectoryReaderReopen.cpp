using namespace std;

#include "TestDirectoryReaderReopen.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using FakeIOException =
    org::apache::lucene::store::MockDirectoryWrapper::FakeIOException;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDirectoryReaderReopen::testReopen() 
{
  shared_ptr<Directory> *const dir1 = newDirectory();

  createIndex(random(), dir1, false);
  performDefaultTests(
      make_shared<TestReopenAnonymousInnerClass>(shared_from_this(), dir1));
  delete dir1;

  shared_ptr<Directory> *const dir2 = newDirectory();

  createIndex(random(), dir2, true);
  performDefaultTests(
      make_shared<TestReopenAnonymousInnerClass2>(shared_from_this(), dir2));
  delete dir2;
}

TestDirectoryReaderReopen::TestReopenAnonymousInnerClass::
    TestReopenAnonymousInnerClass(
        shared_ptr<TestDirectoryReaderReopen> outerInstance,
        shared_ptr<Directory> dir1)
{
  this->outerInstance = outerInstance;
  this->dir1 = dir1;
}

void TestDirectoryReaderReopen::TestReopenAnonymousInnerClass::modifyIndex(
    int i) 
{
  TestDirectoryReaderReopen::modifyIndex(i, dir1);
}

shared_ptr<DirectoryReader>
TestDirectoryReaderReopen::TestReopenAnonymousInnerClass::openReader() throw(
    IOException)
{
  return DirectoryReader::open(dir1);
}

TestDirectoryReaderReopen::TestReopenAnonymousInnerClass2::
    TestReopenAnonymousInnerClass2(
        shared_ptr<TestDirectoryReaderReopen> outerInstance,
        shared_ptr<Directory> dir2)
{
  this->outerInstance = outerInstance;
  this->dir2 = dir2;
}

void TestDirectoryReaderReopen::TestReopenAnonymousInnerClass2::modifyIndex(
    int i) 
{
  TestDirectoryReaderReopen::modifyIndex(i, dir2);
}

shared_ptr<DirectoryReader>
TestDirectoryReaderReopen::TestReopenAnonymousInnerClass2::openReader() throw(
    IOException)
{
  return DirectoryReader::open(dir2);
}

void TestDirectoryReaderReopen::testCommitReopen() 
{
  shared_ptr<Directory> dir = newDirectory();
  doTestReopenWithCommit(random(), dir, true);
  delete dir;
}

void TestDirectoryReaderReopen::testCommitRecreate() 
{
  shared_ptr<Directory> dir = newDirectory();
  doTestReopenWithCommit(random(), dir, false);
  delete dir;
}

void TestDirectoryReaderReopen::doTestReopenWithCommit(
    shared_ptr<Random> random, shared_ptr<Directory> dir,
    bool withReopen) 
{
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setOpenMode(OpenMode::CREATE)
               ->setMergeScheduler(make_shared<SerialMergeScheduler>())
               ->setMergePolicy(newLogMergePolicy()));
  iwriter->commit();
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  try {
    int M = 3;
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_STORED);
    customType->setTokenized(false);
    shared_ptr<FieldType> customType2 =
        make_shared<FieldType>(TextField::TYPE_STORED);
    customType2->setTokenized(false);
    customType2->setOmitNorms(true);
    shared_ptr<FieldType> customType3 = make_shared<FieldType>();
    customType3->setStored(true);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < M; j++) {
        shared_ptr<Document> doc = make_shared<Document>();
        doc->push_back(
            newField(L"id", to_wstring(i) + L"_" + to_wstring(j), customType));
        doc->push_back(newField(L"id2", to_wstring(i) + L"_" + to_wstring(j),
                                customType2));
        doc->push_back(newField(L"id3", to_wstring(i) + L"_" + to_wstring(j),
                                customType3));
        iwriter->addDocument(doc);
        if (i > 0) {
          int k = i - 1;
          int n = j + k * M;
          shared_ptr<Document> prevItereationDoc = reader->document(n);
          assertNotNull(prevItereationDoc);
          wstring id = prevItereationDoc[L"id"];
          TestUtil::assertEquals(to_wstring(k) + L"_" + to_wstring(j), id);
        }
      }
      iwriter->commit();
      if (withReopen) {
        // reopen
        shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(reader);
        if (r2 != nullptr) {
          reader->close();
          reader = r2;
        }
      } else {
        // recreate
        reader->close();
        reader = DirectoryReader::open(dir);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete iwriter;
    reader->close();
  }
}

void TestDirectoryReaderReopen::performDefaultTests(
    shared_ptr<TestReopen> test) 
{

  shared_ptr<DirectoryReader> index1 = test->openReader();
  shared_ptr<DirectoryReader> index2 = test->openReader();

  TestDirectoryReader::assertIndexEquals(index1, index2);

  // verify that reopen() does not return a new reader instance
  // in case the index has no changes
  shared_ptr<ReaderCouple> couple = refreshReader(index2, false);
  assertTrue(couple->refreshedReader == index2);

  couple = refreshReader(index2, test, 0, true);
  index1->close();
  index1 = couple->newReader;

  shared_ptr<DirectoryReader> index2_refreshed = couple->refreshedReader;
  index2->close();

  // test if refreshed reader and newly opened reader return equal results
  TestDirectoryReader::assertIndexEquals(index1, index2_refreshed);

  index2_refreshed->close();
  assertReaderClosed(index2, true);
  assertReaderClosed(index2_refreshed, true);

  index2 = test->openReader();

  for (int i = 1; i < 4; i++) {

    index1->close();
    couple = refreshReader(index2, test, i, true);
    // refresh DirectoryReader
    index2->close();

    index2 = couple->refreshedReader;
    index1 = couple->newReader;
    TestDirectoryReader::assertIndexEquals(index1, index2);
  }

  index1->close();
  index2->close();
  assertReaderClosed(index1, true);
  assertReaderClosed(index2, true);
}

void TestDirectoryReaderReopen::testThreadSafety() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  // NOTE: this also controls the number of threads!
  constexpr int n = TestUtil::nextInt(random(), 20, 40);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < n; i++) {
    writer->addDocument(createDocument(i, 3));
  }
  writer->forceMerge(1);
  delete writer;

  shared_ptr<TestReopen> *const test =
      make_shared<TestReopenAnonymousInnerClass3>(shared_from_this(), dir, n);

  const deque<std::shared_ptr<ReaderCouple>> readers =
      Collections::synchronizedList(deque<std::shared_ptr<ReaderCouple>>());
  shared_ptr<DirectoryReader> firstReader = DirectoryReader::open(dir);
  shared_ptr<DirectoryReader> reader = firstReader;

  std::deque<std::shared_ptr<ReaderThread>> threads(n);
  shared_ptr<Set<std::shared_ptr<DirectoryReader>>> *const readersToClose =
      Collections::synchronizedSet(
          unordered_set<std::shared_ptr<DirectoryReader>>());

  for (int i = 0; i < n; i++) {
    if (i % 2 == 0) {
      shared_ptr<DirectoryReader> refreshed =
          DirectoryReader::openIfChanged(reader);
      if (refreshed != nullptr) {
        readersToClose->add(reader);
        reader = refreshed;
      }
    }
    shared_ptr<DirectoryReader> *const r = reader;

    constexpr int index = i;

    shared_ptr<ReaderThreadTask> task;

    if (i < 4 || (i >= 10 && i < 14) || i > 18) {
      task = make_shared<ReaderThreadTaskAnonymousInnerClass>(
          shared_from_this(), test, readers, readersToClose, r, index);
    } else {
      task = make_shared<ReaderThreadTaskAnonymousInnerClass2>(
          shared_from_this(), readers);
    }

    threads[i] = make_shared<ReaderThread>(task);
    threads[i]->start();
  }

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this()) { wait(1000); }

  for (int i = 0; i < n; i++) {
    if (threads[i] != nullptr) {
      threads[i]->stopThread();
    }
  }

  for (int i = 0; i < n; i++) {
    if (threads[i] != nullptr) {
      threads[i]->join();
      if (threads[i]->error != nullptr) {
        wstring msg = L"Error occurred in thread " + threads[i]->getName() +
                      L":\n" + threads[i]->error.what();
        fail(msg);
      }
    }
  }

  for (auto readerToClose : readersToClose) {
    readerToClose->close();
  }

  firstReader->close();
  reader->close();

  for (auto readerToClose : readersToClose) {
    assertReaderClosed(readerToClose, true);
  }

  assertReaderClosed(reader, true);
  assertReaderClosed(firstReader, true);

  delete dir;
}

TestDirectoryReaderReopen::TestReopenAnonymousInnerClass3::
    TestReopenAnonymousInnerClass3(
        shared_ptr<TestDirectoryReaderReopen> outerInstance,
        shared_ptr<Directory> dir, int n)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->n = n;
}

void TestDirectoryReaderReopen::TestReopenAnonymousInnerClass3::modifyIndex(
    int i) 
{
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(
               make_shared<MockAnalyzer>(LuceneTestCase::random())));
  modifier->addDocument(createDocument(n + i, 6));
  delete modifier;
}

shared_ptr<DirectoryReader>
TestDirectoryReaderReopen::TestReopenAnonymousInnerClass3::openReader() throw(
    IOException)
{
  return DirectoryReader::open(dir);
}

TestDirectoryReaderReopen::ReaderThreadTaskAnonymousInnerClass::
    ReaderThreadTaskAnonymousInnerClass(
        shared_ptr<TestDirectoryReaderReopen> outerInstance,
        shared_ptr<
            org::apache::lucene::index::TestDirectoryReaderReopen::TestReopen>
            test,
        deque<std::shared_ptr<ReaderCouple>> &readers,
        shared_ptr<Set<std::shared_ptr<DirectoryReader>>> readersToClose,
        shared_ptr<org::apache::lucene::index::DirectoryReader> r, int index)
{
  this->outerInstance = outerInstance;
  this->test = test;
  this->readers = readers;
  this->readersToClose = readersToClose;
  this->r = r;
  this->index = index;
}

void TestDirectoryReaderReopen::ReaderThreadTaskAnonymousInnerClass::
    run() 
{
  shared_ptr<Random> rnd = LuceneTestCase::random();
  while (!stopped) {
    if (index % 2 == 0) {
      // refresh reader synchronized
      shared_ptr<ReaderCouple> c =
          (outerInstance->refreshReader(r, test, index, true));
      readersToClose->add(c->newReader);
      readersToClose->add(c->refreshedReader);
      readers.push_back(c);
      // prevent too many readers
      break;
    } else {
      // not synchronized
      shared_ptr<DirectoryReader> refreshed = DirectoryReader::openIfChanged(r);
      if (refreshed == nullptr) {
        refreshed = r;
      }

      shared_ptr<IndexSearcher> searcher =
          LuceneTestCase::newSearcher(refreshed);
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          searcher
              ->search(
                  make_shared<TermQuery>(make_shared<Term>(
                      L"field1", L"a" + rnd->nextInt(refreshed->maxDoc()))),
                  1000)
              ->scoreDocs;
      if (hits.size() > 0) {
        searcher->doc(hits[0]->doc);
      }
      if (refreshed != r) {
        refreshed->close();
      }
    }
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      wait(TestUtil::nextInt(LuceneTestCase::random(), 1, 100));
    }
  }
}

TestDirectoryReaderReopen::ReaderThreadTaskAnonymousInnerClass2::
    ReaderThreadTaskAnonymousInnerClass2(
        shared_ptr<TestDirectoryReaderReopen> outerInstance,
        deque<std::shared_ptr<ReaderCouple>> &readers)
{
  this->outerInstance = outerInstance;
  this->readers = readers;
}

void TestDirectoryReaderReopen::ReaderThreadTaskAnonymousInnerClass2::
    run() 
{
  shared_ptr<Random> rnd = LuceneTestCase::random();
  while (!stopped) {
    int numReaders = readers.size();
    if (numReaders > 0) {
      shared_ptr<ReaderCouple> c = readers[rnd->nextInt(numReaders)];
      TestDirectoryReader::assertIndexEquals(c->newReader, c->refreshedReader);
    }

    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      wait(TestUtil::nextInt(LuceneTestCase::random(), 1, 100));
    }
  }
}

TestDirectoryReaderReopen::ReaderCouple::ReaderCouple(
    shared_ptr<DirectoryReader> r1, shared_ptr<DirectoryReader> r2)
{
  newReader = r1;
  refreshedReader = r2;
}

void TestDirectoryReaderReopen::ReaderThreadTask::stop()
{
  this->stopped = true;
}

TestDirectoryReaderReopen::ReaderThread::ReaderThread(
    shared_ptr<ReaderThreadTask> task)
{
  this->task = task;
}

void TestDirectoryReaderReopen::ReaderThread::stopThread()
{
  this->task->stop();
}

void TestDirectoryReaderReopen::ReaderThread::run()
{
  try {
    this->task->run();
  } catch (const runtime_error &r) {
    r.printStackTrace(System::out);
    this->error = r;
  }
}

shared_ptr<ReaderCouple>
TestDirectoryReaderReopen::refreshReader(shared_ptr<DirectoryReader> reader,
                                         bool hasChanges) 
{
  return refreshReader(reader, nullptr, -1, hasChanges);
}

shared_ptr<ReaderCouple> TestDirectoryReaderReopen::refreshReader(
    shared_ptr<DirectoryReader> reader, shared_ptr<TestReopen> test, int modify,
    bool hasChanges) 
{
  {
    lock_guard<mutex> lock(createReaderMutex);
    shared_ptr<DirectoryReader> r = nullptr;
    if (test != nullptr) {
      test->modifyIndex(modify);
      r = test->openReader();
    }

    shared_ptr<DirectoryReader> refreshed = nullptr;
    try {
      refreshed = DirectoryReader::openIfChanged(reader);
      if (refreshed == nullptr) {
        refreshed = reader;
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (refreshed == nullptr && r != nullptr) {
        // Hit exception -- close opened reader
        r->close();
      }
    }

    if (hasChanges) {
      if (refreshed == reader) {
        fail(L"No new DirectoryReader instance created during refresh.");
      }
    } else {
      if (refreshed != reader) {
        fail(L"New DirectoryReader instance created during refresh even though "
             L"index had no changes.");
      }
    }

    return make_shared<ReaderCouple>(r, refreshed);
  }
}

void TestDirectoryReaderReopen::createIndex(
    shared_ptr<Random> random, shared_ptr<Directory> dir,
    bool multiSegment) 
{
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, LuceneTestCase::newIndexWriterConfig(
               random, make_shared<MockAnalyzer>(random))
               ->setMergePolicy(make_shared<LogDocMergePolicy>()));

  for (int i = 0; i < 100; i++) {
    w->addDocument(createDocument(i, 4));
    if (multiSegment && (i % 10) == 0) {
      w->commit();
    }
  }

  if (!multiSegment) {
    w->forceMerge(1);
  }

  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  if (multiSegment) {
    assertTrue(r->leaves()->size() > 1);
  } else {
    assertTrue(r->leaves()->size() == 1);
  }
  r->close();
}

shared_ptr<Document> TestDirectoryReaderReopen::createDocument(int n,
                                                               int numFields)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  shared_ptr<Document> doc = make_shared<Document>();
  sb->append(L"a");
  sb->append(n);
  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType2->setTokenized(false);
  customType2->setOmitNorms(true);
  shared_ptr<FieldType> customType3 = make_shared<FieldType>();
  customType3->setStored(true);
  doc->push_back(
      make_shared<TextField>(L"field1", sb->toString(), Field::Store::YES));
  doc->push_back(make_shared<Field>(L"fielda", sb->toString(), customType2));
  doc->push_back(make_shared<Field>(L"fieldb", sb->toString(), customType3));
  sb->append(L" b");
  sb->append(n);
  for (int i = 1; i < numFields; i++) {
    doc->push_back(make_shared<TextField>(L"field" + to_wstring(i + 1),
                                          sb->toString(), Field::Store::YES));
  }
  return doc;
}

void TestDirectoryReaderReopen::modifyIndex(
    int i, shared_ptr<Directory> dir) 
{
  switch (i) {
  case 0: {
    if (VERBOSE) {
      wcout << L"TEST: modify index" << endl;
    }
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir,
        make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
    w->deleteDocuments({make_shared<Term>(L"field2", L"a11")});
    w->deleteDocuments({make_shared<Term>(L"field2", L"b30")});
    delete w;
    break;
  }
  case 1: {
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir,
        make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
    w->forceMerge(1);
    delete w;
    break;
  }
  case 2: {
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir,
        make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
    w->addDocument(createDocument(101, 4));
    w->forceMerge(1);
    w->addDocument(createDocument(102, 4));
    w->addDocument(createDocument(103, 4));
    delete w;
    break;
  }
  case 3: {
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir,
        make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
    w->addDocument(createDocument(101, 4));
    delete w;
    break;
  }
  }
}

void TestDirectoryReaderReopen::assertReaderClosed(
    shared_ptr<IndexReader> reader, bool checkSubReaders)
{
  TestUtil::assertEquals(0, reader->getRefCount());

  if (checkSubReaders &&
      std::dynamic_pointer_cast<CompositeReader>(reader) != nullptr) {
    // we cannot use reader context here, as reader is
    // already closed and calling getTopReaderContext() throws AlreadyClosed!
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: java.util.List<? extends IndexReader> subReaders =
    // ((CompositeReader) reader).getSequentialSubReaders();
    deque < ? extends IndexReader > subReaders =
                   (std::static_pointer_cast<CompositeReader>(reader))
                       ->getSequentialSubReaders();
    for (auto r : subReaders) {
      assertReaderClosed(r, checkSubReaders);
    }
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits)
void TestDirectoryReaderReopen::KeepAllCommits::onInit(deque<T1> commits)
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits)
void TestDirectoryReaderReopen::KeepAllCommits::onCommit(deque<T1> commits)
{
}

void TestDirectoryReaderReopen::testReopenOnCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(make_shared<KeepAllCommits>())
               ->setMaxBufferedDocs(-1)
               ->setMergePolicy(newLogMergePolicy(10)));
  for (int i = 0; i < 4; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    writer->addDocument(doc);
    unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
    data.emplace(L"index", to_wstring(i) + L"");
    writer->setLiveCommitData(data.entrySet());
    writer->commit();
  }
  for (int i = 0; i < 4; i++) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
    unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
    data.emplace(L"index", to_wstring(4 + i) + L"");
    writer->setLiveCommitData(data.entrySet());
    writer->commit();
  }
  delete writer;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, r->numDocs());

  shared_ptr<deque<std::shared_ptr<IndexCommit>>> commits =
      DirectoryReader::listCommits(dir);
  for (auto commit : commits) {
    shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r, commit);
    assertNotNull(r2);
    assertTrue(r2 != r);

    const unordered_map<wstring, wstring> s = commit->getUserData();
    constexpr int v;
    if (s.empty()) {
      // First commit created by IW
      v = -1;
    } else {
      v = stoi(s[L"index"]);
    }
    if (v < 4) {
      TestUtil::assertEquals(1 + v, r2->numDocs());
    } else {
      TestUtil::assertEquals(7 - v, r2->numDocs());
    }
    r->close();
    r = r2;
  }
  r->close();
  delete dir;
}

void TestDirectoryReaderReopen::testOpenIfChangedNRTToCommit() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();

  // Can't use RIW because it randomly commits:
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"value", Field::Store::NO));
  w->addDocument(doc);
  w->commit();
  deque<std::shared_ptr<IndexCommit>> commits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(1, commits.size());
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);

  TestUtil::assertEquals(2, r->numDocs());
  shared_ptr<IndexReader> r2 = DirectoryReader::openIfChanged(r, commits[0]);
  assertNotNull(r2);
  r->close();
  TestUtil::assertEquals(1, r2->numDocs());
  delete w;
  delete r2;
  delete dir;
}

void TestDirectoryReaderReopen::testOverDecRefDuringReopen() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(TestUtil::getDefaultCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id", Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id2", Field::Store::NO));
  w->addDocument(doc);
  w->commit();

  // Open reader w/ one segment w/ 2 docs:
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);

  // Delete 1 doc from the segment:
  // System.out.println("TEST: now delete");
  w->deleteDocuments({make_shared<Term>(L"id", L"id")});
  // System.out.println("TEST: now commit");
  w->commit();

  // Fail when reopen tries to open the live docs file:
  dir->failOn(make_shared<FailureAnonymousInnerClass>(shared_from_this(), dir));

  // Now reopen:
  // System.out.println("TEST: now reopen");
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid,
               [&]() { DirectoryReader::openIfChanged(r); });

  shared_ptr<IndexSearcher> s = newSearcher(r);
  TestUtil::assertEquals(
      1, s->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
             ->totalHits);

  r->close();
  delete w;
  delete dir;
}

TestDirectoryReaderReopen::FailureAnonymousInnerClass::
    FailureAnonymousInnerClass(
        shared_ptr<TestDirectoryReaderReopen> outerInstance,
        shared_ptr<MockDirectoryWrapper> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

void TestDirectoryReaderReopen::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (failed) {
    return;
  }
  // System.out.println("failOn: ");
  // new Throwable().printStackTrace(System.out);
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: StackTraceElement[] trace = new Exception().getStackTrace();
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      (runtime_error())->getStackTrace();
  for (int i = 0; i < trace.size(); i++) {
    if (L"readLiveDocs" == trace[i]->getMethodName()) {
      if (VERBOSE) {
        wcout << L"TEST: now fail; exc:" << endl;
        // C++ TODO: This exception's constructor requires an argument:
        // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
        (runtime_error())->printStackTrace(System::out);
      }
      failed = true;
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void TestDirectoryReaderReopen::testNPEAfterInvalidReindex1() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id", Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id2", Field::Store::NO));
  w->addDocument(doc);
  w->deleteDocuments({make_shared<Term>(L"id", L"id")});
  w->commit();
  delete w;

  // Open reader w/ one segment w/ 2 docs, 1 deleted:
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);

  // Blow away the index:
  for (auto fileName : dir->listAll()) {
    dir->deleteFile(fileName);
  }

  w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 13));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id2", Field::Store::NO));
  w->addDocument(doc);
  w->commit();
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id2", Field::Store::NO));
  w->addDocument(doc);
  w->updateNumericDocValue(make_shared<Term>(L"id", L"id"), L"ndv", 17LL);
  w->commit();
  delete w;

  expectThrows(IllegalStateException::typeid,
               [&]() { DirectoryReader::openIfChanged(r); });

  r->close();
  delete w;
  delete dir;
}

void TestDirectoryReaderReopen::testNPEAfterInvalidReindex2() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id", Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id2", Field::Store::NO));
  w->addDocument(doc);
  w->deleteDocuments({make_shared<Term>(L"id", L"id")});
  w->commit();
  delete w;

  // Open reader w/ one segment w/ 2 docs, 1 deleted:
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);

  // Blow away the index:
  for (auto name : dir->listAll()) {
    dir->deleteFile(name);
  }

  w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 13));
  w->addDocument(doc);
  w->commit();
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id2", Field::Store::NO));
  w->addDocument(doc);
  w->commit();
  delete w;

  expectThrows(IllegalStateException::typeid,
               [&]() { DirectoryReader::openIfChanged(r); });

  r->close();
  delete dir;
}

void TestDirectoryReaderReopen::testNRTMdeletes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SnapshotDeletionPolicy> snapshotter =
      make_shared<SnapshotDeletionPolicy>(
          make_shared<KeepOnlyLastCommitDeletionPolicy>());
  iwc->setIndexDeletionPolicy(snapshotter);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  writer->commit(); // make sure all index metadata is written out

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value1", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value2", Field::Store::YES));
  writer->addDocument(doc);

  writer->commit();

  shared_ptr<IndexCommit> ic1 = snapshotter->snapshot();

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value3", Field::Store::YES));
  writer->updateDocument(make_shared<Term>(L"key", L"value1"), doc);

  writer->commit();

  shared_ptr<IndexCommit> ic2 = snapshotter->snapshot();
  shared_ptr<DirectoryReader> latest = DirectoryReader::open(ic2);
  TestUtil::assertEquals(2, latest->leaves()->size());

  // This reader will be used for searching against commit point 1
  shared_ptr<DirectoryReader> oldest =
      DirectoryReader::openIfChanged(latest, ic1);
  TestUtil::assertEquals(1, oldest->leaves()->size());

  // sharing same core
  assertSame(latest->leaves()->get(0).reader().getCoreCacheHelper().getKey(),
             oldest->leaves()->get(0).reader().getCoreCacheHelper().getKey());

  latest->close();
  oldest->close();

  snapshotter->release(ic1);
  snapshotter->release(ic2);
  delete writer;
  delete dir;
}

void TestDirectoryReaderReopen::testNRTMdeletes2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SnapshotDeletionPolicy> snapshotter =
      make_shared<SnapshotDeletionPolicy>(
          make_shared<KeepOnlyLastCommitDeletionPolicy>());
  iwc->setIndexDeletionPolicy(snapshotter);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  writer->commit(); // make sure all index metadata is written out

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value1", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value2", Field::Store::YES));
  writer->addDocument(doc);

  writer->commit();

  shared_ptr<IndexCommit> ic1 = snapshotter->snapshot();

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value3", Field::Store::YES));
  writer->updateDocument(make_shared<Term>(L"key", L"value1"), doc);

  shared_ptr<DirectoryReader> latest = DirectoryReader::open(writer);
  TestUtil::assertEquals(2, latest->leaves()->size());

  // This reader will be used for searching against commit point 1
  shared_ptr<DirectoryReader> oldest =
      DirectoryReader::openIfChanged(latest, ic1);

  // This reader should not see the deletion:
  TestUtil::assertEquals(2, oldest->numDocs());
  assertFalse(oldest->hasDeletions());

  snapshotter->release(ic1);
  TestUtil::assertEquals(1, oldest->leaves()->size());

  // sharing same core
  assertSame(latest->leaves()->get(0).reader().getCoreCacheHelper().getKey(),
             oldest->leaves()->get(0).reader().getCoreCacheHelper().getKey());

  latest->close();
  oldest->close();

  delete writer;
  delete dir;
}

void TestDirectoryReaderReopen::testNRTMupdates() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SnapshotDeletionPolicy> snapshotter =
      make_shared<SnapshotDeletionPolicy>(
          make_shared<KeepOnlyLastCommitDeletionPolicy>());
  iwc->setIndexDeletionPolicy(snapshotter);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  writer->commit(); // make sure all index metadata is written out

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 1));
  writer->addDocument(doc);

  writer->commit();

  shared_ptr<IndexCommit> ic1 = snapshotter->snapshot();

  writer->updateNumericDocValue(make_shared<Term>(L"key", L"value1"), L"dv", 2);

  writer->commit();

  shared_ptr<IndexCommit> ic2 = snapshotter->snapshot();
  shared_ptr<DirectoryReader> latest = DirectoryReader::open(ic2);
  TestUtil::assertEquals(1, latest->leaves()->size());

  // This reader will be used for searching against commit point 1
  shared_ptr<DirectoryReader> oldest =
      DirectoryReader::openIfChanged(latest, ic1);
  TestUtil::assertEquals(1, oldest->leaves()->size());

  // sharing same core
  assertSame(latest->leaves()->get(0).reader().getCoreCacheHelper().getKey(),
             oldest->leaves()->get(0).reader().getCoreCacheHelper().getKey());

  shared_ptr<NumericDocValues> values =
      getOnlyLeafReader(oldest)->getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());

  values = getOnlyLeafReader(latest)->getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());

  latest->close();
  oldest->close();

  snapshotter->release(ic1);
  snapshotter->release(ic2);
  delete writer;
  delete dir;
}

void TestDirectoryReaderReopen::testNRTMupdates2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SnapshotDeletionPolicy> snapshotter =
      make_shared<SnapshotDeletionPolicy>(
          make_shared<KeepOnlyLastCommitDeletionPolicy>());
  iwc->setIndexDeletionPolicy(snapshotter);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  writer->commit(); // make sure all index metadata is written out

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"key", L"value1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 1));
  writer->addDocument(doc);

  writer->commit();

  shared_ptr<IndexCommit> ic1 = snapshotter->snapshot();

  writer->updateNumericDocValue(make_shared<Term>(L"key", L"value1"), L"dv", 2);

  shared_ptr<DirectoryReader> latest = DirectoryReader::open(writer);
  TestUtil::assertEquals(1, latest->leaves()->size());

  // This reader will be used for searching against commit point 1
  shared_ptr<DirectoryReader> oldest =
      DirectoryReader::openIfChanged(latest, ic1);
  TestUtil::assertEquals(1, oldest->leaves()->size());

  // sharing same core
  assertSame(latest->leaves()->get(0).reader().getCoreCacheHelper().getKey(),
             oldest->leaves()->get(0).reader().getCoreCacheHelper().getKey());

  shared_ptr<NumericDocValues> values =
      getOnlyLeafReader(oldest)->getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());

  values = getOnlyLeafReader(latest)->getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());

  latest->close();
  oldest->close();

  snapshotter->release(ic1);
  delete writer;
  delete dir;
}

void TestDirectoryReaderReopen::
    testDeleteIndexFilesWhileReaderStillOpen() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"value", Field::Store::NO));
  w->addDocument(doc);
  // Creates single segment index:
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);

  // Abuse: remove all files while reader is open; one is supposed to use
  // IW.deleteAll, or open a new IW with OpenMode.CREATE instead:
  for (auto file : dir->listAll()) {
    dir->deleteFile(file);
  }

  w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"value", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"value2", Field::Store::NO));
  w->addDocument(doc);

  // Writes same segment, this time with two documents:
  w->commit();

  w->deleteDocuments({make_shared<Term>(L"field", L"value2")});

  w->addDocument(doc);

  // Writes another segments file, so openIfChanged sees that the index has in
  // fact changed:
  delete w;

  expectThrows(IllegalStateException::typeid,
               [&]() { DirectoryReader::openIfChanged(r); });
}

void TestDirectoryReaderReopen::testReuseUnchangedLeafReaderOnDVUpdate() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  indexWriterConfig->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"some_docvalue", 2));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(2, reader->numDocs());
  TestUtil::assertEquals(2, reader->maxDoc());
  TestUtil::assertEquals(0, reader->numDeletedDocs());

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"2", Field::Store::YES));
  writer->updateDocValues(
      make_shared<Term>(L"id", L"1"),
      {make_shared<NumericDocValuesField>(L"some_docvalue", 1)});
  writer->commit();
  shared_ptr<DirectoryReader> newReader =
      DirectoryReader::openIfChanged(reader);
  assertNotSame(newReader, reader);
  reader->close();
  reader = newReader;
  TestUtil::assertEquals(2, reader->numDocs());
  TestUtil::assertEquals(2, reader->maxDoc());
  TestUtil::assertEquals(0, reader->numDeletedDocs());

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"3", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"3", Field::Store::YES));
  writer->updateDocument(make_shared<Term>(L"id", L"3"), doc);
  writer->commit();

  newReader = DirectoryReader::openIfChanged(reader);
  assertNotSame(newReader, reader);
  TestUtil::assertEquals(2, newReader->getSequentialSubReaders().size());
  TestUtil::assertEquals(1, reader->getSequentialSubReaders().size());
  assertSame(reader->getSequentialSubReaders()[0],
             newReader->getSequentialSubReaders()[0]);
  reader->close();
  reader = newReader;
  TestUtil::assertEquals(3, reader->numDocs());
  TestUtil::assertEquals(3, reader->maxDoc());
  TestUtil::assertEquals(0, reader->numDeletedDocs());
  IOUtils::close({reader, writer, dir});
}
} // namespace org::apache::lucene::index