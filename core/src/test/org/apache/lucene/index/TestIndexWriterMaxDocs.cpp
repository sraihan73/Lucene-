using namespace std;

#include "TestIndexWriterMaxDocs.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using NoLockFactory = org::apache::lucene::store::NoLockFactory;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Monster("takes over two hours") public void
// testExactlyAtTrueLimit() throws Exception
void TestIndexWriterMaxDocs::testExactlyAtTrueLimit() 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir(L"2BDocs3"));
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"text", Field::Store::NO));
  for (int i = 0; i < IndexWriter::MAX_DOCS; i++) {
    iw->addDocument(doc);
    /*
    if (i%1000000 == 0) {
      System.out.println((i/1000000) + " M docs...");
    }
    */
  }
  iw->commit();

  // First unoptimized, then optimized:
  for (int i = 0; i < 2; i++) {
    shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
    TestUtil::assertEquals(IndexWriter::MAX_DOCS, ir->maxDoc());
    TestUtil::assertEquals(IndexWriter::MAX_DOCS, ir->numDocs());
    shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(ir);
    shared_ptr<TopDocs> hits = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"field", L"text")), 10);
    TestUtil::assertEquals(IndexWriter::MAX_DOCS, hits->totalHits);

    // Sort by docID reversed:
    hits = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"field", L"text")), 10,
        make_shared<Sort>(
            make_shared<SortField>(nullptr, SortField::Type::DOC, true)));
    TestUtil::assertEquals(IndexWriter::MAX_DOCS, hits->totalHits);
    TestUtil::assertEquals(10, hits->scoreDocs.size());
    TestUtil::assertEquals(IndexWriter::MAX_DOCS - 1, hits->scoreDocs[0]->doc);
    ir->close();

    iw->forceMerge(1);
  }

  delete iw;
  delete dir;
}

void TestIndexWriterMaxDocs::testAddDocument() 
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      w->addDocument(make_shared<Document>());
    }

    // 11th document should fail:
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testAddDocuments() 
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      w->addDocument(make_shared<Document>());
    }

    // 11th document should fail:
    expectThrows(invalid_argument::typeid, [&]() {
      w->addDocuments(Collections::singletonList(make_shared<Document>()));
    });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testUpdateDocument() 
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      w->addDocument(make_shared<Document>());
    }

    // 11th document should fail:
    expectThrows(invalid_argument::typeid, [&]() {
      w->updateDocument(make_shared<Term>(L"field", L"foo"),
                        make_shared<Document>());
    });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testUpdateDocuments() 
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      w->addDocument(make_shared<Document>());
    }

    // 11th document should fail:
    expectThrows(invalid_argument::typeid, [&]() {
      w->updateDocuments(make_shared<Term>(L"field", L"foo"),
                         Collections::singletonList(make_shared<Document>()));
    });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testReclaimedDeletes() 
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
      w->addDocument(doc);
    }

    // Delete 5 of them:
    for (int i = 0; i < 5; i++) {
      w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
    }

    w->forceMerge(1);

    TestUtil::assertEquals(5, w->maxDoc());

    // Add 5 more docs
    for (int i = 0; i < 5; i++) {
      w->addDocument(make_shared<Document>());
    }

    // 11th document should fail:
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testReclaimedDeletesWholeSegments() throw(
    runtime_error)
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
    iwc->setMergePolicy(NoMergePolicy::INSTANCE);
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
      w->addDocument(doc);
      if (i % 2 == 0) {
        // Make a new segment every 2 docs:
        w->commit();
      }
    }

    // Delete 5 of them:
    for (int i = 0; i < 5; i++) {
      w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
    }

    w->forceMerge(1);

    TestUtil::assertEquals(5, w->maxDoc());

    // Add 5 more docs
    for (int i = 0; i < 5; i++) {
      w->addDocument(make_shared<Document>());
    }

    // 11th document should fail:
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testAddIndexes() 
{
  setIndexWriterMaxDocs(10);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      w->addDocument(make_shared<Document>());
    }
    delete w;

    shared_ptr<Directory> dir2 = newDirectory();
    shared_ptr<IndexWriter> w2 =
        make_shared<IndexWriter>(dir2, make_shared<IndexWriterConfig>(nullptr));
    w2->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid, [&]() {
      w2->addIndexes(std::deque<std::shared_ptr<Directory>>{dir});
    });

    TestUtil::assertEquals(1, w2->maxDoc());
    shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
    expectThrows(invalid_argument::typeid,
                 [&]() { TestUtil::addIndexesSlowly(w2, {ir}); });

    delete w2;
    ir->close();
    delete dir;
    delete dir2;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testMultiReaderExactLimit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  for (int i = 0; i < 100000; i++) {
    w->addDocument(doc);
  }
  delete w;

  int remainder = IndexWriter::MAX_DOCS % 100000;
  shared_ptr<Directory> dir2 = newDirectory();
  w = make_shared<IndexWriter>(dir2, make_shared<IndexWriterConfig>(nullptr));
  for (int i = 0; i < remainder; i++) {
    w->addDocument(doc);
  }
  delete w;

  int copies = IndexWriter::MAX_DOCS / 100000;

  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  shared_ptr<DirectoryReader> ir2 = DirectoryReader::open(dir2);
  std::deque<std::shared_ptr<IndexReader>> subReaders(copies + 1);
  Arrays::fill(subReaders, ir);
  subReaders[subReaders.size() - 1] = ir2;

  shared_ptr<MultiReader> mr = make_shared<MultiReader>(subReaders);
  TestUtil::assertEquals(IndexWriter::MAX_DOCS, mr->maxDoc());
  TestUtil::assertEquals(IndexWriter::MAX_DOCS, mr->numDocs());
  ir->close();
  ir2->close();
  delete dir;
  delete dir2;
}

void TestIndexWriterMaxDocs::testMultiReaderBeyondLimit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  for (int i = 0; i < 100000; i++) {
    w->addDocument(doc);
  }
  delete w;

  int remainder = IndexWriter::MAX_DOCS % 100000;

  // One too many:
  remainder++;

  shared_ptr<Directory> dir2 = newDirectory();
  w = make_shared<IndexWriter>(dir2, make_shared<IndexWriterConfig>(nullptr));
  for (int i = 0; i < remainder; i++) {
    w->addDocument(doc);
  }
  delete w;

  int copies = IndexWriter::MAX_DOCS / 100000;

  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  shared_ptr<DirectoryReader> ir2 = DirectoryReader::open(dir2);
  std::deque<std::shared_ptr<IndexReader>> subReaders(copies + 1);
  Arrays::fill(subReaders, ir);
  subReaders[subReaders.size() - 1] = ir2;

  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<MultiReader>(subReaders); });

  ir->close();
  ir2->close();
  delete dir;
  delete dir2;
}

void TestIndexWriterMaxDocs::testAddTooManyIndexesDir() 
{
  // we cheat and add the same one over again... IW wants a write lock on each
  shared_ptr<Directory> dir = newDirectory(random(), NoLockFactory::INSTANCE);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  for (int i = 0; i < 100000; i++) {
    w->addDocument(doc);
  }
  w->forceMerge(1);
  w->commit();
  delete w;

  // wrap this with disk full, so test fails faster and doesn't fill up real
  // disks.
  shared_ptr<MockDirectoryWrapper> dir2 = newMockDirectory();
  w = make_shared<IndexWriter>(dir2, make_shared<IndexWriterConfig>(nullptr));
  w->commit(); // don't confuse checkindex
  dir2->setMaxSizeInBytes(dir2->sizeInBytes() + 65536); // 64KB
  std::deque<std::shared_ptr<Directory>> dirs(
      1 + (IndexWriter::MAX_DOCS / 100000));
  for (int i = 0; i < dirs.size(); i++) {
    // bypass iw check for duplicate dirs
    dirs[i] = make_shared<FilterDirectoryAnonymousInnerClass>(
        shared_from_this(), dir);
  }

  try {
    w->addIndexes(dirs);
    fail(L"didn't get expected exception");
  } catch (const invalid_argument &expected) {
    // pass
  } catch (const IOException &fakeDiskFull) {
    constexpr runtime_error e;
    if (fakeDiskFull->getMessage() != nullptr &&
        fakeDiskFull->getMessage()->startsWith(L"fake disk full")) {
      e = runtime_error(L"test failed: IW checks aren't working and we are "
                        L"executing addIndexes");
      e.addSuppressed(fakeDiskFull);
    } else {
      e = fakeDiskFull;
    }
    throw e;
  }

  delete w;
  delete dir;
  delete dir2;
}

TestIndexWriterMaxDocs::FilterDirectoryAnonymousInnerClass::
    FilterDirectoryAnonymousInnerClass(
        shared_ptr<TestIndexWriterMaxDocs> outerInstance,
        shared_ptr<Directory> dir)
    : org::apache::lucene::store::FilterDirectory(dir)
{
  this->outerInstance = outerInstance;
}

void TestIndexWriterMaxDocs::testAddTooManyIndexesCodecReader() throw(
    runtime_error)
{
  // we cheat and add the same one over again... IW wants a write lock on each
  shared_ptr<Directory> dir = newDirectory(random(), NoLockFactory::INSTANCE);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  for (int i = 0; i < 100000; i++) {
    w->addDocument(doc);
  }
  w->forceMerge(1);
  w->commit();
  delete w;

  // wrap this with disk full, so test fails faster and doesn't fill up real
  // disks.
  shared_ptr<MockDirectoryWrapper> dir2 = newMockDirectory();
  w = make_shared<IndexWriter>(dir2, make_shared<IndexWriterConfig>(nullptr));
  w->commit(); // don't confuse checkindex
  dir2->setMaxSizeInBytes(dir2->sizeInBytes() + 65536); // 64KB
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<CodecReader> segReader =
      std::static_pointer_cast<CodecReader>(r->leaves()[0]->reader());

  std::deque<std::shared_ptr<CodecReader>> readers(
      1 + (IndexWriter::MAX_DOCS / 100000));
  for (int i = 0; i < readers.size(); i++) {
    readers[i] = segReader;
  }

  try {
    w->addIndexes(readers);
    fail(L"didn't get expected exception");
  } catch (const invalid_argument &expected) {
    // pass
  } catch (const IOException &fakeDiskFull) {
    constexpr runtime_error e;
    if (fakeDiskFull->getMessage() != nullptr &&
        fakeDiskFull->getMessage()->startsWith(L"fake disk full")) {
      e = runtime_error(L"test failed: IW checks aren't working and we are "
                        L"executing addIndexes");
      e.addSuppressed(fakeDiskFull);
    } else {
      e = fakeDiskFull;
    }
    throw e;
  }

  delete r;
  delete w;
  delete dir;
  delete dir2;
}

void TestIndexWriterMaxDocs::testTooLargeMaxDocs() 
{
  expectThrows(invalid_argument::typeid,
               [&]() { IndexWriter::setMaxDocs(numeric_limits<int>::max()); });
}

void TestIndexWriterMaxDocs::testDeleteAll() 
{
  setIndexWriterMaxDocs(1);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    w->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    w->deleteAll();
    w->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testDeleteAllAfterFlush() 
{
  setIndexWriterMaxDocs(2);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    w->addDocument(make_shared<Document>());
    w->getReader()->close();
    w->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    w->deleteAll();
    w->addDocument(make_shared<Document>());
    w->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testDeleteAllAfterCommit() 
{
  setIndexWriterMaxDocs(2);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    w->addDocument(make_shared<Document>());
    w->commit();
    w->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    w->deleteAll();
    w->addDocument(make_shared<Document>());
    w->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testDeleteAllMultipleThreads() 
{
  int limit = TestUtil::nextInt(random(), 2, 10);
  setIndexWriterMaxDocs(limit);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));

    shared_ptr<CountDownLatch> startingGun = make_shared<CountDownLatch>(1);
    std::deque<std::shared_ptr<Thread>> threads(limit);
    for (int i = 0; i < limit; i++) {
      threads[i] = make_shared<ThreadAnonymousInnerClass>(shared_from_this(), w,
                                                          startingGun);
      threads[i]->start();
    }

    startingGun->countDown();

    for (auto thread : threads) {
      thread->join();
    }

    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    w->deleteAll();
    for (int i = 0; i < limit; i++) {
      w->addDocument(make_shared<Document>());
    }
    expectThrows(invalid_argument::typeid,
                 [&]() { w->addDocument(make_shared<Document>()); });

    delete w;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

TestIndexWriterMaxDocs::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexWriterMaxDocs> outerInstance,
    shared_ptr<org::apache::lucene::index::IndexWriter> w,
    shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
}

void TestIndexWriterMaxDocs::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    w->addDocument(make_shared<Document>());
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestIndexWriterMaxDocs::testDeleteAllAfterClose() 
{
  setIndexWriterMaxDocs(2);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    w->addDocument(make_shared<Document>());
    delete w;

    shared_ptr<IndexWriter> w2 =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    w2->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w2->addDocument(make_shared<Document>()); });

    w2->deleteAll();
    w2->addDocument(make_shared<Document>());
    w2->addDocument(make_shared<Document>());
    expectThrows(invalid_argument::typeid,
                 [&]() { w2->addDocument(make_shared<Document>()); });

    delete w2;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testAcrossTwoIndexWriters() 
{
  setIndexWriterMaxDocs(1);
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> w =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    w->addDocument(make_shared<Document>());
    delete w;
    shared_ptr<IndexWriter> w2 =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    expectThrows(invalid_argument::typeid,
                 [&]() { w2->addDocument(make_shared<Document>()); });

    delete w2;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }
}

void TestIndexWriterMaxDocs::testCorruptIndexExceptionTooLarge() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  delete w;

  setIndexWriterMaxDocs(1);
  try {
    expectThrows(CorruptIndexException::typeid,
                 [&]() { DirectoryReader::open(dir); });
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }

  delete dir;
}

void TestIndexWriterMaxDocs::testCorruptIndexExceptionTooLargeWriter() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  delete w;

  setIndexWriterMaxDocs(1);
  try {
    expectThrows(CorruptIndexException::typeid, [&]() {
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
    });
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    restoreIndexWriterMaxDocs();
  }

  delete dir;
}
} // namespace org::apache::lucene::index