using namespace std;

#include "SearchTravRetTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

SearchTravRetTask::SearchTravRetTask(shared_ptr<PerfRunData> runData)
    : SearchTravTask(runData)
{
}

bool SearchTravRetTask::withRetrieve() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks