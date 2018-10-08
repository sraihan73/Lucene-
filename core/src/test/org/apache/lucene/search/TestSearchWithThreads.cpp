using namespace std;

#include "TestSearchWithThreads.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;

void TestSearchWithThreads::setUp() 
{
  LuceneTestCase::setUp();
  NUM_DOCS = atLeast(10000);
  RUN_TIME_MSEC = atLeast(1000);
}

void TestSearchWithThreads::test() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  constexpr int64_t startTime = System::currentTimeMillis();

  // TODO: replace w/ the @nightly test data; make this
  // into an optional @nightly stress test
  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<Field> *const body = newTextField(L"body", L"", Field::Store::NO);
  doc->push_back(body);
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  for (int docCount = 0; docCount < NUM_DOCS; docCount++) {
    constexpr int numTerms = random()->nextInt(10);
    for (int termCount = 0; termCount < numTerms; termCount++) {
      sb->append(random()->nextBoolean() ? L"aaa" : L"bbb");
      sb->append(L' ');
    }
    body->setStringValue(sb->toString());
    w->addDocument(doc);
    sb->remove(0, sb->length());
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  constexpr int64_t endTime = System::currentTimeMillis();
  if (VERBOSE) {
    wcout << L"BUILD took " << (endTime - startTime) << endl;
  }

  shared_ptr<IndexSearcher> *const s = newSearcher(r);

  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();
  shared_ptr<AtomicLong> *const netSearch = make_shared<AtomicLong>();

  std::deque<std::shared_ptr<Thread>> threads(NUM_SEARCH_THREADS);
  for (int threadID = 0; threadID < NUM_SEARCH_THREADS; threadID++) {
    threads[threadID] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), s, failed, netSearch);
    threads[threadID]->setDaemon(true);
  }

  for (auto t : threads) {
    t->start();
  }

  for (auto t : threads) {
    t->join();
  }

  if (VERBOSE) {
    wcout << NUM_SEARCH_THREADS << L" threads did " << netSearch->get()
          << L" searches" << endl;
  }

  delete r;
  delete dir;
}

TestSearchWithThreads::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestSearchWithThreads> outerInstance,
    shared_ptr<org::apache::lucene::search::IndexSearcher> s,
    shared_ptr<AtomicBoolean> failed, shared_ptr<AtomicLong> netSearch)
{
  this->outerInstance = outerInstance;
  this->s = s;
  this->failed = failed;
  this->netSearch = netSearch;
  col = make_shared<TotalHitCountCollector>();
}

void TestSearchWithThreads::ThreadAnonymousInnerClass::run()
{
  try {
    int64_t totHits = 0;
    int64_t totSearch = 0;
    int64_t stopAt =
        System::currentTimeMillis() + outerInstance->RUN_TIME_MSEC;
    while (System::currentTimeMillis() < stopAt && !failed->get()) {
      s->search(make_shared<TermQuery>(make_shared<Term>(L"body", L"aaa")),
                col);
      totHits += col::getTotalHits();
      s->search(make_shared<TermQuery>(make_shared<Term>(L"body", L"bbb")),
                col);
      totHits += col::getTotalHits();
      totSearch++;
    }
    assertTrue(totSearch > 0 && totHits > 0);
    netSearch->addAndGet(totSearch);
  } catch (const runtime_error &exc) {
    failed->set(true);
    throw runtime_error(exc);
  }
}
} // namespace org::apache::lucene::search