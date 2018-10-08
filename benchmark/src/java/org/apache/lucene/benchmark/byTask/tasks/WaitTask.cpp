using namespace std;

#include "WaitTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

WaitTask::WaitTask(shared_ptr<PerfRunData> runData) : PerfTask(runData) {}

void WaitTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  if (params != L"") {
    int multiplier;
    if (StringHelper::endsWith(params, L"s")) {
      multiplier = 1;
      params = params.substr(0, params.length() - 1);
    } else if (StringHelper::endsWith(params, L"m")) {
      multiplier = 60;
      params = params.substr(0, params.length() - 1);
    } else if (StringHelper::endsWith(params, L"h")) {
      multiplier = 3600;
      params = params.substr(0, params.length() - 1);
    } else {
      // Assume seconds
      multiplier = 1;
    }

    waitTimeSec = stod(params) * multiplier;
  } else {
    throw invalid_argument(
        L"you must specify the wait time, eg: 10.0s, 4.5m, 2h");
  }
}

int WaitTask::doLogic() 
{
  delay(static_cast<int64_t>(1000 * waitTimeSec));
  return 0;
}

bool WaitTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks