using namespace std;

#include "TestIndexWriterReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using FakeIOException =
    org::apache::lucene::store::MockDirectoryWrapper::FakeIOException;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using org::junit::Test;

int TestIndexWriterReader::count(shared_ptr<Term> t,
                                 shared_ptr<IndexReader> r) 
{
  int count = 0;
  shared_ptr<PostingsEnum> td = TestUtil::docs(
      random(), r, t->field(), make_shared<BytesRef>(t->text()), nullptr, 0);

  if (td != nullptr) {
    shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(r);
    while (td->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      td->docID();
      if (liveDocs == nullptr || liveDocs->get(td->docID())) {
        count++;
      }
    }
  }
  return count;
}

void TestIndexWriterReader::testAddCloseOpen() 
{
  // Can't use assertNoDeletes: this test pulls a non-NRT
  // reader in the end:
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir1, iwc);
  for (int i = 0; i < 97; i++) {
    shared_ptr<DirectoryReader> reader = writer->getReader();
    if (i == 0) {
      writer->addDocument(
          DocHelper::createDocument(i, L"x", 1 + random()->nextInt(5)));
    } else {
      int previous = random()->nextInt(i);
      // a check if the reader is current here could fail since there might be
      // merges going on.
      switch (random()->nextInt(5)) {
      case 0:
      case 1:
      case 2:
        writer->addDocument(
            DocHelper::createDocument(i, L"x", 1 + random()->nextInt(5)));
        break;
      case 3:
        writer->updateDocument(
            make_shared<Term>(L"id", L"" + to_wstring(previous)),
            DocHelper::createDocument(previous, L"x",
                                      1 + random()->nextInt(5)));
        break;
      case 4:
        writer->deleteDocuments(
            {make_shared<Term>(L"id", L"" + to_wstring(previous))});
      }
    }
    assertFalse(reader->isCurrent());
    reader->close();
  }
  writer->forceMerge(1); // make sure all merging is done etc.
  shared_ptr<DirectoryReader> reader = writer->getReader();
  writer->commit(); // no changes that are not visible to the reader

  // A commit is now seen as a change to an NRT reader:
  assertFalse(reader->isCurrent());
  reader->close();
  reader = writer->getReader();
  assertTrue(reader->isCurrent());
  delete writer;

  assertTrue(reader->isCurrent());
  iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  writer = make_shared<IndexWriter>(dir1, iwc);
  assertTrue(reader->isCurrent());
  writer->addDocument(
      DocHelper::createDocument(1, L"x", 1 + random()->nextInt(5)));
  assertTrue(reader->isCurrent()); // segments in ram but IW is different to the
                                   // readers one
  delete writer;
  assertFalse(reader->isCurrent()); // segments written
  reader->close();
  delete dir1;
}

void TestIndexWriterReader::testUpdateDocument() 
{
  bool doFullMerge = true;

  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  if (iwc->getMaxBufferedDocs() < 20) {
    iwc->setMaxBufferedDocs(20);
  }
  // no merging
  iwc->setMergePolicy(NoMergePolicy::INSTANCE);
  if (VERBOSE) {
    wcout << L"TEST: make index" << endl;
  }
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir1, iwc);

  // create the index
  createIndexNoClose(!doFullMerge, L"index1", writer);

  // writer.flush(false, true, true);

  // get a reader
  shared_ptr<DirectoryReader> r1 = writer->getReader();
  assertTrue(r1->isCurrent());

  wstring id10 = r1->document(10).getField(L"id").stringValue();

  shared_ptr<Document> newDoc = r1->document(10);
  newDoc->removeField(L"id");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  newDoc->push_back(
      newStringField(L"id", Integer::toString(8000), Field::Store::YES));
  writer->updateDocument(make_shared<Term>(L"id", id10), newDoc);
  assertFalse(r1->isCurrent());

  wcout << L"TEST: now get reader" << endl;
  shared_ptr<DirectoryReader> r2 = writer->getReader();
  assertTrue(r2->isCurrent());
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id10), r2));
  if (VERBOSE) {
    wcout << L"TEST: verify id" << endl;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      1, count(make_shared<Term>(L"id", Integer::toString(8000)), r2));

  r1->close();
  assertTrue(r2->isCurrent());
  delete writer;
  // writer.close wrote a new commit
  assertFalse(r2->isCurrent());

  shared_ptr<DirectoryReader> r3 = DirectoryReader::open(dir1);
  assertTrue(r3->isCurrent());
  assertFalse(r2->isCurrent());
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id10), r3));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      1, count(make_shared<Term>(L"id", Integer::toString(8000)), r3));

  writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c", Field::Store::NO));
  writer->addDocument(doc);
  assertFalse(r2->isCurrent());
  assertTrue(r3->isCurrent());

  delete writer;

  assertFalse(r2->isCurrent());
  assertTrue(!r3->isCurrent());

  r2->close();
  r3->close();

  delete dir1;
}

void TestIndexWriterReader::testIsCurrent() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c", Field::Store::NO));
  writer->addDocument(doc);
  delete writer;

  iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  writer = make_shared<IndexWriter>(dir, iwc);
  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c", Field::Store::NO));
  shared_ptr<DirectoryReader> nrtReader = writer->getReader();
  assertTrue(nrtReader->isCurrent());
  writer->addDocument(doc);
  assertFalse(nrtReader->isCurrent()); // should see the changes
  writer->forceMerge(1); // make sure we don't have a merge going on
  assertFalse(nrtReader->isCurrent());
  nrtReader->close();

  shared_ptr<DirectoryReader> dirReader = DirectoryReader::open(dir);
  nrtReader = writer->getReader();

  assertTrue(dirReader->isCurrent());
  assertTrue(
      nrtReader
          ->isCurrent()); // nothing was committed yet so we are still current
  TestUtil::assertEquals(2,
                         nrtReader->maxDoc()); // sees the actual document added
  TestUtil::assertEquals(1, dirReader->maxDoc());
  delete writer; // close is actually a commit both should see the changes
  assertFalse(nrtReader->isCurrent());
  assertFalse(dirReader->isCurrent()); // this reader has been opened before the
                                       // writer was closed / committed

  dirReader->close();
  nrtReader->close();
  delete dir;
}

void TestIndexWriterReader::testAddIndexes() 
{
  bool doFullMerge = false;

  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  if (iwc->getMaxBufferedDocs() < 20) {
    iwc->setMaxBufferedDocs(20);
  }
  // no merging
  iwc->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir1, iwc);

  // create the index
  createIndexNoClose(!doFullMerge, L"index1", writer);
  writer->flush(false, true);

  // create a 2nd index
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  createIndexNoClose(!doFullMerge, L"index2", writer2);
  delete writer2;

  shared_ptr<DirectoryReader> r0 = writer->getReader();
  assertTrue(r0->isCurrent());
  writer->addIndexes({dir2});
  assertFalse(r0->isCurrent());
  r0->close();

  shared_ptr<DirectoryReader> r1 = writer->getReader();
  assertTrue(r1->isCurrent());

  writer->commit();

  // A commit is seen as a change to NRT reader:
  assertFalse(r1->isCurrent());

  TestUtil::assertEquals(200, r1->maxDoc());

  int index2df = r1->docFreq(make_shared<Term>(L"indexname", L"index2"));

  TestUtil::assertEquals(100, index2df);

  // verify the docs are from different indexes
  shared_ptr<Document> doc5 = r1->document(5);
  TestUtil::assertEquals(L"index1", doc5[L"indexname"]);
  shared_ptr<Document> doc150 = r1->document(150);
  TestUtil::assertEquals(L"index2", doc150[L"indexname"]);
  r1->close();
  delete writer;
  delete dir1;
  delete dir2;
}

void TestIndexWriterReader::testAddIndexes2() 
{
  bool doFullMerge = false;

  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // create a 2nd index
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  createIndexNoClose(!doFullMerge, L"index2", writer2);
  delete writer2;

  writer->addIndexes({dir2});
  writer->addIndexes({dir2});
  writer->addIndexes({dir2});
  writer->addIndexes({dir2});
  writer->addIndexes({dir2});

  shared_ptr<IndexReader> r1 = writer->getReader();
  TestUtil::assertEquals(500, r1->maxDoc());

  delete r1;
  delete writer;
  delete dir1;
  delete dir2;
}

void TestIndexWriterReader::testDeleteFromIndexWriter() 
{
  bool doFullMerge = true;

  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  // create the index
  createIndexNoClose(!doFullMerge, L"index1", writer);
  writer->flush(false, true);
  // get a reader
  shared_ptr<IndexReader> r1 = writer->getReader();

  wstring id10 = r1->document(10)->getField(L"id")->stringValue();

  // deleted IW docs should not show up in the next getReader
  writer->deleteDocuments({make_shared<Term>(L"id", id10)});
  shared_ptr<IndexReader> r2 = writer->getReader();
  TestUtil::assertEquals(1, count(make_shared<Term>(L"id", id10), r1));
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id10), r2));

  wstring id50 = r1->document(50)->getField(L"id")->stringValue();
  TestUtil::assertEquals(1, count(make_shared<Term>(L"id", id50), r1));

  writer->deleteDocuments({make_shared<Term>(L"id", id50)});

  shared_ptr<IndexReader> r3 = writer->getReader();
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id10), r3));
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id50), r3));

  wstring id75 = r1->document(75)->getField(L"id")->stringValue();
  writer->deleteDocuments(
      {make_shared<TermQuery>(make_shared<Term>(L"id", id75))});
  shared_ptr<IndexReader> r4 = writer->getReader();
  TestUtil::assertEquals(1, count(make_shared<Term>(L"id", id75), r3));
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id75), r4));

  delete r1;
  delete r2;
  delete r3;
  delete r4;
  delete writer;

  // reopen the writer to verify the delete made it to the directory
  writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<IndexReader> w2r1 = writer->getReader();
  TestUtil::assertEquals(0, count(make_shared<Term>(L"id", id10), w2r1));
  delete w2r1;
  delete writer;
  delete dir1;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testAddIndexesAndDoDeletesThreads() throws
// Throwable
void TestIndexWriterReader::testAddIndexesAndDoDeletesThreads() throw(
    runtime_error)
{
  constexpr int numIter = 2;
  int numDirs = 3;

  shared_ptr<Directory> mainDir = getAssertNoDeletesDirectory(newDirectory());

  shared_ptr<IndexWriter> mainWriter = make_shared<IndexWriter>(
      mainDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMergePolicy(newLogMergePolicy()));
  TestUtil::reduceOpenFiles(mainWriter);

  shared_ptr<AddDirectoriesThreads> addDirThreads =
      make_shared<AddDirectoriesThreads>(shared_from_this(), numIter,
                                         mainWriter);
  addDirThreads->launchThreads(numDirs);
  addDirThreads->joinThreads();

  // assertEquals(100 + numDirs * (3 * numIter / 4) * addDirThreads.numThreads
  //    * addDirThreads.NUM_INIT_DOCS, addDirThreads.mainWriter.numDocs());
  TestUtil::assertEquals(addDirThreads->count->intValue(),
                         addDirThreads->mainWriter->numDocs());

  addDirThreads->close(true);

  assertTrue(addDirThreads->failures.empty());

  TestUtil::checkIndex(mainDir);

  shared_ptr<IndexReader> reader = DirectoryReader::open(mainDir);
  TestUtil::assertEquals(addDirThreads->count->intValue(), reader->numDocs());
  // assertEquals(100 + numDirs * (3 * numIter / 4) * addDirThreads.numThreads
  //    * addDirThreads.NUM_INIT_DOCS, reader.numDocs());
  delete reader;

  addDirThreads->closeDir();
  delete mainDir;
}

TestIndexWriterReader::AddDirectoriesThreads::AddDirectoriesThreads(
    shared_ptr<TestIndexWriterReader> outerInstance, int numDirs,
    shared_ptr<IndexWriter> mainWriter) 
    : outerInstance(outerInstance)
{
  this->numDirs = numDirs;
  this->mainWriter = mainWriter;
  addDir = LuceneTestCase::newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      addDir, LuceneTestCase::newIndexWriterConfig(
                  make_shared<MockAnalyzer>(LuceneTestCase::random()))
                  ->setMaxBufferedDocs(2));
  TestUtil::reduceOpenFiles(writer);
  for (int i = 0; i < NUM_INIT_DOCS; i++) {
    shared_ptr<Document> doc = DocHelper::createDocument(i, L"addindex", 4);
    writer->addDocument(doc);
  }

  delete writer;

  readers = std::deque<std::shared_ptr<DirectoryReader>>(numDirs);
  for (int i = 0; i < numDirs; i++) {
    readers[i] = DirectoryReader::open(addDir);
  }
}

void TestIndexWriterReader::AddDirectoriesThreads::joinThreads()
{
  for (int i = 0; i < outerInstance->numThreads; i++) {
    try {
      threads[i]->join();
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  }
}

void TestIndexWriterReader::AddDirectoriesThreads::close(bool doWait) throw(
    runtime_error)
{
  didClose = true;
  if (doWait) {
    delete mainWriter;
  } else {
    mainWriter->rollback();
  }
}

void TestIndexWriterReader::AddDirectoriesThreads::closeDir() throw(
    runtime_error)
{
  for (int i = 0; i < numDirs; i++) {
    readers[i]->close();
  }
  delete addDir;
}

void TestIndexWriterReader::AddDirectoriesThreads::handle(runtime_error t)
{
  t.printStackTrace(System::out);
  {
    lock_guard<mutex> lock(failures);
    failures.push_back(t);
  }
}

void TestIndexWriterReader::AddDirectoriesThreads::launchThreads(
    int const numIter)
{
  for (int i = 0; i < outerInstance->numThreads; i++) {
    threads[i] =
        make_shared<ThreadAnonymousInnerClass>(shared_from_this(), numIter);
  }
  for (int i = 0; i < outerInstance->numThreads; i++) {
    threads[i]->start();
  }
}

TestIndexWriterReader::AddDirectoriesThreads::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<AddDirectoriesThreads> outerInstance,
                              int numIter)
{
  this->outerInstance = outerInstance;
  this->numIter = numIter;
}

void TestIndexWriterReader::AddDirectoriesThreads::ThreadAnonymousInnerClass::
    run()
{
  try {
    std::deque<std::shared_ptr<Directory>> dirs(outerInstance->numDirs);
    for (int k = 0; k < outerInstance->numDirs; k++) {
      dirs[k] = make_shared<MockDirectoryWrapper>(
          LuceneTestCase::random(), TestUtil::ramCopyOf(outerInstance->addDir));
    }
    // int j = 0;
    // while (true) {
    // System.out.println(Thread.currentThread().getName() + ": iter
    // j=" + j);
    for (int x = 0; x < numIter; x++) {
      // only do addIndexes
      outerInstance->doBody(x, dirs);
    }
    // if (numIter > 0 && j == numIter)
    //  break;
    // doBody(j++, dirs);
    // doBody(5, dirs);
    //}
  } catch (const runtime_error &t) {
    outerInstance->handle(t);
  }
}

void TestIndexWriterReader::AddDirectoriesThreads::doBody(
    int j, std::deque<std::shared_ptr<Directory>> &dirs) 
{
  switch (j % 4) {
  case 0:
    mainWriter->addIndexes(dirs);
    mainWriter->forceMerge(1);
    break;
  case 1:
    mainWriter->addIndexes(dirs);
    numaddIndexes->incrementAndGet();
    break;
  case 2:
    TestUtil::addIndexesSlowly(mainWriter, readers);
    break;
  case 3:
    mainWriter->commit();
  }
  count->addAndGet(dirs.size() * NUM_INIT_DOCS);
}

void TestIndexWriterReader::testIndexWriterReopenSegmentFullMerge() throw(
    runtime_error)
{
  doTestIndexWriterReopenSegment(true);
}

void TestIndexWriterReader::testIndexWriterReopenSegment() 
{
  doTestIndexWriterReopenSegment(false);
}

void TestIndexWriterReader::doTestIndexWriterReopenSegment(
    bool doFullMerge) 
{
  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<IndexReader> r1 = writer->getReader();
  TestUtil::assertEquals(0, r1->maxDoc());
  createIndexNoClose(false, L"index1", writer);
  writer->flush(!doFullMerge, true);

  shared_ptr<IndexReader> iwr1 = writer->getReader();
  TestUtil::assertEquals(100, iwr1->maxDoc());

  shared_ptr<IndexReader> r2 = writer->getReader();
  TestUtil::assertEquals(r2->maxDoc(), 100);
  // add 100 documents
  for (int x = 10000; x < 10000 + 100; x++) {
    shared_ptr<Document> d = DocHelper::createDocument(x, L"index1", 5);
    writer->addDocument(d);
  }
  writer->flush(false, true);
  // verify the reader was reopened internally
  shared_ptr<IndexReader> iwr2 = writer->getReader();
  assertTrue(iwr2 != r1);
  TestUtil::assertEquals(200, iwr2->maxDoc());
  // should have flushed out a segment
  shared_ptr<IndexReader> r3 = writer->getReader();
  assertTrue(r2 != r3);
  TestUtil::assertEquals(200, r3->maxDoc());

  // dec ref the readers rather than close them because
  // closing flushes changes to the writer
  delete r1;
  delete iwr1;
  delete r2;
  delete r3;
  delete iwr2;
  delete writer;

  // test whether the changes made it to the directory
  writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<IndexReader> w2r1 = writer->getReader();
  // insure the deletes were actually flushed to the directory
  TestUtil::assertEquals(200, w2r1->maxDoc());
  delete w2r1;
  delete writer;

  delete dir1;
}

void TestIndexWriterReader::createIndex(shared_ptr<Random> random,
                                        shared_ptr<Directory> dir1,
                                        const wstring &indexName,
                                        bool multiSegment) 
{
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir1, LuceneTestCase::newIndexWriterConfig(
                random, make_shared<MockAnalyzer>(random))
                ->setMergePolicy(make_shared<LogDocMergePolicy>()));
  for (int i = 0; i < 100; i++) {
    w->addDocument(DocHelper::createDocument(i, indexName, 4));
  }
  if (!multiSegment) {
    w->forceMerge(1);
  }
  delete w;
}

void TestIndexWriterReader::createIndexNoClose(
    bool multiSegment, const wstring &indexName,
    shared_ptr<IndexWriter> w) 
{
  for (int i = 0; i < 100; i++) {
    w->addDocument(DocHelper::createDocument(i, indexName, 4));
  }
  if (!multiSegment) {
    w->forceMerge(1);
  }
}

void TestIndexWriterReader::testMergeWarmer() 
{
  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  // Enroll warmer
  shared_ptr<AtomicInteger> warmCount = make_shared<AtomicInteger>();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMaxBufferedDocs(2)
                ->setMergedSegmentWarmer(
                    [&](leafReader) { warmCount->incrementAndGet(); })
                ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                ->setMergePolicy(newLogMergePolicy()));

  // create the index
  createIndexNoClose(false, L"test", writer);

  // get a reader to put writer into near real-time mode
  shared_ptr<IndexReader> r1 = writer->getReader();

  (std::static_pointer_cast<LogMergePolicy>(
       writer->getConfig()->getMergePolicy()))
      ->setMergeFactor(2);

  int num = TEST_NIGHTLY ? atLeast(100) : atLeast(10);
  for (int i = 0; i < num; i++) {
    writer->addDocument(DocHelper::createDocument(i, L"test", 4));
  }
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer->getConfig()->getMergeScheduler()))
      ->sync();

  assertTrue(warmCount->get() > 0);
  constexpr int count = warmCount->get();

  writer->addDocument(DocHelper::createDocument(17, L"test", 4));
  writer->forceMerge(1);
  assertTrue(warmCount->get() > count);

  delete writer;
  delete r1;
  delete dir1;
}

void TestIndexWriterReader::testAfterCommit() 
{
  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()));
  writer->commit();

  // create the index
  createIndexNoClose(false, L"test", writer);

  // get a reader to put writer into near real-time mode
  shared_ptr<DirectoryReader> r1 = writer->getReader();
  TestUtil::checkIndex(dir1);
  writer->commit();
  TestUtil::checkIndex(dir1);
  TestUtil::assertEquals(100, r1->numDocs());

  for (int i = 0; i < 10; i++) {
    writer->addDocument(DocHelper::createDocument(i, L"test", 4));
  }
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer->getConfig()->getMergeScheduler()))
      ->sync();

  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r1);
  if (r2 != nullptr) {
    r1->close();
    r1 = r2;
  }
  TestUtil::assertEquals(110, r1->numDocs());
  delete writer;
  r1->close();
  delete dir1;
}

void TestIndexWriterReader::testAfterClose() 
{
  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // create the index
  createIndexNoClose(false, L"test", writer);

  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;

  TestUtil::checkIndex(dir1);

  // reader should remain usable even after IndexWriter is closed:
  TestUtil::assertEquals(100, r->numDocs());
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(L"indexname", L"test"));
  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  TestUtil::assertEquals(100, searcher->search(q, 10)->totalHits);

  expectThrows(AlreadyClosedException::typeid,
               [&]() { DirectoryReader::openIfChanged(r); });

  r->close();
  delete dir1;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testDuringAddIndexes() throws Exception
void TestIndexWriterReader::testDuringAddIndexes() 
{
  shared_ptr<Directory> dir1 = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergePolicy(newLogMergePolicy(2)));

  // create the index
  createIndexNoClose(false, L"test", writer);
  writer->commit();

  std::deque<std::shared_ptr<Directory>> dirs(10);
  for (int i = 0; i < 10; i++) {
    dirs[i] =
        make_shared<MockDirectoryWrapper>(random(), TestUtil::ramCopyOf(dir1));
  }

  shared_ptr<DirectoryReader> r = writer->getReader();

  constexpr int numIterations = 10;
  const deque<runtime_error> excs =
      Collections::synchronizedList(deque<runtime_error>());

  // Only one thread can addIndexes at a time, because
  // IndexWriter acquires a write lock in each directory:
  std::deque<std::shared_ptr<Thread>> threads(1);
  shared_ptr<AtomicBoolean> *const threadDone =
      make_shared<AtomicBoolean>(false);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), writer, dirs, numIterations, excs, threadDone);
    threads[i]->setDaemon(true);
    threads[i]->start();
  }

  int64_t lastCount = 0;
  while (threadDone->get() == false) {
    shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
    if (r2 != nullptr) {
      r->close();
      r = r2;
      shared_ptr<Query> q =
          make_shared<TermQuery>(make_shared<Term>(L"indexname", L"test"));
      shared_ptr<IndexSearcher> searcher = newSearcher(r);
      constexpr int64_t count = searcher->search(q, 10)->totalHits;
      assertTrue(count >= lastCount);
      lastCount = count;
    }
  }

  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }
  // final check
  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  if (r2 != nullptr) {
    r->close();
    r = r2;
  }
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(L"indexname", L"test"));
  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  constexpr int64_t count = searcher->search(q, 10)->totalHits;
  assertTrue(count >= lastCount);

  TestUtil::assertEquals(0, excs.size());
  r->close();
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir1) != nullptr) {
    shared_ptr<deque<wstring>> *const openDeletedFiles =
        (std::static_pointer_cast<MockDirectoryWrapper>(dir1))
            ->getOpenDeletedFiles();
    assertEquals(L"openDeleted=" + openDeletedFiles, 0,
                 openDeletedFiles->size());
  }

  delete writer;

  delete dir1;
}

TestIndexWriterReader::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexWriterReader> outerInstance,
    shared_ptr<org::apache::lucene::index::IndexWriter> writer,
    deque<std::shared_ptr<Directory>> &dirs, int numIterations,
    deque<runtime_error> &excs, shared_ptr<AtomicBoolean> threadDone)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->dirs = dirs;
  this->numIterations = numIterations;
  this->excs = excs;
  this->threadDone = threadDone;
}

void TestIndexWriterReader::ThreadAnonymousInnerClass::run()
{
  int count = 0;
  do {
    count++;
    try {
      writer->addIndexes(dirs);
      writer->maybeMerge();
    } catch (const runtime_error &t) {
      excs.push_back(t);
      throw runtime_error(t);
    }
  } while (count < numIterations);
  threadDone->set(true);
}

shared_ptr<Directory> TestIndexWriterReader::getAssertNoDeletesDirectory(
    shared_ptr<Directory> directory)
{
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(directory) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(directory))
        ->setAssertNoDeleteOpenFile(true);
  }
  return directory;
}

void TestIndexWriterReader::testDuringAddDelete() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergePolicy(newLogMergePolicy(2)));

  // create the index
  createIndexNoClose(false, L"test", writer);
  writer->commit();

  shared_ptr<DirectoryReader> r = writer->getReader();

  constexpr int iters = TEST_NIGHTLY ? 1000 : 10;
  const deque<runtime_error> excs =
      Collections::synchronizedList(deque<runtime_error>());

  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  shared_ptr<AtomicInteger> *const remainingThreads =
      make_shared<AtomicInteger>(numThreads);
  for (int i = 0; i < numThreads; i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), writer, r, iters, excs, remainingThreads);
    threads[i]->setDaemon(true);
    threads[i]->start();
  }

  int sum = 0;
  while (remainingThreads->get() > 0) {
    shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
    if (r2 != nullptr) {
      r->close();
      r = r2;
      shared_ptr<Query> q =
          make_shared<TermQuery>(make_shared<Term>(L"indexname", L"test"));
      shared_ptr<IndexSearcher> searcher = newSearcher(r);
      sum += searcher->search(q, 10)->totalHits;
    }
  }

  for (int i = 0; i < numThreads; i++) {
    threads[i]->join();
  }
  // at least search once
  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  if (r2 != nullptr) {
    r->close();
    r = r2;
  }
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(L"indexname", L"test"));
  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  sum += searcher->search(q, 10)->totalHits;
  assertTrue(L"no documents found at all", sum > 0);

  TestUtil::assertEquals(0, excs.size());
  delete writer;

  r->close();
  delete dir1;
}

TestIndexWriterReader::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestIndexWriterReader> outerInstance,
    shared_ptr<org::apache::lucene::index::IndexWriter> writer,
    shared_ptr<org::apache::lucene::index::DirectoryReader> r, int iters,
    deque<runtime_error> &excs, shared_ptr<AtomicInteger> remainingThreads)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->r = r;
  this->iters = iters;
  this->excs = excs;
  this->remainingThreads = remainingThreads;
  r = make_shared<Random>(LuceneTestCase::random()->nextLong());
}

void TestIndexWriterReader::ThreadAnonymousInnerClass2::run()
{
  int count = 0;
  do {
    try {
      for (int docUpto = 0; docUpto < 10; docUpto++) {
        writer->addDocument(
            DocHelper::createDocument(10 * count + docUpto, L"test", 4));
      }
      count++;
      constexpr int limit = count * 10;
      for (int delUpto = 0; delUpto < 5; delUpto++) {
        int x = r->nextInt(limit);
        writer->deleteDocuments(
            {make_shared<Term>(L"field3", L"b" + to_wstring(x))});
      }
    } catch (const runtime_error &t) {
      excs.push_back(t);
      throw runtime_error(t);
    }
  } while (count < iters);
  remainingThreads->decrementAndGet();
}

void TestIndexWriterReader::testForceMergeDeletes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c", Field::Store::NO));
  shared_ptr<Field> id = newStringField(L"id", L"", Field::Store::NO);
  doc->push_back(id);
  id->setStringValue(L"0");
  w->addDocument(doc);
  id->setStringValue(L"1");
  w->addDocument(doc);
  w->deleteDocuments({make_shared<Term>(L"id", L"0")});

  shared_ptr<IndexReader> r = w->getReader();
  w->forceMergeDeletes();
  delete w;
  delete r;
  r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  assertFalse(r->hasDeletions());
  delete r;
  delete dir;
}

void TestIndexWriterReader::testDeletesNumDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c", Field::Store::NO));
  shared_ptr<Field> id = newStringField(L"id", L"", Field::Store::NO);
  doc->push_back(id);
  id->setStringValue(L"0");
  w->addDocument(doc);
  id->setStringValue(L"1");
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  TestUtil::assertEquals(2, r->numDocs());
  delete r;

  w->deleteDocuments({make_shared<Term>(L"id", L"0")});
  r = w->getReader();
  TestUtil::assertEquals(1, r->numDocs());
  delete r;

  w->deleteDocuments({make_shared<Term>(L"id", L"1")});
  r = w->getReader();
  TestUtil::assertEquals(0, r->numDocs());
  delete r;

  delete w;
  delete dir;
}

void TestIndexWriterReader::testEmptyIndex() 
{
  // Ensures that getReader works on an empty index, which hasn't been committed
  // yet.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<IndexReader> r = w->getReader();
  TestUtil::assertEquals(0, r->numDocs());
  delete r;
  delete w;
  delete dir;
}

void TestIndexWriterReader::testSegmentWarmer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<AtomicBoolean> *const didWarm = make_shared<AtomicBoolean>();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setReaderPooling(true)
               ->setMergedSegmentWarmer([&](r) {
                 shared_ptr<IndexSearcher> s = newSearcher(r);
                 shared_ptr<TopDocs> hits = s->search(
                     make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                     10);
                 TestUtil::assertEquals(20, hits->totalHits);
                 didWarm->set(true);
               })
               ->setMergePolicy(newLogMergePolicy(10)));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"foo", L"bar", Field::Store::NO));
  for (int i = 0; i < 20; i++) {
    w->addDocument(doc);
  }
  w->waitForMerges();
  delete w;
  delete dir;
  assertTrue(didWarm->get());
}

void TestIndexWriterReader::testSimpleMergedSegmentWarmer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<AtomicBoolean> *const didWarm = make_shared<AtomicBoolean>();
  shared_ptr<InfoStream> infoStream =
      make_shared<InfoStreamAnonymousInnerClass>(shared_from_this(), didWarm);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setReaderPooling(true)
               ->setInfoStream(infoStream)
               ->setMergedSegmentWarmer(
                   make_shared<SimpleMergedSegmentWarmer>(infoStream))
               ->setMergePolicy(newLogMergePolicy(10)));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"foo", L"bar", Field::Store::NO));
  for (int i = 0; i < 20; i++) {
    w->addDocument(doc);
  }
  w->waitForMerges();
  delete w;
  delete dir;
  assertTrue(didWarm->get());
}

TestIndexWriterReader::InfoStreamAnonymousInnerClass::
    InfoStreamAnonymousInnerClass(
        shared_ptr<TestIndexWriterReader> outerInstance,
        shared_ptr<AtomicBoolean> didWarm)
{
  this->outerInstance = outerInstance;
  this->didWarm = didWarm;
}

TestIndexWriterReader::InfoStreamAnonymousInnerClass::
    ~InfoStreamAnonymousInnerClass()
{
}

void TestIndexWriterReader::InfoStreamAnonymousInnerClass::message(
    const wstring &component, const wstring &message)
{
  if (L"SMSW" == component) {
    didWarm->set(true);
  }
}

bool TestIndexWriterReader::InfoStreamAnonymousInnerClass::isEnabled(
    const wstring &component)
{
  return true;
}

void TestIndexWriterReader::testReopenAfterNoRealChange() 
{
  shared_ptr<Directory> d = getAssertNoDeletesDirectory(newDirectory());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<DirectoryReader> r = w->getReader(); // start pooling readers

  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  assertNull(r2);

  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> r3 = DirectoryReader::openIfChanged(r);
  assertNotNull(r3);
  assertTrue(r3->getVersion() != r->getVersion());
  assertTrue(r3->isCurrent());

  // Deletes nothing in reality...:
  w->deleteDocuments({make_shared<Term>(L"foo", L"bar")});

  // ... but IW marks this as not current:
  assertFalse(r3->isCurrent());
  shared_ptr<DirectoryReader> r4 = DirectoryReader::openIfChanged(r3);
  assertNull(r4);

  // Deletes nothing in reality...:
  w->deleteDocuments({make_shared<Term>(L"foo", L"bar")});
  shared_ptr<DirectoryReader> r5 = DirectoryReader::openIfChanged(r3, w);
  assertNull(r5);

  r3->close();

  delete w;
  delete d;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNRTOpenExceptions() throws Exception
void TestIndexWriterReader::testNRTOpenExceptions() 
{
  // LUCENE-5262: test that several failed attempts to obtain an NRT reader
  // don't leak file handles.
  shared_ptr<MockDirectoryWrapper> dir =
      std::static_pointer_cast<MockDirectoryWrapper>(
          getAssertNoDeletesDirectory(newMockDirectory()));
  shared_ptr<AtomicBoolean> *const shouldFail = make_shared<AtomicBoolean>();
  dir->failOn(make_shared<FailureAnonymousInnerClass>(shared_from_this(), dir,
                                                      shouldFail));

  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergePolicy(
      NoMergePolicy::INSTANCE); // prevent merges from getting in the way
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  // create a segment and open an NRT reader
  writer->addDocument(make_shared<Document>());
  writer->getReader()->close();

  // add a new document so a new NRT reader is required
  writer->addDocument(make_shared<Document>());

  // try to obtain an NRT reader twice: first time it fails and closes all the
  // other NRT readers. second time it fails, but also fails to close the
  // other NRT reader, since it is already marked closed!
  for (int i = 0; i < 2; i++) {
    shouldFail->set(true);
    expectThrows(FakeIOException::typeid,
                 [&]() { writer->getReader()->close(); });
  }

  delete writer;
  delete dir;
}

TestIndexWriterReader::FailureAnonymousInnerClass::FailureAnonymousInnerClass(
    shared_ptr<TestIndexWriterReader> outerInstance,
    shared_ptr<MockDirectoryWrapper> dir, shared_ptr<AtomicBoolean> shouldFail)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->shouldFail = shouldFail;
}

void TestIndexWriterReader::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: StackTraceElement[] trace = new Exception().getStackTrace();
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      (runtime_error())->getStackTrace();
  if (shouldFail->get()) {
    for (int i = 0; i < trace.size(); i++) {
      if (L"getReadOnlyClone" == trace[i]->getMethodName()) {
        if (VERBOSE) {
          wcout << L"TEST: now fail; exc:" << endl;
          // C++ TODO: This exception's constructor requires an argument:
          // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
          (runtime_error())->printStackTrace(System::out);
        }
        shouldFail->set(false);
        throw make_shared<FakeIOException>();
      }
    }
  }
}

void TestIndexWriterReader::testTooManySegments() 
{
  shared_ptr<Directory> dir =
      getAssertNoDeletesDirectory(make_shared<RAMDirectory>());
  // Don't use newIndexWriterConfig, because we need a
  // "sane" mergePolicy:
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  // Create 500 segments:
  for (int i = 0; i < 500; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
    shared_ptr<IndexReader> r = DirectoryReader::open(w);
    // Make sure segment count never exceeds 100:
    assertTrue(r->leaves().size() < 100);
    delete r;
  }
  delete w;
  delete dir;
}

void TestIndexWriterReader::testReopenNRTReaderOnCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  w->addDocument(make_shared<Document>());

  // Pull NRT reader; it has 1 segment:
  shared_ptr<DirectoryReader> r1 = DirectoryReader::open(w);
  TestUtil::assertEquals(1, r1->leaves()->size());
  w->addDocument(make_shared<Document>());
  w->commit();

  deque<std::shared_ptr<IndexCommit>> commits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(1, commits.size());
  shared_ptr<DirectoryReader> r2 =
      DirectoryReader::openIfChanged(r1, commits[0]);
  TestUtil::assertEquals(2, r2->leaves()->size());

  // Make sure we shared same instance of SegmentReader w/ first reader:
  assertTrue(r1->leaves()->get(0).reader() == r2->leaves()->get(0).reader());
  r1->close();
  r2->close();
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index