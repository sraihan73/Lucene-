#pragma once
#include "ReportTask.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}

namespace org::apache::lucene::benchmark::byTask::stats
{
class Report;
}
namespace org::apache::lucene::benchmark::byTask::stats
{
class TaskStats;
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
namespace org::apache::lucene::benchmark::byTask::tasks
{

using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;

/**
 * Report all statistics with no aggregations.
 * <br>Other side effects: None.
 */
class RepAllTask : public ReportTask
{
  GET_CLASS_NAME(RepAllTask)

public:
  RepAllTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

  /**
   * Report detailed statistics as a string
   * @return the report
   */
protected:
  virtual std::shared_ptr<Report>
  reportAll(std::deque<std::shared_ptr<TaskStats>> &taskStats);

protected:
  std::shared_ptr<RepAllTask> shared_from_this()
  {
    return std::static_pointer_cast<RepAllTask>(ReportTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
