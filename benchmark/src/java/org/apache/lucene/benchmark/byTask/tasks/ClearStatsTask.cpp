using namespace std;

#include "ClearStatsTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

ClearStatsTask::ClearStatsTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int ClearStatsTask::doLogic() 
{
  getRunData()->getPoints()->clearData();
  return 0;
}

bool ClearStatsTask::shouldNotRecordStats() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks