using namespace std;

#include "TestStressDeletes.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestStressDeletes::test() 
{
  constexpr int numIDs = atLeast(100);
  const std::deque<any> locks = std::deque<any>(numIDs);
  for (int i = 0; i < locks.size(); i++) {
    locks[i] = any();
  }

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
  constexpr int iters = atLeast(2000);
  const unordered_map<int, bool> exists =
      make_shared<ConcurrentHashMap<int, bool>>();
  std::deque<std::shared_ptr<Thread>> threads(
      TestUtil::nextInt(random(), 2, 6));
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  constexpr int deleteMode = random()->nextInt(3);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), numIDs, locks, w, iters, exists, startingGun,
        deleteMode);
    threads[i]->start();
  }

  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  for (auto ent : exists) {
    int id = ent.first;
    shared_ptr<TopDocs> hits = s->search(
        make_shared<TermQuery>(make_shared<Term>(L"id", L"" + to_wstring(id))),
        1);
    if (ent.second) {
      TestUtil::assertEquals(1, hits->totalHits);
    } else {
      TestUtil::assertEquals(0, hits->totalHits);
    }
  }
  delete r;
  delete w;
  delete dir;
}

TestStressDeletes::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestStressDeletes> outerInstance, int numIDs, deque<any> &locks,
    shared_ptr<org::apache::lucene::index::IndexWriter> w, int iters,
    unordered_map<int, bool> &exists, shared_ptr<CountDownLatch> startingGun,
    int deleteMode)
{
  this->outerInstance = outerInstance;
  this->numIDs = numIDs;
  this->locks = locks;
  this->w = w;
  this->iters = iters;
  this->exists = exists;
  this->startingGun = startingGun;
  this->deleteMode = deleteMode;
}

void TestStressDeletes::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    for (int iter = 0; iter < iters; iter++) {
      int id = LuceneTestCase::random()->nextInt(numIDs);
      {
        lock_guard<mutex> lock(locks[id]);
        optional<bool> v = exists[id];
        if (!v || v.value() == false) {
          shared_ptr<Document> doc = make_shared<Document>();
          doc->push_back(LuceneTestCase::newStringField(
              L"id", L"" + to_wstring(id), Field::Store::NO));
          w->addDocument(doc);
          exists.emplace(id, true);
        } else {
          if (deleteMode == 0) {
            // Always delete by term
            w->deleteDocuments(
                {make_shared<Term>(L"id", L"" + to_wstring(id))});
          } else if (deleteMode == 1) {
            // Always delete by query
            w->deleteDocuments({make_shared<TermQuery>(
                make_shared<Term>(L"id", L"" + to_wstring(id)))});
          } else {
            // Mixed
            if (LuceneTestCase::random()->nextBoolean()) {
              w->deleteDocuments(
                  {make_shared<Term>(L"id", L"" + to_wstring(id))});
            } else {
              w->deleteDocuments({make_shared<TermQuery>(
                  make_shared<Term>(L"id", L"" + to_wstring(id)))});
            }
          }
          exists.emplace(id, false);
        }
      }
      if (LuceneTestCase::random()->nextInt(500) == 2) {
        DirectoryReader::open(w, LuceneTestCase::random()->nextBoolean(), false)
            ->close();
      }
      if (LuceneTestCase::random()->nextInt(500) == 2) {
        w->commit();
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::index