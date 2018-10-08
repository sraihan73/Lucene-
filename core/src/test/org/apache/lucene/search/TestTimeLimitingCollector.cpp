using namespace std;

#include "TestTimeLimitingCollector.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TimeExceededException =
    org::apache::lucene::search::TimeLimitingCollector::TimeExceededException;
using TimerThread =
    org::apache::lucene::search::TimeLimitingCollector::TimerThread;
using Directory = org::apache::lucene::store::Directory;
using Counter = org::apache::lucene::util::Counter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

void TestTimeLimitingCollector::setUp() 
{
  LuceneTestCase::setUp();
  counter = Counter::newCounter(true);
  counterThread = make_shared<TimerThread>(counter);
  counterThread->start();
  const std::deque<wstring> docText = {
      L"docThatNeverMatchesSoWeCanRequireLastDocCollectedToBeGreaterThanZero",
      L"one blah three",
      L"one foo three multiOne",
      L"one foobar three multiThree",
      L"blueberry pancakes",
      L"blueberry pie",
      L"blueberry strudel",
      L"blueberry pizza"};
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  for (int i = 0; i < N_DOCS; i++) {
    add(docText[i % docText.size()], iw);
  }
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"one")),
      BooleanClause::Occur::SHOULD);
  // start from 1, so that the 0th doc never matches
  for (int i = 1; i < docText.size(); i++) {
    std::deque<wstring> docTextParts = docText[i].split(L"\\s+");
    for (auto docTextPart :
         docTextParts) { // large query so that search will be longer
      booleanQuery->add(
          make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, docTextPart)),
          BooleanClause::Occur::SHOULD);
    }
  }

  query = booleanQuery->build();

  // warm the searcher
  searcher->search(query, 1000);
}

void TestTimeLimitingCollector::tearDown() 
{
  delete reader;
  delete directory;
  counterThread->stopTimer();
  counterThread->join();
  LuceneTestCase::tearDown();
}

void TestTimeLimitingCollector::add(
    const wstring &value, shared_ptr<RandomIndexWriter> iw) 
{
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(FIELD_NAME, value, Field::Store::NO));
  iw->addDocument(d);
}

void TestTimeLimitingCollector::search(shared_ptr<Collector> collector) throw(
    runtime_error)
{
  searcher->search(query, collector);
}

void TestTimeLimitingCollector::testSearch() { doTestSearch(); }

void TestTimeLimitingCollector::doTestSearch()
{
  int totalResults = 0;
  int totalTLCResults = 0;
  try {
    shared_ptr<MyHitCollector> myHc = make_shared<MyHitCollector>();
    search(myHc);
    totalResults = myHc->hitCount();

    myHc = make_shared<MyHitCollector>();
    int64_t oneHour = 3600000;
    int64_t duration =
        TestUtil::nextLong(random(), oneHour, numeric_limits<int64_t>::max());
    shared_ptr<Collector> tlCollector =
        createTimedCollector(myHc, duration, false);
    search(tlCollector);
    totalTLCResults = myHc->hitCount();
  } catch (const runtime_error &e) {
    e.printStackTrace();
    assertTrue(L"Unexpected exception: " + e, false); //==fail
  }
  assertEquals(L"Wrong number of results!", totalResults, totalTLCResults);
}

shared_ptr<Collector> TestTimeLimitingCollector::createTimedCollector(
    shared_ptr<MyHitCollector> hc, int64_t timeAllowed, bool greedy)
{
  shared_ptr<TimeLimitingCollector> res =
      make_shared<TimeLimitingCollector>(hc, counter, timeAllowed);
  res->setGreedy(
      greedy); // set to true to make sure at least one doc is collected.
  return res;
}

void TestTimeLimitingCollector::testTimeoutGreedy()
{
  doTestTimeout(false, true);
}

void TestTimeLimitingCollector::testTimeoutNotGreedy()
{
  doTestTimeout(false, false);
}

void TestTimeLimitingCollector::doTestTimeout(bool multiThreaded, bool greedy)
{
  // setup
  shared_ptr<MyHitCollector> myHc = make_shared<MyHitCollector>();
  myHc->setSlowDown(SLOW_DOWN);
  shared_ptr<Collector> tlCollector =
      createTimedCollector(myHc, TIME_ALLOWED, greedy);

  // search: must get exception
  shared_ptr<TimeExceededException> timeoutException = expectThrows(
      TimeExceededException::typeid, [&]() { search(tlCollector); });

  // greediness affect last doc collected
  int exceptionDoc = timeoutException->getLastDocCollected();
  int lastCollected = myHc->getLastDocCollected();

  // exceptionDoc == -1 means we hit the timeout in getLeafCollector:
  if (exceptionDoc != -1) {
    assertTrue(L"doc collected at timeout must be > 0! or == -1 but was: " +
                   to_wstring(exceptionDoc),
               exceptionDoc > 0);
    if (greedy) {
      assertTrue(L"greedy=" + StringHelper::toString(greedy) +
                     L" exceptionDoc=" + to_wstring(exceptionDoc) +
                     L" != lastCollected=" + to_wstring(lastCollected),
                 exceptionDoc == lastCollected);
      assertTrue(L"greedy, but no hits found!", myHc->hitCount() > 0);
    } else {
      assertTrue(L"greedy=" + StringHelper::toString(greedy) +
                     L" exceptionDoc=" + to_wstring(exceptionDoc) +
                     L" not > lastCollected=" + to_wstring(lastCollected),
                 exceptionDoc > lastCollected);
    }
  }

  // verify that elapsed time at exception is within valid limits
  TestUtil::assertEquals(timeoutException->getTimeAllowed(), TIME_ALLOWED);
  // a) Not too early
  assertTrue(L"elapsed=" + to_wstring(timeoutException->getTimeElapsed()) +
                 L" <= (allowed-resolution)=" +
                 to_wstring(TIME_ALLOWED - counterThread->getResolution()),
             timeoutException->getTimeElapsed() >
                 TIME_ALLOWED - counterThread->getResolution());
  // b) Not too late.
  //    This part is problematic in a busy test system, so we just print a
  //    warning. We already verified that a timeout occurred, we just can't be
  //    picky about how long it took.
  if (timeoutException->getTimeElapsed() > maxTime(multiThreaded)) {
    wcout << L"Informative: timeout exceeded (no action required: most "
             L"probably just "
          << L" because the test machine is slower than usual):  "
          << L"lastDoc=" << exceptionDoc << L" ,&& allowed="
          << timeoutException->getTimeAllowed() << L" ,&& elapsed="
          << timeoutException->getTimeElapsed() << L" >= "
          << maxTimeStr(multiThreaded) << endl;
  }
}

int64_t TestTimeLimitingCollector::maxTime(bool multiThreaded)
{
  int64_t res = 2 * counterThread->getResolution() + TIME_ALLOWED +
                  SLOW_DOWN; // some slack for less noise in this test
  if (multiThreaded) {
    res *= MULTI_THREAD_SLACK; // larger slack
  }
  return res;
}

wstring TestTimeLimitingCollector::maxTimeStr(bool multiThreaded)
{
  wstring s = wstring(L"( ") + L"2*resolution +  TIME_ALLOWED + SLOW_DOWN = " +
              L"2*" + to_wstring(counterThread->getResolution()) + L" + " +
              to_wstring(TIME_ALLOWED) + L" + " + to_wstring(SLOW_DOWN) + L")";
  if (multiThreaded) {
    s = to_wstring(MULTI_THREAD_SLACK) + L" * " + s;
  }
  return to_wstring(maxTime(multiThreaded)) + L" = " + s;
}

void TestTimeLimitingCollector::testModifyResolution()
{
  try {
    // increase and test
    int64_t resolution = 20 * TimerThread::DEFAULT_RESOLUTION; // 400
    counterThread->setResolution(resolution);
    TestUtil::assertEquals(resolution, counterThread->getResolution());
    doTestTimeout(false, true);
    // decrease much and test
    resolution = 5;
    counterThread->setResolution(resolution);
    TestUtil::assertEquals(resolution, counterThread->getResolution());
    doTestTimeout(false, true);
    // return to default and test
    resolution = TimerThread::DEFAULT_RESOLUTION;
    counterThread->setResolution(resolution);
    TestUtil::assertEquals(resolution, counterThread->getResolution());
    doTestTimeout(false, true);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    counterThread->setResolution(TimerThread::DEFAULT_RESOLUTION);
  }
}

void TestTimeLimitingCollector::testNoHits() 
{
  shared_ptr<MyHitCollector> myHc = make_shared<MyHitCollector>();
  shared_ptr<Collector> collector =
      createTimedCollector(myHc, -1, random()->nextBoolean());
  // search: must get exception
  expectThrows(TimeExceededException::typeid, [&]() {
    shared_ptr<BooleanQuery::Builder> booleanQuery =
        make_shared<BooleanQuery::Builder>();
    booleanQuery->add(
        make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"one")),
        BooleanClause::Occur::MUST);
    booleanQuery->add(
        make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"blueberry")),
        BooleanClause::Occur::MUST);
    searcher->search(booleanQuery->build(), collector);
  });
  TestUtil::assertEquals(-1, myHc->getLastDocCollected());
}

void TestTimeLimitingCollector::testSearchMultiThreaded() 
{
  doTestMultiThreads(false);
}

void TestTimeLimitingCollector::testTimeoutMultiThreaded() 
{
  doTestMultiThreads(true);
}

void TestTimeLimitingCollector::doTestMultiThreads(
    bool const withTimeout) 
{
  std::deque<std::shared_ptr<Thread>> threadArray(N_THREADS);
  shared_ptr<BitSet> *const success = make_shared<BitSet>(N_THREADS);
  for (int i = 0; i < threadArray.size(); ++i) {
    constexpr int num = i;
    threadArray[num] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), withTimeout, success, num);
  }
  for (int i = 0; i < threadArray.size(); ++i) {
    threadArray[i]->start();
  }
  for (int i = 0; i < threadArray.size(); ++i) {
    threadArray[i]->join();
  }
  assertEquals(L"some threads failed!", N_THREADS, success->cardinality());
}

TestTimeLimitingCollector::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestTimeLimitingCollector> outerInstance, bool withTimeout,
    shared_ptr<BitSet> success, int num)
{
  this->outerInstance = outerInstance;
  this->withTimeout = withTimeout;
  this->success = success;
  this->num = num;
}

void TestTimeLimitingCollector::ThreadAnonymousInnerClass::run()
{
  if (withTimeout) {
    outerInstance->doTestTimeout(true, true);
  } else {
    outerInstance->doTestSearch();
  }
  {
    lock_guard<mutex> lock(success);
    success->set(num);
  }
}

void TestTimeLimitingCollector::MyHitCollector::setSlowDown(int milliseconds)
{
  slowdown = milliseconds;
}

int TestTimeLimitingCollector::MyHitCollector::hitCount()
{
  return bits->cardinality();
}

int TestTimeLimitingCollector::MyHitCollector::getLastDocCollected()
{
  return lastDocCollected;
}

void TestTimeLimitingCollector::MyHitCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  // scorer is not needed
}

void TestTimeLimitingCollector::MyHitCollector::collect(int const doc) throw(
    IOException)
{
  int docId = doc + docBase;
  if (slowdown > 0) {
    try {
      delay(slowdown);
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  }
  assert((docId >= 0,
          L" base=" + to_wstring(docBase) + L" doc=" + to_wstring(doc)));
  bits->set(docId);
  lastDocCollected = docId;
}

void TestTimeLimitingCollector::MyHitCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

bool TestTimeLimitingCollector::MyHitCollector::needsScores() { return false; }
} // namespace org::apache::lucene::search