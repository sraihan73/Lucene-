#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::stats
{
class TaskStats;
}

namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
}
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
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

/**
 * Test run data points collected as the test proceeds.
 */
class Points : public std::enable_shared_from_this<Points>
{
  GET_CLASS_NAME(Points)

  // stat points ordered by their start time.
  // for now we collect points as TaskStats objects.
  // later might optimize to collect only native data.
private:
  std::deque<std::shared_ptr<TaskStats>> points =
      std::deque<std::shared_ptr<TaskStats>>();

  // C++ NOTE: Fields cannot have the same name as methods:
  int nextTaskRunNum_ = 0;

  std::shared_ptr<TaskStats> currentStats;

  /**
   * Create a Points statistics object.
   */
public:
  Points(std::shared_ptr<Config> config);

  /**
   * Return the current task stats.
   * the actual task stats are returned, so caller should not modify this task
   * stats.
   * @return current {@link TaskStats}.
   */
  virtual std::deque<std::shared_ptr<TaskStats>> taskStats();

  /**
   * Mark that a task is starting.
   * Create a task stats for it and store it as a point.
   * @param task the starting task.
   * @return the new task stats created for the starting task.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<TaskStats>
  markTaskStart(std::shared_ptr<PerfTask> task, int round);

  virtual std::shared_ptr<TaskStats> getCurrentStats();

  // return next task num
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int nextTaskRunNum();

  /**
   * mark the end of a task
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void markTaskEnd(std::shared_ptr<TaskStats> stats, int count);

  /**
   * Clear all data, prepare for more tests.
   */
  virtual void clearData();
};

} // namespace org::apache::lucene::benchmark::byTask::stats
