using namespace std;

#include "ResetInputsTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

ResetInputsTask::ResetInputsTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int ResetInputsTask::doLogic() 
{
  getRunData()->resetInputs();
  return 0;
}

bool ResetInputsTask::shouldNotRecordStats() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks