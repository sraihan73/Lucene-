#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::tasks
{
class PerfTask;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::benchmark::byTask::stats
{

using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;

/**
 * Statistics for a task run.
 * <br>The same task can run more than once, but, if that task records
 * statistics, each run would create its own TaskStats.
 */
class TaskStats : public std::enable_shared_from_this<TaskStats>,
                  public Cloneable
{
  GET_CLASS_NAME(TaskStats)

  /** task for which data was collected */
private:
  std::shared_ptr<PerfTask> task;

  /** round in which task run started */
  int round = 0;

  /** task start time */
  int64_t start = 0;

  /** task elapsed time.  elapsed &gt;= 0 indicates run completion! */
  int64_t elapsed = -1;

  /** max tot mem during task */
  int64_t maxTotMem = 0;

  /** max used mem during task */
  int64_t maxUsedMem = 0;

  /** serial run number of this task run in the perf run */
  int taskRunNum = 0;

  /** number of other tasks that started to run while this task was still
   * running */
  int numParallelTasks = 0;

  /** number of work items done by this task.
   * For indexing that can be number of docs added.
   * For warming that can be number of scanned items, etc.
   * For repeating tasks, this is a sum over repetitions.
   */
  int count = 0;

  /** Number of similar tasks aggregated into this record.
   * Used when summing up on few runs/instances of similar tasks.
   */
  int numRuns = 1;

  /**
   * Create a run data for a task that is starting now.
   * To be called from Points.
   */
public:
  TaskStats(std::shared_ptr<PerfTask> task, int taskRunNum, int round);

  /**
   * mark the end of a task
   */
  virtual void markEnd(int numParallelTasks, int count);

private:
  std::deque<int> countsByTime;
  int64_t countsByTimeStepMSec = 0;

public:
  virtual void setCountsByTime(std::deque<int> &counts, int64_t msecStep);

  virtual std::deque<int> getCountsByTime();

  virtual int64_t getCountsByTimeStepMSec();

  /**
   * @return the taskRunNum.
   */
  virtual int getTaskRunNum();

  /* (non-Javadoc)
   * @see java.lang.Object#toString()
   */
  virtual std::wstring toString();

  /**
   * @return Returns the count.
   */
  virtual int getCount();

  /**
   * @return elapsed time.
   */
  virtual int64_t getElapsed();

  /**
   * @return Returns the maxTotMem.
   */
  virtual int64_t getMaxTotMem();

  /**
   * @return Returns the maxUsedMem.
   */
  virtual int64_t getMaxUsedMem();

  /**
   * @return Returns the numParallelTasks.
   */
  virtual int getNumParallelTasks();

  /**
   * @return Returns the task.
   */
  virtual std::shared_ptr<PerfTask> getTask();

  /**
   * @return Returns the numRuns.
   */
  virtual int getNumRuns();

  /**
   * Add data from another stat, for aggregation
   * @param stat2 the added stat data.
   */
  virtual void add(std::shared_ptr<TaskStats> stat2);

  /* (non-Javadoc)
   * @see java.lang.Object#clone()
   */
  std::shared_ptr<TaskStats> clone()  override;

  /**
   * @return the round number.
   */
  virtual int getRound();
};

} // namespace org::apache::lucene::benchmark::byTask::stats
