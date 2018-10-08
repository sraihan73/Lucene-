using namespace std;

#include "TestDocumentsWriterStallControl.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

void TestDocumentsWriterStallControl::testSimpleStall() throw(
    InterruptedException)
{
  shared_ptr<DocumentsWriterStallControl> ctrl =
      make_shared<DocumentsWriterStallControl>();

  ctrl->updateStalled(false);
  std::deque<std::shared_ptr<Thread>> waitThreads =
      TestDocumentsWriterStallControl::waitThreads(atLeast(1), ctrl);
  start(waitThreads);
  assertFalse(ctrl->hasBlocked());
  assertFalse(ctrl->anyStalledThreads());
  join(waitThreads);

  // now stall threads and wake them up again
  ctrl->updateStalled(true);
  waitThreads = TestDocumentsWriterStallControl::waitThreads(atLeast(1), ctrl);
  start(waitThreads);
  awaitState(Thread::State::TIMED_WAITING, waitThreads);
  assertTrue(ctrl->hasBlocked());
  assertTrue(ctrl->anyStalledThreads());
  ctrl->updateStalled(false);
  assertFalse(ctrl->anyStalledThreads());
  join(waitThreads);
}

void TestDocumentsWriterStallControl::testRandom() 
{
  shared_ptr<DocumentsWriterStallControl> *const ctrl =
      make_shared<DocumentsWriterStallControl>();
  ctrl->updateStalled(false);

  std::deque<std::shared_ptr<Thread>> stallThreads(atLeast(3));
  for (int i = 0; i < stallThreads.size(); i++) {
    constexpr int stallProbability = 1 + random()->nextInt(10);
    stallThreads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), ctrl, stallProbability);
  }
  start(stallThreads);
  int64_t time = System::currentTimeMillis();
  /*
   * use a 100 sec timeout to make sure we not hang forever. join will fail in
   * that case
   */
  while ((System::currentTimeMillis() - time) < 100 * 1000 &&
         !terminated(stallThreads)) {
    ctrl->updateStalled(false);
    if (random()->nextBoolean()) {
      Thread::yield();
    } else {
      delay(1);
    }
  }
  join(stallThreads);
}

TestDocumentsWriterStallControl::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestDocumentsWriterStallControl> outerInstance,
        shared_ptr<org::apache::lucene::index::DocumentsWriterStallControl>
            ctrl,
        int stallProbability)
{
  this->outerInstance = outerInstance;
  this->ctrl = ctrl;
  this->stallProbability = stallProbability;
}

void TestDocumentsWriterStallControl::ThreadAnonymousInnerClass::run()
{

  int iters = LuceneTestCase::atLeast(1000);
  for (int j = 0; j < iters; j++) {
    ctrl->updateStalled(LuceneTestCase::random()->nextInt(stallProbability) ==
                        0);
    if (LuceneTestCase::random()->nextInt(5) == 0) { // thread 0 only updates
      ctrl->waitIfStalled();
    }
  }
}

void TestDocumentsWriterStallControl::testAccquireReleaseRace() throw(
    InterruptedException)
{
  shared_ptr<DocumentsWriterStallControl> *const ctrl =
      make_shared<DocumentsWriterStallControl>();
  ctrl->updateStalled(false);
  shared_ptr<AtomicBoolean> *const stop = make_shared<AtomicBoolean>(false);
  shared_ptr<AtomicBoolean> *const checkPoint =
      make_shared<AtomicBoolean>(true);

  int numStallers = atLeast(1);
  int numReleasers = atLeast(1);
  int numWaiters = atLeast(1);
  shared_ptr<Synchronizer> *const sync = make_shared<Synchronizer>(
      numStallers + numReleasers, numStallers + numReleasers + numWaiters);
  std::deque<std::shared_ptr<Thread>> threads(numReleasers + numStallers +
                                               numWaiters);
  deque<runtime_error> exceptions =
      Collections::synchronizedList(deque<runtime_error>());
  for (int i = 0; i < numReleasers; i++) {
    threads[i] =
        make_shared<Updater>(stop, checkPoint, ctrl, sync, true, exceptions);
  }
  for (int i = numReleasers; i < numReleasers + numStallers; i++) {
    threads[i] =
        make_shared<Updater>(stop, checkPoint, ctrl, sync, false, exceptions);
  }
  for (int i = numReleasers + numStallers;
       i < numReleasers + numStallers + numWaiters; i++) {
    threads[i] = make_shared<Waiter>(stop, checkPoint, ctrl, sync, exceptions);
  }

  start(threads);
  int iters = atLeast(10000);
  constexpr float checkPointProbability = TEST_NIGHTLY ? 0.5f : 0.1f;
  for (int i = 0; i < iters; i++) {
    if (checkPoint->get()) {

      assertTrue(L"timed out waiting for update threads - deadlock?",
                 sync->updateJoin->await(10, TimeUnit::SECONDS));
      if (!exceptions.empty()) {
        for (auto throwable : exceptions) {
          throwable.printStackTrace();
        }
        fail(L"got exceptions in threads");
      }

      if (ctrl->hasBlocked() && ctrl->isHealthy()) {
        assertState(numReleasers, numStallers, numWaiters, threads, ctrl);
      }

      checkPoint->set(false);
      sync->waiter->countDown();
      sync->leftCheckpoint->await();
    }
    assertFalse(checkPoint->get());
    assertEquals(0, sync->waiter->getCount());
    if (checkPointProbability >= random()->nextFloat()) {
      sync->reset(numStallers + numReleasers,
                  numStallers + numReleasers + numWaiters);
      checkPoint->set(true);
    }
  }
  if (!checkPoint->get()) {
    sync->reset(numStallers + numReleasers,
                numStallers + numReleasers + numWaiters);
    checkPoint->set(true);
  }

  assertTrue(sync->updateJoin->await(10, TimeUnit::SECONDS));
  assertState(numReleasers, numStallers, numWaiters, threads, ctrl);
  checkPoint->set(false);
  stop->set(true);
  sync->waiter->countDown();
  sync->leftCheckpoint->await();

  for (int i = 0; i < threads.size(); i++) {
    ctrl->updateStalled(false);
    threads[i]->join(2000);
    if (threads[i]->isAlive() &&
        std::dynamic_pointer_cast<Waiter>(threads[i]) != nullptr) {
      if (threads[i]->getState() == Thread::State::WAITING) {
        fail(L"waiter is not released - anyThreadsStalled: " +
             StringHelper::toString(ctrl->anyStalledThreads()));
      }
    }
  }
}

void TestDocumentsWriterStallControl::assertState(
    int numReleasers, int numStallers, int numWaiters,
    std::deque<std::shared_ptr<Thread>> &threads,
    shared_ptr<DocumentsWriterStallControl> ctrl) 
{
  int millisToSleep = 100;
  while (true) {
    if (ctrl->hasBlocked() && ctrl->isHealthy()) {
      for (int n = numReleasers + numStallers;
           n < numReleasers + numStallers + numWaiters; n++) {
        if (ctrl->isThreadQueued(threads[n])) {
          if (millisToSleep < 60000) {
            delay(millisToSleep);
            millisToSleep *= 2;
            break;
          } else {
            fail(L"control claims no stalled threads but waiter seems to be "
                 L"blocked ");
          }
        }
      }
      break;
    } else {
      break;
    }
  }
}

TestDocumentsWriterStallControl::Waiter::Waiter(
    shared_ptr<AtomicBoolean> stop, shared_ptr<AtomicBoolean> checkPoint,
    shared_ptr<DocumentsWriterStallControl> ctrl, shared_ptr<Synchronizer> sync,
    deque<runtime_error> &exceptions)
    : Thread(L"waiter")
{
  this->stop = stop;
  this->checkPoint = checkPoint;
  this->ctrl = ctrl;
  this->sync = sync;
  this->exceptions = exceptions;
}

void TestDocumentsWriterStallControl::Waiter::run()
{
  try {
    while (!stop->get()) {
      ctrl->waitIfStalled();
      if (checkPoint->get()) {
        try {
          assertTrue(sync->await());
        } catch (const InterruptedException &e) {
          wcout << L"[Waiter] got interrupted - wait count: "
                << sync->waiter->getCount() << endl;
          throw make_shared<ThreadInterruptedException>(e);
        }
      }
    }
  } catch (const runtime_error &e) {
    e.printStackTrace();
    exceptions.push_back(e);
  }
}

TestDocumentsWriterStallControl::Updater::Updater(
    shared_ptr<AtomicBoolean> stop, shared_ptr<AtomicBoolean> checkPoint,
    shared_ptr<DocumentsWriterStallControl> ctrl, shared_ptr<Synchronizer> sync,
    bool release, deque<runtime_error> &exceptions)
    : Thread(L"updater")
{
  this->stop = stop;
  this->checkPoint = checkPoint;
  this->ctrl = ctrl;
  this->sync = sync;
  this->release = release;
  this->exceptions = exceptions;
}

void TestDocumentsWriterStallControl::Updater::run()
{
  try {

    while (!stop->get()) {
      int internalIters = release && LuceneTestCase::random()->nextBoolean()
                              ? LuceneTestCase::atLeast(5)
                              : 1;
      for (int i = 0; i < internalIters; i++) {
        ctrl->updateStalled(LuceneTestCase::random()->nextBoolean());
      }
      if (checkPoint->get()) {
        sync->updateJoin->countDown();
        try {
          assertTrue(sync->await());
        } catch (const InterruptedException &e) {
          wcout << L"[Updater] got interrupted - wait count: "
                << sync->waiter->getCount() << endl;
          throw make_shared<ThreadInterruptedException>(e);
        }
        sync->leftCheckpoint->countDown();
      }
      if (LuceneTestCase::random()->nextBoolean()) {
        Thread::yield();
      }
    }
  } catch (const runtime_error &e) {
    e.printStackTrace();
    exceptions.push_back(e);
  }
  sync->updateJoin->countDown();
}

bool TestDocumentsWriterStallControl::terminated(
    std::deque<std::shared_ptr<Thread>> &threads)
{
  for (auto thread : threads) {
    if (Thread::State::TERMINATED != thread->getState()) {
      return false;
    }
  }
  return true;
}

void TestDocumentsWriterStallControl::start(
    std::deque<std::shared_ptr<Thread>> &tostart) 
{
  for (auto thread : tostart) {
    thread->start();
  }
  delay(1); // let them start
}

void TestDocumentsWriterStallControl::join(
    std::deque<std::shared_ptr<Thread>> &toJoin) 
{
  for (auto thread : toJoin) {
    thread->join();
  }
}

std::deque<std::shared_ptr<Thread>>
TestDocumentsWriterStallControl::waitThreads(
    int num, shared_ptr<DocumentsWriterStallControl> ctrl)
{
  std::deque<std::shared_ptr<Thread>> array_(num);
  for (int i = 0; i < array_.size(); i++) {
    array_[i] = make_shared<ThreadAnonymousInnerClass>(ctrl);
  }
  return array_;
}

TestDocumentsWriterStallControl::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::DocumentsWriterStallControl>
            ctrl)
{
  this->ctrl = ctrl;
}

void TestDocumentsWriterStallControl::ThreadAnonymousInnerClass::run()
{
  ctrl->waitIfStalled();
}

void TestDocumentsWriterStallControl::awaitState(
    shared_ptr<Thread::State> state,
    deque<Thread> &threads) 
{
  while (true) {
    bool done = true;
    for (shared_ptr<Thread> thread : threads) {
      if (thread->getState() != state) {
        done = false;
        break;
      }
    }
    if (done) {
      return;
    }
    if (random()->nextBoolean()) {
      Thread::yield();
    } else {
      delay(1);
    }
  }
}

TestDocumentsWriterStallControl::Synchronizer::Synchronizer(int numUpdater,
                                                            int numThreads)
{
  reset(numUpdater, numThreads);
}

void TestDocumentsWriterStallControl::Synchronizer::reset(int numUpdaters,
                                                          int numThreads)
{
  this->waiter = make_shared<CountDownLatch>(1);
  this->updateJoin = make_shared<CountDownLatch>(numUpdaters);
  this->leftCheckpoint = make_shared<CountDownLatch>(numUpdaters);
}

bool TestDocumentsWriterStallControl::Synchronizer::await() throw(
    InterruptedException)
{
  return waiter->await(10, TimeUnit::SECONDS);
}
} // namespace org::apache::lucene::index