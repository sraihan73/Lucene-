using namespace std;

#include "TestIndexWriterCommit.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::analysis;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexWriterCommit::testCommitOnClose() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < 14; i++) {
    TestIndexWriter::addDoc(writer);
  }
  delete writer;

  shared_ptr<Term> searchTerm = make_shared<Term>(L"content", L"aaa");
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"first number of hits", 14, hits.size());
  reader->close();

  reader = DirectoryReader::open(dir);

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 11; j++) {
      TestIndexWriter::addDoc(writer);
    }
    shared_ptr<IndexReader> r = DirectoryReader::open(dir);
    searcher = newSearcher(r);
    hits =
        searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
    assertEquals(L"reader incorrectly sees changes from writer", 14,
                 hits.size());
    delete r;
    assertTrue(L"reader should have still been current", reader->isCurrent());
  }

  // Now, close the writer:
  delete writer;
  assertFalse(L"reader should not be current now", reader->isCurrent());

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  searcher = newSearcher(r);
  hits = searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"reader did not see changes after writer was closed", 47,
               hits.size());
  delete r;
  reader->close();
  delete dir;
}

void TestIndexWriterCommit::testCommitOnCloseAbort() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10));
  for (int i = 0; i < 14; i++) {
    TestIndexWriter::addDoc(writer);
  }
  delete writer;

  shared_ptr<Term> searchTerm = make_shared<Term>(L"content", L"aaa");
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"first number of hits", 14, hits.size());
  delete reader;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMaxBufferedDocs(10));
  for (int j = 0; j < 17; j++) {
    TestIndexWriter::addDoc(writer);
  }
  // Delete all docs:
  writer->deleteDocuments({searchTerm});

  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"reader incorrectly sees changes from writer", 14, hits.size());
  delete reader;

  // Now, close the writer:
  writer->rollback();

  TestIndexWriter::assertNoUnreferencedFiles(
      dir, L"unreferenced files remain after rollback()");

  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"saw changes after writer.abort", 14, hits.size());
  delete reader;

  // Now make sure we can re-open the index, add docs,
  // and all is good:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMaxBufferedDocs(10));

  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 17; j++) {
      TestIndexWriter::addDoc(writer);
    }
    shared_ptr<IndexReader> r = DirectoryReader::open(dir);
    searcher = newSearcher(r);
    hits =
        searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
    assertEquals(L"reader incorrectly sees changes from writer", 14,
                 hits.size());
    delete r;
  }

  delete writer;
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  searcher = newSearcher(r);
  hits = searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"didn't see changes after close", 218, hits.size());
  delete r;

  delete dir;
}

void TestIndexWriterCommit::testCommitOnCloseDiskUsage() 
{
  // MemoryCodec, since it uses FST, is not necessarily
  // "additive", ie if you add up N small FSTs, then merge
  // them, the merged result can easily be larger than the
  // sum because the merged FST may use array encoding for
  // some arcs (which uses more space):

  const wstring idFormat = TestUtil::getPostingsFormat(L"id");
  const wstring contentFormat = TestUtil::getPostingsFormat(L"content");
  assumeFalse(L"This test cannot run with Memory codec",
              idFormat == L"Memory" || contentFormat == L"Memory");
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<Analyzer> analyzer;
  if (random()->nextBoolean()) {
    // no payloads
    analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  } else {
    // fixed length payloads
    constexpr int length = random()->nextInt(200);
    analyzer =
        make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), length);
  }

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)
               ->setMaxBufferedDocs(10)
               ->setReaderPooling(false)
               ->setMergePolicy(newLogMergePolicy(10)));
  for (int j = 0; j < 30; j++) {
    TestIndexWriter::addDocWithIndex(writer, j);
  }
  delete writer;
  dir->resetMaxUsedSizeInBytes();

  dir->setTrackDiskUsage(true);
  int64_t startDiskUsage = dir->getMaxUsedSizeInBytes();
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)
               ->setOpenMode(OpenMode::APPEND)
               ->setMaxBufferedDocs(10)
               ->setMergeScheduler(make_shared<SerialMergeScheduler>())
               ->setReaderPooling(false)
               ->setMergePolicy(newLogMergePolicy(10)));
  for (int j = 0; j < 1470; j++) {
    TestIndexWriter::addDocWithIndex(writer, j);
  }
  int64_t midDiskUsage = dir->getMaxUsedSizeInBytes();
  dir->resetMaxUsedSizeInBytes();
  writer->forceMerge(1);
  delete writer;

  DirectoryReader::open(dir)->close();

  int64_t endDiskUsage = dir->getMaxUsedSizeInBytes();

  // Ending index is 50X as large as starting index; due
  // to 3X disk usage normally we allow 150X max
  // transient usage.  If something is wrong w/ deleter
  // and it doesn't delete intermediate segments then it
  // will exceed this 150X:
  // System.out.println("start " + startDiskUsage + "; mid " + midDiskUsage +
  // ";end " + endDiskUsage);
  assertTrue(L"writer used too much space while adding documents: mid=" +
                 to_wstring(midDiskUsage) + L" start=" +
                 to_wstring(startDiskUsage) + L" end=" +
                 to_wstring(endDiskUsage) + L" max=" +
                 to_wstring(startDiskUsage * 150),
             midDiskUsage < 150 * startDiskUsage);
  assertTrue(L"writer used too much space after close: endDiskUsage=" +
                 to_wstring(endDiskUsage) + L" startDiskUsage=" +
                 to_wstring(startDiskUsage) + L" max=" +
                 to_wstring(startDiskUsage * 150),
             endDiskUsage < 150 * startDiskUsage);
  delete dir;
}

TestIndexWriterCommit::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestIndexWriterCommit> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestIndexWriterCommit::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true));
}

TestIndexWriterCommit::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestIndexWriterCommit> outerInstance, int length)
{
  this->outerInstance = outerInstance;
  this->length = length;
}

shared_ptr<TokenStreamComponents>
TestIndexWriterCommit::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<MockFixedLengthPayloadFilter>(
                     LuceneTestCase::random(), tokenizer, length));
}

void TestIndexWriterCommit::testCommitOnCloseForceMerge() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(newLogMergePolicy(10)));
  for (int j = 0; j < 17; j++) {
    TestIndexWriter::addDocWithIndex(writer, j);
  }
  delete writer;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);

  // Open a reader before closing (commiting) the writer:
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);

  // Reader should see index as multi-seg at this
  // point:
  assertTrue(L"Reader incorrectly sees one segment",
             reader->leaves()->size() > 1);
  reader->close();

  // Abort the writer:
  writer->rollback();
  TestIndexWriter::assertNoUnreferencedFiles(
      dir, L"aborted writer after forceMerge");

  // Open a reader after aborting writer:
  reader = DirectoryReader::open(dir);

  // Reader should still see index as multi-segment
  assertTrue(L"Reader incorrectly sees one segment",
             reader->leaves()->size() > 1);
  reader->close();

  if (VERBOSE) {
    wcout << L"TEST: do real full merge" << endl;
  }
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);
  delete writer;

  if (VERBOSE) {
    wcout << L"TEST: writer closed" << endl;
  }
  TestIndexWriter::assertNoUnreferencedFiles(
      dir, L"aborted writer after forceMerge");

  // Open a reader after aborting writer:
  reader = DirectoryReader::open(dir);

  // Reader should see index as one segment
  assertEquals(L"Reader incorrectly sees more than one segment", 1,
               reader->leaves()->size());
  reader->close();
  delete dir;
}

void TestIndexWriterCommit::testCommitThreadSafety() 
{
  constexpr int NUM_THREADS = 5;
  constexpr double RUN_SEC = 0.5;
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  TestUtil::reduceOpenFiles(w->w);
  w->commit();
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();
  std::deque<std::shared_ptr<Thread>> threads(NUM_THREADS);
  constexpr int64_t endTime =
      System::currentTimeMillis() + (static_cast<int64_t>(RUN_SEC * 1000));
  for (int i = 0; i < NUM_THREADS; i++) {
    constexpr int finalI = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass3>(
        shared_from_this(), dir, w, failed, endTime, finalI);
    threads[i]->start();
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i]->join();
  }
  assertFalse(failed->get());
  delete w;
  delete dir;
}

TestIndexWriterCommit::ThreadAnonymousInnerClass3::ThreadAnonymousInnerClass3(
    shared_ptr<TestIndexWriterCommit> outerInstance, shared_ptr<Directory> dir,
    shared_ptr<org::apache::lucene::index::RandomIndexWriter> w,
    shared_ptr<AtomicBoolean> failed, int64_t endTime, int finalI)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->w = w;
  this->failed = failed;
  this->endTime = endTime;
  this->finalI = finalI;
}

void TestIndexWriterCommit::ThreadAnonymousInnerClass3::run()
{
  try {
    shared_ptr<Document> *const doc = make_shared<Document>();
    shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
    shared_ptr<Field> f =
        LuceneTestCase::newStringField(L"f", L"", Field::Store::NO);
    doc->push_back(f);
    int count = 0;
    do {
      if (failed->get()) {
        break;
      }
      for (int j = 0; j < 10; j++) {
        const wstring s = to_wstring(finalI) + L"_" + to_wstring(count++);
        f->setStringValue(s);
        w->addDocument(doc);
        w->commit();
        shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
        assertNotNull(r2);
        assertTrue(r2 != r);
        r->close();
        r = r2;
        assertEquals(L"term=f:" + s + L"; r=" + r, 1,
                     r->docFreq(make_shared<Term>(L"f", s)));
      }
    } while (System::currentTimeMillis() < endTime);
    r->close();
  } catch (const runtime_error &t) {
    failed->set(true);
    throw runtime_error(t);
  }
}

void TestIndexWriterCommit::testForceCommit() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy(5)));
  writer->commit();

  for (int i = 0; i < 23; i++) {
    TestIndexWriter::addDoc(writer);
  }

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());
  writer->commit();
  shared_ptr<DirectoryReader> reader2 = DirectoryReader::openIfChanged(reader);
  assertNotNull(reader2);
  TestUtil::assertEquals(0, reader->numDocs());
  TestUtil::assertEquals(23, reader2->numDocs());
  reader->close();

  for (int i = 0; i < 17; i++) {
    TestIndexWriter::addDoc(writer);
  }
  TestUtil::assertEquals(23, reader2->numDocs());
  reader2->close();
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(23, reader->numDocs());
  reader->close();
  writer->commit();

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(40, reader->numDocs());
  reader->close();
  delete writer;
  delete dir;
}

void TestIndexWriterCommit::testFutureCommit() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE));
  shared_ptr<Document> doc = make_shared<Document>();
  w->addDocument(doc);

  // commit to "first"
  unordered_map<wstring, wstring> commitData =
      unordered_map<wstring, wstring>();
  commitData.emplace(L"tag", L"first");
  w->setLiveCommitData(commitData.entrySet());
  w->commit();

  // commit to "second"
  w->addDocument(doc);
  commitData.emplace(L"tag", L"second");
  w->setLiveCommitData(commitData.entrySet());
  delete w;

  // open "first" with IndexWriter
  shared_ptr<IndexCommit> commit = nullptr;
  for (auto c : DirectoryReader::listCommits(dir)) {
    if (c->getUserData()[L"tag"] == L"first") {
      commit = c;
      break;
    }
  }

  assertNotNull(commit);

  w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE)
               ->setIndexCommit(commit));

  TestUtil::assertEquals(1, w->numDocs());

  // commit IndexWriter to "third"
  w->addDocument(doc);
  commitData.emplace(L"tag", L"third");
  w->setLiveCommitData(commitData.entrySet());
  delete w;

  // make sure "second" commit is still there
  commit.reset();
  for (auto c : DirectoryReader::listCommits(dir)) {
    if (c->getUserData()[L"tag"] == L"second") {
      commit = c;
      break;
    }
  }

  assertNotNull(commit);

  delete dir;
}

void TestIndexWriterCommit::testZeroCommits() 
{
  // Tests that if we don't call commit(), the directory has 0 commits. This has
  // changed since LUCENE-2386, where before IW would always commit on a fresh
  // new index.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  expectThrows(IndexNotFoundException::typeid,
               [&]() { DirectoryReader::listCommits(dir); });

  // No changes still should generate a commit, because it's a new index.
  delete writer;
  assertEquals(L"expected 1 commits!", 1,
               DirectoryReader::listCommits(dir).size());
  delete dir;
}

void TestIndexWriterCommit::testPrepareCommit() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy(5)));
  writer->commit();

  for (int i = 0; i < 23; i++) {
    TestIndexWriter::addDoc(writer);
  }

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());

  writer->prepareCommit();

  shared_ptr<IndexReader> reader2 = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader2->numDocs());

  writer->commit();

  shared_ptr<IndexReader> reader3 = DirectoryReader::openIfChanged(reader);
  assertNotNull(reader3);
  TestUtil::assertEquals(0, reader->numDocs());
  TestUtil::assertEquals(0, reader2->numDocs());
  TestUtil::assertEquals(23, reader3->numDocs());
  reader->close();
  delete reader2;

  for (int i = 0; i < 17; i++) {
    TestIndexWriter::addDoc(writer);
  }

  TestUtil::assertEquals(23, reader3->numDocs());
  delete reader3;
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(23, reader->numDocs());
  reader->close();

  writer->prepareCommit();

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(23, reader->numDocs());
  reader->close();

  writer->commit();
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(40, reader->numDocs());
  reader->close();
  delete writer;
  delete dir;
}

void TestIndexWriterCommit::testPrepareCommitRollback() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy(5)));
  writer->commit();

  for (int i = 0; i < 23; i++) {
    TestIndexWriter::addDoc(writer);
  }

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());

  writer->prepareCommit();

  shared_ptr<IndexReader> reader2 = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader2->numDocs());

  writer->rollback();

  shared_ptr<IndexReader> reader3 = DirectoryReader::openIfChanged(reader);
  assertNull(reader3);
  TestUtil::assertEquals(0, reader->numDocs());
  TestUtil::assertEquals(0, reader2->numDocs());
  reader->close();
  delete reader2;

  // System.out.println("TEST: after rollback: " +
  // Arrays.toString(dir.listAll()));

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < 17; i++) {
    TestIndexWriter::addDoc(writer);
  }

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());
  reader->close();

  writer->prepareCommit();

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());
  reader->close();

  writer->commit();
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(17, reader->numDocs());
  reader->close();
  delete writer;
  delete dir;
}

void TestIndexWriterCommit::testPrepareCommitNoChanges() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->prepareCommit();
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->numDocs());
  delete reader;
  delete dir;
}

void TestIndexWriterCommit::testCommitUserData() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
  for (int j = 0; j < 17; j++) {
    TestIndexWriter::addDoc(w);
  }
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  // commit(Map) never called for this index
  TestUtil::assertEquals(0, r->getIndexCommit()->getUserData().size());
  r->close();

  w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
  for (int j = 0; j < 17; j++) {
    TestIndexWriter::addDoc(w);
  }
  unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
  data.emplace(L"label", L"test1");
  w->setLiveCommitData(data.entrySet());
  delete w;

  r = DirectoryReader::open(dir);
  TestUtil::assertEquals(L"test1",
                         r->getIndexCommit()->getUserData()[L"label"]);
  r->close();

  w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  w->forceMerge(1);
  delete w;

  delete dir;
}

void TestIndexWriterCommit::testPrepareCommitThenClose() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  w->addDocument(make_shared<Document>());

  w->prepareCommit();
  expectThrows(IllegalStateException::typeid, [&]() { delete w; });
  w->commit();
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->maxDoc());
  r->close();
  delete dir;
}

void TestIndexWriterCommit::testCommitDataIsLive() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  w->addDocument(make_shared<Document>());

  const unordered_map<wstring, wstring> commitData =
      unordered_map<wstring, wstring>();
  commitData.emplace(L"foo", L"bar");

  // make sure "foo" / "bar" doesn't take
  w->setLiveCommitData(commitData.entrySet());

  commitData.clear();
  commitData.emplace(L"boo", L"baz");

  // this finally does the commit, and should burn "boo" / "baz"
  delete w;

  deque<std::shared_ptr<IndexCommit>> commits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(1, commits.size());

  shared_ptr<IndexCommit> commit = commits[0];
  unordered_map<wstring, wstring> data = commit->getUserData();
  TestUtil::assertEquals(1, data.size());
  TestUtil::assertEquals(L"baz", data[L"boo"]);
  delete dir;
}
} // namespace org::apache::lucene::index