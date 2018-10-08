using namespace std;

#include "TestIndexWriterDelete.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Ignore;

void TestIndexWriterDelete::testSimpleCase() 
{
  std::deque<wstring> keywords = {L"1", L"2"};
  std::deque<wstring> unindexed = {L"Netherlands", L"Italy"};
  std::deque<wstring> unstored = {L"Amsterdam has lots of bridges",
                                   L"Venice has lots of canals"};
  std::deque<wstring> text = {L"Amsterdam", L"Venice"};

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
               random(), MockTokenizer::WHITESPACE, false)));

  shared_ptr<FieldType> custom1 = make_shared<FieldType>();
  custom1->setStored(true);
  for (int i = 0; i < keywords.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", keywords[i], Field::Store::YES));
    doc->push_back(newField(L"country", unindexed[i], custom1));
    doc->push_back(newTextField(L"contents", unstored[i], Field::Store::NO));
    doc->push_back(newTextField(L"city", text[i], Field::Store::YES));
    modifier->addDocument(doc);
  }
  modifier->forceMerge(1);
  modifier->commit();

  shared_ptr<Term> term = make_shared<Term>(L"city", L"Amsterdam");
  int64_t hitCount = getHitCount(dir, term);
  TestUtil::assertEquals(1, hitCount);
  if (VERBOSE) {
    wcout << L"\nTEST: now delete by term=" << term << endl;
  }
  modifier->deleteDocuments({term});
  modifier->commit();

  if (VERBOSE) {
    wcout << L"\nTEST: now getHitCount" << endl;
  }
  hitCount = getHitCount(dir, term);
  TestUtil::assertEquals(0, hitCount);

  delete modifier;
  delete dir;
}

void TestIndexWriterDelete::testNonRAMDelete() 
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(2));
  int id = 0;
  int value = 100;

  for (int i = 0; i < 7; i++) {
    addDoc(modifier, ++id, value);
  }
  modifier->commit();

  TestUtil::assertEquals(0, modifier->getNumBufferedDocuments());
  assertTrue(0 < modifier->getSegmentCount());

  modifier->commit();

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  modifier->deleteDocuments({make_shared<Term>(L"value", to_wstring(value))});

  modifier->commit();

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());
  delete reader;
  delete modifier;
  delete dir;
}

void TestIndexWriterDelete::testRAMDeletes() 
{
  for (int t = 0; t < 2; t++) {
    if (VERBOSE) {
      wcout << L"TEST: t=" << t << endl;
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
        dir,
        newIndexWriterConfig(make_shared<MockAnalyzer>(
                                 random(), MockTokenizer::WHITESPACE, false))
            ->setMaxBufferedDocs(4));
    int id = 0;
    int value = 100;

    addDoc(modifier, ++id, value);
    if (0 == t) {
      modifier->deleteDocuments(
          {make_shared<Term>(L"value", to_wstring(value))});
    } else {
      modifier->deleteDocuments({make_shared<TermQuery>(
          make_shared<Term>(L"value", to_wstring(value)))});
    }
    addDoc(modifier, ++id, value);
    if (0 == t) {
      modifier->deleteDocuments(
          {make_shared<Term>(L"value", to_wstring(value))});
      TestUtil::assertEquals(2, modifier->getNumBufferedDeleteTerms());
      TestUtil::assertEquals(1, modifier->getBufferedDeleteTermsSize());
    } else {
      modifier->deleteDocuments({make_shared<TermQuery>(
          make_shared<Term>(L"value", to_wstring(value)))});
    }

    addDoc(modifier, ++id, value);
    TestUtil::assertEquals(0, modifier->getSegmentCount());
    modifier->commit();

    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    TestUtil::assertEquals(1, reader->numDocs());

    int64_t hitCount =
        getHitCount(dir, make_shared<Term>(L"id", to_wstring(id)));
    TestUtil::assertEquals(1, hitCount);
    delete reader;
    delete modifier;
    delete dir;
  }
}

void TestIndexWriterDelete::testBothDeletes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(100));

  int id = 0;
  int value = 100;

  for (int i = 0; i < 5; i++) {
    addDoc(modifier, ++id, value);
  }

  value = 200;
  for (int i = 0; i < 5; i++) {
    addDoc(modifier, ++id, value);
  }
  modifier->commit();

  for (int i = 0; i < 5; i++) {
    addDoc(modifier, ++id, value);
  }
  modifier->deleteDocuments({make_shared<Term>(L"value", to_wstring(value))});

  modifier->commit();

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(5, reader->numDocs());
  delete modifier;
  delete reader;
  delete dir;
}

void TestIndexWriterDelete::testBatchDeletes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(2));

  int id = 0;
  int value = 100;

  for (int i = 0; i < 7; i++) {
    addDoc(modifier, ++id, value);
  }
  modifier->commit();

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  id = 0;
  modifier->deleteDocuments({make_shared<Term>(L"id", to_wstring(++id))});
  modifier->deleteDocuments({make_shared<Term>(L"id", to_wstring(++id))});

  modifier->commit();

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(5, reader->numDocs());
  delete reader;

  std::deque<std::shared_ptr<Term>> terms(3);
  for (int i = 0; i < terms.size(); i++) {
    terms[i] = make_shared<Term>(L"id", to_wstring(++id));
  }
  modifier->deleteDocuments(terms);
  modifier->commit();
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(2, reader->numDocs());
  delete reader;

  delete modifier;
  delete dir;
}

void TestIndexWriterDelete::testDeleteAllSimple() 
{
  if (VERBOSE) {
    wcout << L"TEST: now start" << endl;
  }
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(2));

  int id = 0;
  int value = 100;

  for (int i = 0; i < 7; i++) {
    addDoc(modifier, ++id, value);
  }
  if (VERBOSE) {
    wcout << L"TEST: now commit" << endl;
  }
  modifier->commit();

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  // Add 1 doc (so we will have something buffered)
  addDoc(modifier, 99, value);

  // Delete all
  if (VERBOSE) {
    wcout << L"TEST: now delete all" << endl;
  }
  modifier->deleteAll();

  // Delete all shouldn't be on disk yet
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  // Add a doc and update a doc (after the deleteAll, before the commit)
  addDoc(modifier, 101, value);
  updateDoc(modifier, 102, value);
  if (VERBOSE) {
    wcout << L"TEST: now 2nd commit" << endl;
  }

  // commit the delete all
  modifier->commit();

  // Validate there are no docs left
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(2, reader->numDocs());
  delete reader;

  delete modifier;
  delete dir;
}

void TestIndexWriterDelete::testDeleteAllNoDeadLock() throw(
    IOException, InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const modifier =
      make_shared<RandomIndexWriter>(random(), dir);
  int numThreads = atLeast(2);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  shared_ptr<CountDownLatch> *const latch = make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> *const doneLatch =
      make_shared<CountDownLatch>(numThreads);
  for (int i = 0; i < numThreads; i++) {
    constexpr int offset = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), modifier, latch, doneLatch, offset);
    threads[i]->start();
  }
  latch->countDown();
  while (!doneLatch->await(1, TimeUnit::MILLISECONDS)) {
    if (VERBOSE) {
      wcout << L"\nTEST: now deleteAll" << endl;
    }
    modifier->deleteAll();
    if (VERBOSE) {
      wcout << L"del all" << endl;
    }
  }

  if (VERBOSE) {
    wcout << L"\nTEST: now final deleteAll" << endl;
  }

  modifier->deleteAll();
  for (auto thread : threads) {
    thread->join();
  }

  if (VERBOSE) {
    wcout << L"\nTEST: now close" << endl;
  }
  delete modifier;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  if (VERBOSE) {
    wcout << L"\nTEST: got reader=" << reader << endl;
  }
  TestUtil::assertEquals(0, reader->maxDoc());
  TestUtil::assertEquals(0, reader->numDocs());
  assertEquals(0, reader->numDeletedDocs(), 0);
  reader->close();

  delete dir;
}

TestIndexWriterDelete::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexWriterDelete> outerInstance,
    shared_ptr<org::apache::lucene::index::RandomIndexWriter> modifier,
    shared_ptr<CountDownLatch> latch, shared_ptr<CountDownLatch> doneLatch,
    int offset)
{
  this->outerInstance = outerInstance;
  this->modifier = modifier;
  this->latch = latch;
  this->doneLatch = doneLatch;
  this->offset = offset;
}

void TestIndexWriterDelete::ThreadAnonymousInnerClass::run()
{
  int id = offset * 1000;
  int value = 100;
  try {
    latch->await();
    for (int j = 0; j < 1000; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          LuceneTestCase::newTextField(L"content", L"aaa", Field::Store::NO));
      doc->push_back(LuceneTestCase::newStringField(L"id", to_wstring(id++),
                                                    Field::Store::YES));
      doc->push_back(LuceneTestCase::newStringField(L"value", to_wstring(value),
                                                    Field::Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"dv", value));
      modifier->addDocument(doc);
      if (VERBOSE) {
        wcout << L"\tThread[" << offset << L"]: add doc: " << id << endl;
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    doneLatch->countDown();
    if (VERBOSE) {
      wcout << L"\tThread[" << offset << L"]: done indexing" << endl;
    }
  }
}

void TestIndexWriterDelete::testDeleteAllRollback() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(2));

  int id = 0;
  int value = 100;

  for (int i = 0; i < 7; i++) {
    addDoc(modifier, ++id, value);
  }
  modifier->commit();

  addDoc(modifier, ++id, value);

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  // Delete all
  modifier->deleteAll();

  // Roll it back
  modifier->rollback();

  // Validate that the docs are still there
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  delete dir;
}

void TestIndexWriterDelete::testDeleteAllNRT() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(2));

  int id = 0;
  int value = 100;

  for (int i = 0; i < 7; i++) {
    addDoc(modifier, ++id, value);
  }
  modifier->commit();

  shared_ptr<IndexReader> reader = modifier->getReader();
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  addDoc(modifier, ++id, value);
  addDoc(modifier, ++id, value);

  // Delete all
  modifier->deleteAll();

  reader = modifier->getReader();
  TestUtil::assertEquals(0, reader->numDocs());
  delete reader;

  // Roll it back
  modifier->rollback();

  // Validate that the docs are still there
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(7, reader->numDocs());
  delete reader;

  delete dir;
}

void TestIndexWriterDelete::updateDoc(shared_ptr<IndexWriter> modifier, int id,
                                      int value) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  doc->push_back(newStringField(L"id", to_wstring(id), Field::Store::YES));
  doc->push_back(newStringField(L"value", to_wstring(value), Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", value));
  modifier->updateDocument(make_shared<Term>(L"id", to_wstring(id)), doc);
}

void TestIndexWriterDelete::addDoc(shared_ptr<IndexWriter> modifier, int id,
                                   int value) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  doc->push_back(newStringField(L"id", to_wstring(id), Field::Store::YES));
  doc->push_back(newStringField(L"value", to_wstring(value), Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", value));
  modifier->addDocument(doc);
}

int64_t
TestIndexWriterDelete::getHitCount(shared_ptr<Directory> dir,
                                   shared_ptr<Term> term) 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  int64_t hitCount =
      searcher->search(make_shared<TermQuery>(term), 1000)->totalHits;
  delete reader;
  return hitCount;
}

void TestIndexWriterDelete::testDeletesOnDiskFull() 
{
  doTestOperationsOnDiskFull(false);
}

void TestIndexWriterDelete::testUpdatesOnDiskFull() 
{
  doTestOperationsOnDiskFull(true);
}

void TestIndexWriterDelete::doTestOperationsOnDiskFull(bool updates) throw(
    IOException)
{

  shared_ptr<Term> searchTerm = make_shared<Term>(L"content", L"aaa");
  int START_COUNT = 157;
  int END_COUNT = 144;

  // First build up a starting index:
  shared_ptr<MockDirectoryWrapper> startDir = newMockDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      startDir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                    random(), MockTokenizer::WHITESPACE, false)));
  for (int i = 0; i < 157; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(
        newStringField(L"id", Integer::toString(i), Field::Store::YES));
    d->push_back(
        newTextField(L"content", L"aaa " + to_wstring(i), Field::Store::NO));
    d->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->addDocument(d);
  }
  delete writer;

  int64_t diskUsage = startDir->sizeInBytes();
  int64_t diskFree = diskUsage + 10;

  shared_ptr<IOException> err = nullptr;

  bool done = false;

  // Iterate w/ ever increasing free disk space:
  while (!done) {
    if (VERBOSE) {
      wcout << L"TEST: cycle" << endl;
    }
    shared_ptr<MockDirectoryWrapper> dir = make_shared<MockDirectoryWrapper>(
        random(), TestUtil::ramCopyOf(startDir));
    dir->setAllowRandomFileNotFoundException(false);
    shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
        dir,
        newIndexWriterConfig(make_shared<MockAnalyzer>(
                                 random(), MockTokenizer::WHITESPACE, false))
            ->setMaxBufferedDocs(1000)
            ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()));
    (std::static_pointer_cast<ConcurrentMergeScheduler>(
         modifier->getConfig()->getMergeScheduler()))
        ->setSuppressExceptions();

    // For each disk size, first try to commit against
    // dir that will hit random IOExceptions & disk
    // full; after, give it infinite disk space & turn
    // off random IOExceptions & retry w/ same reader:
    bool success = false;

    for (int x = 0; x < 2; x++) {
      if (VERBOSE) {
        wcout << L"TEST: x=" << x << endl;
      }

      double rate = 0.1;
      double diskRatio = (static_cast<double>(diskFree)) / diskUsage;
      int64_t thisDiskFree;
      wstring testName;

      if (0 == x) {
        thisDiskFree = diskFree;
        if (diskRatio >= 2.0) {
          rate /= 2;
        }
        if (diskRatio >= 4.0) {
          rate /= 2;
        }
        if (diskRatio >= 6.0) {
          rate = 0.0;
        }
        if (VERBOSE) {
          wcout << L"\ncycle: " << diskFree << L" bytes" << endl;
        }
        testName = L"disk full during reader.close() @ " +
                   to_wstring(thisDiskFree) + L" bytes";
        dir->setRandomIOExceptionRateOnOpen(random()->nextDouble() * 0.01);
      } else {
        thisDiskFree = 0;
        rate = 0.0;
        if (VERBOSE) {
          wcout << L"\ncycle: same writer: unlimited disk space" << endl;
        }
        testName = L"reader re-use after disk full";
        dir->setRandomIOExceptionRateOnOpen(0.0);
      }

      dir->setMaxSizeInBytes(thisDiskFree);
      dir->setRandomIOExceptionRate(rate);

      try {
        if (0 == x) {
          int docId = 12;
          for (int i = 0; i < 13; i++) {
            if (updates) {
              shared_ptr<Document> d = make_shared<Document>();
              // C++ TODO: There is no native C++ equivalent to 'toString':
              d->push_back(newStringField(L"id", Integer::toString(i),
                                          Field::Store::YES));
              d->push_back(newTextField(L"content", L"bbb " + to_wstring(i),
                                        Field::Store::NO));
              d->push_back(make_shared<NumericDocValuesField>(L"dv", i));
              // C++ TODO: There is no native C++ equivalent to 'toString':
              modifier->updateDocument(
                  make_shared<Term>(L"id", Integer::toString(docId)), d);
            } else { // deletes
              // C++ TODO: There is no native C++ equivalent to 'toString':
              modifier->deleteDocuments(
                  {make_shared<Term>(L"id", Integer::toString(docId))});
              // modifier.setNorm(docId, "contents", (float)2.0);
            }
            docId += 12;
          }
          try {
            delete modifier;
          } catch (const IllegalStateException &ise) {
            // ok
            throw std::static_pointer_cast<IOException>(ise->getCause());
          }
        }
        success = true;
        if (0 == x) {
          done = true;
        }
      } catch (const IOException &e) {
        if (VERBOSE) {
          wcout << L"  hit IOException: " << e << endl;
          e->printStackTrace(System::out);
        }
        err = e;
        if (1 == x) {
          e->printStackTrace();
          fail(testName + L" hit IOException after disk space was freed up");
        }
      }
      // prevent throwing a random exception here!!
      constexpr double randomIOExceptionRate = dir->getRandomIOExceptionRate();
      constexpr int64_t maxSizeInBytes = dir->getMaxSizeInBytes();
      dir->setRandomIOExceptionRate(0.0);
      dir->setRandomIOExceptionRateOnOpen(0.0);
      dir->setMaxSizeInBytes(0);
      if (!success) {
        // Must force the close else the writer can have
        // open files which cause exc in MockRAMDir.close
        if (VERBOSE) {
          wcout << L"TEST: now rollback" << endl;
        }
        modifier->rollback();
      }

      // If the close() succeeded, make sure index is OK:
      if (success) {
        TestUtil::checkIndex(dir);
      }
      dir->setRandomIOExceptionRate(randomIOExceptionRate);
      dir->setMaxSizeInBytes(maxSizeInBytes);

      // Finally, verify index is not corrupt, and, if
      // we succeeded, we see all docs changed, and if
      // we failed, we see either all docs or no docs
      // changed (transactional semantics):
      shared_ptr<IndexReader> newReader = nullptr;
      try {
        newReader = DirectoryReader::open(dir);
      } catch (const IOException &e) {
        e->printStackTrace();
        fail(testName +
             L":exception when creating IndexReader after disk full during "
             L"close: " +
             e);
      }

      shared_ptr<IndexSearcher> searcher = newSearcher(newReader);
      std::deque<std::shared_ptr<ScoreDoc>> hits;
      try {
        hits = searcher->search(make_shared<TermQuery>(searchTerm), 1000)
                   ->scoreDocs;
      } catch (const IOException &e) {
        e->printStackTrace();
        fail(testName + L": exception when searching: " + e);
      }
      int result2 = hits.size();
      if (success) {
        if (x == 0 && result2 != END_COUNT) {
          fail(testName +
               L": method did not throw exception but hits.length for search "
               L"on term 'aaa' is " +
               to_wstring(result2) + L" instead of expected " +
               to_wstring(END_COUNT));
        } else if (x == 1 && result2 != START_COUNT && result2 != END_COUNT) {
          // It's possible that the first exception was
          // "recoverable" wrt pending deletes, in which
          // case the pending deletes are retained and
          // then re-flushing (with plenty of disk
          // space) will succeed in flushing the
          // deletes:
          fail(testName +
               L": method did not throw exception but hits.length for search "
               L"on term 'aaa' is " +
               to_wstring(result2) + L" instead of expected " +
               to_wstring(START_COUNT) + L" or " + to_wstring(END_COUNT));
        }
      } else {
        // On hitting exception we still may have added
        // all docs:
        if (result2 != START_COUNT && result2 != END_COUNT) {
          err->printStackTrace();
          fail(testName +
               L": method did throw exception but hits.length for search on "
               L"term 'aaa' is " +
               to_wstring(result2) + L" instead of expected " +
               to_wstring(START_COUNT) + L" or " + to_wstring(END_COUNT));
        }
      }
      delete newReader;
      if (result2 == END_COUNT) {
        break;
      }
    }
    delete dir;

    // Try again with 10 more bytes of free space:
    diskFree += 10;
  }
  delete startDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testErrorAfterApplyDeletes() throws
// java.io.IOException
void TestIndexWriterDelete::testErrorAfterApplyDeletes() 
{

  shared_ptr<MockDirectoryWrapper::Failure> failure =
      make_shared<FailureAnonymousInnerClass>(shared_from_this());

  // create a couple of files

  std::deque<wstring> keywords = {L"1", L"2"};
  std::deque<wstring> unindexed = {L"Netherlands", L"Italy"};
  std::deque<wstring> unstored = {L"Amsterdam has lots of bridges",
                                   L"Venice has lots of canals"};
  std::deque<wstring> text = {L"Amsterdam", L"Venice"};

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setReaderPooling(false)
               ->setMergePolicy(newLogMergePolicy()));

  shared_ptr<MergePolicy> lmp = modifier->getConfig()->getMergePolicy();
  lmp->setNoCFSRatio(1.0);

  dir->failOn(failure->reset());

  shared_ptr<FieldType> custom1 = make_shared<FieldType>();
  custom1->setStored(true);
  for (int i = 0; i < keywords.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", keywords[i], Field::Store::YES));
    doc->push_back(newField(L"country", unindexed[i], custom1));
    doc->push_back(newTextField(L"contents", unstored[i], Field::Store::NO));
    doc->push_back(newTextField(L"city", text[i], Field::Store::YES));
    modifier->addDocument(doc);
  }
  // flush

  if (VERBOSE) {
    wcout << L"TEST: now full merge" << endl;
  }

  modifier->forceMerge(1);
  if (VERBOSE) {
    wcout << L"TEST: now commit" << endl;
  }
  modifier->commit();

  // one of the two files hits

  shared_ptr<Term> term = make_shared<Term>(L"city", L"Amsterdam");
  int64_t hitCount = getHitCount(dir, term);
  TestUtil::assertEquals(1, hitCount);

  // open the writer again (closed above)

  // delete the doc
  // max buf del terms is two, so this is buffered

  if (VERBOSE) {
    wcout << L"TEST: delete term=" << term << endl;
  }

  modifier->deleteDocuments({term});

  // add a doc
  // doc remains buffered

  if (VERBOSE) {
    wcout << L"TEST: add empty doc" << endl;
  }
  shared_ptr<Document> doc = make_shared<Document>();
  modifier->addDocument(doc);

  // commit the changes, the buffered deletes, and the new doc

  // The failure object will fail on the first write after the del
  // file gets created when processing the buffered delete

  // in the ac case, this will be when writing the new segments
  // files so we really don't need the new doc, but it's harmless

  // a new segments file won't be created but in this
  // case, creation of the cfs file happens next so we
  // need the doc (to test that it's okay that we don't
  // lose deletes if failing while creating the cfs file)

  if (VERBOSE) {
    wcout << L"TEST: now commit for failure" << endl;
  }
  runtime_error expected =
      expectThrows(runtime_error::typeid, [&]() { modifier->commit(); });
  if (VERBOSE) {
    wcout << L"TEST: hit exc:" << endl;
    expected.printStackTrace(System::out);
  }

  // The commit above failed, so we need to retry it (which will
  // succeed, because the failure is a one-shot)

  bool writerClosed;
  try {
    modifier->commit();
    writerClosed = false;
  } catch (const IllegalStateException &ise) {
    // The above exc struck during merge, and closed the writer
    writerClosed = true;
  }

  if (writerClosed == false) {
    hitCount = getHitCount(dir, term);

    // Make sure the delete was successfully flushed:
    TestUtil::assertEquals(0, hitCount);

    delete modifier;
  }
  delete dir;
}

TestIndexWriterDelete::FailureAnonymousInnerClass::FailureAnonymousInnerClass(
    shared_ptr<TestIndexWriterDelete> outerInstance)
{
  this->outerInstance = outerInstance;
  sawMaybe = false;
  failed = false;
}

shared_ptr<MockDirectoryWrapper::Failure>
TestIndexWriterDelete::FailureAnonymousInnerClass::reset()
{
  thread = Thread::currentThread();
  sawMaybe = false;
  failed = false;
  return shared_from_this();
}

void TestIndexWriterDelete::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (Thread::currentThread() != thread) {
    // don't fail during merging
    return;
  }
  if (VERBOSE) {
    wcout << L"FAIL EVAL:" << endl;
  }
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
  (runtime_error())->printStackTrace(System::out);
  if (sawMaybe && !failed) {
    bool seen = false;
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    for (int i = 0; i < trace.size(); i++) {
      if (L"applyDeletesAndUpdates" == trace[i]->getMethodName() ||
          L"slowFileExists" == trace[i]->getMethodName()) {
        seen = true;
        break;
      }
    }
    if (!seen) {
      // Only fail once we are no longer in applyDeletes
      failed = true;
      if (VERBOSE) {
        wcout << L"TEST: mock failure: now fail" << endl;
        // C++ TODO: This exception's constructor requires an argument:
        // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
        (runtime_error())->printStackTrace(System::out);
      }
      throw runtime_error(L"fail after applyDeletes");
    }
  }
  if (!failed) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    for (int i = 0; i < trace.size(); i++) {
      if (L"applyDeletesAndUpdates" == trace[i]->getMethodName()) {
        if (VERBOSE) {
          wcout << L"TEST: mock failure: saw applyDeletes" << endl;
          // C++ TODO: This exception's constructor requires an argument:
          // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
          (runtime_error())->printStackTrace(System::out);
        }
        sawMaybe = true;
        break;
      }
    }
  }
}

void TestIndexWriterDelete::testErrorInDocsWriterAdd() 
{

  shared_ptr<MockDirectoryWrapper::Failure> failure =
      make_shared<FailureAnonymousInnerClass2>(shared_from_this());

  // create a couple of files

  std::deque<wstring> keywords = {L"1", L"2"};
  std::deque<wstring> unindexed = {L"Netherlands", L"Italy"};
  std::deque<wstring> unstored = {L"Amsterdam has lots of bridges",
                                   L"Venice has lots of canals"};
  std::deque<wstring> text = {L"Amsterdam", L"Venice"};

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
               random(), MockTokenizer::WHITESPACE, false)));
  modifier->commit();
  dir->failOn(failure->reset());

  shared_ptr<FieldType> custom1 = make_shared<FieldType>();
  custom1->setStored(true);
  for (int i = 0; i < keywords.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", keywords[i], Field::Store::YES));
    doc->push_back(newField(L"country", unindexed[i], custom1));
    doc->push_back(newTextField(L"contents", unstored[i], Field::Store::NO));
    doc->push_back(newTextField(L"city", text[i], Field::Store::YES));
    try {
      modifier->addDocument(doc);
    } catch (const IOException &io) {
      if (VERBOSE) {
        wcout << L"TEST: got expected exc:" << endl;
        io->printStackTrace(System::out);
      }
      break;
    }
  }
  assertTrue(modifier->deleter->isClosed());

  TestIndexWriter::assertNoUnreferencedFiles(
      dir, L"docsWriter.abort() failed to delete unreferenced files");
  delete dir;
}

TestIndexWriterDelete::FailureAnonymousInnerClass2::FailureAnonymousInnerClass2(
    shared_ptr<TestIndexWriterDelete> outerInstance)
{
  this->outerInstance = outerInstance;
  failed = false;
}

shared_ptr<MockDirectoryWrapper::Failure>
TestIndexWriterDelete::FailureAnonymousInnerClass2::reset()
{
  failed = false;
  return shared_from_this();
}

void TestIndexWriterDelete::FailureAnonymousInnerClass2::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (!failed) {
    failed = true;
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"fail in add doc");
  }
}

void TestIndexWriterDelete::testDeleteNullQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(
               random(), MockTokenizer::WHITESPACE, false)));

  for (int i = 0; i < 5; i++) {
    addDoc(modifier, i, 2 * i);
  }

  modifier->deleteDocuments(
      {make_shared<TermQuery>(make_shared<Term>(L"nada", L"nada"))});
  modifier->commit();
  TestUtil::assertEquals(5, modifier->numDocs());
  delete modifier;
  delete dir;
}

void TestIndexWriterDelete::testDeleteAllSlowly() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int NUM_DOCS = atLeast(1000);
  const deque<int> ids = deque<int>(NUM_DOCS);
  for (int id = 0; id < NUM_DOCS; id++) {
    ids.push_back(id);
  }
  Collections::shuffle(ids, random());
  for (auto id : ids) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    w->addDocument(doc);
  }
  Collections::shuffle(ids, random());
  int upto = 0;
  while (upto < ids.size()) {
    constexpr int left = ids.size() - upto;
    constexpr int inc = min(left, TestUtil::nextInt(random(), 1, 20));
    constexpr int limit = upto + inc;
    while (upto < limit) {
      if (VERBOSE) {
        wcout << L"TEST: delete id=" << ids[upto] << endl;
      }
      w->deleteDocuments(
          make_shared<Term>(L"id", L"" + to_wstring(ids[upto++])));
    }
    if (VERBOSE) {
      wcout << L"\nTEST: now open reader" << endl;
    }
    shared_ptr<IndexReader> *const r = w->getReader();
    TestUtil::assertEquals(NUM_DOCS - upto, r->numDocs());
    delete r;
  }

  delete w;
  delete dir;
}

void TestIndexWriterDelete::testIndexingThenDeleting() 
{
  // TODO: move this test to its own class and just @SuppressCodecs?
  // TODO: is it enough to just use newFSDirectory?
  const wstring fieldFormat = TestUtil::getPostingsFormat(L"field");
  assumeFalse(L"This test cannot run with Memory codec",
              fieldFormat == L"Memory");
  assumeFalse(L"This test cannot run with SimpleText codec",
              fieldFormat == L"SimpleText");
  assumeFalse(L"This test cannot run with Direct codec",
              fieldFormat == L"Direct");
  shared_ptr<Random> *const r = random();
  shared_ptr<Directory> dir = newDirectory();
  // note this test explicitly disables payloads
  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)
               ->setRAMBufferSizeMB(4.0)
               ->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(
      L"field", L"go 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20",
      Field::Store::NO));
  int num = atLeast(3);
  for (int iter = 0; iter < num; iter++) {
    int count = 0;

    constexpr bool doIndexing = r->nextBoolean();
    if (VERBOSE) {
      wcout << L"TEST: iter doIndexing=" << doIndexing << endl;
    }
    if (doIndexing) {
      // Add docs until a flush is triggered
      constexpr int startFlushCount = w->getFlushCount();
      while (w->getFlushCount() == startFlushCount) {
        w->addDocument(doc);
        count++;
      }
    } else {
      // Delete docs until a flush is triggered
      constexpr int startFlushCount = w->getFlushCount();
      while (w->getFlushCount() == startFlushCount) {
        w->deleteDocuments(
            {make_shared<Term>(L"foo", L"" + to_wstring(count))});
        count++;
      }
    }
    assertTrue(L"flush happened too quickly during " +
                   (doIndexing ? L"indexing" : L"deleting") + L" count=" +
                   to_wstring(count),
               count > 2500);
  }
  delete w;
  delete dir;
}

TestIndexWriterDelete::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestIndexWriterDelete> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterDelete::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true));
}

void TestIndexWriterDelete::testFlushPushedDeletesByRAM() 
{
  shared_ptr<Directory> dir = newDirectory();
  // Cannot use RandomIndexWriter because we don't want to
  // ever call commit() for this test:
  // note: tiny rambuffer used, as with a 1MB buffer the test is too slow (flush
  // @ 128,999)
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setRAMBufferSizeMB(0.1f)
               ->setMaxBufferedDocs(1000)
               ->setMergePolicy(NoMergePolicy::INSTANCE)
               ->setReaderPooling(false));
  int count = 0;
  while (true) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", to_wstring(count) + L"",
                                            Field::Store::NO));
    shared_ptr<Term> *const delTerm;
    if (count == 1010) {
      // This is the only delete that applies
      delTerm = make_shared<Term>(L"id", L"" + to_wstring(0));
    } else {
      // These get buffered, taking up RAM, but delete
      // nothing when applied:
      delTerm = make_shared<Term>(L"id", L"x" + to_wstring(count));
    }
    w->updateDocument(delTerm, doc);
    // Eventually segment 0 should get a del docs:
    // TODO: fix this test
    if (slowFileExists(dir, L"_0_1.del") || slowFileExists(dir, L"_0_1.liv")) {
      if (VERBOSE) {
        wcout << L"TEST: deletes created @ count=" << count << endl;
      }
      break;
    }
    count++;

    // Today we applyDeletes @ count=21553; even if we make
    // sizable improvements to RAM efficiency of buffered
    // del term we're unlikely to go over 100K:
    if (count > 100000) {
      fail(L"delete's were not applied");
    }
  }
  delete w;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testApplyDeletesOnFlush() throws
// Exception
void TestIndexWriterDelete::testApplyDeletesOnFlush() 
{
  shared_ptr<Directory> dir = newDirectory();
  // Cannot use RandomIndexWriter because we don't want to
  // ever call commit() for this test:
  shared_ptr<AtomicInteger> *const docsInSegment = make_shared<AtomicInteger>();
  shared_ptr<AtomicBoolean> *const closing = make_shared<AtomicBoolean>();
  shared_ptr<AtomicBoolean> *const sawAfterFlush = make_shared<AtomicBoolean>();
  shared_ptr<IndexWriter> w = make_shared<IndexWriterAnonymousInnerClass>(
      shared_from_this(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setRAMBufferSizeMB(0.5)
          ->setMaxBufferedDocs(-1)
          ->setMergePolicy(NoMergePolicy::INSTANCE)
          ->setReaderPooling(false)
          ->setUseCompoundFile(true),
      docsInSegment, closing, sawAfterFlush);
  int id = 0;
  while (true) {
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    for (int termIDX = 0; termIDX < 100; termIDX++) {
      sb->append(L' ')->append(
          TestUtil::randomRealisticUnicodeString(random()));
    }
    if (id == 500) {
      w->deleteDocuments({make_shared<Term>(L"id", L"0")});
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    doc->push_back(newTextField(L"body", sb->toString(), Field::Store::NO));
    w->updateDocument(make_shared<Term>(L"id", L"" + to_wstring(id)), doc);
    docsInSegment->incrementAndGet();
    // TODO: fix this test
    if (slowFileExists(dir, L"_0_1.del") || slowFileExists(dir, L"_0_1.liv")) {
      if (VERBOSE) {
        wcout << L"TEST: deletes created @ id=" << id << endl;
      }
      break;
    }
    id++;
  }
  closing->set(true);
  assertTrue(sawAfterFlush->get());
  delete w;
  delete dir;
}

TestIndexWriterDelete::IndexWriterAnonymousInnerClass::
    IndexWriterAnonymousInnerClass(
        shared_ptr<TestIndexWriterDelete> outerInstance,
        shared_ptr<Directory> dir,
        shared_ptr<org::apache::lucene::index::IndexWriterConfig>
            setUseCompoundFile,
        shared_ptr<AtomicInteger> docsInSegment,
        shared_ptr<AtomicBoolean> closing,
        shared_ptr<AtomicBoolean> sawAfterFlush)
    : IndexWriter(dir, setUseCompoundFile)
{
  this->outerInstance = outerInstance;
  this->docsInSegment = docsInSegment;
  this->closing = closing;
  this->sawAfterFlush = sawAfterFlush;
}

void TestIndexWriterDelete::IndexWriterAnonymousInnerClass::doAfterFlush()
{
  assertTrue(L"only " + docsInSegment->get() + L" in segment",
             closing->get() || docsInSegment->get() >= 7);
  docsInSegment->set(0);
  sawAfterFlush->set(true);
}

void TestIndexWriterDelete::testDeletesCheckIndexOutput() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"field", L"0", StringField::TYPE_NOT_STORED));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newField(L"field", L"1", StringField::TYPE_NOT_STORED));
  w->addDocument(doc);
  w->commit();
  TestUtil::assertEquals(1, w->getSegmentCount());

  w->deleteDocuments({make_shared<Term>(L"field", L"0")});
  w->commit();
  TestUtil::assertEquals(1, w->getSegmentCount());
  delete w;

  shared_ptr<ByteArrayOutputStream> bos =
      make_shared<ByteArrayOutputStream>(1024);
  shared_ptr<CheckIndex> checker = make_shared<CheckIndex>(dir);
  checker->setInfoStream(make_shared<PrintStream>(bos, false, IOUtils::UTF_8),
                         false);
  shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex(nullptr);
  assertTrue(indexStatus->clean);
  delete checker;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = bos->toString(IOUtils::UTF_8);

  // Segment should have deletions:
  assertTrue(s.find(L"has deletions") != wstring::npos);
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  w = make_shared<IndexWriter>(dir, iwc);
  w->forceMerge(1);
  delete w;

  bos = make_shared<ByteArrayOutputStream>(1024);
  checker = make_shared<CheckIndex>(dir);
  checker->setInfoStream(make_shared<PrintStream>(bos, false, IOUtils::UTF_8),
                         false);
  indexStatus = checker->checkIndex(nullptr);
  assertTrue(indexStatus->clean);
  delete checker;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  s = bos->toString(IOUtils::UTF_8);
  assertFalse(s.find(L"has deletions") != wstring::npos);
  delete dir;
}

void TestIndexWriterDelete::testTryDeleteDocument() 
{

  shared_ptr<Directory> d = newDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(d, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  w->addDocument(doc);
  w->addDocument(doc);
  w->addDocument(doc);
  delete w;

  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setOpenMode(IndexWriterConfig::OpenMode::APPEND);
  w = make_shared<IndexWriter>(d, iwc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w, false, false);
  assertTrue(w->tryDeleteDocument(r, 1) != -1);
  assertFalse(
      (std::static_pointer_cast<StandardDirectoryReader>(r))->isCurrent());
  assertTrue(w->tryDeleteDocument(r->leaves()[0]->reader(), 0) != -1);
  assertFalse(
      (std::static_pointer_cast<StandardDirectoryReader>(r))->isCurrent());
  delete r;
  delete w;

  r = DirectoryReader::open(d);
  TestUtil::assertEquals(2, r->numDeletedDocs());
  assertNotNull(MultiFields::getLiveDocs(r));
  delete r;
  delete d;
}

void TestIndexWriterDelete::testNRTIsCurrentAfterDelete() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(d, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  w->addDocument(doc);
  w->addDocument(doc);
  w->addDocument(doc);
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);
  delete w;
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setOpenMode(IndexWriterConfig::OpenMode::APPEND);
  w = make_shared<IndexWriter>(d, iwc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w, false, false);
  w->deleteDocuments({make_shared<Term>(L"id", L"1")});
  shared_ptr<IndexReader> r2 = DirectoryReader::open(w, true, true);
  assertFalse(
      (std::static_pointer_cast<StandardDirectoryReader>(r))->isCurrent());
  assertTrue(
      (std::static_pointer_cast<StandardDirectoryReader>(r2))->isCurrent());
  IOUtils::close({r, r2, w, d});
}

void TestIndexWriterDelete::testOnlyDeletesTriggersMergeOnClose() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMinMergeDocs(1);
  iwc->setMergePolicy(mp);
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 38; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }
  w->commit();

  for (int i = 0; i < 18; i++) {
    w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }

  delete w;
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->leaves()->size());
  r->close();

  delete dir;
}

void TestIndexWriterDelete::testOnlyDeletesTriggersMergeOnGetReader() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMinMergeDocs(1);
  iwc->setMergePolicy(mp);
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 38; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }
  w->commit();

  for (int i = 0; i < 18; i++) {
    w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }

  // First one triggers, but does not reflect, the merge:
  wcout << L"TEST: now get reader" << endl;
  DirectoryReader::open(w)->close();
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(1, r->leaves().size());
  delete r;

  delete w;
  delete dir;
}

void TestIndexWriterDelete::testOnlyDeletesTriggersMergeOnFlush() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMinMergeDocs(1);
  iwc->setMergePolicy(mp);
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 38; i++) {
    if (VERBOSE) {
      wcout << L"TEST: add doc " << i << endl;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }
  if (VERBOSE) {
    wcout << L"TEST: commit1" << endl;
  }
  w->commit();

  // Deleting 18 out of the 20 docs in the first segment make it the same
  // "level" as the other 9 which should cause a merge to kick off:
  for (int i = 0; i < 18; i++) {
    w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  if (VERBOSE) {
    wcout << L"TEST: commit2" << endl;
  }
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->leaves()->size());
  r->close();

  delete dir;
}

void TestIndexWriterDelete::testOnlyDeletesDeleteAllDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMinMergeDocs(1);
  iwc->setMergePolicy(mp);
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 38; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }
  w->commit();

  for (int i = 0; i < 38; i++) {
    w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(0, r->leaves()->size());
  TestUtil::assertEquals(0, r->maxDoc());
  r->close();

  delete w;
  delete dir;
}

void TestIndexWriterDelete::testMergingAfterDeleteAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMinMergeDocs(1);
  iwc->setMergePolicy(mp);
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }
  w->commit();
  w->deleteAll();

  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }

  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(1, r->leaves()->size());
  r->close();

  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index