using namespace std;

#include "TestIndexingSequenceNumbers.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexingSequenceNumbers::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  int64_t a = w->addDocument(make_shared<Document>());
  int64_t b = w->addDocument(make_shared<Document>());
  assertTrue(b > a);
  delete w;
  delete dir;
}

void TestIndexingSequenceNumbers::testAfterRefresh() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  int64_t a = w->addDocument(make_shared<Document>());
  DirectoryReader::open(w)->close();
  int64_t b = w->addDocument(make_shared<Document>());
  assertTrue(b > a);
  delete w;
  delete dir;
}

void TestIndexingSequenceNumbers::testAfterCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  int64_t a = w->addDocument(make_shared<Document>());
  w->commit();
  int64_t b = w->addDocument(make_shared<Document>());
  assertTrue(b > a);
  delete w;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testStressUpdateSameID() throws Exception
void TestIndexingSequenceNumbers::testStressUpdateSameID() 
{
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const w =
        make_shared<RandomIndexWriter>(random(), dir);
    std::deque<std::shared_ptr<Thread>> threads(
        TestUtil::nextInt(random(), 2, 5));
    shared_ptr<CountDownLatch> *const startingGun =
        make_shared<CountDownLatch>(1);
    const std::deque<int64_t> seqNos =
        std::deque<int64_t>(threads.size());
    shared_ptr<Term> *const id = make_shared<Term>(L"id", L"id");
    // multiple threads update the same document
    for (int i = 0; i < threads.size(); i++) {
      constexpr int threadID = i;
      threads[i] = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), w, startingGun, seqNos, id, threadID);
      threads[i]->start();
    }
    startingGun->countDown();
    for (auto thread : threads) {
      thread->join();
    }

    // now confirm that the reported sequence numbers agree with the index:
    int maxThread = 0;
    shared_ptr<Set<int64_t>> allSeqNos = unordered_set<int64_t>();
    for (int i = 0; i < threads.size(); i++) {
      allSeqNos->add(seqNos[i]);
      if (seqNos[i] > seqNos[maxThread]) {
        maxThread = i;
      }
    }
    // make sure all sequence numbers were different
    TestUtil::assertEquals(threads.size(), allSeqNos->size());
    shared_ptr<DirectoryReader> r = w->getReader();
    shared_ptr<IndexSearcher> s = newSearcher(r);
    shared_ptr<TopDocs> hits = s->search(make_shared<TermQuery>(id), 1);
    assertEquals(L"maxDoc: " + to_wstring(r->maxDoc()), 1, hits->totalHits);
    shared_ptr<Document> doc = r->document(hits->scoreDocs[0]->doc);
    TestUtil::assertEquals(
        maxThread, doc->getField(L"thread")->numericValue()->intValue());
    r->close();
    delete w;
    delete dir;
  }
}

TestIndexingSequenceNumbers::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestIndexingSequenceNumbers> outerInstance,
        shared_ptr<org::apache::lucene::index::RandomIndexWriter> w,
        shared_ptr<CountDownLatch> startingGun, deque<int64_t> &seqNos,
        shared_ptr<org::apache::lucene::index::Term> id, int threadID)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
  this->seqNos = seqNos;
  this->id = id;
  this->threadID = threadID;
}

void TestIndexingSequenceNumbers::ThreadAnonymousInnerClass::run()
{
  try {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StoredField>(L"thread", threadID));
    doc->push_back(make_shared<StringField>(L"id", L"id", Field::Store::NO));
    startingGun->await();
    for (int j = 0; j < 100; j++) {
      if (LuceneTestCase::random()->nextBoolean()) {
        seqNos[threadID] = w->updateDocument(id, doc);
      } else {
        seqNos[threadID] = w->updateDocuments(id, Arrays::asList(doc));
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testStressConcurrentCommit() throws
// Exception
void TestIndexingSequenceNumbers::testStressConcurrentCommit() throw(
    runtime_error)
{
  constexpr int opCount = atLeast(10000);
  constexpr int idCount = TestUtil::nextInt(random(), 10, 1000);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE);

  // Cannot use RIW since it randomly commits:
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

  constexpr int numThreads = TestUtil::nextInt(random(), 2, 10);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  // System.out.println("TEST: iter=" + iter + " opCount=" + opCount + "
  // idCount=" + idCount + " threadCount=" + threads.length);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  deque<deque<std::shared_ptr<Operation>>> threadOps =
      deque<deque<std::shared_ptr<Operation>>>();

  any commitLock;
  const deque<std::shared_ptr<Operation>> commits =
      deque<std::shared_ptr<Operation>>();

  // multiple threads update the same set of documents, and we randomly commit,
  // recording the commit seqNo and then opening each commit in the end to
  // verify it reflects the correct updates
  for (int i = 0; i < threads.size(); i++) {
    const deque<std::shared_ptr<Operation>> ops =
        deque<std::shared_ptr<Operation>>();
    threadOps.push_back(ops);
    constexpr int threadID = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), opCount, idCount, w, startingGun, commitLock,
        commits, i, ops, threadID);
    threads[i]->start();
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }

  shared_ptr<Operation> commitOp = make_shared<Operation>();
  commitOp->seqNo = w->commit();
  if (commitOp->seqNo != -1) {
    commits.push_back(commitOp);
  }

  deque<std::shared_ptr<IndexCommit>> indexCommits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(commits.size(), indexCommits.size());

  std::deque<int> expectedThreadIDs(idCount);
  std::deque<int64_t> seqNos(idCount);

  // System.out.println("TEST: " + commits.size() + " commits");
  for (int i = 0; i < commits.size(); i++) {
    // this commit point should reflect all operations <= this seqNo
    int64_t commitSeqNo = commits[i]->seqNo;
    // System.out.println("  commit " + i + ": seqNo=" + commitSeqNo + " segs="
    // + indexCommits.get(i));

    Arrays::fill(expectedThreadIDs, -1);
    Arrays::fill(seqNos, 0);

    for (int threadID = 0; threadID < threadOps.size(); threadID++) {
      int64_t lastSeqNo = 0;
      for (auto op : threadOps[threadID]) {
        if (op->seqNo <= commitSeqNo && op->seqNo > seqNos[op->id]) {
          seqNos[op->id] = op->seqNo;
          if (op->what == 0) {
            expectedThreadIDs[op->id] = threadID;
          } else {
            expectedThreadIDs[op->id] = -1;
          }
        }

        assertTrue(op->seqNo > lastSeqNo);
        lastSeqNo = op->seqNo;
      }
    }

    shared_ptr<DirectoryReader> r = DirectoryReader::open(indexCommits[i]);
    shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);

    for (int id = 0; id < idCount; id++) {
      // System.out.println("TEST: check id=" + id + " expectedThreadID=" +
      // expectedThreadIDs[id]);
      shared_ptr<TopDocs> hits =
          s->search(make_shared<TermQuery>(
                        make_shared<Term>(L"id", L"" + to_wstring(id))),
                    1);

      if (expectedThreadIDs[id] != -1) {
        TestUtil::assertEquals(1, hits->totalHits);
        shared_ptr<Document> doc = r->document(hits->scoreDocs[0]->doc);
        int actualThreadID =
            doc->getField(L"thread")->numericValue()->intValue();
        if (expectedThreadIDs[id] != actualThreadID) {
          wcout << L"FAIL: id=" << id << L" expectedThreadID="
                << expectedThreadIDs[id] << L" vs actualThreadID="
                << actualThreadID << L" commitSeqNo=" << commitSeqNo
                << L" numThreads=" << numThreads << endl;
          for (int threadID = 0; threadID < threadOps.size(); threadID++) {
            for (auto op : threadOps[threadID]) {
              if (id == op->id) {
                wcout << L"  threadID=" << threadID << L" seqNo=" << op->seqNo
                      << L" " << (op->what == 2 ? L"updated" : L"deleted")
                      << endl;
              }
            }
          }
          assertEquals(L"id=" + to_wstring(id), expectedThreadIDs[id],
                       actualThreadID);
        }
      } else if (hits->totalHits != 0) {
        wcout << L"FAIL: id=" << id << L" expectedThreadID="
              << expectedThreadIDs[id] << L" vs totalHits=" << hits->totalHits
              << L" commitSeqNo=" << commitSeqNo << L" numThreads="
              << numThreads << endl;
        for (int threadID = 0; threadID < threadOps.size(); threadID++) {
          for (auto op : threadOps[threadID]) {
            if (id == op->id) {
              wcout << L"  threadID=" << threadID << L" seqNo=" << op->seqNo
                    << L" " << (op->what == 2 ? L"updated" : L"del") << endl;
            }
          }
        }
        TestUtil::assertEquals(0, hits->totalHits);
      }
    }
    delete w;
    r->close();
  }

  delete dir;
}

TestIndexingSequenceNumbers::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestIndexingSequenceNumbers> outerInstance, int opCount,
        int idCount, shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<CountDownLatch> startingGun, any commitLock,
        deque<std::shared_ptr<Operation>> &commits, int i,
        deque<std::shared_ptr<Operation>> &ops, int threadID)
{
  this->outerInstance = outerInstance;
  this->opCount = opCount;
  this->idCount = idCount;
  this->w = w;
  this->startingGun = startingGun;
  this->commitLock = commitLock;
  this->commits = commits;
  this->i = i;
  this->ops = ops;
  this->threadID = threadID;
}

void TestIndexingSequenceNumbers::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    for (int i = 0; i < opCount; i++) {
      shared_ptr<Operation> op = make_shared<Operation>();
      op->threadID = threadID;
      if (LuceneTestCase::random()->nextInt(500) == 17) {
        op->what = 2;
        {
          lock_guard<mutex> lock(commitLock);
          op->seqNo = w->commit();
          if (op->seqNo != -1) {
            commits.push_back(op);
          }
        }
      } else {
        op->id = LuceneTestCase::random()->nextInt(idCount);
        shared_ptr<Term> idTerm =
            make_shared<Term>(L"id", L"" + to_wstring(op->id));
        if (LuceneTestCase::random()->nextInt(10) == 1) {
          op->what = 1;
          if (LuceneTestCase::random()->nextBoolean()) {
            op->seqNo = w->deleteDocuments(idTerm);
          } else {
            op->seqNo = w->deleteDocuments(make_shared<TermQuery>(idTerm));
          }
        } else {
          shared_ptr<Document> doc = make_shared<Document>();
          doc->push_back(make_shared<StoredField>(L"thread", threadID));
          doc->push_back(make_shared<StringField>(
              L"id", L"" + to_wstring(op->id), Field::Store::NO));
          if (LuceneTestCase::random()->nextBoolean()) {
            deque<std::shared_ptr<Document>> docs =
                deque<std::shared_ptr<Document>>();
            docs.push_back(doc);
            op->seqNo = w->updateDocuments(idTerm, docs);
          } else {
            op->seqNo = w->updateDocument(idTerm, doc);
          }
          op->what = 0;
        }
        ops.push_back(op);
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testStressConcurrentDocValuesUpdatesCommit()
// throws Exception
void TestIndexingSequenceNumbers::
    testStressConcurrentDocValuesUpdatesCommit() 
{
  constexpr int opCount = atLeast(10000);
  constexpr int idCount = TestUtil::nextInt(random(), 10, 1000);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE);

  // Cannot use RIW since it randomly commits:
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

  constexpr int numThreads = TestUtil::nextInt(random(), 2, 10);
  if (VERBOSE) {
    wcout << L"TEST: numThreads=" << numThreads << endl;
  }
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  // System.out.println("TEST: iter=" + iter + " opCount=" + opCount + "
  // idCount=" + idCount + " threadCount=" + threads.length);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  deque<deque<std::shared_ptr<Operation>>> threadOps =
      deque<deque<std::shared_ptr<Operation>>>();

  any commitLock;
  const deque<std::shared_ptr<Operation>> commits =
      deque<std::shared_ptr<Operation>>();

  deque<std::shared_ptr<Operation>> ops1 =
      deque<std::shared_ptr<Operation>>();
  threadOps.push_back(ops1);

  // pre-index every ID so none are missing:
  for (int id = 0; id < idCount; id++) {
    int threadID = 0;
    shared_ptr<Operation> op = make_shared<Operation>();
    op->threadID = threadID;
    op->id = id;

    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StoredField>(L"thread", threadID));
    doc->push_back(make_shared<NumericDocValuesField>(L"thread", threadID));
    doc->push_back(make_shared<StringField>(L"id", L"" + to_wstring(id),
                                            Field::Store::NO));
    op->seqNo = w->addDocument(doc);
    ops1.push_back(op);
  }

  // multiple threads update the same set of documents, and we randomly commit,
  // recording the commit seqNo and then opening each commit in the end to
  // verify it reflects the correct updates
  for (int i = 0; i < threads.size(); i++) {
    const deque<std::shared_ptr<Operation>> ops;
    if (i == 0) {
      ops = threadOps[0];
    } else {
      ops = deque<>();
      threadOps.push_back(ops);
    }

    constexpr int threadID = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), opCount, idCount, w, startingGun, commitLock,
        commits, i, ops, threadID);
    threads[i]->setName(L"thread" + to_wstring(i));
    threads[i]->start();
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }

  shared_ptr<Operation> commitOp = make_shared<Operation>();
  commitOp->seqNo = w->commit();
  if (commitOp->seqNo != -1) {
    commits.push_back(commitOp);
  }

  deque<std::shared_ptr<IndexCommit>> indexCommits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(commits.size(), indexCommits.size());

  std::deque<int> expectedThreadIDs(idCount);
  std::deque<int64_t> seqNos(idCount);

  // System.out.println("TEST: " + commits.size() + " commits");
  for (int i = 0; i < commits.size(); i++) {
    // this commit point should reflect all operations <= this seqNo
    int64_t commitSeqNo = commits[i]->seqNo;
    // System.out.println("  commit " + i + ": seqNo=" + commitSeqNo + " segs="
    // + indexCommits.get(i));

    Arrays::fill(expectedThreadIDs, -1);
    Arrays::fill(seqNos, 0);

    for (int threadID = 0; threadID < threadOps.size(); threadID++) {
      int64_t lastSeqNo = 0;
      for (auto op : threadOps[threadID]) {
        if (op->seqNo <= commitSeqNo && op->seqNo > seqNos[op->id]) {
          seqNos[op->id] = op->seqNo;
          assert(op->what == 0);
          expectedThreadIDs[op->id] = threadID;
        }

        assertTrue(op->seqNo > lastSeqNo);
        lastSeqNo = op->seqNo;
      }
    }

    shared_ptr<DirectoryReader> r = DirectoryReader::open(indexCommits[i]);
    shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);

    for (int id = 0; id < idCount; id++) {
      // System.out.println("TEST: check id=" + id + " expectedThreadID=" +
      // expectedThreadIDs[id]);
      shared_ptr<TopDocs> hits =
          s->search(make_shared<TermQuery>(
                        make_shared<Term>(L"id", L"" + to_wstring(id))),
                    1);
      shared_ptr<NumericDocValues> docValues =
          MultiDocValues::getNumericValues(r, L"thread");

      // We pre-add all ids up front:
      assert(expectedThreadIDs[id] != -1);
      TestUtil::assertEquals(1, hits->totalHits);
      int hitDoc = hits->scoreDocs[0]->doc;
      TestUtil::assertEquals(hitDoc, docValues->advance(hitDoc));
      int actualThreadID = static_cast<int>(docValues->longValue());
      if (expectedThreadIDs[id] != actualThreadID) {
        wcout << L"FAIL: commit=" << i << L" (of " << commits.size() << L") id="
              << id << L" expectedThreadID=" << expectedThreadIDs[id]
              << L" vs actualThreadID=" << actualThreadID << L" commitSeqNo="
              << commitSeqNo << L" numThreads=" << numThreads << L" reader="
              << r << L" commit=" << indexCommits[i] << endl;
        for (int threadID = 0; threadID < threadOps.size(); threadID++) {
          for (auto op : threadOps[threadID]) {
            if (id == op->id) {
              wcout << L"  threadID=" << threadID << L" seqNo=" << op->seqNo
                    << endl;
            }
          }
        }
        assertEquals(L"id=" + to_wstring(id) + L" docID=" +
                         to_wstring(hits->scoreDocs[0]->doc),
                     expectedThreadIDs[id], actualThreadID);
      }
    }
    delete w;
    r->close();
  }

  delete dir;
}

TestIndexingSequenceNumbers::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<TestIndexingSequenceNumbers> outerInstance, int opCount,
        int idCount, shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<CountDownLatch> startingGun, any commitLock,
        deque<std::shared_ptr<Operation>> &commits, int i,
        deque<std::shared_ptr<Operation>> &ops, int threadID)
{
  this->outerInstance = outerInstance;
  this->opCount = opCount;
  this->idCount = idCount;
  this->w = w;
  this->startingGun = startingGun;
  this->commitLock = commitLock;
  this->commits = commits;
  this->i = i;
  this->ops = ops;
  this->threadID = threadID;
}

void TestIndexingSequenceNumbers::ThreadAnonymousInnerClass2::run()
{
  try {
    startingGun->await();
    for (int i = 0; i < opCount; i++) {
      shared_ptr<Operation> op = make_shared<Operation>();
      op->threadID = threadID;
      if (LuceneTestCase::random()->nextInt(500) == 17) {
        op->what = 2;
        {
          lock_guard<mutex> lock(commitLock);
          op->seqNo = w->commit();
          if (op->seqNo != -1) {
            commits.push_back(op);
          }
        }
      } else {
        op->id = LuceneTestCase::random()->nextInt(idCount);
        shared_ptr<Term> idTerm =
            make_shared<Term>(L"id", L"" + to_wstring(op->id));
        op->seqNo = w->updateNumericDocValue(idTerm, L"thread", threadID);
        op->what = 0;
        ops.push_back(op);
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testStressConcurrentAddAndDeleteAndCommit()
// throws Exception
void TestIndexingSequenceNumbers::
    testStressConcurrentAddAndDeleteAndCommit() 
{
  constexpr int opCount = atLeast(10000);
  constexpr int idCount = TestUtil::nextInt(random(), 10, 1000);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE);

  // Cannot use RIW since it randomly commits:
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

  constexpr int numThreads = TestUtil::nextInt(random(), 2, 5);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  // System.out.println("TEST: iter=" + iter + " opCount=" + opCount + "
  // idCount=" + idCount + " threadCount=" + threads.length);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  deque<deque<std::shared_ptr<Operation>>> threadOps =
      deque<deque<std::shared_ptr<Operation>>>();

  any commitLock;
  const deque<std::shared_ptr<Operation>> commits =
      deque<std::shared_ptr<Operation>>();

  // multiple threads update the same set of documents, and we randomly commit
  for (int i = 0; i < threads.size(); i++) {
    const deque<std::shared_ptr<Operation>> ops =
        deque<std::shared_ptr<Operation>>();
    threadOps.push_back(ops);
    constexpr int threadID = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass3>(
        shared_from_this(), opCount, idCount, w, startingGun, commitLock,
        commits, i, ops, threadID);
    threads[i]->setName(L"thread" + to_wstring(threadID));
    threads[i]->start();
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }

  shared_ptr<Operation> commitOp = make_shared<Operation>();
  commitOp->seqNo = w->commit();
  if (commitOp->seqNo != -1) {
    commits.push_back(commitOp);
  }

  deque<std::shared_ptr<IndexCommit>> indexCommits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(commits.size(), indexCommits.size());

  // how many docs with this id are expected:
  std::deque<int> expectedCounts(idCount);
  std::deque<int64_t> lastDelSeqNos(idCount);

  // System.out.println("TEST: " + commits.size() + " commits");
  for (int i = 0; i < commits.size(); i++) {
    // this commit point should reflect all operations <= this seqNo
    int64_t commitSeqNo = commits[i]->seqNo;
    // System.out.println("  commit " + i + ": seqNo=" + commitSeqNo + " segs="
    // + indexCommits.get(i));

    // first find the highest seqNo of the last delete op, for each id, prior to
    // this commit:
    Arrays::fill(lastDelSeqNos, -1);
    for (int threadID = 0; threadID < threadOps.size(); threadID++) {
      int64_t lastSeqNo = 0;
      for (auto op : threadOps[threadID]) {
        if (op->what == 1 && op->seqNo <= commitSeqNo &&
            op->seqNo > lastDelSeqNos[op->id]) {
          lastDelSeqNos[op->id] = op->seqNo;
        }

        // within one thread the seqNos must only increase:
        assertTrue(op->seqNo > lastSeqNo);
        lastSeqNo = op->seqNo;
      }
    }

    // then count how many adds happened since the last delete and before this
    // commit:
    Arrays::fill(expectedCounts, 0);
    for (int threadID = 0; threadID < threadOps.size(); threadID++) {
      for (auto op : threadOps[threadID]) {
        if (op->what == 3 && op->seqNo <= commitSeqNo &&
            op->seqNo > lastDelSeqNos[op->id]) {
          expectedCounts[op->id]++;
        }
      }
    }

    shared_ptr<DirectoryReader> r = DirectoryReader::open(indexCommits[i]);
    shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);

    for (int id = 0; id < idCount; id++) {
      // System.out.println("TEST: check id=" + id + " expectedThreadID=" +
      // expectedThreadIDs[id]);
      int actualCount = s->count(make_shared<TermQuery>(
          make_shared<Term>(L"id", L"" + to_wstring(id))));
      if (expectedCounts[id] != actualCount) {
        wcout << L"TEST: FAIL r=" << r << L" id=" << id << L" commitSeqNo="
              << commitSeqNo << endl;
        for (int threadID = 0; threadID < threadOps.size(); threadID++) {
          int opCount2 = 0;
          for (auto op : threadOps[threadID]) {
            if (op->id == id) {
              bool shouldCount =
                  op->seqNo <= commitSeqNo && op->seqNo > lastDelSeqNos[op->id];
              wcout << L"  id=" << id << L" what=" << op->what << L" threadop="
                    << threadID << L"-" << opCount2 << L" seqNo=" << op->seqNo
                    << L" vs lastDelSeqNo=" << lastDelSeqNos[op->id]
                    << L" shouldCount=" << shouldCount << endl;
            }
            opCount2++;
          }
        }
        shared_ptr<TopDocs> hits =
            s->search(make_shared<TermQuery>(
                          make_shared<Term>(L"id", L"" + to_wstring(id))),
                      1 + actualCount);
        for (auto hit : hits->scoreDocs) {
          wcout << L"  hit: " << s->doc(hit->doc)[L"threadop"] << endl;
        }

        for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
          wcout << L"  sub=" << ctx->reader() << endl;
          shared_ptr<Bits> liveDocs = ctx->reader()->getLiveDocs();
          for (int docID = 0; docID < ctx->reader()->maxDoc(); docID++) {
            wcout << L"    docID=" << docID << L" threadop="
                  << ctx->reader()->document(docID)[L"threadop"]
                  << (liveDocs != nullptr && liveDocs->get(docID) == false
                          ? L" (deleted)"
                          : L"")
                  << endl;
          }
        }

        assertEquals(L"commit " + to_wstring(i) + L" of " + commits.size() +
                         L" id=" + to_wstring(id) + L" reader=" + r,
                     expectedCounts[id], actualCount);
      }
    }
    delete w;
    r->close();
  }

  delete dir;
}

TestIndexingSequenceNumbers::ThreadAnonymousInnerClass3::
    ThreadAnonymousInnerClass3(
        shared_ptr<TestIndexingSequenceNumbers> outerInstance, int opCount,
        int idCount, shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<CountDownLatch> startingGun, any commitLock,
        deque<std::shared_ptr<Operation>> &commits, int i,
        deque<std::shared_ptr<Operation>> &ops, int threadID)
{
  this->outerInstance = outerInstance;
  this->opCount = opCount;
  this->idCount = idCount;
  this->w = w;
  this->startingGun = startingGun;
  this->commitLock = commitLock;
  this->commits = commits;
  this->i = i;
  this->ops = ops;
  this->threadID = threadID;
}

void TestIndexingSequenceNumbers::ThreadAnonymousInnerClass3::run()
{
  try {
    startingGun->await();
    for (int i = 0; i < opCount; i++) {
      shared_ptr<Operation> op = make_shared<Operation>();
      op->threadID = threadID;
      if (LuceneTestCase::random()->nextInt(500) == 17) {
        op->what = 2;
        {
          lock_guard<mutex> lock(commitLock);
          op->seqNo = w->commit();
          if (op->seqNo != -1) {
            commits.push_back(op);
          }
        }
      } else {
        op->id = LuceneTestCase::random()->nextInt(idCount);
        shared_ptr<Term> idTerm =
            make_shared<Term>(L"id", L"" + to_wstring(op->id));
        if (LuceneTestCase::random()->nextInt(10) == 1) {
          op->what = 1;
          if (LuceneTestCase::random()->nextBoolean()) {
            op->seqNo = w->deleteDocuments({idTerm});
          } else {
            op->seqNo = w->deleteDocuments({make_shared<TermQuery>(idTerm)});
          }
        } else {
          shared_ptr<Document> doc = make_shared<Document>();
          doc->push_back(make_shared<StoredField>(
              L"threadop", to_wstring(threadID) + L"-" + ops.size()));
          doc->push_back(make_shared<StringField>(
              L"id", L"" + to_wstring(op->id), Field::Store::NO));
          if (LuceneTestCase::random()->nextBoolean()) {
            deque<std::shared_ptr<Document>> docs =
                deque<std::shared_ptr<Document>>();
            docs.push_back(doc);
            op->seqNo = w->addDocuments(docs);
          } else {
            op->seqNo = w->addDocument(doc);
          }
          op->what = 3;
        }
        ops.push_back(op);
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestIndexingSequenceNumbers::testDeleteAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  int64_t a = w->addDocument(make_shared<Document>());
  int64_t b = w->deleteAll();
  assertTrue(a < b);
  int64_t c = w->commit();
  assertTrue(b < c);
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index