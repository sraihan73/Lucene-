using namespace std;

#include "StressRamUsageEstimator.h"

namespace org::apache::lucene::util
{

shared_ptr<Entry> StressRamUsageEstimator::Entry::createNext(any o)
{
  shared_ptr<Entry> e = make_shared<Entry>();
  e->o = o;
  e->next = next;
  this->next = e;
  return e;
}

void StressRamUsageEstimator::testLargeSetOfByteArrays()
{

  System::gc();
  int64_t before = Runtime::getRuntime().totalMemory();
  std::deque<any> all(1000000);
  for (int i = 0; i < all.size(); i++) {
    all[i] = std::deque<char>(random()->nextInt(3));
  }
  System::gc();
  int64_t after = Runtime::getRuntime().totalMemory();
  wcout << L"mx:  " << RamUsageEstimator::humanReadableUnits(after - before)
        << endl;
  wcout << L"rue: " << RamUsageEstimator::humanReadableUnits(shallowSizeOf(all))
        << endl;

  guard = all;
}

int64_t StressRamUsageEstimator::shallowSizeOf(std::deque<any> &all)
{
  int64_t s = RamUsageEstimator::shallowSizeOf(all);
  for (auto o : all) {
    s += RamUsageEstimator::shallowSizeOf(o);
  }
  return s;
}

int64_t
StressRamUsageEstimator::shallowSizeOf(std::deque<std::deque<any>> &all)
{
  int64_t s = RamUsageEstimator::shallowSizeOf(all);
  for (auto o : all) {
    s += RamUsageEstimator::shallowSizeOf(o);
    for (auto o2 : o) {
      s += RamUsageEstimator::shallowSizeOf(o2);
    }
  }
  return s;
}

void StressRamUsageEstimator::testSimpleByteArrays()
{
  std::deque<std::deque<any>> all(0);
  try {
    while (true) {
      // Check the current memory consumption and provide the estimate.
      System::gc();
      int64_t estimated = shallowSizeOf(all);
      if (estimated > 50 * RamUsageEstimator::ONE_MB) {
        break;
      }

      // Make another batch of objects.
      std::deque<any> seg(10000);
      all = Arrays::copyOf(all, all.size() + 1);
      all[all.size() - 1] = seg;
      for (int i = 0; i < seg.size(); i++) {
        seg[i] = std::deque<char>(random()->nextInt(7));
      }
    }
  } catch (const OutOfMemoryError &e) {
    // Release and quit.
  }
}
} // namespace org::apache::lucene::util