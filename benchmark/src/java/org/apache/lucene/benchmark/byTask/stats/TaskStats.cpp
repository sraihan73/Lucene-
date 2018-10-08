using namespace std;

#include "TaskStats.h"
#include "../tasks/PerfTask.h"

namespace org::apache::lucene::benchmark::byTask::stats
{
using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;

TaskStats::TaskStats(shared_ptr<PerfTask> task, int taskRunNum, int round)
{
  this->task = task;
  this->taskRunNum = taskRunNum;
  this->round = round;
  maxTotMem = Runtime::getRuntime().totalMemory();
  maxUsedMem = maxTotMem - Runtime::getRuntime().freeMemory();
  start = System::currentTimeMillis();
}

void TaskStats::markEnd(int numParallelTasks, int count)
{
  elapsed = System::currentTimeMillis() - start;
  int64_t totMem = Runtime::getRuntime().totalMemory();
  if (totMem > maxTotMem) {
    maxTotMem = totMem;
  }
  int64_t usedMem = totMem - Runtime::getRuntime().freeMemory();
  if (usedMem > maxUsedMem) {
    maxUsedMem = usedMem;
  }
  this->numParallelTasks = numParallelTasks;
  this->count = count;
}

void TaskStats::setCountsByTime(std::deque<int> &counts, int64_t msecStep)
{
  countsByTime = counts;
  countsByTimeStepMSec = msecStep;
}

std::deque<int> TaskStats::getCountsByTime() { return countsByTime; }

int64_t TaskStats::getCountsByTimeStepMSec() { return countsByTimeStepMSec; }

int TaskStats::getTaskRunNum() { return taskRunNum; }

wstring TaskStats::toString()
{
  shared_ptr<StringBuilder> res = make_shared<StringBuilder>(task->getName());
  res->append(L" ");
  res->append(count);
  res->append(L" ");
  res->append(elapsed);
  return res->toString();
}

int TaskStats::getCount() { return count; }

int64_t TaskStats::getElapsed() { return elapsed; }

int64_t TaskStats::getMaxTotMem() { return maxTotMem; }

int64_t TaskStats::getMaxUsedMem() { return maxUsedMem; }

int TaskStats::getNumParallelTasks() { return numParallelTasks; }

shared_ptr<PerfTask> TaskStats::getTask() { return task; }

int TaskStats::getNumRuns() { return numRuns; }

void TaskStats::add(shared_ptr<TaskStats> stat2)
{
  numRuns += stat2->getNumRuns();
  elapsed += stat2->getElapsed();
  maxTotMem += stat2->getMaxTotMem();
  maxUsedMem += stat2->getMaxUsedMem();
  count += stat2->getCount();
  if (round != stat2->round) {
    round = -1; // no meaning if aggregating tasks of different round.
  }

  if (countsByTime.size() > 0 && stat2->countsByTime.size() > 0) {
    if (countsByTimeStepMSec != stat2->countsByTimeStepMSec) {
      throw make_shared<IllegalStateException>(L"different by-time msec step");
    }
    if (countsByTime.size() != stat2->countsByTime.size()) {
      throw make_shared<IllegalStateException>(L"different by-time msec count");
    }
    for (int i = 0; i < stat2->countsByTime.size(); i++) {
      countsByTime[i] += stat2->countsByTime[i];
    }
  }
}

shared_ptr<TaskStats> TaskStats::clone() 
{
  shared_ptr<TaskStats> c =
      std::static_pointer_cast<TaskStats>(__super::clone());
  if (c->countsByTime.size() > 0) {
    c->countsByTime = c->countsByTime.clone();
  }
  return c;
}

int TaskStats::getRound() { return round; }
} // namespace org::apache::lucene::benchmark::byTask::stats