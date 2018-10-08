using namespace std;

#include "PerfTaskTest.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

PerfTaskTest::MyPerfTask::MyPerfTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int PerfTaskTest::MyPerfTask::doLogic()  { return 0; }

int PerfTaskTest::MyPerfTask::getLogStep() { return logStep; }

shared_ptr<PerfRunData>
PerfTaskTest::createPerfRunData(bool setLogStep, int logStepVal,
                                bool setTaskLogStep,
                                int taskLogStepVal) 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  if (setLogStep) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    props->setProperty(L"log.step", Integer::toString(logStepVal));
  }
  if (setTaskLogStep) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    props->setProperty(L"log.step.MyPerf", Integer::toString(taskLogStepVal));
  }
  props->setProperty(L"directory", L"RAMDirectory"); // no accidental FS dir.
  shared_ptr<Config> config = make_shared<Config>(props);
  return make_shared<PerfRunData>(config);
}

void PerfTaskTest::doLogStepTest(bool setLogStep, int logStepVal,
                                 bool setTaskLogStep, int taskLogStepVal,
                                 int expLogStepValue) 
{
  shared_ptr<PerfRunData> runData =
      createPerfRunData(setLogStep, logStepVal, setTaskLogStep, taskLogStepVal);
  shared_ptr<MyPerfTask> mpt = make_shared<MyPerfTask>(runData);
  assertEquals(expLogStepValue, mpt->getLogStep());
}

void PerfTaskTest::testLogStep() 
{
  doLogStepTest(false, -1, false, -1, PerfTask::DEFAULT_LOG_STEP);
  doLogStepTest(true, -1, false, -1, numeric_limits<int>::max());
  doLogStepTest(true, 100, false, -1, 100);
  doLogStepTest(false, -1, true, -1, numeric_limits<int>::max());
  doLogStepTest(false, -1, true, 100, 100);
}
} // namespace org::apache::lucene::benchmark::byTask::tasks