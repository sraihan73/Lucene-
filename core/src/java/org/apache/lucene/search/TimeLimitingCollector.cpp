using namespace std;

#include "TimeLimitingCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Counter = org::apache::lucene::util::Counter;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

TimeLimitingCollector::TimeExceededException::TimeExceededException(
    int64_t timeAllowed, int64_t timeElapsed, int lastDocCollected)
    : RuntimeException(L"Elapsed time: " + timeElapsed +
                       L".  Exceeded allowed search time: " + timeAllowed +
                       L" ms.")
{
  this->timeAllowed = timeAllowed;
  this->timeElapsed = timeElapsed;
  this->lastDocCollected = lastDocCollected;
}

int64_t TimeLimitingCollector::TimeExceededException::getTimeAllowed()
{
  return timeAllowed;
}

int64_t TimeLimitingCollector::TimeExceededException::getTimeElapsed()
{
  return timeElapsed;
}

int TimeLimitingCollector::TimeExceededException::getLastDocCollected()
{
  return lastDocCollected;
}

TimeLimitingCollector::TimeLimitingCollector(shared_ptr<Collector> collector,
                                             shared_ptr<Counter> clock,
                                             int64_t const ticksAllowed)
    : clock(clock), ticksAllowed(ticksAllowed)
{
  this->collector = collector;
}

void TimeLimitingCollector::setBaseline(int64_t clockTime)
{
  t0 = clockTime;
  timeout = t0 + ticksAllowed;
}

void TimeLimitingCollector::setBaseline() { setBaseline(clock->get()); }

bool TimeLimitingCollector::isGreedy() { return greedy; }

void TimeLimitingCollector::setGreedy(bool greedy) { this->greedy = greedy; }

shared_ptr<LeafCollector> TimeLimitingCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  this->docBase = context->docBase;
  if (numeric_limits<int64_t>::min() == t0) {
    setBaseline();
  }
  constexpr int64_t time = clock->get();
  if (time - timeout > 0LL) {
    throw make_shared<TimeExceededException>(timeout - t0, time - t0, -1);
  }
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this(), collector->getLeafCollector(context), time);
}

TimeLimitingCollector::FilterLeafCollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass(
        shared_ptr<TimeLimitingCollector> outerInstance,
        shared_ptr<org::apache::lucene::search::LeafCollector> getLeafCollector,
        int64_t time)
    : FilterLeafCollector(getLeafCollector)
{
  this->outerInstance = outerInstance;
  this->time = time;
}

void TimeLimitingCollector::FilterLeafCollectorAnonymousInnerClass::collect(
    int doc) 
{
  constexpr int64_t time = outerInstance->clock->get();
  if (time - outerInstance->timeout > 0LL) {
    if (outerInstance->greedy) {
      // System.out.println(this+"  greedy: before failing, collecting doc:
      // "+(docBase + doc)+"  "+(time-t0));
      in_::collect(doc);
    }
    // System.out.println(this+"  failing on:  "+(docBase + doc)+" "+(time-t0));
    throw make_shared<TimeExceededException>(
        outerInstance->timeout - outerInstance->t0, time - outerInstance->t0,
        outerInstance->docBase + doc);
  }
  // System.out.println(this+"  collecting: "+(docBase + doc)+"  "+(time-t0));
  in_::collect(doc);
}

bool TimeLimitingCollector::needsScores() { return collector->needsScores(); }

void TimeLimitingCollector::setCollector(shared_ptr<Collector> collector)
{
  this->collector = collector;
}

shared_ptr<Counter> TimeLimitingCollector::getGlobalCounter()
{
  return TimerThreadHolder::THREAD->counter;
}

shared_ptr<TimerThread> TimeLimitingCollector::getGlobalTimerThread()
{
  return TimerThreadHolder::THREAD;
}

const shared_ptr<TimerThread> TimeLimitingCollector::TimerThreadHolder::THREAD;

TimeLimitingCollector::TimerThreadHolder::StaticConstructor::StaticConstructor()
{
  THREAD = make_shared<TimerThread>(Counter::newCounter(true));
  THREAD->start();
}

TimerThreadHolder::StaticConstructor
    TimeLimitingCollector::TimerThreadHolder::staticConstructor;
const wstring TimeLimitingCollector::TimerThread::THREAD_NAME =
    L"TimeLimitedCollector timer thread";

TimeLimitingCollector::TimerThread::TimerThread(int64_t resolution,
                                                shared_ptr<Counter> counter)
    : Thread(THREAD_NAME), counter(counter)
{
  this->resolution = resolution;
  this->setDaemon(true);
}

TimeLimitingCollector::TimerThread::TimerThread(shared_ptr<Counter> counter)
    : TimerThread(DEFAULT_RESOLUTION, counter)
{
}

void TimeLimitingCollector::TimerThread::run()
{
  while (!stop) {
    // TODO: Use System.nanoTime() when Lucene moves to Java SE 5.
    counter->addAndGet(resolution);
    try {
      delay(resolution);
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  }
}

int64_t TimeLimitingCollector::TimerThread::getMilliseconds() { return time; }

void TimeLimitingCollector::TimerThread::stopTimer() { stop = true; }

int64_t TimeLimitingCollector::TimerThread::getResolution()
{
  return resolution;
}

void TimeLimitingCollector::TimerThread::setResolution(int64_t resolution)
{
  this->resolution =
      max(resolution, 5); // 5 milliseconds is about the minimum reasonable time
                          // for a Object.wait(long) call.
}
} // namespace org::apache::lucene::search