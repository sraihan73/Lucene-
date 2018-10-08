using namespace std;

#include "TestLiveFieldValues.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StoredField = org::apache::lucene::document::StoredField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestLiveFieldValues::test() 
{

  shared_ptr<Directory> dir =
      newFSDirectory(createTempDir(L"livefieldupdates"));
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));

  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<SearcherManager> *const mgr = make_shared<SearcherManager>(
      w, make_shared<SearcherFactoryAnonymousInnerClass>(shared_from_this()));

  const optional<int> missing = -1;

  shared_ptr<LiveFieldValues<std::shared_ptr<IndexSearcher>, int>> *const rt =
      make_shared<LiveFieldValuesAnonymousInnerClass>(shared_from_this());

  int numThreads = TestUtil::nextInt(random(), 2, 5);
  if (VERBOSE) {
    wcout << numThreads << L" threads" << endl;
  }

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  deque<std::shared_ptr<Thread>> threads = deque<std::shared_ptr<Thread>>();

  constexpr int iters = atLeast(1000);
  constexpr int idCount = TestUtil::nextInt(random(), 100, 10000);

  constexpr double reopenChance = random()->nextDouble() * 0.01;
  constexpr double deleteChance = random()->nextDouble() * 0.25;
  constexpr double addChance = random()->nextDouble() * 0.5;

  for (int t = 0; t < numThreads; t++) {
    constexpr int threadID = t;
    shared_ptr<Random> *const threadRandom =
        make_shared<Random>(random()->nextLong());
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), w, mgr, missing, rt, startingGun, iters, idCount,
        reopenChance, deleteChance, addChance, t, threadID, threadRandom);
    threads.push_back(thread);
    thread->start();
  }

  startingGun->countDown();

  for (auto thread : threads) {
    thread->join();
  }
  mgr->maybeRefresh();
  TestUtil::assertEquals(0, rt->size());

  delete rt;
  delete mgr;
  delete w;
  delete dir;
}

TestLiveFieldValues::SearcherFactoryAnonymousInnerClass::
    SearcherFactoryAnonymousInnerClass(
        shared_ptr<TestLiveFieldValues> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<IndexSearcher>
TestLiveFieldValues::SearcherFactoryAnonymousInnerClass::LuceneTestCase::
    newSearcher(shared_ptr<IndexReader> r, shared_ptr<IndexReader> previous)
{
  return make_shared<IndexSearcher>(r);
}

TestLiveFieldValues::LiveFieldValuesAnonymousInnerClass::
    LiveFieldValuesAnonymousInnerClass(
        shared_ptr<TestLiveFieldValues> outerInstance)
    : LiveFieldValues<IndexSearcher, int>(mgr, missing)
{
  this->outerInstance = outerInstance;
}

optional<int>
TestLiveFieldValues::LiveFieldValuesAnonymousInnerClass::lookupFromSearcher(
    shared_ptr<IndexSearcher> s, const wstring &id) 
{
  shared_ptr<TermQuery> tq =
      make_shared<TermQuery>(make_shared<Term>(L"id", id));
  shared_ptr<TopDocs> hits = s->search(tq, 1);
  assertTrue(hits->totalHits <= 1);
  if (hits->totalHits == 0) {
    return nullopt;
  } else {
    shared_ptr<Document> doc = s->doc(hits->scoreDocs[0]->doc);
    return static_cast<optional<int>>(doc->getField(L"field")->numericValue());
  }
}

TestLiveFieldValues::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestLiveFieldValues> outerInstance, shared_ptr<IndexWriter> w,
    shared_ptr<org::apache::lucene::search::SearcherManager> mgr,
    optional<int> &missing,
    shared_ptr<org::apache::lucene::search::LiveFieldValues<
        std::shared_ptr<IndexSearcher>, int>>
        rt,
    shared_ptr<CountDownLatch> startingGun, int iters, int idCount,
    double reopenChance, double deleteChance, double addChance, int t,
    int threadID, shared_ptr<Random> threadRandom)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->mgr = mgr;
  this->missing = missing;
  this->rt = rt;
  this->startingGun = startingGun;
  this->iters = iters;
  this->idCount = idCount;
  this->reopenChance = reopenChance;
  this->deleteChance = deleteChance;
  this->addChance = addChance;
  this->t = t;
  this->threadID = threadID;
  this->threadRandom = threadRandom;
}

void TestLiveFieldValues::ThreadAnonymousInnerClass::run()
{
  try {
    unordered_map<wstring, int> values = unordered_map<wstring, int>();
    deque<wstring> allIDs = Collections::synchronizedList(deque<wstring>());

    startingGun->await();
    for (int iter = 0; iter < iters; iter++) {
      // Add/update a document
      shared_ptr<Document> doc = make_shared<Document>();
      // Threads must not update the same id at the
      // same time:
      if (threadRandom->nextDouble() <= addChance) {
        wstring id = wstring::format(Locale::ROOT, L"%d_%04x", threadID,
                                     threadRandom->nextInt(idCount));
        optional<int> field = threadRandom->nextInt(numeric_limits<int>::max());
        doc->push_back(LuceneTestCase::newStringField(
            L"id", make_shared<BytesRef>(id), Field::Store::YES));
        doc->push_back(make_shared<StoredField>(L"field", field.value()));
        w->updateDocument(make_shared<Term>(L"id", id), doc);
        rt->add(id, field);
        if (values.emplace(id, field) == nullptr) {
          allIDs.push_back(id);
        }
      }

      if (allIDs.size() > 0 && threadRandom->nextDouble() <= deleteChance) {
        wstring randomID = allIDs[threadRandom->nextInt(allIDs.size())];
        w->deleteDocuments(
            {make_shared<Term>(L"id", make_shared<BytesRef>(randomID))});
        rt->delete (randomID);
        values.emplace(randomID, missing);
      }

      if (threadRandom->nextDouble() <= reopenChance || rt->size() > 10000) {
        // System.out.println("refresh @ " + rt.size());
        mgr->maybeRefresh();
        if (VERBOSE) {
          shared_ptr<IndexSearcher> s = mgr->acquire();
          try {
            wcout << L"TEST: reopen " << s << endl;
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            mgr->release(s);
          }
          wcout << L"TEST: " << values.size() << L" values" << endl;
        }
      }

      if (threadRandom->nextInt(10) == 7) {
        TestUtil::assertEquals(nullptr, rt->get(L"foo"));
      }

      if (allIDs.size() > 0) {
        wstring randomID = allIDs[threadRandom->nextInt(allIDs.size())];
        optional<int> expected = values[randomID];
        if (expected == missing) {
          expected = nullopt;
        }
        assertEquals(L"id=" + randomID, expected, rt->get(randomID));
      }
    }
  } catch (const runtime_error &t) {
    throw runtime_error(t);
  }
}
} // namespace org::apache::lucene::search