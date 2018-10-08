using namespace std;

#include "CountingSearchTestTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
int CountingSearchTestTask::numSearches = 0;
int64_t CountingSearchTestTask::startMillis = 0;
int64_t CountingSearchTestTask::lastMillis = 0;
int64_t CountingSearchTestTask::prevLastMillis = 0;

CountingSearchTestTask::CountingSearchTestTask(shared_ptr<PerfRunData> runData)
    : SearchTask(runData)
{
}

int CountingSearchTestTask::doLogic() 
{
  int res = SearchTask::doLogic();
  incrNumSearches();
  return res;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void CountingSearchTestTask::incrNumSearches()
{
  prevLastMillis = lastMillis;
  lastMillis = System::currentTimeMillis();
  if (0 == numSearches) {
    startMillis = prevLastMillis = lastMillis;
  }
  numSearches++;
}

int64_t CountingSearchTestTask::getElapsedMillis()
{
  return lastMillis - startMillis;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks