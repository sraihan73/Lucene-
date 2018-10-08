using namespace std;

#include "ResetSystemEraseTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

ResetSystemEraseTask::ResetSystemEraseTask(shared_ptr<PerfRunData> runData)
    : ResetSystemSoftTask(runData)
{
}

int ResetSystemEraseTask::doLogic() 
{
  getRunData()->reinit(true);
  return 0;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks