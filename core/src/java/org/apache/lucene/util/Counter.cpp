using namespace std;

#include "Counter.h"

namespace org::apache::lucene::util
{

shared_ptr<Counter> Counter::newCounter() { return newCounter(false); }

shared_ptr<Counter> Counter::newCounter(bool threadSafe)
{
  return threadSafe ? make_shared<AtomicCounter>()
                    : make_shared<SerialCounter>();
}

int64_t Counter::SerialCounter::addAndGet(int64_t delta)
{
  return count += delta;
}

int64_t Counter::SerialCounter::get() { return count; };

int64_t Counter::AtomicCounter::addAndGet(int64_t delta)
{
  return count->addAndGet(delta);
}

int64_t Counter::AtomicCounter::get() { return count->get(); }
} // namespace org::apache::lucene::util