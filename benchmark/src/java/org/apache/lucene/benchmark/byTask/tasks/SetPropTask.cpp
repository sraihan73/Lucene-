using namespace std;

#include "SetPropTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

SetPropTask::SetPropTask(shared_ptr<PerfRunData> runData) : PerfTask(runData) {}

int SetPropTask::doLogic() 
{
  if (name == L"" || value == L"") {
    throw runtime_error(getName() + L" - undefined name or value: name=" +
                        name + L" value=" + value);
  }
  getRunData()->getConfig()->set(name, value);
  return 0;
}

void SetPropTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  int k = (int)params.find(L",");
  name = params.substr(0, k)->trim();
  value = params.substr(k + 1)->trim();
}

bool SetPropTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks