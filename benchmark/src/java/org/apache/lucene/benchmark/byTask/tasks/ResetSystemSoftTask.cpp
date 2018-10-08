using namespace std;

#include "ResetSystemSoftTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

ResetSystemSoftTask::ResetSystemSoftTask(shared_ptr<PerfRunData> runData)
    : ResetInputsTask(runData)
{
}

int ResetSystemSoftTask::doLogic() 
{
  getRunData()->reinit(false);
  return 0;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks