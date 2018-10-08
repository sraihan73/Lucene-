using namespace std;

#include "Points.h"
#include "../tasks/PerfTask.h"
#include "../utils/Config.h"
#include "TaskStats.h"

namespace org::apache::lucene::benchmark::byTask::stats
{
using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

Points::Points(shared_ptr<Config> config) {}

deque<std::shared_ptr<TaskStats>> Points::taskStats() { return points; }

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<TaskStats> Points::markTaskStart(shared_ptr<PerfTask> task,
                                            int round)
{
  shared_ptr<TaskStats> stats =
      make_shared<TaskStats>(task, nextTaskRunNum(), round);
  this->currentStats = stats;
  points.push_back(stats);
  return stats;
}

shared_ptr<TaskStats> Points::getCurrentStats() { return currentStats; }

// C++ WARNING: The following method was originally marked 'synchronized':
int Points::nextTaskRunNum() { return nextTaskRunNum_++; }

// C++ WARNING: The following method was originally marked 'synchronized':
void Points::markTaskEnd(shared_ptr<TaskStats> stats, int count)
{
  int numParallelTasks = nextTaskRunNum_ - 1 - stats->getTaskRunNum();
  // note: if the stats were cleared, might be that this stats object is
  // no longer in points, but this is just ok.
  stats->markEnd(numParallelTasks, count);
}

void Points::clearData() { points.clear(); }
} // namespace org::apache::lucene::benchmark::byTask::stats