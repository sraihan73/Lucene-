using namespace std;

#include "SearchTask.h"
#include "../PerfRunData.h"
#include "../feeds/QueryMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;

SearchTask::SearchTask(shared_ptr<PerfRunData> runData) : ReadTask(runData) {}

bool SearchTask::withRetrieve() { return false; }

bool SearchTask::withSearch() { return true; }

bool SearchTask::withTraverse() { return false; }

bool SearchTask::withWarm() { return false; }

shared_ptr<QueryMaker> SearchTask::getQueryMaker()
{
  return getRunData()->getQueryMaker(shared_from_this());
}
} // namespace org::apache::lucene::benchmark::byTask::tasks