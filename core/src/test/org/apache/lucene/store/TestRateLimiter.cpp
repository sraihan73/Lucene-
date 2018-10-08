using namespace std;

#include "TestRateLimiter.h"

namespace org::apache::lucene::store
{
using SimpleRateLimiter =
    org::apache::lucene::store::RateLimiter::SimpleRateLimiter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

void TestRateLimiter::testOverflowInt() 
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this());
  t->start();
  delay(10);
  t->interrupt();
}

TestRateLimiter::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestRateLimiter> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestRateLimiter::ThreadAnonymousInnerClass::run()
{
  try {
    (make_shared<SimpleRateLimiter>(1))
        ->pause(static_cast<int64_t>(1.5 * numeric_limits<int>::max() * 1024 *
                                       1024 / 1000));
    fail(L"should have been interrupted");
  } catch (const ThreadInterruptedException &tie) {
    // expected
  }
}

void TestRateLimiter::testThreads() 
{

  double targetMBPerSec = 10.0 + 20 * random()->nextDouble();
  shared_ptr<SimpleRateLimiter> *const limiter =
      make_shared<SimpleRateLimiter>(targetMBPerSec);

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);

  std::deque<std::shared_ptr<Thread>> threads(
      TestUtil::nextInt(random(), 3, 6));
  shared_ptr<AtomicLong> *const totBytes = make_shared<AtomicLong>();
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), limiter, startingGun, totBytes, i);
    threads[i]->start();
  }

  int64_t startNS = System::nanoTime();
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }
  int64_t endNS = System::nanoTime();
  double actualMBPerSec =
      (totBytes->get() / 1024 / 1024.0) / ((endNS - startNS) / 1000000000.0);

  // TODO: this may false trip .... could be we can only assert that it never
  // exceeds the max, so slow jenkins doesn't trip:
  double ratio = actualMBPerSec / targetMBPerSec;

  // Only enforce that it wasn't too fast; if machine is bogged down (can't
  // schedule threads / sleep properly) then it may falsely be too slow:
  assumeTrue(L"actualMBPerSec=" + to_wstring(actualMBPerSec) +
                 L" targetMBPerSec=" + to_wstring(targetMBPerSec),
             0.9 <= ratio);
  assertTrue(L"targetMBPerSec=" + to_wstring(targetMBPerSec) +
                 L" actualMBPerSec=" + to_wstring(actualMBPerSec),
             ratio <= 1.1);
}

TestRateLimiter::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestRateLimiter> outerInstance,
    shared_ptr<SimpleRateLimiter> limiter,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicLong> totBytes,
    int i)
{
  this->outerInstance = outerInstance;
  this->limiter = limiter;
  this->startingGun = startingGun;
  this->totBytes = totBytes;
  this->i = i;
}

void TestRateLimiter::ThreadAnonymousInnerClass2::run()
{
  try {
    startingGun->await();
  } catch (const InterruptedException &ie) {
    throw make_shared<ThreadInterruptedException>(ie);
  }
  int64_t bytesSinceLastPause = 0;
  for (int i = 0; i < 500; i++) {
    int64_t numBytes =
        TestUtil::nextInt(LuceneTestCase::random(), 1000, 10000);
    totBytes->addAndGet(numBytes);
    bytesSinceLastPause += numBytes;
    if (bytesSinceLastPause > limiter->getMinPauseCheckBytes()) {
      limiter->pause(bytesSinceLastPause);
      bytesSinceLastPause = 0;
    }
  }
}
} // namespace org::apache::lucene::store