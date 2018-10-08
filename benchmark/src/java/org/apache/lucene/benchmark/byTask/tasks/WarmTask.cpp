using namespace std;

#include "WarmTask.h"
#include "../PerfRunData.h"
#include "../feeds/QueryMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;

WarmTask::WarmTask(shared_ptr<PerfRunData> runData) : ReadTask(runData) {}

bool WarmTask::withRetrieve() { return false; }

bool WarmTask::withSearch() { return false; }

bool WarmTask::withTraverse() { return false; }

bool WarmTask::withWarm() { return true; }

shared_ptr<QueryMaker> WarmTask::getQueryMaker()
{
  return nullptr; // not required for this task.
}
} // namespace org::apache::lucene::benchmark::byTask::tasks