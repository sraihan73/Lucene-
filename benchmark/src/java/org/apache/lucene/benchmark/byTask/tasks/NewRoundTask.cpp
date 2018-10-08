using namespace std;

#include "NewRoundTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

NewRoundTask::NewRoundTask(shared_ptr<PerfRunData> runData) : PerfTask(runData)
{
}

int NewRoundTask::doLogic() 
{
  getRunData()->getConfig()->newRound();
  return 0;
}

bool NewRoundTask::shouldNotRecordStats() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks