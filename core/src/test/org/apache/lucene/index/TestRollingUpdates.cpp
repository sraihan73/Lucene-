using namespace std;

#include "TestRollingUpdates.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using MemoryPostingsFormat =
    org::apache::lucene::codecs::memory::MemoryPostingsFormat;
using namespace org::apache::lucene::document;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using namespace org::apache::lucene::store;
using namespace org::apache::lucene::util;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRollingUpdates() throws Exception
void TestRollingUpdates::testRollingUpdates() 
{
  shared_ptr<Random> random =
      make_shared<Random>(TestRollingUpdates::random()->nextLong());
  shared_ptr<BaseDirectoryWrapper> *const dir = newDirectory();

  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random);

  // provider.register(new MemoryCodec());
  if (TestRollingUpdates::random()->nextBoolean()) {
    Codec::setDefault(
        TestUtil::alwaysPostingsFormat(make_shared<MemoryPostingsFormat>(
            TestRollingUpdates::random()->nextBoolean(), random->nextFloat())));
  }

  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(TestRollingUpdates::random());
  analyzer->setMaxTokenLength(TestUtil::nextInt(TestRollingUpdates::random(), 1,
                                                IndexWriter::MAX_TERM_LENGTH));

  shared_ptr<IndexWriter> *const w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  constexpr int SIZE = atLeast(20);
  int id = 0;
  shared_ptr<IndexReader> r = nullptr;
  shared_ptr<IndexSearcher> s = nullptr;
  constexpr int numUpdates = static_cast<int>(
      SIZE *
      (2 + (TEST_NIGHTLY ? 200 * TestRollingUpdates::random()->nextDouble()
                         : 5 * TestRollingUpdates::random()->nextDouble())));
  if (VERBOSE) {
    wcout << L"TEST: numUpdates=" << numUpdates << endl;
  }
  int updateCount = 0;
  // TODO: sometimes update ids not in order...
  for (int docIter = 0; docIter < numUpdates; docIter++) {
    shared_ptr<Document> *const doc = docs->nextDoc();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring myID = Integer::toString(id);
    if (id == SIZE - 1) {
      id = 0;
    } else {
      id++;
    }
    if (VERBOSE) {
      wcout << L"  docIter=" << docIter << L" id=" << id << endl;
    }
    (std::static_pointer_cast<Field>(doc->getField(L"docid")))
        ->setStringValue(myID);

    shared_ptr<Term> idTerm = make_shared<Term>(L"docid", myID);

    constexpr bool doUpdate;
    if (s != nullptr && updateCount < SIZE) {
      shared_ptr<TopDocs> hits = s->search(make_shared<TermQuery>(idTerm), 1);
      assertEquals(1, hits->totalHits);
      doUpdate = w->tryDeleteDocument(r, hits->scoreDocs[0]->doc) == -1;
      if (VERBOSE) {
        if (doUpdate) {
          wcout << L"  tryDeleteDocument failed" << endl;
        } else {
          wcout << L"  tryDeleteDocument succeeded" << endl;
        }
      }
    } else {
      doUpdate = true;
      if (VERBOSE) {
        wcout << L"  no searcher: doUpdate=true" << endl;
      }
    }

    updateCount++;

    if (doUpdate) {
      if (TestRollingUpdates::random()->nextBoolean()) {
        w->updateDocument(idTerm, doc);
      } else {
        // It's OK to not be atomic for this test (no separate thread reopening
        // readers):
        w->deleteDocuments({make_shared<TermQuery>(idTerm)});
        w->addDocument(doc);
      }
    } else {
      w->addDocument(doc);
    }

    if (docIter >= SIZE && TestRollingUpdates::random()->nextInt(50) == 17) {
      if (r != nullptr) {
        delete r;
      }

      constexpr bool applyDeletions =
          TestRollingUpdates::random()->nextBoolean();

      if (VERBOSE) {
        wcout << L"TEST: reopen applyDeletions=" << applyDeletions << endl;
      }

      r = w->getReader(applyDeletions, false);
      if (applyDeletions) {
        s = newSearcher(r);
      } else {
        s.reset();
      }
      assertTrue(L"applyDeletions=" + StringHelper::toString(applyDeletions) +
                     L" r.numDocs()=" + to_wstring(r->numDocs()) +
                     L" vs SIZE=" + to_wstring(SIZE),
                 !applyDeletions || r->numDocs() == SIZE);
      updateCount = 0;
    }
  }

  if (r != nullptr) {
    delete r;
  }

  w->commit();
  assertEquals(SIZE, w->numDocs());

  delete w;

  TestIndexWriter::assertNoUnreferencedFiles(
      dir, L"leftover files after rolling updates");

  delete docs;

  // LUCENE-4455:
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
  int64_t totalBytes = 0;
  for (auto sipc : infos) {
    totalBytes += sipc->sizeInBytes();
  }
  int64_t totalBytes2 = 0;

  for (auto fileName : dir->listAll()) {
    if (IndexFileNames::CODEC_FILE_PATTERN->matcher(fileName).matches()) {
      totalBytes2 += dir->fileLength(fileName);
    }
  }
  assertEquals(totalBytes2, totalBytes);
  delete dir;
}

void TestRollingUpdates::testUpdateSameDoc() 
{
  shared_ptr<Directory> *const dir = newDirectory();

  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random());
  for (int r = 0; r < 3; r++) {
    shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2));
    constexpr int numUpdates = atLeast(20);
    int numThreads = TestUtil::nextInt(random(), 2, 6);
    std::deque<std::shared_ptr<IndexingThread>> threads(numThreads);
    for (int i = 0; i < numThreads; i++) {
      threads[i] = make_shared<IndexingThread>(docs, w, numUpdates);
      threads[i]->start();
    }

    for (int i = 0; i < numThreads; i++) {
      threads[i]->join();
    }

    delete w;
  }

  shared_ptr<IndexReader> open = DirectoryReader::open(dir);
  assertEquals(1, open->numDocs());
  delete open;
  delete docs;
  delete dir;
}

TestRollingUpdates::IndexingThread::IndexingThread(
    shared_ptr<LineFileDocs> docs, shared_ptr<IndexWriter> writer, int num)
    : Thread(), docs(docs), writer(writer), num(num)
{
}

void TestRollingUpdates::IndexingThread::run()
{
  try {
    shared_ptr<DirectoryReader> open = nullptr;
    for (int i = 0; i < num; i++) {
      shared_ptr<Document> doc = make_shared<Document>(); // docs.nextDoc();
      shared_ptr<BytesRef> br = make_shared<BytesRef>(L"test");
      doc->push_back(newStringField(L"id", br, Field::Store::NO));
      writer->updateDocument(make_shared<Term>(L"id", br), doc);
      if (random()->nextInt(3) == 0) {
        if (open == nullptr) {
          open = DirectoryReader::open(writer);
        }
        shared_ptr<DirectoryReader> reader =
            DirectoryReader::openIfChanged(open);
        if (reader != nullptr) {
          open->close();
          open = reader;
        }
        assertEquals(L"iter: " + to_wstring(i) + L" numDocs: " +
                         to_wstring(open->numDocs()) + L" del: " +
                         open->numDeletedDocs() + L" max: " +
                         to_wstring(open->maxDoc()),
                     1, open->numDocs());
      }
    }
    if (open != nullptr) {
      open->close();
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::index