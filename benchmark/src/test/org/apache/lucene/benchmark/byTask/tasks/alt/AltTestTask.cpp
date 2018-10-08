using namespace std;

#include "AltTestTask.h"
#include "../../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks::alt
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;

AltTestTask::AltTestTask(shared_ptr<PerfRunData> runData)
    : org::apache::lucene::benchmark::byTask::tasks::PerfTask(runData)
{
}

int AltTestTask::doLogic()  { return 0; }
} // namespace org::apache::lucene::benchmark::byTask::tasks::alt